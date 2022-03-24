################################################################################
#
# HMI Testproject Qt CV-Touch
#
################################################################################

CVTOUCHDEMO_VERSION = 1.0
CVTOUCHDEMO_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/CVTouchDemo/src
CVTOUCHDEMO_SITE_METHOD = local

define CVTOUCHDEMO_CONFIGURE_CMDS
	(cd $(@D); $(QT5_QMAKE))
endef

define CVTOUCHDEMO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define CVTOUCHDEMO_INSTALL_TARGET_CMDS
   $(INSTALL) -D -m 0755 $(@D)/test2 $(TARGET_DIR)/root/CVTouchDemo
endef

$(eval $(generic-package))
