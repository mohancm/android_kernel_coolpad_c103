/*
 * Synaptics RMI4 touchscreen driver
 *
 * Copyright (C) 2012 Synaptics Incorporated
 *
 * Copyright (C) 2012 Alexandra Chin <alexandra.chin@tw.synaptics.com>
 * Copyright (C) 2012 Scott Lin <scott.lin@tw.synaptics.com>
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _SYNAPTICS_DSX_H_
#define _SYNAPTICS_DSX_H_

/*
 * struct synaptics_rmi4_capacitance_button_map - 0d button map
 * @nbuttons: number of buttons
 * @map: button map
 */
struct synaptics_rmi4_capacitance_button_map {
	unsigned char nbuttons;
	unsigned char *map;
};

/*
 * struct synaptics_rmi4_platform_data - rmi4 platform data
 * @x_flip: x flip flag
 * @y_flip: y flip flag
 * @i2c_pull_up: pull up i2c bus with regulator
 * @power_down_enable: enable complete regulator shutdown in suspend
 * @irq_gpio: attention interrupt gpio
 * @irq_flags: flags used by the irq
 * @reset_flags: flags used by reset line
 * @reset_gpio: reset gpio
 * @panel_x: panel maximum values on the x
 * @panel_y: panel maximum values on the y
 * @disp_maxx: display panel maximum values on the x
 * @disp_maxy: display panel maximum values on the y
 * @disp_minx: display panel minimum values on the x
 * @disp_miny: display panel minimum values on the y
 * @panel_maxx: touch panel maximum values on the x
 * @panel_maxy: touch panel maximum values on the y
 * @panel_minx: touch panel minimum values on the x
 * @panel_miny: touch panel minimum values on the y
 * @reset_delay: reset delay
 * @gpio_config: pointer to gpio configuration function
 * @capacitance_button_map: pointer to 0d button map
 */
struct synaptics_rmi4_platform_data {
	bool x_flip;
	bool y_flip;
	bool swap_axes;
	bool i2c_pull_up;
	bool power_down_enable;
	bool disable_gpios;
	bool do_lockdown;
	bool detect_device;
	bool modify_reso;
	unsigned irq_gpio;
	u32 irq_flags;
	u32 reset_flags;
	unsigned reset_gpio;
	unsigned panel_minx;
	unsigned panel_miny;
	unsigned panel_maxx;
	unsigned panel_maxy;
	unsigned disp_minx;
	unsigned disp_miny;
	unsigned disp_maxx;
	unsigned disp_maxy;
	unsigned reset_delay;
	const char *fw_image_name;
	unsigned int package_id;
	int (*gpio_config)(unsigned gpio, bool configure);
	struct synaptics_rmi4_capacitance_button_map *capacitance_button_map;
	int (*init)(void);
	void (*release)(void);
	int (*power)(int on);
	int (*reset)(int ms);
	int (*suspend)(void);
	int (*resume)(void);
	unsigned char (*get_id_pin)(void);
	struct virtual_keys_button *buttons;
	int nbuttons;
	int screen_x;
	int screen_y;
	int key_debounce;
	struct regulator *ts_vcc_i2c;
	struct regulator *ts_vdd;
	u32 reset_gpio_flags;
	u32 irq_gpio_flags;
	unsigned char hw_type;	/*COB:1  COF:2*/
	unsigned int ic_type;
};

extern unsigned char SYNA_DEBUG_ON;
#define SYNA_DEBUG(fmt, arg...)  do {\
	if (SYNA_DEBUG_ON)\
		pr_notice("<< SYNA-DEBUG >> [%d]"fmt"\n", __LINE__, ##arg);\
} while (0)

#endif
