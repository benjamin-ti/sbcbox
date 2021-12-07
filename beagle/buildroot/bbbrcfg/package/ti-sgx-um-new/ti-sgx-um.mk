################################################################################
#
# ti-sgx-um-new
#
################################################################################

# This correpsonds to SDK 06.01.00.08
TI_SGX_UM_NEW_VERSION = 909e237baf47d0bde006ff25552f5403fd7e359d
TI_SGX_UM_NEW_SITE = http://git.ti.com/git/graphics/omap5-sgx-ddk-um-linux.git
TI_SGX_UM_NEW_SITE_METHOD = git
TI_SGX_UM_NEW_LICENSE = TI TSPA License
TI_SGX_UM_NEW_LICENSE_FILES = TI-Linux-Graphics-DDK-UM-Manifest.doc
TI_SGX_UM_NEW_INSTALL_STAGING = YES
TI_SGX_UM_NEW_TARGET=ti335x

# ti-sgx-um is a egl/gles provider only if libdrm is installed
TI_SGX_UM_NEW_DEPENDENCIES = libdrm wayland

define TI_SGX_UM_NEW_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DISCIMAGE=$(STAGING_DIR) \
		TARGET_PRODUCT=$(TI_SGX_UM_NEW_TARGET) install
endef

define TI_SGX_UM_NEW_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DISCIMAGE=$(TARGET_DIR) \
		TARGET_PRODUCT=$(TI_SGX_UM_NEW_TARGET) install
endef

TI_SGX_UM_NEW_POST_INSTALL_TARGET_HOOKS += TI_SGX_UM_NEW_INSTALL_CONF

define TI_SGX_UM_NEW_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 $(BR2_EXTERNAL_BENSBBB_PATH)/package/ti-sgx-um-new/S80ti-sgx \
		$(TARGET_DIR)/etc/init.d/S80ti-sgx
endef

$(eval $(generic-package))
