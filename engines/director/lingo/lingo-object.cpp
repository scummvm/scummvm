/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/endian.h"

#include "graphics/macgui/mactext.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/lingo/lingo-ast.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-the.h"

#include "director/lingo/xlibs/a/aiff.h"
#include "director/lingo/xlibs/a/applecdxobj.h"
#include "director/lingo/xlibs/a/askuser.h"
#include "director/lingo/xlibs/b/backdrop.h"
#include "director/lingo/xlibs/b/barakeobj.h"
#include "director/lingo/xlibs/b/batqt.h"
#include "director/lingo/xlibs/b/bimxobj.h"
#include "director/lingo/xlibs/b/blitpict.h"
#include "director/lingo/xlibs/b/blockthedrawingxobj.h"
#include "director/lingo/xlibs/c/cdromxobj.h"
#include "director/lingo/xlibs/c/closebleedwindowxcmd.h"
#include "director/lingo/xlibs/c/colorxobj.h"
#include "director/lingo/xlibs/c/colorcursorxobj.h"
#include "director/lingo/xlibs/c/consumer.h"
#include "director/lingo/xlibs/c/cursorxobj.h"
#include "director/lingo/xlibs/d/darkenscreen.h"
#include "director/lingo/xlibs/d/dateutil.h"
#include "director/lingo/xlibs/d/developerStack.h"
#include "director/lingo/xlibs/d/dialogsxobj.h"
#include "director/lingo/xlibs/d/dirutil.h"
#include "director/lingo/xlibs/d/dllglue.h"
#include "director/lingo/xlibs/d/dpwavi.h"
#include "director/lingo/xlibs/d/dpwqtw.h"
#include "director/lingo/xlibs/d/draw.h"
#include "director/lingo/xlibs/e/ednox.h"
#include "director/lingo/xlibs/e/eventq.h"
#include "director/lingo/xlibs/f/fadegammadownxcmd.h"
#include "director/lingo/xlibs/f/fadegammaupxcmd.h"
#include "director/lingo/xlibs/f/fadegammaxcmd.h"
#include "director/lingo/xlibs/f/fedracul.h"
#include "director/lingo/xlibs/f/feimasks.h"
#include "director/lingo/xlibs/f/feiprefs.h"
#include "director/lingo/xlibs/f/fileexists.h"
#include "director/lingo/xlibs/f/fileio.h"
#include "director/lingo/xlibs/f/findereventsxcmd.h"
#include "director/lingo/xlibs/f/findfolder.h"
#include "director/lingo/xlibs/f/findsys.h"
#include "director/lingo/xlibs/f/findwin.h"
#include "director/lingo/xlibs/f/flushxobj.h"
#include "director/lingo/xlibs/f/fplayxobj.h"
#include "director/lingo/xlibs/f/fsutil.h"
#include "director/lingo/xlibs/g/genutils.h"
#include "director/lingo/xlibs/g/getscreenrectsxfcn.h"
#include "director/lingo/xlibs/g/getscreensizexfcn.h"
#include "director/lingo/xlibs/g/getsoundinlevel.h"
#include "director/lingo/xlibs/g/gpid.h"
#include "director/lingo/xlibs/h/henry.h"
#include "director/lingo/xlibs/h/hitmap.h"
#include "director/lingo/xlibs/i/inixobj.h"
#include "director/lingo/xlibs/i/instobj.h"
#include "director/lingo/xlibs/j/jwxini.h"
#include "director/lingo/xlibs/i/iscd.h"
#include "director/lingo/xlibs/i/ispippin.h"
#include "director/lingo/xlibs/j/jitdraw3.h"
#include "director/lingo/xlibs/l/labeldrvxobj.h"
#include "director/lingo/xlibs/l/listdev.h"
#include "director/lingo/xlibs/m/maniacbg.h"
#include "director/lingo/xlibs/m/mapnavigatorxobj.h"
#include "director/lingo/xlibs/m/memcheckxobj.h"
#include "director/lingo/xlibs/m/memoryxobj.h"
#include "director/lingo/xlibs/m/misc.h"
#include "director/lingo/xlibs/m/miscx.h"
#include "director/lingo/xlibs/m/mmaskxobj.h"
#include "director/lingo/xlibs/m/mmovie.h"
#include "director/lingo/xlibs/m/moovxobj.h"
#include "director/lingo/xlibs/m/movemousejp.h"
#include "director/lingo/xlibs/m/movemousexobj.h"
#include "director/lingo/xlibs/m/movieidxxobj.h"
#include "director/lingo/xlibs/m/movutils.h"
#include "director/lingo/xlibs/m/msfile.h"
#include "director/lingo/xlibs/m/mystisle.h"
#include "director/lingo/xlibs/m/mazexobj.h"
#include "director/lingo/xlibs/o/openbleedwindowxcmd.h"
#include "director/lingo/xlibs/o/orthoplayxobj.h"
#include "director/lingo/xlibs/p/paco.h"
#include "director/lingo/xlibs/p/palxobj.h"
#include "director/lingo/xlibs/p/panel.h"
#include "director/lingo/xlibs/p/pharaohs.h"
#include "director/lingo/xlibs/p/popupmenuxobj.h"
#include "director/lingo/xlibs/p/porta.h"
#include "director/lingo/xlibs/p/prefpath.h"
#include "director/lingo/xlibs/p/printomatic.h"
#include "director/lingo/xlibs/p/processxobj.h"
#include "director/lingo/xlibs/p/putcurs.h"
#include "director/lingo/xlibs/p/playsoundmoviexobj.h"
#include "director/lingo/xlibs/q/qtmovie.h"
#include "director/lingo/xlibs/q/qtcatmovieplayerxobj.h"
#include "director/lingo/xlibs/q/qtvr.h"
#include "director/lingo/xlibs/q/quicktime.h"
#include "director/lingo/xlibs/r/registercomponent.h"
#include "director/lingo/xlibs/r/remixxcmd.h"
#include "director/lingo/xlibs/s/serialportxobj.h"
#include "director/lingo/xlibs/s/smallutil.h"
#include "director/lingo/xlibs/s/soundjam.h"
#include "director/lingo/xlibs/s/spacemgr.h"
#include "director/lingo/xlibs/s/stagetc.h"
#include "director/lingo/xlibs/s/syscolor.h"
#include "director/lingo/xlibs/s/savenrestorexobj.h"
#include "director/lingo/xlibs/t/tengu.h"
#include "director/lingo/xlibs/t/temnotaxobj.h"
#include "director/lingo/xlibs/u/unittest.h"
#include "director/lingo/xlibs/v/valkyrie.h"
#include "director/lingo/xlibs/v/versions.h"
#include "director/lingo/xlibs/v/videodiscxobj.h"
#include "director/lingo/xlibs/v/vmisonxfcn.h"
#include "director/lingo/xlibs/v/vmpresent.h"
#include "director/lingo/xlibs/v/volumelist.h"
#include "director/lingo/xlibs/v/voyagerxsound.h"
#include "director/lingo/xlibs/w/widgetxobj.h"
#include "director/lingo/xlibs/w/window.h"
#include "director/lingo/xlibs/w/wininfo.h"
#include "director/lingo/xlibs/w/winxobj.h"
#include "director/lingo/xlibs/x/xcmdglue.h"
#include "director/lingo/xlibs/x/xio.h"
#include "director/lingo/xlibs/x/xplayanim.h"
#include "director/lingo/xlibs/x/xplaypacoxfcn.h"
#include "director/lingo/xlibs/x/xsoundxfcn.h"
#include "director/lingo/xlibs/x/xwin.h"
#include "director/lingo/xlibs/y/yasix.h"
#include "director/lingo/xtras/a/audio.h"
#include "director/lingo/xtras/b/budapi.h"
#include "director/lingo/xtras/directsound.h"
#include "director/lingo/xtras/d/displayres.h"
#include "director/lingo/xtras/filextra.h"
#include "director/lingo/xtras/keypoll.h"
#include "director/lingo/xtras/masterapp.h"
#include "director/lingo/xtras/m/mui.h"
#include "director/lingo/xtras/m/mui.h"
#include "director/lingo/xtras/openurl.h"
#include "director/lingo/xtras/oscheck.h"
#include "director/lingo/xtras/qtvrxtra.h"
#include "director/lingo/xtras/r/registryreader.h"
#include "director/lingo/xtras/rtk.h"
#include "director/lingo/xtras/scrnutil.h"
#include "director/lingo/xtras/s/smacker.h"
#include "director/lingo/xtras/s/staytoonedhall.h"
#include "director/lingo/xtras/s/staytoonedball.h"
#include "director/lingo/xtras/s/staytoonedglop.h"
#include "director/lingo/xtras/s/staytoonedhigh.h"
#include "director/lingo/xtras/s/staytoonedober.h"
#include "director/lingo/xtras/s/staytoonedtoon.h"
#include "director/lingo/xtras/timextra.h"
#include "director/lingo/xtras/xsound.h"

namespace Director {

static const struct PredefinedProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	int type;
	int version;
} predefinedMethods[] = {
	// all except window
	{ "new",					LM::m_new,					-1, 0,	kAllObj,				200 },	// D2

	// factory and XObject
	{ "describe",				LM::m_describe,				 0, 0,	kXObj,					200 },	// D2
	{ "dispose",				LM::m_dispose,				 0, 0,	kFactoryObj | kXObj,	200 },	// D2
	{ "get",					LM::m_get,					 1, 1,	kFactoryObj,			200 },	// D2
	{ "instanceRespondsTo",		LM::m_instanceRespondsTo,	 1, 1,	kXObj,					300 },	// D3
	{ "messageList",			LM::m_messageList,			 0, 0,	kXObj,					300 },	// D3
	{ "name",					LM::m_name,					 0, 0,	kXObj,					300 },	// D3
	{ "perform",				LM::m_perform,				-1, 0,	kFactoryObj | kXObj,	300 },	// D3
	{ "put",					LM::m_put,					 2, 2,	kFactoryObj,			200 },	// D2
	{ "respondsTo",				LM::m_respondsTo,			 1, 1,	kXObj,					200 },	// D2

	// script object and Xtra
	{ "birth",					LM::m_new,					-1, 0,	kScriptObj | kXtraObj,	400 },	// D4

	{ nullptr, nullptr, 0, 0, 0, 0 }
};

static const MethodProto windowMethods[] = {
	// window / stage
	{ "close",					LM::m_close,				 0, 0,	400 },			// D4
	{ "forget",					LM::m_forget,				 0, 0,	400 },			// D4
	{ "open",					LM::m_open,					 0, 0,	400 },			// D4
	{ "moveToBack",				LM::m_moveToBack,			 0, 0,	400 },			// D4
	{ "moveToFront",			LM::m_moveToFront,			 0, 0,	400 },			// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void Lingo::initMethods() {
	for (const PredefinedProto *mtd = predefinedMethods; mtd->name; mtd++) {
		if (mtd->version > _vm->getVersion())
			continue;

		Symbol sym;
		sym.name = new Common::String(mtd->name);
		sym.type = HBLTIN;
		sym.nargs = mtd->minArgs;
		sym.maxArgs = mtd->maxArgs;
		sym.targetType = mtd->type;
		sym.u.bltin = mtd->func;
		_methods[mtd->name] = sym;
	}
	Window::initMethods(windowMethods);
}

void Lingo::cleanupMethods() {
	_methods.clear();
	Window::cleanupMethods();
}

#define XLIBDEF(class, flags, version) \
	{ #class, class::fileNames, class::open, class::close, flags, version }

static const struct XLibProto {
	const char *className;
	const XlibFileDesc *names;
	XLibOpenerFunc opener;
	XLibCloserFunc closer;
	int type;
	int version;
} xlibs[] = {
	XLIBDEF(AiffXObj,			kXObj,			400),	// D4
	XLIBDEF(AppleCDXObj,		kXObj,			300),	// D3
	XLIBDEF(AskUser,			kXObj,			400),	// D4
	XLIBDEF(AudioXtra,			kXtraObj,					500),	// D5
	XLIBDEF(BackdropXObj,		kXObj,			400),	// D4
	XLIBDEF(BarakeObj,			kXObj,			400),	// D4
	XLIBDEF(BatQT,				kXObj,			400),	// D4
	XLIBDEF(BIMXObj,			kXObj,			400),	// D4
	XLIBDEF(BlitPictXObj,		kXObj,			400),	// D4
	XLIBDEF(BlockTheDrawingXObj,			kXObj,					400),	// D4
	XLIBDEF(BudAPIXtra,			kXtraObj,					500),	// D5
	XLIBDEF(CDROMXObj,			kXObj,			200),	// D2
	XLIBDEF(CloseBleedWindowXCMD,kXObj,			300),	// D3
	XLIBDEF(ColorXObj,			kXObj,			400),	// D4
	XLIBDEF(ColorCursorXObj,	kXObj,			400),	// D4
	XLIBDEF(ConsumerXObj,		kXObj,			400),	// D4
	XLIBDEF(CursorXObj,			kXObj,			400),	// D4
	XLIBDEF(DLLGlueXObj,		kXObj,			400),	// D4
	XLIBDEF(DPWAVIXObj,			kXObj,			300),	// D3
	XLIBDEF(DPWQTWXObj,			kXObj,			300),	// D3
	XLIBDEF(DarkenScreen,		kXObj,			300),	// D3
	XLIBDEF(DateUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(DeveloperStack,		kXObj,			300),	// D3
	XLIBDEF(DialogsXObj,		kXObj,			400),	// D4
	XLIBDEF(DirUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(DirectsoundXtra,	kXtraObj,		500),	// D5
	XLIBDEF(DisplayResXtra,			kXtraObj,					500),	// D5
	XLIBDEF(DrawXObj,			kXObj,			400),	// D4
	XLIBDEF(Ednox,				kXObj,			300),	// D3
	XLIBDEF(EventQXObj,			kXObj,			400),	// D4
	XLIBDEF(FEDraculXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIMasksXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIPrefsXObj,		kXObj,			400),	// D4
	XLIBDEF(FSUtilXObj,			kXObj,			400),	// D4
	XLIBDEF(FadeGammaDownXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaUpXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaXCMD,		kXObj,			400),	// D4
	XLIBDEF(FileExists,			kXObj,			300),	// D3
	XLIBDEF(FileIO,				kXObj | kXtraObj,200),	// D2
	XLIBDEF(FileXtra,			kXtraObj,		500),	// D5
	XLIBDEF(FindFolder,			kXObj,			300),	// D3
	XLIBDEF(FindSys,			kXObj,			400),	// D4
	XLIBDEF(FindWin,			kXObj,			400),	// D4
	XLIBDEF(FinderEventsXCMD,	kXObj,			400),	// D4
	XLIBDEF(FlushXObj,			kXObj,			300),	// D3
	XLIBDEF(FPlayXObj,			kXObj,			200),	// D2
	XLIBDEF(GenUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(GetScreenRectsXFCN,	kXObj,			300),	// D3
	XLIBDEF(GetScreenSizeXFCN,	kXObj,			300),	// D3
	XLIBDEF(GetSoundInLevelXObj,kXObj,			400),	// D4
	XLIBDEF(GpidXObj,			kXObj,			400),	// D4
	XLIBDEF(HenryXObj,			kXObj,			400),	// D4
	XLIBDEF(HitMap,				kXObj,			400),	// D4
	XLIBDEF(IniXObj,			kXObj,			400),	// D4
	XLIBDEF(InstObjXObj,		kXObj,			400),	// D4
	XLIBDEF(IsCD,				kXObj,			300),	// D3
	XLIBDEF(IsPippin,			kXObj,			400),	// D4
	XLIBDEF(JITDraw3XObj,		kXObj,			400),	// D4
	XLIBDEF(JourneyWareXINIXObj,kXObj,			400),	// D4
	XLIBDEF(KeypollXtra,		kXtraObj,		500),	// D5
	XLIBDEF(LabelDrvXObj,		kXObj,			400),	// D4
	XLIBDEF(ListDevXObj,		kXObj,			500),	// D5
	XLIBDEF(MMovieXObj,			kXObj,			400),	// D4
	XLIBDEF(ManiacBgXObj,		kXObj,			300),	// D3
	XLIBDEF(MapNavigatorXObj,	kXObj,			400),	// D4
	XLIBDEF(MasterAppXtra,		kXtraObj,		500),	// D5
	XLIBDEF(MazeXObj,			kXObj,					400),	// D4
	XLIBDEF(MemCheckXObj,		kXObj,			400),	// D4
	XLIBDEF(MemoryXObj,			kXObj,			300),	// D3
	XLIBDEF(Misc,				kXObj,			400),	// D4
	XLIBDEF(MiscX,				kXObj,			400),	// D4
	XLIBDEF(MMaskXObj,			kXObj,			400),	// D4
	XLIBDEF(MoovXObj,			kXObj,			300),	// D3
	XLIBDEF(MoveMouseJPXObj,	kXObj,			400),	// D4
	XLIBDEF(MoveMouseXObj,		kXObj,			400),	// D4
	XLIBDEF(MovieIdxXObj,		kXObj,			400),	// D4
	XLIBDEF(MovUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(MSFile,             kXObj,          400),   // D4
	XLIBDEF(MuiXtra,			kXtraObj,					500),	// D5
	XLIBDEF(MystIsleXObj,		kXObj,			400),	// D4
	XLIBDEF(OSCheckXtra,		kXtraObj,		400),	// D4
	XLIBDEF(OpenBleedWindowXCMD,kXObj,			300),	// D3
	XLIBDEF(OpenURLXtra,		kXtraObj,		500),	// D5
	XLIBDEF(OrthoPlayXObj,		kXObj,			400),	// D4
	XLIBDEF(PACoXObj,			kXObj,			300),	// D3
	XLIBDEF(PalXObj,			kXObj,			400),	// D4
	XLIBDEF(PanelXObj,			kXObj,			200),	// D2
	XLIBDEF(PharaohsXObj,		kXObj,			400),	// D4
	XLIBDEF(PlaySoundMovieXObj,			kXObj,					400),	// D4
	XLIBDEF(PopUpMenuXObj,		kXObj,			200),	// D2
	XLIBDEF(Porta,				kXObj,			300),	// D3
	XLIBDEF(PrefPath,			kXObj,			400),	// D4
	XLIBDEF(PrintOMaticXObj,	kXObj | kXtraObj,400),	// D4
	XLIBDEF(ProcessXObj,		kXObj,			400),	// D4
	XLIBDEF(PutcursXObj,		kXObj,			400),	// D4
	XLIBDEF(QTCatMoviePlayerXObj,kXObj,			400),	// D4
	XLIBDEF(QTMovie,			kXObj,			400),	// D4
	XLIBDEF(QTVR,				kXObj,			400),	// D4
	XLIBDEF(QtvrxtraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(Quicktime,			kXObj,			300),	// D3
	XLIBDEF(RearWindowXObj,		kXObj,			400),	// D4
	XLIBDEF(RegisterComponent,	kXObj,			400),	// D4
	XLIBDEF(RegistryReaderXtra,			kXtraObj,					500),	// D5
	XLIBDEF(RemixXCMD,			kXObj,			300),	// D3
	XLIBDEF(RolloverToolkitXtra,kXtraObj,		500),	// D5
	XLIBDEF(SaveNRestoreXObj,			kXObj,					400),	// D4
	XLIBDEF(ScrnUtilXtra,		kXtraObj,		500),	// D5
	XLIBDEF(SerialPortXObj,		kXObj,			200),	// D2
	XLIBDEF(SmackerXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SmallUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(SoundJam,			kXObj,			400),	// D4
	XLIBDEF(SpaceMgr,			kXObj,			400),	// D4
	XLIBDEF(StageTCXObj,		kXObj,			400),	// D4
	XLIBDEF(StayToonedBallXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedGlopXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedHallXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedHighXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedOberXtra,			kXtraObj,					500),	// D5
	XLIBDEF(StayToonedToonXtra,			kXtraObj,					500),	// D5
	XLIBDEF(SysColorXObj,		kXObj,			400),	// D4
	XLIBDEF(TemnotaXObj,			kXObj,					400),	// D4
	XLIBDEF(TenguXObj,			kXObj,			400),	// D4
	XLIBDEF(TimextraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(UnitTestXObj,		kXObj,			400),	// D4
	XLIBDEF(VMPresentXObj,		kXObj,			400),	// D4
	XLIBDEF(VMisOnXFCN,			kXObj,			400),	// D4
	XLIBDEF(ValkyrieXObj,		kXObj,			400),	// D4
	XLIBDEF(VersionsXObj,		kXObj,			400),	// D4
	XLIBDEF(VideodiscXObj,		kXObj,			200),	// D2
	XLIBDEF(VolumeList,			kXObj,			300),	// D3
	XLIBDEF(VoyagerXSoundXObj,	kXObj,			400),	// D4
	XLIBDEF(WinInfoXObj,		kXObj,			400),	// D4
	XLIBDEF(WidgetXObj, 		kXObj,			400),	// D4
	XLIBDEF(WindowXObj,			kXObj,			200),	// D2
	XLIBDEF(XCMDGlueXObj,		kXObj,			200),	// D2
	XLIBDEF(XPlayPACoXFCN,		kXObj,			300),	// D3
	XLIBDEF(XSoundXFCN,			kXObj,			400),	// D4
	XLIBDEF(XWINXObj,			kXObj,			300),	// D3
	XLIBDEF(XioXObj,			kXObj,			400),	// D3
	XLIBDEF(XPlayAnim,			kXObj,			300),	// D3
	XLIBDEF(XsoundXtra,			kXtraObj,		500),	// D5
	XLIBDEF(Yasix,				kXObj,			300),	// D3
	{ nullptr, nullptr, nullptr, nullptr, 0, 0 }
};

void Lingo::initXLibs() {
	Common::HashMap<Common::String, uint32, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> quirks;
	for (const XLibProto *lib = xlibs; lib->names; lib++) {
		if (lib->version > _vm->getVersion())
			continue;

		for (uint i = 0; lib->names[i].name; i++) {
			bool isQuirk = false;
			if (lib->names[i].gameId) {
				isQuirk = strcmp(lib->names[i].gameId, g_director->getGameId()) == 0;
				// If this entry belongs to a specific game, skip it unless matched
				if (!isQuirk)
					continue;
			}

			if (isQuirk) {
				quirks[lib->names[i].name] = i;
			} else if (quirks.contains(lib->names[i].name)) {
				// Ignore new entries that conflict with per-game quirks
				continue;
			}

			if (!isQuirk && _xlibOpeners.contains(lib->names[i].name))
				warning("Lingo::initXLibs(): Duplicate entry for %s", lib->names[i].name);

			debugC(5, kDebugLingoExec, "Lingo::initXLibs(): %s -> %s", lib->names[i].name, lib->className);

			_xlibOpeners[lib->names[i].name] = lib->opener;
			_xlibClosers[lib->names[i].name] = lib->closer;
			_xlibTypes[lib->names[i].name] = lib->type;
		}
	}
}

void Lingo::cleanupXLibs() {
	_xlibOpeners.clear();
	_xlibClosers.clear();
}

Common::String Lingo::normalizeXLibName(Common::String name) {
	// Normalize to remove machintosh path delimiters (':', '@:')
	name = convertPath(name);

	size_t pos = name.findLastOf(g_director->_dirSeparator);
	if (pos != Common::String::npos)
		name = name.substr(pos + 1, name.size());

	Common::Platform platform = _vm->getPlatform();
	if (platform == Common::kPlatformMacintosh || platform == Common::kPlatformMacintoshII) {
		if (name.hasSuffixIgnoreCase(".xlib"))
			name = name.substr(0, name.size() - 5);
	} else if (platform == Common::kPlatformWindows) {
		if (name.hasSuffixIgnoreCase(".dll"))
			name = name.substr(0, name.size() - 4);
		if (name.hasSuffixIgnoreCase(".x16"))
			name = name.substr(0, name.size() - 4);
		if (name.hasSuffixIgnoreCase(".x32"))
			name = name.substr(0, name.size() - 4);
	}

	name.trim();

	return name;
}

void Lingo::openXLib(Common::String name, ObjectType type, const Common::Path &path) {
	name = normalizeXLibName(name);

	if (_openXLibs.contains(name))
		return;

	if (type == 0 && _xlibTypes.contains(name)) {
		type = (_xlibTypes[name] & kXtraObj) ? kXtraObj : kXObj;
	}

	// manual override for game quirks
	if (name.equalsIgnoreCase("fileio")) {
		if (g_director->_fileIOType == kXtraObj && g_director->getVersion() >= 500) {
			type = kXtraObj;
		} else if (g_director->_fileIOType == kXObj) {
			type = kXObj;
		}
	}

	_openXLibs[name] = type;

	if (_xlibOpeners.contains(name)) {
		(*_xlibOpeners[name])(type, path);
	} else {
		warning("Lingo::openXLib: Unimplemented xlib: '%s'", name.c_str());
	}
}

void Lingo::closeXLib(Common::String name) {
	name = normalizeXLibName(name);

	if (!_openXLibs.contains(name)) {
		warning("Lingo::closeXLib: xlib %s is not open", name.c_str());
		return;
	}

	ObjectType type = _openXLibs[name];
	_openXLibs.erase(name);

	if (_xlibClosers.contains(name)) {
		(*_xlibClosers[name])(type);
	} else {
		warning("Lingo::closeXLib: Unimplemented xlib: '%s'", name.c_str());
	}
}

void Lingo::closeOpenXLibs() {
	for (auto &it : _openXLibs) {
		// does not affect Xtras
		if (it._value == kXObj) {
			closeXLib(it._key);
		}
	}
}

void Lingo::reloadOpenXLibs() {
	OpenXLibsHash openXLibsCopy = _openXLibs;
	for (auto &it : openXLibsCopy) {
		closeXLib(it._key);
		// FIXME: keep track of where the xlib path is
		openXLib(it._key, it._value, Common::Path());
	}
}

// Initialization/disposal

void LM::m_new(int nargs) {
	// This is usually overridden by a user-defined mNew
	//
	// However, in behaviors it is often absent, and it is
	// in essence our default constructor.
	g_lingo->push(g_lingo->_state->me);
}

void LM::m_dispose(int nargs) {
	g_lingo->_state->me.u.obj->dispose();
}

/* ScriptContext */

ScriptContext::ScriptContext(Common::String name, ScriptType type, int id, uint16 castLibHint, uint16 parentNumber, int scriptId)
	: Object<ScriptContext>(name), _scriptType(type), _id(id), _castLibHint(castLibHint), _parentNumber(parentNumber), _scriptId(scriptId) {
	_objType = kScriptObj;
}

ScriptContext::ScriptContext(const ScriptContext &sc) : Object<ScriptContext>(sc) {
	_scriptType = sc._scriptType;
	_functionNames = sc._functionNames;
	for (auto &it : sc._functionHandlers) {
		_functionHandlers[it._key] = it._value;
		_functionHandlers[it._key].ctx = this;
	}
	for (auto &it : sc._eventHandlers) {
		_eventHandlers[it._key] = it._value;
		_eventHandlers[it._key].ctx = this;
	}
	_constants = sc._constants;
	_properties = sc._properties;
	_propertyNames = sc._propertyNames;
	_parentNumber = sc._parentNumber;
	_scriptId = sc._scriptId;

	_id = sc._id;
	_castLibHint = sc._castLibHint;
}

ScriptContext::~ScriptContext() {
}

Common::String ScriptContext::asString() {
	return Common::String::format("script: %d \"%s\" %d %p", _id, _name.c_str(), _inheritanceLevel, (void *)this);
}

Symbol ScriptContext::define(const Common::String &name, ScriptData *code, Common::Array<Common::String> *argNames, Common::Array<Common::String> *varNames) {
	Symbol sym;
	sym.name = new Common::String(name);
	sym.type = HANDLER;
	sym.u.defn = code;
	sym.nargs = argNames->size();
	sym.maxArgs = argNames->size();
	sym.argNames = argNames;
	sym.varNames = varNames;
	sym.ctx = this;

	if (debugChannelSet(1, kDebugCompile)) {
		debugC(1, kDebugCompile, "%s", g_lingo->formatFunctionBody(sym).c_str());
		debugC(1, kDebugCompile, "<end define code>");
	}

	_functionHandlers[name] = sym;
	if (g_lingo->_eventHandlerTypeIds.contains(name)) {
		_eventHandlers[g_lingo->_eventHandlerTypeIds[name]] = sym;
	}

	return sym;
}

Symbol ScriptContext::getMethod(const Common::String &methodName) {
	Symbol sym;

	if (_functionHandlers.contains(methodName)) {
		sym = _functionHandlers[methodName];
		sym.target = this;
		return sym;
	}

	sym = Object<ScriptContext>::getMethod(methodName);
	if (sym.type != VOIDSYM)
		return sym;

	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			// ancestor method
			sym = _properties["ancestor"].u.obj->getMethod(methodName);
			if (sym.type != VOIDSYM)
				debugC(3, kDebugLingoExec, "Calling method '%s' on ancestor: <%s>", methodName.c_str(), _properties["ancestor"].asString(true).c_str());
		}
	}

	return sym;
}

bool ScriptContext::hasProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return true;
	}
	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			return _properties["ancestor"].u.obj->hasProp(propName);
		}

		// This is used by behaviors
		if (propName.equalsIgnoreCase("spriteNum")) {
			return true;
		}
	}

	return false;
}

Datum ScriptContext::getProp(const Common::String &propName) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		return _properties[propName];
	}
	if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->getProp(propName);
		}

		// This is used by behaviors
		if (propName.equalsIgnoreCase("spriteNum")) {
			return Datum((int)g_director->getCurrentMovie()->_currentSpriteNum);
		}
	}
	_propertyNames.push_back(propName);
	return _properties[propName]; // return new property
}

Common::String ScriptContext::getPropAt(uint32 index) {
	uint32 target = 1;
	for (auto &it : _propertyNames) {
		if (target == index) {
			return it;
		}
		target += 1;
	}
	return Common::String();
}

uint32 ScriptContext::getPropCount() {
	return _propertyNames.size();
}

void ScriptContext::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		_properties[propName] = value;
		return;
	}
	if (force) {
		// used by e.g. the script compiler to add properties
		_propertyNames.push_back(propName);
		_properties[propName] = value;
	} else if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			_properties["ancestor"].u.obj->setProp(propName, value, force);
		}
	} else if (_objType == kFactoryObj) {
		// D3 style anonymous objects/factories, set whatever properties you like
		_propertyNames.push_back(propName);
		_properties[propName] = value;
	}
}

Common::String ScriptContext::formatFunctionList(const char *prefix) {
	Common::String result;
	for (auto it = _functionHandlers.begin(); it != _functionHandlers.end(); ++it) {
		result += Common::String::format("%s%s\n", prefix, g_lingo->formatFunctionName(it->_value).c_str());
	}
	return result;
}


// Object array

void LM::m_get(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_state->me.u.obj);
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	if (me->_objArray.contains(index)) {
		g_lingo->push(me->_objArray[index]);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_put(int nargs) {
	ScriptContext *me = static_cast<ScriptContext *>(g_lingo->_state->me.u.obj);
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	uint index = MAX(0, indexD.asInt());
	me->_objArray[index] = value;
}

// Other

void LM::m_perform(int nargs) {
	bool allowRetVal = g_lingo->pop().asInt() != 0; // Pop allowRetVal that should be used for the LC::Call

	// Lingo doesn't seem to bother cloning the object when
	// mNew is called with mPerform
	Datum d(g_lingo->_state->me);
	AbstractObject *me = d.u.obj;
	Datum methodName = g_lingo->_state->stack.remove_at(g_lingo->_state->stack.size() - nargs); // Take method name out of stack
	Symbol funcSym = me->getMethod(*methodName.u.s);
	// Object methods expect the first argument to be the object
	g_lingo->_state->stack.insert_at(g_lingo->_state->stack.size() - nargs + 1, d);
	LC::call(funcSym, nargs, allowRetVal);

	if (allowRetVal) {
		// If the method expects a return value, push dummy on stack
		g_lingo->pushVoid();
	}
}

// XObject

void LM::m_describe(int nargs) {
	warning("STUB: m_describe");
}

void LM::m_instanceRespondsTo(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

void LM::m_messageList(int nargs) {
	warning("STUB: m_messageList");
	g_lingo->push(Datum(""));
}

void LM::m_name(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	g_lingo->push(me->getName());
}

void LM::m_respondsTo(int nargs) {
	AbstractObject *me = g_lingo->_state->me.u.obj;
	Datum d = g_lingo->pop();
	Common::String methodName = d.asString();

	// TODO: Check inheritance level
	if (me->getMethod(methodName).type != VOIDSYM) {
		g_lingo->push(Datum(1));
	} else {
		g_lingo->push(Datum(0));
	}
}

// Window

Common::String Window::asString() {
	return "window \"" + getName() + "\"";
}

bool Window::hasProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	return g_lingo->_theEntityFields.contains(fieldName) && hasField(g_lingo->_theEntityFields[fieldName]->field);
}

Datum Window::getProp(const Common::String &propName) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return getField(g_lingo->_theEntityFields[fieldName]->field);
	}

	warning("Window::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Window::setProp(const Common::String &propName, const Datum &value, bool force) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		setField(g_lingo->_theEntityFields[fieldName]->field, value);
		return;
	}

	warning("Window::setProp: unknown property '%s'", propName.c_str());
}

bool Window::hasField(int field) {
	switch (field) {
	case kTheDrawRect:
	case kTheFileName:
	case kTheModal:
	case kTheRect:
	case kTheSourceRect:
	case kTheTitle:
	case kTheTitleVisible:
	case kTheVisible:
	case kTheWindowType:
		return true;
	default:
		break;
	}
	return false;
}

Datum Window::getField(int field) {
	switch (field) {
	case kTheTitle:
		return _window->getTitle();
	case kTheTitleVisible:
		return _window->isTitleVisible();
	case kTheVisible:
		return _window->isVisible();
	case kTheWindowType:
		return getWindowType();
	case kTheRect:
		return getStageRect();
	case kTheModal:
		return getModal();
	case kTheFileName:
		return getFileName();
	case kTheDrawRect:
		warning("Window::getField: poorly handled getting field 'drawRect'");
		ensureMovieIsLoaded();

		// TODO: This should allow stretching or panning
		return getStageRect();
	case kTheSourceRect:
	// case kTheImage:
	// case kThePicture::
		ensureMovieIsLoaded();  // Remove fallthrough once implemented
		// fallthrough
	default:
		warning("Window::getField: unhandled field '%s'", g_lingo->field2str(field));
		return Datum();
	}
}

void Window::setField(int field, const Datum &value) {
	switch (field) {
	case kTheTitle:
		setTitle(value.asString());
		break;
	case kTheTitleVisible:
		setTitleVisible((bool)value.asInt());
		break;
	case kTheVisible:
		setVisible((bool)value.asInt());
		break;
	case kTheWindowType:
		setWindowType(value.asInt());
		break;
	case kTheDrawRect:
		warning("Window::setField: poorly handled setting field 'drawRect'");
		// fallthrough
	case kTheRect:
		setStageRect(value);
		break;
	case kTheModal:
		setModal((bool)value.asInt());
		break;
	case kTheFileName:
		setFileName(value.asString());
		break;
	default:
		warning("Window::setField: unhandled field '%s'", g_lingo->field2str(field));
		break;
	}
}

void LM::m_close(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->setVisible(false);
}

void LM::m_forget(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	FArray *windowList = g_lingo->_windowList.u.farr;

	int windowIndex = -1;
	for (int i = 0; i < (int)windowList->arr.size(); i++) {
		if (windowList->arr[i].type != OBJECT || windowList->arr[i].u.obj->getObjType() != kWindowObj)
			continue;

		Window *window = static_cast<Window *>(windowList->arr[i].u.obj);
		if (window == me) {
			windowIndex = i;
			break;
		}
	}

	if (windowIndex == -1) {
		warning("m_forget: me object %s not found in window list", g_lingo->_state->me.asString().c_str());
		return;
	}

	if (windowIndex < (int)windowList->arr.size())
		windowList->arr.remove_at(windowIndex);

	// remove me from global vars
	for (auto &it : g_lingo->_globalvars) {
		if (it._value.type != OBJECT || it._value.u.obj->getObjType() != kWindowObj)
			continue;

		if (it._value.u.obj == me)
			g_lingo->_globalvars[it._key] = 0;
	}
	g_director->forgetWindow(me);
}

void LM::m_open(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	bool wasVisible = me->_window->isVisible();
	me->setVisible(true);

	if (!wasVisible)
		me->sendWindowEvent(kEventOpenWindow);
}

void LM::m_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LM::m_moveToFront(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->ensureMovieIsLoaded();

	bool wasActive = (g_director->_wm->getActiveWindow() == me->getId());
	g_director->_wm->setActiveWindow(me->getId());

	if (!wasActive)
		me->sendWindowEvent(kEventOpenWindow);
}

// Actor
/*
  collectChangeRects
  getAProp
  hitTest
  ilk
  mouseDown
  mouseHitTest
  mouseTrack
  mouseUp
  setAProp
  stepFrame
  updateRect
*/


} // End of namespace Director
