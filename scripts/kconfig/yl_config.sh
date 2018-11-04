#!/bin/bash
#
# Copyright (c) 2000-2017  YULONG Company
#
# PROPRIETARY RIGHTS of YULONG Company are involved in the
# subject matter of this material.  All manufacturing, reproduction, use,
# and sales rights pertaining to this subject matter are governed by the
# license agreement.  The recipient of this software implicitly accepts
# the terms of the license.
#

usage() {
cat << EOF
Usage: bash `basename $0` PRODUCT_NAME HARDWARE_CONFIG DEFAULT_CONFIG
example:
    bash `basename $0` cp8860u p0 msm8960_defconfig
EOF
	exit 1
}

if test "$#" != "3"; then
	usage
fi

YL_PROD_NAME=$1
YL_HW_CONFIG=$2
DEFAULT_CONFIG=$3

echo "KERNEL: Yulong Product Config: ${YL_HW_CONFIG}_${YL_PROD_NAME}"

DEFCONFIG_SRC=${srctree}/arch/${SRCARCH}/configs/
PRDCONFIG_SRC=${srctree}/arch/${SRCARCH}/mach-msm/board-${YL_PROD_NAME}

DEST_CONFIG_F=${DEFCONFIG_SRC}/.${DEFAULT_CONFIG}
BASE_CONFIG_F=${DEFCONFIG_SRC}/${DEFAULT_CONFIG}
PROD_CONFIG_F=${PRDCONFIG_SRC}/yl_product_configs
BOARD_CONFIG_F=${PRDCONFIG_SRC}/board_config_${YL_HW_CONFIG}
DEBUG_CONFIG_F=${PRDCONFIG_SRC}/board_config_debug

echo "# This file was automatically generated by YuLong." > ${DEST_CONFIG_F}
echo "# `date`" >> ${DEST_CONFIG_F}
echo "" >> ${DEST_CONFIG_F}
cat ${BASE_CONFIG_F} >> ${DEST_CONFIG_F}
echo "" >> ${DEST_CONFIG_F}
echo "# <YuLong> Product Configs" >> ${DEST_CONFIG_F}
echo CONFIG_YULONG_PRODUCT=y >> ${DEST_CONFIG_F}
echo CONFIG_BOARD_`echo ${YL_PROD_NAME}|tr a-z A-Z`=y >> ${DEST_CONFIG_F}
echo CONFIG_BOARD_VER_`echo ${YL_HW_CONFIG}|tr a-z A-Z`=y >> ${DEST_CONFIG_F}
if [ -f ${PROD_CONFIG_F} ]
then
	cat ${PROD_CONFIG_F} >> ${DEST_CONFIG_F}
fi
if [ -f ${BOARD_CONFIG_F} ]
then
	echo "" >> ${DEST_CONFIG_F}
	echo "# <YuLong> board_config_${YL_HW_CONFIG}" >> ${DEST_CONFIG_F}
	cat ${BOARD_CONFIG_F} >> ${DEST_CONFIG_F}
fi
if [ "${TARGET_BUILD_VARIANT}" = "eng" -a -f ${DEBUG_CONFIG_F} ]
then
	echo "" >> ${DEST_CONFIG_F}
	echo "# <YuLong> board_config_debug" >> ${DEST_CONFIG_F}
	cat ${DEBUG_CONFIG_F} >> ${DEST_CONFIG_F}
fi
echo "" >> ${DEST_CONFIG_F}
echo "# <YuLong> End" >> ${DEST_CONFIG_F}

