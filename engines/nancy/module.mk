MODULE := engines/nancy

MODULE_OBJS = \
  action/actionmanager.o \
  action/actionrecord.o \
  action/arfactory.o \
  action/leverpuzzle.o \
  action/lightning.o \
  action/orderingpuzzle.o \
  action/passwordpuzzle.o \
  action/primaryvideo.o \
  action/recordtypes.o \
  action/rotatinglockpuzzle.o \
  action/secondarymovie.o \
  action/secondaryvideo.o \
  action/sliderpuzzle.o \
  action/staticbitmapanim.o \
  action/telephone.o \
  ui/fullscreenimage.o \
  ui/animatedbutton.o \
  ui/button.o \
  ui/clock.o \
  ui/inventorybox.o \
  ui/ornaments.o \
  ui/scrollbar.o \
  ui/textbox.o \
  ui/viewport.o \
  state/credits.o \
  state/logo.o \
  state/help.o \
  state/mainmenu.o \
  state/map.o \
  state/scene.o \
  commontypes.o \
  console.o \
  cursor.o \
  decompress.o \
  dialogs.o \
  font.o \
  graphics.o \
  iff.o \
  input.o \
  metaengine.o \
  nancy.o \
  renderobject.o \
  resource.o \
  sound.o \
  util.o \
  video.o

# This module can be built as a plugin
ifeq ($(ENABLE_NANCY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
