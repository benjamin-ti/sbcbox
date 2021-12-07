################################################################################
#
# qt-Sample-demo
#
################################################################################

SAMPLE_VERSION = 1.0
SAMPLE_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/Sample/src
SAMPLE_SITE_METHOD = local

define SAMPLE_CONFIGURE_CMDS
	(cd $(@D); $(QT5_QMAKE))
endef

define SAMPLE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define SAMPLE_INSTALL_TARGET_CMDS
   $(INSTALL) -D -m 0755 $(@D)/Sample $(TARGET_DIR)/root
endef

$(eval $(generic-package))
