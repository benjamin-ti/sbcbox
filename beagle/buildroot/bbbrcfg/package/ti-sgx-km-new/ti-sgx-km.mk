################################################################################
#
# ti-sgx-km
#
################################################################################

# This correpsonds to SDK 06.01.00.08
TI_SGX_KM_VERSION = cf7f48cb30abfd5df7a60c9bf4bbb1dde0d496d9
TI_SGX_KM_SITE = http://git.ti.com/git/graphics/omap5-sgx-ddk-linux.git
TI_SGX_KM_SITE_METHOD = git
TI_SGX_KM_LICENSE = GPL-2.0
TI_SGX_KM_LICENSE_FILES = eurasia_km/GPL-COPYING

TI_SGX_KM_DEPENDENCIES = linux

TI_SGX_KM_MAKE_OPTS = \
	$(LINUX_MAKE_FLAGS) \
	KERNELDIR=$(LINUX_DIR) \
	TARGET_PRODUCT=$(TI_SGX_KM_NEWPLATFORM_NAME)

TI_SGX_KM_NEWPLATFORM_NAME = ti335x

TI_SGX_KM_NEWSUBDIR = eurasia_km/eurasiacon/build/linux2/omap_linux

define TI_SGX_KM_NEWBUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TI_SGX_KM_NEWMAKE_OPTS) \
		-C $(@D)/$(TI_SGX_KM_NEWSUBDIR)
endef

define TI_SGX_KM_NEWINSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TI_SGX_KM_NEWMAKE_OPTS) \
		DISCIMAGE=$(TARGET_DIR) \
		kbuild_install -C $(@D)/$(TI_SGX_KM_NEWSUBDIR)
endef

$(eval $(generic-package))
