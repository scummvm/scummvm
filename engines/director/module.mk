MODULE := engines/director

MODULE_OBJS = \
	archive.o \
	archive-save.o \
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
	castmember/richtext.o \
	castmember/script.o \
	castmember/shape.o \
	castmember/sound.o \
	castmember/text.o \
	castmember/transition.o \
	castmember/xtra.o \
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
	lingo/xlibs/a/aiff.o \
	lingo/xlibs/a/applecdxobj.o \
	lingo/xlibs/a/askuser.o \
	lingo/xlibs/b/backdrop.o \
	lingo/xlibs/b/barakeobj.o \
	lingo/xlibs/b/batqt.o \
	lingo/xlibs/b/bimxobj.o \
	lingo/xlibs/b/blitpict.o \
	lingo/xlibs/c/cdromxobj.o \
	lingo/xlibs/c/closebleedwindowxcmd.o \
	lingo/xlibs/c/colorxobj.o \
	lingo/xlibs/c/colorcursorxobj.o \
	lingo/xlibs/c/consumer.o \
	lingo/xlibs/c/cursorxobj.o \
	lingo/xlibs/d/darkenscreen.o \
	lingo/xlibs/d/dateutil.o \
	lingo/xlibs/d/developerStack.o \
	lingo/xlibs/d/dialogsxobj.o \
	lingo/xlibs/d/dirutil.o \
	lingo/xlibs/d/dllglue.o \
	lingo/xlibs/d/dpwavi.o \
	lingo/xlibs/d/dpwqtw.o \
	lingo/xlibs/d/draw.o \
	lingo/xlibs/e/ednox.o \
	lingo/xlibs/e/eventq.o \
	lingo/xlibs/f/fadegammadownxcmd.o \
	lingo/xlibs/f/fadegammaupxcmd.o \
	lingo/xlibs/f/fadegammaxcmd.o \
	lingo/xlibs/f/fedracul.o \
	lingo/xlibs/f/feimasks.o \
	lingo/xlibs/f/feiprefs.o \
	lingo/xlibs/f/fileexists.o \
	lingo/xlibs/f/fileio.o \
	lingo/xlibs/f/findereventsxcmd.o \
	lingo/xlibs/f/findfolder.o \
	lingo/xlibs/f/findsys.o \
	lingo/xlibs/f/findwin.o \
	lingo/xlibs/f/flushxobj.o \
	lingo/xlibs/f/fplayxobj.o \
	lingo/xlibs/f/fsutil.o \
	lingo/xlibs/g/genutils.o \
	lingo/xlibs/g/getscreenrectsxfcn.o \
	lingo/xlibs/g/getscreensizexfcn.o \
	lingo/xlibs/g/getsoundinlevel.o \
	lingo/xlibs/g/gpid.o \
	lingo/xlibs/h/henry.o \
	lingo/xlibs/h/hitmap.o \
	lingo/xlibs/i/inixobj.o \
	lingo/xlibs/i/instobj.o \
	lingo/xlibs/i/iscd.o \
	lingo/xlibs/i/ispippin.o \
	lingo/xlibs/j/jitdraw3.o \
	lingo/xlibs/j/jwxini.o \
	lingo/xlibs/l/labeldrvxobj.o \
	lingo/xlibs/l/listdev.o \
	lingo/xlibs/m/maniacbg.o \
	lingo/xlibs/m/mapnavigatorxobj.o \
	lingo/xlibs/m/memcheckxobj.o \
	lingo/xlibs/m/memoryxobj.o \
	lingo/xlibs/m/misc.o \
	lingo/xlibs/m/miscx.o \
	lingo/xlibs/m/mmaskxobj.o \
	lingo/xlibs/m/mmovie.o \
	lingo/xlibs/m/moovxobj.o \
	lingo/xlibs/m/movemousejp.o \
	lingo/xlibs/m/movemousexobj.o \
	lingo/xlibs/m/movieidxxobj.o \
	lingo/xlibs/m/movutils.o \
	lingo/xlibs/m/msfile.o \
	lingo/xlibs/m/mystisle.o \
	lingo/xlibs/b/blockthedrawingxobj.o \
	lingo/xlibs/m/mazexobj.o \
	lingo/xlibs/o/openbleedwindowxcmd.o \
 	lingo/xlibs/p/playsoundmoviexobj.o \
 	lingo/xlibs/s/savenrestorexobj.o \
 	lingo/xlibs/t/temnotaxobj.o \
	lingo/xlibs/o/orthoplayxobj.o \
	lingo/xlibs/p/paco.o \
	lingo/xlibs/p/palxobj.o \
	lingo/xlibs/p/panel.o \
	lingo/xlibs/p/pharaohs.o \
	lingo/xlibs/p/popupmenuxobj.o \
	lingo/xlibs/p/porta.o \
	lingo/xlibs/p/prefpath.o \
	lingo/xlibs/p/printomatic.o \
	lingo/xlibs/p/processxobj.o \
	lingo/xlibs/p/putcurs.o \
	lingo/xlibs/q/qtcatmovieplayerxobj.o \
	lingo/xlibs/q/qtmovie.o \
	lingo/xlibs/q/qtvr.o \
	lingo/xlibs/q/quicktime.o \
	lingo/xlibs/r/registercomponent.o \
	lingo/xlibs/r/remixxcmd.o \
	lingo/xlibs/s/serialportxobj.o \
	lingo/xlibs/s/smallutil.o \
	lingo/xlibs/s/soundjam.o \
	lingo/xlibs/s/spacemgr.o \
	lingo/xlibs/s/stagetc.o \
	lingo/xlibs/s/syscolor.o \
	lingo/xlibs/t/tengu.o \
	lingo/xlibs/u/unittest.o \
	lingo/xlibs/v/valkyrie.o \
	lingo/xlibs/v/versions.o \
	lingo/xlibs/v/videodiscxobj.o \
	lingo/xlibs/v/vmisonxfcn.o \
	lingo/xlibs/v/vmpresent.o \
	lingo/xlibs/v/volumelist.o \
	lingo/xlibs/v/voyagerxsound.o \
	lingo/xlibs/w/widgetxobj.o \
	lingo/xlibs/w/window.o \
	lingo/xlibs/w/winxobj.o \
	lingo/xlibs/w/wininfo.o \
	lingo/xlibs/x/xcmdglue.o \
	lingo/xlibs/x/xio.o \
	lingo/xlibs/x/xplayanim.o \
	lingo/xlibs/x/xplaypacoxfcn.o \
	lingo/xlibs/x/xsoundxfcn.o \
	lingo/xlibs/x/xwin.o \
	lingo/xlibs/y/yasix.o \
	lingo/xtras/b/budapi.o \
	lingo/xtras/a/audio.o \
	lingo/xtras/directsound.o \
	lingo/xtras/d/displayres.o \
	lingo/xtras/filextra.o \
	lingo/xtras/keypoll.o \
	lingo/xtras/masterapp.o \
	lingo/xtras/m/mui.o \
	lingo/xtras/openurl.o \
	lingo/xtras/oscheck.o \
	lingo/xtras/qtvrxtra.o \
	lingo/xtras/r/registryreader.o \
	lingo/xtras/rtk.o \
	lingo/xtras/scrnutil.o \
	lingo/xtras/s/smacker.o \
	lingo/xtras/s/staytoonedhall.o \
	lingo/xtras/s/staytoonedball.o \
	lingo/xtras/s/staytoonedglop.o \
	lingo/xtras/s/staytoonedhigh.o \
	lingo/xtras/s/staytoonedober.o \
	lingo/xtras/s/staytoonedtoon.o \
	lingo/xtras/timextra.o \
	lingo/xtras/xsound.o


ifdef USE_IMGUI
MODULE_OBJS += \
	debugger/debugtools.o \
	debugger/dt-cast.o \
	debugger/dt-controlpanel.o \
	debugger/dt-lists.o \
	debugger/dt-save-state.o \
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
