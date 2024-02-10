MODULE := engines/nancy

MODULE_OBJS = \
  action/actionmanager.o \
  action/actionrecord.o \
  action/arfactory.o \
  action/autotext.o \
  action/datarecords.o \
  action/inventoryrecords.o \
  action/navigationrecords.o \
  action/soundrecords.o \
  action/miscrecords.o \
  action/conversation.o \
  action/interactivevideo.o \
  action/overlay.o \
  action/secondarymovie.o \
  action/secondaryvideo.o \
  action/puzzle/assemblypuzzle.o \
  action/puzzle/bballpuzzle.o \
  action/puzzle/bulpuzzle.o \
  action/puzzle/bombpuzzle.o \
  action/puzzle/collisionpuzzle.o \
  action/puzzle/cubepuzzle.o \
  action/puzzle/hamradiopuzzle.o \
  action/puzzle/leverpuzzle.o \
  action/puzzle/mazechasepuzzle.o \
  action/puzzle/mouselightpuzzle.o \
  action/puzzle/orderingpuzzle.o \
  action/puzzle/overridelockpuzzle.o \
  action/puzzle/passwordpuzzle.o \
  action/puzzle/peepholepuzzle.o \
  action/puzzle/raycastpuzzle.o \
  action/puzzle/riddlepuzzle.o \
  action/puzzle/rippedletterpuzzle.o \
  action/puzzle/rotatinglockpuzzle.o \
  action/puzzle/safedialpuzzle.o \
  action/puzzle/setplayerclock.o \
  action/puzzle/sliderpuzzle.o \
  action/puzzle/soundequalizerpuzzle.o \
  action/puzzle/spigotpuzzle.o \
  action/puzzle/tangrampuzzle.o \
  action/puzzle/telephone.o \
  action/puzzle/towerpuzzle.o \
  action/puzzle/turningpuzzle.o \
  action/puzzle/twodialpuzzle.o \
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
  state/loadsave.o \
  state/help.o \
  state/mainmenu.o \
  state/map.o \
  state/savedialog.o \
  state/scene.o \
  state/setupmenu.o \
  misc/hypertext.o \
  misc/lightning.o \
  misc/mousefollow.o \
  misc/specialeffect.o \
  cif.o \
  commontypes.o \
  console.o \
  cursor.o \
  decompress.o \
  enginedata.o \
  font.o \
  graphics.o \
  iff.o \
  input.o \
  metaengine.o \
  nancy.o \
  puzzledata.o \
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
