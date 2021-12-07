################################################################################
#
# HMI Testproject Qt test2
#
################################################################################

TEST2_VERSION = 1.0
TEST2_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/test2/src
TEST2_SITE_METHOD = local

define TEST2_CONFIGURE_CMDS
	(cd $(@D); $(QT5_QMAKE))
endef

define TEST2_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define TEST2_INSTALL_TARGET_CMDS
   $(INSTALL) -D -m 0755 $(@D)/test2 $(TARGET_DIR)/root
endef

$(eval $(generic-package))
