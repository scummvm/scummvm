MODULE := engines/director

MODULE_OBJS = \
	archive.o \
	cast.o \
	channel.o \
	cursor.o \
	debugger.o \
	director.o \
	events.o \
	fonts.o \
	frame.o \
	game-quirks.o \
	graphics.o \
	images.o \
	metaengine.o \
	movie.o \
	picture.o \
	resource.o \
	rte.o \
	score.o \
	sound.o \
	sprite.o \
	stxt.o \
	tests.o \
	transitions.o \
	types.o \
	util.o \
	window.o \
	castmember/castmember.o \
	castmember/bitmap.o \
	castmember/digitalvideo.o \
	castmember/filmloop.o \
	castmember/movie.o \
	castmember/palette.o \
	castmember/script.o \
	castmember/shape.o \
	castmember/sound.o \
	castmember/text.o \
	castmember/transition.o \
	lingo/lingo.o \
	lingo/lingo-builtins.o \
	lingo/lingo-bytecode.o \
	lingo/lingo-code.o \
	lingo/lingo-codegen.o \
	lingo/lingo-events.o \
	lingo/lingo-funcs.o \
	lingo/lingo-gr.o \
	lingo/lingo-lex.o \
	lingo/lingo-mci.o \
	lingo/lingo-object.o \
	lingo/lingo-patcher.o \
	lingo/lingo-preprocessor.o \
	lingo/lingo-the.o \
	lingo/lingo-utils.o \
	lingo/lingodec/ast.o \
	lingo/lingodec/context.o \
	lingo/lingodec/codewritervisitor.o \
	lingo/lingodec/handler.o \
	lingo/lingodec/names.o \
	lingo/lingodec/script.o \
	lingo/xlibs/aiff.o \
	lingo/xlibs/applecdxobj.o \
	lingo/xlibs/askuser.o \
	lingo/xlibs/backdrop.o \
	lingo/xlibs/barakeobj.o \
	lingo/xlibs/batqt.o \
	lingo/xlibs/bimxobj.o \
	lingo/xlibs/blitpict.o \
	lingo/xlibs/cdromxobj.o \
	lingo/xlibs/closebleedwindowxcmd.o \
	lingo/xlibs/colorxobj.o \
	lingo/xlibs/colorcursorxobj.o \
	lingo/xlibs/consumer.o \
	lingo/xlibs/cursorxobj.o \
	lingo/xlibs/darkenscreen.o \
	lingo/xlibs/dateutil.o \
	lingo/xlibs/developerStack.o \
	lingo/xlibs/dialogsxobj.o \
	lingo/xlibs/dirutil.o \
	lingo/xlibs/dllglue.o \
	lingo/xlibs/dpwavi.o \
	lingo/xlibs/dpwqtw.o \
	lingo/xlibs/draw.o \
	lingo/xlibs/ednox.o \
	lingo/xlibs/eventq.o \
	lingo/xlibs/fadegammadownxcmd.o \
	lingo/xlibs/fadegammaupxcmd.o \
	lingo/xlibs/fadegammaxcmd.o \
	lingo/xlibs/fedracul.o \
	lingo/xlibs/feimasks.o \
	lingo/xlibs/feiprefs.o \
	lingo/xlibs/fileexists.o \
	lingo/xlibs/fileio.o \
	lingo/xlibs/findereventsxcmd.o \
	lingo/xlibs/findfolder.o \
	lingo/xlibs/findsys.o \
	lingo/xlibs/findwin.o \
	lingo/xlibs/flushxobj.o \
	lingo/xlibs/fplayxobj.o \
	lingo/xlibs/fsutil.o \
	lingo/xlibs/genutils.o \
	lingo/xlibs/getscreenrectsxfcn.o \
	lingo/xlibs/getscreensizexfcn.o \
	lingo/xlibs/gpid.o \
	lingo/xlibs/henry.o \
	lingo/xlibs/hitmap.o \
	lingo/xlibs/inixobj.o \
	lingo/xlibs/instobj.o \
	lingo/xlibs/iscd.o \
	lingo/xlibs/ispippin.o \
	lingo/xlibs/jitdraw3.o \
	lingo/xlibs/jwxini.o \
	lingo/xlibs/labeldrvxobj.o \
	lingo/xlibs/maniacbg.o \
	lingo/xlibs/mapnavigatorxobj.o \
	lingo/xlibs/memcheckxobj.o \
	lingo/xlibs/memoryxobj.o \
	lingo/xlibs/misc.o \
	lingo/xlibs/miscx.o \
	lingo/xlibs/mmaskxobj.o \
	lingo/xlibs/mmovie.o \
	lingo/xlibs/moovxobj.o \
	lingo/xlibs/movemousejp.o \
	lingo/xlibs/movemousexobj.o \
	lingo/xlibs/movieidxxobj.o \
	lingo/xlibs/movutils.o \
	lingo/xlibs/mystisle.o \
	lingo/xlibs/openbleedwindowxcmd.o \
	lingo/xlibs/orthoplayxobj.o \
	lingo/xlibs/paco.o \
	lingo/xlibs/palxobj.o \
	lingo/xlibs/panel.o \
	lingo/xlibs/popupmenuxobj.o \
	lingo/xlibs/porta.o \
	lingo/xlibs/portaxcmd.o \
	lingo/xlibs/prefpath.o \
	lingo/xlibs/printomatic.o \
	lingo/xlibs/processxobj.o \
	lingo/xlibs/qtcatmovieplayerxobj.o \
	lingo/xlibs/qtmovie.o \
	lingo/xlibs/qtvr.o \
	lingo/xlibs/quicktime.o \
	lingo/xlibs/registercomponent.o \
	lingo/xlibs/remixxcmd.o \
	lingo/xlibs/serialportxobj.o \
	lingo/xlibs/soundjam.o \
	lingo/xlibs/spacemgr.o \
	lingo/xlibs/stagetc.o \
	lingo/xlibs/syscolor.o \
	lingo/xlibs/unittest.o \
	lingo/xlibs/valkyrie.o \
	lingo/xlibs/videodiscxobj.o \
	lingo/xlibs/vmisonxfcn.o \
	lingo/xlibs/volumelist.o \
	lingo/xlibs/widgetxobj.o \
	lingo/xlibs/window.o \
	lingo/xlibs/winxobj.o \
	lingo/xlibs/wininfo.o \
	lingo/xlibs/xcmdglue.o \
	lingo/xlibs/xio.o \
	lingo/xlibs/xplayanim.o \
	lingo/xlibs/xsoundxfcn.o \
	lingo/xlibs/xwin.o \
	lingo/xlibs/yasix.o \
	lingo/xtras/directsound.o \
	lingo/xtras/keypoll.o \
	lingo/xtras/qtvrxtra.o \
	lingo/xtras/scrnutil.o \
	lingo/xtras/timextra.o


ifdef USE_IMGUI
MODULE_OBJS += \
	debugger/debugtools.o \
	debugger/dt-cast.o \
	debugger/dt-controlpanel.o \
	debugger/dt-lists.o \
	debugger/dt-logger.o \
	debugger/dt-score.o \
	debugger/dt-script-d2.o \
	debugger/dt-script-d4.o \
	debugger/dt-scripts.o

endif

# HACK: Skip this when including the file for detection objects.
ifeq "$(LOAD_RULES_MK)" "1"
director-grammar:
	`brew --prefix flex`/bin/flex engines/director/lingo/lingo-lex.l
	`brew --prefix bison`/bin/bison -dv engines/director/lingo/lingo-gr.y
endif

# This module can be built as a plugin
ifeq ($(ENABLE_DIRECTOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
