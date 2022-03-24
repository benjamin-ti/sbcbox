################################################################################
#
# Qt-Hello
#
################################################################################

QTHELLO_VERSION = 1.0
QTHELLO_SITE = $(BR2_EXTERNAL_BENSBBB_PATH)/package/QtHello/src
QTHELLO_SITE_METHOD = local

define QTHELLO_CONFIGURE_CMDS
	(cd $(@D); $(QT5_QMAKE))
endef

define QTHELLO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QTHELLO_INSTALL_TARGET_CMDS
   $(INSTALL) -D -m 0755 $(@D)/Sample $(TARGET_DIR)/root/QtHello
endef

$(eval $(generic-package))
