/* Copyright (c) 2011-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include "media/msm_cam_sensor.h"
#include "msm_sensor_module_info.h"

//#define CDBG_MSM_SENSOR_MODULE_INFO
#undef CDBG
#ifdef CDBG_MSM_SENSOR_MODULE_INFO
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

/* [YULONG BEGIN] TFS_386825: modify for back mono camera - chaiyongliang_20160416 */
char msm_sensor_module_info[3][MAX_SENSOR_NAME] = {"unknown", "unknown", "unknown"};
/* [YULONG END] TFS_386825 - chaiyongliang_20160416 */

int msm_sensor_module_info_set(enum msm_sensor_camera_id_t position, char *module_info)
{
    if(position != CAMERA_0 && position != CAMERA_1)
    {
        pr_err(" %s, there was a unknown camera module want to regist\n", __func__);
        return -1;
    }
    CDBG(" %s, there want to set a %s camera info from otp.\n", __func__, (position==CAMERA_0)?"back":"front");
    memcpy(msm_sensor_module_info[position], module_info, strlen(module_info));
    CDBG(" %s, set----name:%s\n", __func__, msm_sensor_module_info[position]);

    return 0;
}

int msm_sensor_module_info_get(enum msm_sensor_camera_id_t position, char *module_info)
{
    /* [YULONG BEGIN] TFS_386825: modify for back mono camera - chaiyongliang_20160416 */
    if(position != CAMERA_0 && position != CAMERA_1 && position != CAMERA_2)
    {
        pr_err(" %s, there was a unknown camera module want to get\n", __func__);
        return -1;
    }

    if(CAMERA_2 == position)
        CDBG(" %s, there want to get a %s camera info from otp.\n", __func__, "back_mono");
    else
        CDBG(" %s, there want to get a %s camera info from otp.\n", __func__, (position==CAMERA_0)?"back":"front");
    /* [YULONG END] TFS_386825 - chaiyongliang_20160416 */
    memcpy(module_info, msm_sensor_module_info[position], strlen(msm_sensor_module_info[position]));
    CDBG(" %s, return----name:%s\n", __func__, module_info);

    return 0;
}
