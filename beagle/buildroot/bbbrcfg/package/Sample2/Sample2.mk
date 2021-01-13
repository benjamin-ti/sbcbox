# sample2

SAMPLE2_VERSION = 1.0
SAMPLE2_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/Sample2/src
SAMPLE2_SITE_METHOD = local

#define SAMPLE2_BUILD_CMDS
#	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
#endef

define SAMPLE_BUILD_CMDS
	$(MAKE) -C $(@D)
endef

#define SAMPLE2_CONFIGURE_CMDS
#	(cd $ @D; $(TARGET_MAKE_ENV) $(QT_QMAKE))
#endef

define SAMPLE2_INSTALL_TARGET_CMDS
   $(INSTALL) -D -m 0755 $(@D)/Sample2 $(TARGET_DIR)/root
endef

$(eval $(generic-package))
