/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/leds-aw2013.h>

/* register address */
#define AW_REG_RESET			0x00
#define AW_REG_GLOBAL_CONTROL		0x01
#define AW_REG_LED_STATUS		0x02
#define AW_REG_LED_ENABLE		0x30
#define AW_REG_LED_CONFIG_BASE		0x31
#define AW_REG_LED_BRIGHTNESS_BASE	0x34
#define AW_REG_TIMESET0_BASE		0x37
#define AW_REG_TIMESET1_BASE		0x38

/* register bits */
#define AW2013_CHIPID			0x33
#define AW_LED_MOUDLE_ENABLE_MASK	0x01
#define AW_LED_FADE_OFF_MASK		0x40
#define AW_LED_FADE_ON_MASK		0x20
#define AW_LED_BREATHE_MODE_MASK	0x10
#define AW_LED_RESET_MASK		0x55

#define AW_LED_RESET_DELAY		8
#define AW2013_VDD_MIN_UV		2600000
#define AW2013_VDD_MAX_UV		3300000
#define AW2013_VI2C_MIN_UV		1800000
#define AW2013_VI2C_MAX_UV		1800000

#define MAX_RISE_TIME_MS		7
/* < LAFITE-370 yanzhiyao 20160104 begin */
#define MAX_HOLD_TIME_MS		7
/* LAFITE-370 yanzhiyao 20160104 end > */
#define MAX_FALL_TIME_MS		7
/* < LAFITE-370 yanzhiyao 20160104 begin */
#define MAX_OFF_TIME_MS			7
/* LAFITE-370 yanzhiyao 20160104 end > */

struct aw2013_led {
	struct i2c_client *client;
	struct led_classdev cdev;
	struct aw2013_platform_data *pdata;
	struct work_struct brightness_work;
	struct mutex lock;
	struct regulator *vdd;
	struct regulator *vcc;
	int num_leds;
	int id;
	bool poweron;
};
/* < LAFITE-1500 yanzhiyao 20160215 begin */
struct aw2013_led *g_led;
/* < LAFITE-2439 lichuangchuang 20160225 begin */
int aw2013_sleep_led = 0;
/* LAFITE-2439 lichuangchuang 20160225 end > */
EXPORT_SYMBOL(aw2013_sleep_led);
int aw2013_usb_state = 0;
EXPORT_SYMBOL(aw2013_usb_state);
/* LAFITE-1500 yanzhiyao 20160215 end > */
/* < LAFITE-3222 yanzhiyao 20160301 begin */
/* < LAFITE-3222 yanzhiyao 20160229 begin */
//static int g_blinking = 0;
/* LAFITE-3222 yanzhiyao 20160229 end > */
/* LAFITE-3222 yanzhiyao 20160301 end > */
/* < LAFITE-6306 yanzhiyao 20160328 begin */
bool userspace_led_ctl= 0;
bool aw2013_flag = 0;
/* LAFITE-6306 yanzhiyao 20160328 end > */
/*yulong add for showing green led in ftm mode 20160421*/
extern int yl_get_ftm(void);
/*yulong add end*/

static int aw2013_write(struct aw2013_led *led, u8 reg, u8 val)
{
/* < LAFITE-370 yanzhiyao 20160104 begin */
	int i = 0;
	i = i2c_smbus_write_byte_data(led->client, reg, val);
	if(i<0)
	{
		printk("aw2013 LED write i2c error\n");
	}
	//return i2c_smbus_write_byte_data(led->client, reg, val);
	return i;
/* LAFITE-370 yanzhiyao 20160104 end > */
}

static int aw2013_read(struct aw2013_led *led, u8 reg, u8 *val)
{
	s32 ret;

	ret = i2c_smbus_read_byte_data(led->client, reg);
	if (ret < 0)
	/* < LAFITE-370 yanzhiyao 20160104 begin */
	{
		printk("aw2013 LED read I2C err\n");
		return ret;
	}
	/* LAFITE-370 yanzhiyao 20160104 end > */

	*val = ret;
	return 0;
}

static int aw2013_power_on(struct aw2013_led *led, bool on)
{
	int rc;

	if (on) {
		rc = regulator_enable(led->vdd);
		if (rc) {
			dev_err(&led->client->dev,
				"Regulator vdd enable failed rc=%d\n", rc);
			return rc;
		}

/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
		rc = regulator_enable(led->vcc);
		if (rc) {
			dev_err(&led->client->dev,
				"Regulator vcc enable failed rc=%d\n", rc);
			goto fail_enable_reg;
		}
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
		led->poweron = true;
	} else {
		rc = regulator_disable(led->vdd);
		if (rc) {
			dev_err(&led->client->dev,
				"Regulator vdd disable failed rc=%d\n", rc);
			return rc;
		}

/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
		rc = regulator_disable(led->vcc);
		if (rc) {
			dev_err(&led->client->dev,
				"Regulator vcc disable failed rc=%d\n", rc);
			goto fail_disable_reg;
		}
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
		led->poweron = false;
	}
/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
	return rc;

fail_enable_reg:
	rc = regulator_disable(led->vdd);
	if (rc)
		dev_err(&led->client->dev,
			"Regulator vdd disable failed rc=%d\n", rc);

	return rc;

fail_disable_reg:
	rc = regulator_enable(led->vdd);
	if (rc)
		dev_err(&led->client->dev,
			"Regulator vdd enable failed rc=%d\n", rc);

#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
	return rc;
}

static int aw2013_power_init(struct aw2013_led *led, bool on)
{
	int rc;

	if (on) {
		led->vdd = regulator_get(&led->client->dev, "vdd");
		if (IS_ERR(led->vdd)) {
			rc = PTR_ERR(led->vdd);
			dev_err(&led->client->dev,
				"Regulator get failed vdd rc=%d\n", rc);
			return rc;
		}

		if (regulator_count_voltages(led->vdd) > 0) {
			rc = regulator_set_voltage(led->vdd, AW2013_VDD_MIN_UV,
						   AW2013_VDD_MAX_UV);
			if (rc) {
				dev_err(&led->client->dev,
					"Regulator set_vtg failed vdd rc=%d\n",
					rc);
				goto reg_vdd_put;
			}
		}

/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
		led->vcc = regulator_get(&led->client->dev, "vcc");
		if (IS_ERR(led->vcc)) {
			rc = PTR_ERR(led->vcc);
			dev_err(&led->client->dev,
				"Regulator get failed vcc rc=%d\n", rc);
			goto reg_vdd_set_vtg;
		}

		if (regulator_count_voltages(led->vcc) > 0) {
			rc = regulator_set_voltage(led->vcc, AW2013_VI2C_MIN_UV,
						   AW2013_VI2C_MAX_UV);
			if (rc) {
				dev_err(&led->client->dev,
				"Regulator set_vtg failed vcc rc=%d\n", rc);
				goto reg_vcc_put;
			}
		}
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
	} else {
		if (regulator_count_voltages(led->vdd) > 0)
			regulator_set_voltage(led->vdd, 0, AW2013_VDD_MAX_UV);

		regulator_put(led->vdd);

/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
		if (regulator_count_voltages(led->vcc) > 0)
			regulator_set_voltage(led->vcc, 0, AW2013_VI2C_MAX_UV);

		regulator_put(led->vcc);
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
	}
	return 0;

/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
reg_vcc_put:
	regulator_put(led->vcc);
reg_vdd_set_vtg:
	if (regulator_count_voltages(led->vdd) > 0)
		regulator_set_voltage(led->vdd, 0, AW2013_VDD_MAX_UV);
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
reg_vdd_put:
	regulator_put(led->vdd);
	return rc;
}

static void aw2013_brightness_work(struct work_struct *work)
{
	struct aw2013_led *led = container_of(work, struct aw2013_led,
					brightness_work);
	u8 val;

	mutex_lock(&led->pdata->led->lock);

	/* enable regulators if they are disabled */
	if (!led->pdata->led->poweron) {
		if (aw2013_power_on(led->pdata->led, true)) {
			dev_err(&led->pdata->led->client->dev, "power on failed");
			mutex_unlock(&led->pdata->led->lock);
			return;
		}
	}

	if (led->cdev.brightness > 0) {
		if (led->cdev.brightness > led->cdev.max_brightness)
			led->cdev.brightness = led->cdev.max_brightness;
		/* < FERRARI-705 > yulong modify for RGB ratio 20160524 begin*/
		if (led->cdev.brightness == 153)
			led->cdev.brightness = 180;
		if (led->cdev.brightness == 204)
			led->cdev.brightness = 255;
		/*< FERRARI-705 > yulong modify for RGB ratio 20160524 end*/
		aw2013_write(led, AW_REG_GLOBAL_CONTROL,
			AW_LED_MOUDLE_ENABLE_MASK);
		aw2013_write(led, AW_REG_LED_CONFIG_BASE + led->id,
			led->pdata->max_current);
		aw2013_write(led, AW_REG_LED_BRIGHTNESS_BASE + led->id,
			led->cdev.brightness);
		aw2013_read(led, AW_REG_LED_ENABLE, &val);
		aw2013_write(led, AW_REG_LED_ENABLE, val | (1 << led->id));
		/* < LAFITE-370 yanzhiyao 20160104 begin */
		printk("aw2013 %s LED brightness=%d,current=%d\n",led->cdev.name,led->cdev.brightness,led->pdata->max_current);
		/* LAFITE-370 yanzhiyao 20160104 end > */
	} else {
		aw2013_read(led, AW_REG_LED_ENABLE, &val);
		aw2013_write(led, AW_REG_LED_ENABLE, val & (~(1 << led->id)));
		/* < LAFITE-370 yanzhiyao 20160104 begin */
		printk("aw2013 %s LED close\n",led->cdev.name);
		/* LAFITE-370 yanzhiyao 20160104 end > */
	}

	aw2013_read(led, AW_REG_LED_ENABLE, &val);
	/*
	 * If value in AW_REG_LED_ENABLE is 0, it means the RGB leds are
	 * all off. So we need to power it off.
	 */
	if (val == 0) {
		if (aw2013_power_on(led->pdata->led, false)) {
			dev_err(&led->pdata->led->client->dev,
				"power off failed");
			mutex_unlock(&led->pdata->led->lock);
			return;
		}
	}

	mutex_unlock(&led->pdata->led->lock);
}

static void aw2013_led_blink_set(struct aw2013_led *led, unsigned long blinking)
{
	u8 val;

	/* enable regulators if they are disabled */
	if (!led->pdata->led->poweron) {
		if (aw2013_power_on(led->pdata->led, true)) {
			dev_err(&led->pdata->led->client->dev, "power on failed");
			return;
		}
	}

	led->cdev.brightness = blinking ? led->cdev.max_brightness : 0;

	if (blinking > 0) {
		aw2013_write(led, AW_REG_GLOBAL_CONTROL,
			AW_LED_MOUDLE_ENABLE_MASK);
		aw2013_write(led, AW_REG_LED_CONFIG_BASE + led->id,
			AW_LED_FADE_OFF_MASK | AW_LED_FADE_ON_MASK |
			AW_LED_BREATHE_MODE_MASK | led->pdata->max_current);
		aw2013_write(led, AW_REG_LED_BRIGHTNESS_BASE + led->id,
			led->cdev.brightness);
		aw2013_write(led, AW_REG_TIMESET0_BASE + led->id * 3,
			led->pdata->rise_time_ms << 4 |
			led->pdata->hold_time_ms);
		aw2013_write(led, AW_REG_TIMESET1_BASE + led->id * 3,
			led->pdata->fall_time_ms << 4 |
			led->pdata->off_time_ms);
		aw2013_read(led, AW_REG_LED_ENABLE, &val);
		aw2013_write(led, AW_REG_LED_ENABLE, val | (1 << led->id));
		/* < LAFITE-370 yanzhiyao 20160104 begin */
		printk("aw2013 %s LED set off_time=%d blink\n",led->cdev.name,led->pdata->off_time_ms);
		/* LAFITE-370 yanzhiyao 20160104 end > */
	} else {
		aw2013_read(led, AW_REG_LED_ENABLE, &val);
		aw2013_write(led, AW_REG_LED_ENABLE, val & (~(1 << led->id)));
		/* < LAFITE-370 yanzhiyao 20160104 begin */
		printk("aw2013 %s LED close\n",led->cdev.name);
		/* LAFITE-370 yanzhiyao 20160104 end > */
	}

	aw2013_read(led, AW_REG_LED_ENABLE, &val);
	/*
	 * If value in AW_REG_LED_ENABLE is 0, it means the RGB leds are
	 * all off. So we need to power it off.
	 */
	if (val == 0) {
		if (aw2013_power_on(led->pdata->led, false)) {
			dev_err(&led->pdata->led->client->dev,
				"power off failed");
			return;
		}
	}
}

static void aw2013_set_brightness(struct led_classdev *cdev,
			     enum led_brightness brightness)
{
	struct aw2013_led *led = container_of(cdev, struct aw2013_led, cdev);
	led->cdev.brightness = brightness;
	/* < LAFITE-6306 yanzhiyao 20160328 begin */
	userspace_led_ctl = 1;
	/* LAFITE-6306 yanzhiyao 20160328 end > */
	schedule_work(&led->brightness_work);
}

static ssize_t aw2013_store_blink(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t len)
{
	unsigned long blinking;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw2013_led *led =
			container_of(led_cdev, struct aw2013_led, cdev);
	ssize_t ret = -EINVAL;

	ret = kstrtoul(buf, 10, &blinking);
	if (ret)
		return ret;
	/* < LAFITE-3222 yanzhiyao 20160301 begin */
	/* < LAFITE-3222 yanzhiyao 20160229 begin */
	//g_blinking = blinking;
	/* LAFITE-3222 yanzhiyao 20160229 end > */
	/* LAFITE-3222 yanzhiyao 20160301 end > */
	mutex_lock(&led->pdata->led->lock);
	aw2013_led_blink_set(led, blinking);
	mutex_unlock(&led->pdata->led->lock);

	return len;
}

static ssize_t aw2013_led_time_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw2013_led *led =
			container_of(led_cdev, struct aw2013_led, cdev);

	return snprintf(buf, PAGE_SIZE, "%d %d %d %d\n",
			led->pdata->rise_time_ms, led->pdata->hold_time_ms,
			led->pdata->fall_time_ms, led->pdata->off_time_ms);
}

static ssize_t aw2013_led_time_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t len)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct aw2013_led *led =
			container_of(led_cdev, struct aw2013_led, cdev);
	int rc, rise_time_ms, hold_time_ms, fall_time_ms, off_time_ms;

	rc = sscanf(buf, "%d %d %d %d",
			&rise_time_ms, &hold_time_ms,
			&fall_time_ms, &off_time_ms);

	mutex_lock(&led->pdata->led->lock);
	led->pdata->rise_time_ms = (rise_time_ms > MAX_RISE_TIME_MS) ?
				MAX_RISE_TIME_MS : rise_time_ms;
	led->pdata->hold_time_ms = (hold_time_ms > MAX_HOLD_TIME_MS) ?
				MAX_HOLD_TIME_MS : hold_time_ms;
	led->pdata->fall_time_ms = (fall_time_ms > MAX_FALL_TIME_MS) ?
				MAX_FALL_TIME_MS : fall_time_ms;
	led->pdata->off_time_ms = (off_time_ms > MAX_OFF_TIME_MS) ?
				MAX_OFF_TIME_MS : off_time_ms;
	aw2013_led_blink_set(led, 1);
	mutex_unlock(&led->pdata->led->lock);
	return len;
}
/* < LAFITE-1500 yanzhiyao 20160215 begin */
static DEVICE_ATTR(blink, 0666, NULL, aw2013_store_blink);
static DEVICE_ATTR(led_time, 0666, aw2013_led_time_show, aw2013_led_time_store);
/* LAFITE-1500 yanzhiyao 20160215 end > */

static struct attribute *aw2013_led_attributes[] = {
	&dev_attr_blink.attr,
	&dev_attr_led_time.attr,
	NULL,
};

static struct attribute_group aw2013_led_attr_group = {
	.attrs = aw2013_led_attributes
};

static int aw_2013_check_chipid(struct aw2013_led *led)
{
	u8 val;
	/* < LAFITE-6306 yanzhiyao 20160328 begin */
	aw2013_read(led, AW_REG_RESET, &val);
	if (val == AW2013_CHIPID)
		return 0;
	else
	{
		aw2013_write(led, AW_REG_RESET, AW_LED_RESET_MASK);
		usleep(AW_LED_RESET_DELAY);
		aw2013_read(led, AW_REG_RESET, &val);
		if (val == AW2013_CHIPID)
			return 0;
		else
			return -EINVAL;
	}
	/* LAFITE-6306 yanzhiyao 20160328 end > */
/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
	if (val == AW2013_CHIPID)
		return 0;
	else
		return -EINVAL;
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
}

static int aw2013_led_err_handle(struct aw2013_led *led_array,
				int parsed_leds)
{
	int i;
	/*
	 * If probe fails, cannot free resource of all LEDs, only free
	 * resources of LEDs which have allocated these resource really.
	 */
	for (i = 0; i < parsed_leds; i++) {
		sysfs_remove_group(&led_array[i].cdev.dev->kobj,
				&aw2013_led_attr_group);
		led_classdev_unregister(&led_array[i].cdev);
		cancel_work_sync(&led_array[i].brightness_work);
		devm_kfree(&led_array->client->dev, led_array[i].pdata);
		led_array[i].pdata = NULL;
	}
	return i;
}

static int aw2013_led_parse_child_node(struct aw2013_led *led_array,
				struct device_node *node)
{
	struct aw2013_led *led;
	struct device_node *temp;
	struct aw2013_platform_data *pdata;
	int rc = 0, parsed_leds = 0;

	for_each_child_of_node(node, temp) {
		led = &led_array[parsed_leds];
		led->client = led_array->client;

		pdata = devm_kzalloc(&led->client->dev,
				sizeof(struct aw2013_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			dev_err(&led->client->dev,
				"Failed to allocate memory\n");
			goto free_err;
		}
		pdata->led = led_array;
		led->pdata = pdata;

		rc = of_property_read_string(temp, "aw2013,name",
			&led->cdev.name);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading led name, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,id",
			&led->id);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading id, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,max-brightness",
			&led->cdev.max_brightness);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading max-brightness, rc = %d\n",
				rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,max-current",
			&led->pdata->max_current);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading max-current, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,rise-time-ms",
			&led->pdata->rise_time_ms);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading rise-time-ms, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,hold-time-ms",
			&led->pdata->hold_time_ms);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading hold-time-ms, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,fall-time-ms",
			&led->pdata->fall_time_ms);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading fall-time-ms, rc = %d\n", rc);
			goto free_pdata;
		}

		rc = of_property_read_u32(temp, "aw2013,off-time-ms",
			&led->pdata->off_time_ms);
		if (rc < 0) {
			dev_err(&led->client->dev,
				"Failure reading off-time-ms, rc = %d\n", rc);
			goto free_pdata;
		}

		INIT_WORK(&led->brightness_work, aw2013_brightness_work);

		led->cdev.brightness_set = aw2013_set_brightness;

		rc = led_classdev_register(&led->client->dev, &led->cdev);
		if (rc) {
			dev_err(&led->client->dev,
				"unable to register led %d,rc=%d\n",
				led->id, rc);
			goto free_pdata;
		}

		rc = sysfs_create_group(&led->cdev.dev->kobj,
				&aw2013_led_attr_group);
		if (rc) {
			dev_err(&led->client->dev, "led sysfs rc: %d\n", rc);
			goto free_class;
		}
		parsed_leds++;
	}

	return 0;

free_class:
	aw2013_led_err_handle(led_array, parsed_leds);
	led_classdev_unregister(&led_array[parsed_leds].cdev);
	cancel_work_sync(&led_array[parsed_leds].brightness_work);
	devm_kfree(&led->client->dev, led_array[parsed_leds].pdata);
	led_array[parsed_leds].pdata = NULL;
	return rc;

free_pdata:
	aw2013_led_err_handle(led_array, parsed_leds);
	devm_kfree(&led->client->dev, led_array[parsed_leds].pdata);
	return rc;

free_err:
	aw2013_led_err_handle(led_array, parsed_leds);
	return rc;
}

/* < LAFITE-1500 yanzhiyao 20160215 begin */
/*led_num:red-0,green-1,blue-3*
  brightness:0-255*/
void aw2013_set_RGB_led_brightness(int led_num,int brightness)
{
	u8 val;
    if (brightness > 0) {
		if (brightness > 255)
			brightness = 255;
		aw2013_write(g_led, AW_REG_GLOBAL_CONTROL,
			AW_LED_MOUDLE_ENABLE_MASK);
		aw2013_write(g_led, AW_REG_LED_CONFIG_BASE + led_num,
			1);
		aw2013_write(g_led, AW_REG_LED_BRIGHTNESS_BASE + led_num,
			brightness);
		aw2013_read(g_led, AW_REG_LED_ENABLE, &val);
		aw2013_write(g_led, AW_REG_LED_ENABLE, val | (1 << led_num));
		/* < LAFITE-3190 yanzhiyao 20160229 begin */
		pr_debug("aw2013 %d LED brightness=%d\n",led_num,brightness);
		/* LAFITE-3190 yanzhiyao 20160229 end > */
	} else {
		aw2013_read(g_led, AW_REG_LED_ENABLE, &val);
		aw2013_write(g_led, AW_REG_LED_ENABLE, val & (~(1 << led_num)));
		/* < LAFITE-3190 yanzhiyao 20160229 begin */
		pr_debug("aw2013 %d LED close\n",led_num);
		/* LAFITE-3190 yanzhiyao 20160229 end > */
	}
}
EXPORT_SYMBOL(aw2013_set_RGB_led_brightness);
/* LAFITE-1500 yanzhiyao 20160215 end > */

static int aw2013_led_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct aw2013_led *led_array;
	struct device_node *node;
	int ret, num_leds = 0;
	/* < LAFITE-370 yanzhiyao 20160104 begin */
	printk("aw2013 led begin\n");
	/* LAFITE-370 yanzhiyao 20160104 end > */
	node = client->dev.of_node;
	if (node == NULL)
		return -EINVAL;

	num_leds = of_get_child_count(node);

	if (!num_leds)
		return -EINVAL;

	led_array = devm_kzalloc(&client->dev,
			(sizeof(struct aw2013_led) * num_leds), GFP_KERNEL);
	if (!led_array) {
		dev_err(&client->dev, "Unable to allocate memory\n");
		return -ENOMEM;
	}
	/* < LAFITE-1500 yanzhiyao 20160215 begin */
	g_led = led_array;
	/* LAFITE-1500 yanzhiyao 20160215 end > */
	led_array->client = client;
	led_array->num_leds = num_leds;

	mutex_init(&led_array->lock);

	ret = aw_2013_check_chipid(led_array);
	if (ret) {
/* < LAFITE-370 yanzhiyao 20160104 begin */
		//dev_err(&client->dev, "Check chip id error\n");
		printk("check chip id error\n");
/* LAFITE-370 yanzhiyao 20160104 end > */
		goto free_led_arry;
	}

	ret = aw2013_led_parse_child_node(led_array, node);
	if (ret) {
		dev_err(&client->dev, "parsed node error\n");
/* < LAFITE-370 yanzhiyao 20160104 begin */
#if 0
		goto free_led_arry;
#endif
/* LAFITE-370 yanzhiyao 20160104 end > */
	}

	i2c_set_clientdata(client, led_array);

	ret = aw2013_power_init(led_array, true);
	if (ret) {
		dev_err(&client->dev, "power init failed");
		goto fail_parsed_node;
	}
/*yulong add for showing green led in ftm mode 20160421*/
	if (1 == yl_get_ftm()) {
		pr_info("%s: ftm mode\n", __func__);
		aw2013_set_RGB_led_brightness(1, 255);
	}
	aw2013_flag = 1;
	return 0;

fail_parsed_node:
	aw2013_led_err_handle(led_array, num_leds);
free_led_arry:
	mutex_destroy(&led_array->lock);
	devm_kfree(&client->dev, led_array);
	led_array = NULL;
	return ret;
}

static int aw2013_led_remove(struct i2c_client *client)
{
	struct aw2013_led *led_array = i2c_get_clientdata(client);
	int i, parsed_leds = led_array->num_leds;

	for (i = 0; i < parsed_leds; i++) {
		sysfs_remove_group(&led_array[i].cdev.dev->kobj,
				&aw2013_led_attr_group);
		led_classdev_unregister(&led_array[i].cdev);
		cancel_work_sync(&led_array[i].brightness_work);
		devm_kfree(&client->dev, led_array[i].pdata);
		led_array[i].pdata = NULL;
	}
	mutex_destroy(&led_array->lock);
	devm_kfree(&client->dev, led_array);
	led_array = NULL;
	return 0;
}
/* < LAFITE-1500 yanzhiyao 20160215 begin */
static int aw2013_suspend(struct device *dev)
{
	/* < LAFITE-3222 yanzhiyao 20160301 begin */
	struct aw2013_led *led_array = i2c_get_clientdata(g_led->client);
	/* LAFITE-3222 yanzhiyao 20160301 end > */
	if(aw2013_sleep_led)
	{
		aw2013_set_RGB_led_brightness(1,0);
	}
	/* < LAFITE-3222 yanzhiyao 20160301 begin */
	/* < LAFITE-3222 yanzhiyao 20160229 begin */
	//if(g_blinking == 0)
	if((led_array[0].cdev.brightness || led_array[1].cdev.brightness||led_array[2].cdev.brightness) == 0)
	{
		aw2013_write(g_led, AW_REG_RESET, AW_LED_RESET_MASK);
	}
	/* LAFITE-3222 yanzhiyao 20160229 end > */
	/* LAFITE-3222 yanzhiyao 20160301 end > */
    return 0;
}

static const struct dev_pm_ops aw2013_pm_ops = {
    .suspend        = aw2013_suspend,
//  .suspend_noirq  = aw2013_suspend_noirq,
//  .resume         = aw2013_resume,
};
/* LAFITE-1500 yanzhiyao 20160215 end > */

static const struct i2c_device_id aw2013_led_id[] = {
	{"aw2013_led", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, aw2013_led_id);

static struct of_device_id aw2013_match_table[] = {
	{ .compatible = "awinic,aw2013",},
	{ },
};

static struct i2c_driver aw2013_led_driver = {
	.probe = aw2013_led_probe,
	.remove = aw2013_led_remove,
	.driver = {
		.name = "aw2013_led",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(aw2013_match_table),
		/* < LAFITE-1500 yanzhiyao 20160215 begin */
		.pm = &aw2013_pm_ops,
		/* LAFITE-1500 yanzhiyao 20160215 end > */
	},
	.id_table = aw2013_led_id,
};

static int __init aw2013_led_init(void)
{
	return i2c_add_driver(&aw2013_led_driver);
}
/*module_init(aw2013_led_init);*/
fs_initcall(aw2013_led_init);

static void __exit aw2013_led_exit(void)
{
	i2c_del_driver(&aw2013_led_driver);
}
module_exit(aw2013_led_exit);
/* < LAFITE-1500 yanzhiyao 20160215 begin */
module_param(aw2013_sleep_led, int, 0664);
MODULE_PARM_DESC(aw2013_sleep_led, "An visible int under sysfs");
/* LAFITE-1500 yanzhiyao 20160215 end > */
MODULE_DESCRIPTION("AWINIC aw2013 LED driver");
MODULE_LICENSE("GPL v2");
