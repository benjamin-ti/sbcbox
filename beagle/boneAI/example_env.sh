# make ipu-examples:
# export TI_RTOS_PATH=/home/user/ti/processor_sdk_rtos_am57xx_06_03_02_08/
# make ti-ipc-linux
# make ti-ipc-linux-examples

# dsp/ipu ram:
# IPU2 CMA    0x95800000  56 MB
# DSP1 CMA    0x99000000  64 MB
# IPU1 CMA 	  0x9d000000  32 MB
# DSP2 CMA    0x9f000000   8 MB
# Default CMA 0xfe400000  24 MB

# ti_ipc_remoteproc_ResourceTable
# processor_sdk_rtos_am57xx_06_03_02_08/ipc_3_50_04_08/packages/ti/ipc/remoteproc/rsc_table_vayu_ipu.h

# BeagleBone Fan Cape
# 5V 20mm Fan

export SDK_INSTALL_PATH=/home/user/ti/processor_sdk_rtos_am57xx_06_03_02_08/
export PDK_INSTALL_PATH=$SDK_INSTALL_PATH/pdk_am57xx_1_0_18/packages/
export TOOLCHAIN_PATH_M4=$SDK_INSTALL_PATH/ti-cgt-arm_18.12.5.LTS
export XDC_INSTALL_DIR=$SDK_INSTALL_PATH/xdctools_3_55_02_22_core
export BOARD=evmAM572x
export CORE=ipu1_0
