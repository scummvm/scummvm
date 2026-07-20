MODULE := engines/nancy

MODULE_OBJS = \
  action/actionmanager.o \
  action/actionrecord.o \
  action/actionzone.o \
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
  action/puzzle/angletosspuzzle.o \
  action/puzzle/arcadepuzzle.o \
  action/puzzle/assemblypuzzle.o \
  action/puzzle/bballpuzzle.o \
  action/puzzle/beadpuzzle.o \
  action/puzzle/blockspuzzle.o \
  action/puzzle/boardgamepuzzle.o \
  action/puzzle/bulpuzzle.o \
  action/puzzle/bombpuzzle.o \
  action/puzzle/cardgamepuzzle.o \
  action/puzzle/collisionpuzzle.o \
  action/puzzle/cubepuzzle.o \
  action/puzzle/cuttingpuzzle.o \
  action/puzzle/dotconnectpuzzle.o \
  action/puzzle/drivingpuzzle.o \
  action/puzzle/dropsortpuzzle.o \
  action/puzzle/gridmappuzzle.o \
  action/puzzle/hamradiopuzzle.o \
  action/puzzle/leverpuzzle.o \
  action/puzzle/magnetmazepuzzle.o \
  action/puzzle/mazechasepuzzle.o \
  action/puzzle/matchpuzzle.o \
  action/puzzle/memorypuzzle.o \
  action/puzzle/mindpuzzle.o \
  action/puzzle/minigolfpuzzle.o \
  action/puzzle/mirrorlightpuzzle.o \
  action/puzzle/mouselightpuzzle.o \
  action/puzzle/multibuildpuzzle.o \
  action/puzzle/onebuildpuzzle.o \
  action/puzzle/orderingpuzzle.o \
  action/puzzle/overridelockpuzzle.o \
  action/puzzle/passwordpuzzle.o \
  action/puzzle/peepholepuzzle.o \
  action/puzzle/pegspuzzle.o \
  action/puzzle/quizpuzzle.o \
  action/puzzle/raycastpuzzle.o \
  action/puzzle/riddlepuzzle.o \
  action/puzzle/rippedletterpuzzle.o \
  action/puzzle/rotatinglockpuzzle.o \
  action/puzzle/safedialpuzzle.o \
  action/puzzle/scalepuzzle.o \
  action/puzzle/setplayerclock.o \
  action/puzzle/sewingmachinepuzzle.o \
  action/puzzle/sliderpuzzle.o \
  action/puzzle/sortpuzzle.o \
  action/puzzle/soundequalizerpuzzle.o \
  action/puzzle/soundmatchpuzzle.o \
  action/puzzle/spigotpuzzle.o \
  action/puzzle/stepobjectspuzzle.o \
  action/puzzle/tangrampuzzle.o \
  action/puzzle/telephone.o \
  action/puzzle/towerpuzzle.o \
  action/puzzle/turningpuzzle.o \
  action/puzzle/twodialpuzzle.o \
  action/puzzle/typingquizpuzzle.o \
  action/puzzle/whalesurvivorpuzzle.o \
  action/puzzle/wordfindpuzzle.o \
  ui/fullscreenimage.o \
  ui/animatedbutton.o \
  ui/button.o \
  ui/clock.o \
  ui/cellphonepopup.o \
  ui/conversationpopup.o \
  ui/inventorybox.o \
  ui/inventorypopup.o \
  ui/notebookpopup.o \
  ui/ornaments.o \
  ui/scrollbar.o \
  ui/scrolltextbox.o \
  ui/taskbar.o \
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
  movieplayer.o \
  input.o \
  metaengine.o \
  nancy.o \
  puzzledata.o \
  renderobject.o \
  resource.o \
  sound.o \
  sound_vorbis.o \
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
