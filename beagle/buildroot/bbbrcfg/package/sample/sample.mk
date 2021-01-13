# sample

SAMPLE_VERSION = 1.0
SAMPLE_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/sample/src
SAMPLE_SITE_METHOD = local

define SAMPLE_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define SAMPLE_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/sample $(TARGET_DIR)/root
endef

$(eval $(generic-package))
