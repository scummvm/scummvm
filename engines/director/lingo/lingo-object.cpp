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

#include "director/lingo/xlibs/aiff.h"
#include "director/lingo/xlibs/applecdxobj.h"
#include "director/lingo/xlibs/askuser.h"
#include "director/lingo/xlibs/backdrop.h"
#include "director/lingo/xlibs/barakeobj.h"
#include "director/lingo/xlibs/batqt.h"
#include "director/lingo/xlibs/bimxobj.h"
#include "director/lingo/xlibs/blitpict.h"
#include "director/lingo/xlibs/cdromxobj.h"
#include "director/lingo/xlibs/closebleedwindowxcmd.h"
#include "director/lingo/xlibs/colorxobj.h"
#include "director/lingo/xlibs/colorcursorxobj.h"
#include "director/lingo/xlibs/consumer.h"
#include "director/lingo/xlibs/cursorxobj.h"
#include "director/lingo/xlibs/darkenscreen.h"
#include "director/lingo/xlibs/dateutil.h"
#include "director/lingo/xlibs/developerStack.h"
#include "director/lingo/xlibs/dialogsxobj.h"
#include "director/lingo/xlibs/dirutil.h"
#include "director/lingo/xlibs/dllglue.h"
#include "director/lingo/xlibs/dpwavi.h"
#include "director/lingo/xlibs/dpwqtw.h"
#include "director/lingo/xlibs/draw.h"
#include "director/lingo/xlibs/ednox.h"
#include "director/lingo/xlibs/eventq.h"
#include "director/lingo/xlibs/fadegammadownxcmd.h"
#include "director/lingo/xlibs/fadegammaupxcmd.h"
#include "director/lingo/xlibs/fadegammaxcmd.h"
#include "director/lingo/xlibs/fedracul.h"
#include "director/lingo/xlibs/feimasks.h"
#include "director/lingo/xlibs/feiprefs.h"
#include "director/lingo/xlibs/fileexists.h"
#include "director/lingo/xlibs/fileio.h"
#include "director/lingo/xlibs/findereventsxcmd.h"
#include "director/lingo/xlibs/findfolder.h"
#include "director/lingo/xlibs/findsys.h"
#include "director/lingo/xlibs/findwin.h"
#include "director/lingo/xlibs/flushxobj.h"
#include "director/lingo/xlibs/fplayxobj.h"
#include "director/lingo/xlibs/fsutil.h"
#include "director/lingo/xlibs/genutils.h"
#include "director/lingo/xlibs/getscreenrectsxfcn.h"
#include "director/lingo/xlibs/getscreensizexfcn.h"
#include "director/lingo/xlibs/gpid.h"
#include "director/lingo/xlibs/henry.h"
#include "director/lingo/xlibs/hitmap.h"
#include "director/lingo/xlibs/inixobj.h"
#include "director/lingo/xlibs/instobj.h"
#include "director/lingo/xlibs/jwxini.h"
#include "director/lingo/xlibs/iscd.h"
#include "director/lingo/xlibs/ispippin.h"
#include "director/lingo/xlibs/jitdraw3.h"
#include "director/lingo/xlibs/labeldrvxobj.h"
#include "director/lingo/xlibs/maniacbg.h"
#include "director/lingo/xlibs/mapnavigatorxobj.h"
#include "director/lingo/xlibs/memcheckxobj.h"
#include "director/lingo/xlibs/memoryxobj.h"
#include "director/lingo/xlibs/misc.h"
#include "director/lingo/xlibs/miscx.h"
#include "director/lingo/xlibs/mmaskxobj.h"
#include "director/lingo/xlibs/mmovie.h"
#include "director/lingo/xlibs/moovxobj.h"
#include "director/lingo/xlibs/movemousejp.h"
#include "director/lingo/xlibs/movemousexobj.h"
#include "director/lingo/xlibs/movieidxxobj.h"
#include "director/lingo/xlibs/movutils.h"
#include "director/lingo/xlibs/mystisle.h"
#include "director/lingo/xlibs/openbleedwindowxcmd.h"
#include "director/lingo/xlibs/orthoplayxobj.h"
#include "director/lingo/xlibs/paco.h"
#include "director/lingo/xlibs/palxobj.h"
#include "director/lingo/xlibs/panel.h"
#include "director/lingo/xlibs/popupmenuxobj.h"
#include "director/lingo/xlibs/porta.h"
#include "director/lingo/xlibs/prefpath.h"
#include "director/lingo/xlibs/printomatic.h"
#include "director/lingo/xlibs/processxobj.h"
#include "director/lingo/xlibs/qtmovie.h"
#include "director/lingo/xlibs/qtcatmovieplayerxobj.h"
#include "director/lingo/xlibs/qtvr.h"
#include "director/lingo/xlibs/quicktime.h"
#include "director/lingo/xlibs/registercomponent.h"
#include "director/lingo/xlibs/remixxcmd.h"
#include "director/lingo/xlibs/serialportxobj.h"
#include "director/lingo/xlibs/soundjam.h"
#include "director/lingo/xlibs/spacemgr.h"
#include "director/lingo/xlibs/stagetc.h"
#include "director/lingo/xlibs/syscolor.h"
#include "director/lingo/xlibs/unittest.h"
#include "director/lingo/xlibs/valkyrie.h"
#include "director/lingo/xlibs/videodiscxobj.h"
#include "director/lingo/xlibs/vmisonxfcn.h"
#include "director/lingo/xlibs/volumelist.h"
#include "director/lingo/xlibs/widgetxobj.h"
#include "director/lingo/xlibs/window.h"
#include "director/lingo/xlibs/wininfo.h"
#include "director/lingo/xlibs/winxobj.h"
#include "director/lingo/xlibs/xcmdglue.h"
#include "director/lingo/xlibs/xio.h"
#include "director/lingo/xlibs/xplayanim.h"
#include "director/lingo/xlibs/xsoundxfcn.h"
#include "director/lingo/xlibs/xwin.h"
#include "director/lingo/xlibs/yasix.h"
#include "director/lingo/xtras/directsound.h"
#include "director/lingo/xtras/keypoll.h"
#include "director/lingo/xtras/qtvrxtra.h"
#include "director/lingo/xtras/scrnutil.h"
#include "director/lingo/xtras/timextra.h"

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
	{ class::fileNames, class::open, class::close, flags, version }

static const struct XLibProto {
	const XlibFileDesc *names;
	XLibOpenerFunc opener;
	XLibCloserFunc closer;
	int type;
	int version;
} xlibs[] = {
	XLIBDEF(AiffXObj,			kXObj,			400),	// D4
	XLIBDEF(AppleCDXObj,		kXObj,			300),	// D3
	XLIBDEF(AskUser,			kXObj,			400),	// D4
	XLIBDEF(BackdropXObj,		kXObj,			400),	// D4
	XLIBDEF(BarakeObj,			kXObj,			400),	// D4
	XLIBDEF(BatQT,				kXObj,			400),	// D4
	XLIBDEF(BIMXObj,			kXObj,			400),	// D4
	XLIBDEF(BlitPictXObj,		kXObj,			400),	// D4
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
	XLIBDEF(DateUtilXObj,			kXObj,					400),	// D4
	XLIBDEF(DeveloperStack,		kXObj,			300),	// D3
	XLIBDEF(DialogsXObj,		kXObj,			400),	// D4
	XLIBDEF(DirUtilXObj,		kXObj,			400),	// D4
	XLIBDEF(DirectsoundXtra,	kXtraObj,		500),	// D5
	XLIBDEF(DrawXObj,			kXObj,			400),	// D4
	XLIBDEF(Ednox,				kXObj,			300),	// D3
	XLIBDEF(EventQXObj,			kXObj,			400),	// D4
	XLIBDEF(FEDraculXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIMasksXObj,		kXObj,			400),	// D4
	XLIBDEF(FEIPrefsXObj,		kXObj,			400),	// D4
	XLIBDEF(FSUtilXObj,			kXObj,					400),	// D4
	XLIBDEF(FadeGammaDownXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaUpXCMD,	kXObj,			400),	// D4
	XLIBDEF(FadeGammaXCMD,		kXObj,			400),	// D4
	XLIBDEF(FileExists,			kXObj,			300),	// D3
	XLIBDEF(FileIO,				kXObj | kXtraObj,200),	// D2
	XLIBDEF(FindFolder,			kXObj,			300),	// D3
	XLIBDEF(FindSys,			kXObj,			400),	// D4
	XLIBDEF(FindWin,			kXObj,			400),	// D4
	XLIBDEF(FinderEventsXCMD,	kXObj,			400),	// D4
	XLIBDEF(FlushXObj,			kXObj,			300),	// D3
	XLIBDEF(FPlayXObj,			kXObj,			200),	// D2
	XLIBDEF(GenUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(GetScreenRectsXFCN,	kXObj,			300),	// D3
	XLIBDEF(GetScreenSizeXFCN,	kXObj,			300),	// D3
	XLIBDEF(GpidXObj,			kXObj,			400),	// D4
	XLIBDEF(HenryXObj,			kXObj,					400),	// D4
	XLIBDEF(HitMap,				kXObj,			400),	// D4
	XLIBDEF(IniXObj,			kXObj,			400),	// D4
	XLIBDEF(InstObjXObj,		kXObj,			400),	// D4
	XLIBDEF(IsCD,				kXObj,			300),	// D3
	XLIBDEF(IsPippin,			kXObj,			400),	// D4
	XLIBDEF(JITDraw3XObj,		kXObj,			400),	// D4
	XLIBDEF(JourneyWareXINIXObj,kXObj,			400),	// D4
	XLIBDEF(KeypollXtra,		kXtraObj,		500),	// D5
	XLIBDEF(LabelDrvXObj,		kXObj,			400),	// D4
	XLIBDEF(MMovieXObj,			kXObj,			400),	// D4
	XLIBDEF(ManiacBgXObj,		kXObj,			300),	// D3
	XLIBDEF(MapNavigatorXObj,	kXObj,			400),	// D4
	XLIBDEF(MemCheckXObj,		kXObj,			400),	// D4
	XLIBDEF(MemoryXObj,			kXObj,			300),	// D3
	XLIBDEF(Misc,				kXObj,			400),	// D4
	XLIBDEF(MiscX,				kXObj,			400),	// D4
	XLIBDEF(MMaskXObj,			kXObj,			400),	// D4
	XLIBDEF(MoovXObj,			kXObj,			300),	// D3
	XLIBDEF(MoveMouseJPXObj,			kXObj,					400),	// D4
	XLIBDEF(MoveMouseXObj,		kXObj,			400),	// D4
	XLIBDEF(MovieIdxXObj,		kXObj,			400),	// D4
	XLIBDEF(MovUtilsXObj,		kXObj,			400),	// D4
	XLIBDEF(MystIsleXObj,			kXObj,					400),	// D4
	XLIBDEF(OpenBleedWindowXCMD,kXObj,			300),	// D3
	XLIBDEF(OrthoPlayXObj,		kXObj,			400),	// D4
	XLIBDEF(PACoXObj,			kXObj,			300),	// D3
	XLIBDEF(PalXObj,			kXObj,			400),	// D4
	XLIBDEF(PanelXObj,			kXObj,			200),	// D2
	XLIBDEF(PopUpMenuXObj,		kXObj,			200),	// D2
	XLIBDEF(Porta,				kXObj,			300),	// D3
	XLIBDEF(PrefPath,			kXObj,			400),	// D4
	XLIBDEF(PrintOMaticXObj,	kXObj,			400),	// D4
	XLIBDEF(ProcessXObj,		kXObj,			400),	// D4
	XLIBDEF(QTCatMoviePlayerXObj,kXObj,			400),	// D4
	XLIBDEF(QTMovie,			kXObj,			400),	// D4
	XLIBDEF(QTVR,				kXObj,			400),	// D4
	XLIBDEF(QtvrxtraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(Quicktime,			kXObj,			300),	// D3
	XLIBDEF(RearWindowXObj,		kXObj,			400),	// D4
	XLIBDEF(RegisterComponent,	kXObj,			400),	// D4
	XLIBDEF(RemixXCMD,			kXObj,			300),	// D3
	XLIBDEF(ScrnUtilXtra,		kXtraObj,		500),	// D5
	XLIBDEF(SerialPortXObj,		kXObj,			200),	// D2
	XLIBDEF(SoundJam,			kXObj,			400),	// D4
	XLIBDEF(SpaceMgr,			kXObj,			400),	// D4
	XLIBDEF(StageTCXObj,		kXObj,			400),	// D4
	XLIBDEF(SysColorXObj,			kXObj,					400),	// D4
	XLIBDEF(TimextraXtra,		kXtraObj,		500),	// D5
	XLIBDEF(UnitTestXObj,		kXObj,			400),	// D4
	XLIBDEF(VMisOnXFCN,			kXObj,			400),	// D4
	XLIBDEF(ValkyrieXObj,		kXObj,			400),	// D4
	XLIBDEF(VideodiscXObj,		kXObj,			200),	// D2
	XLIBDEF(VolumeList,			kXObj,			300),	// D3
	XLIBDEF(WinInfoXObj,		kXObj,			400),	// D4
	XLIBDEF(WidgetXObj, 		kXObj,			400),	// D4
	XLIBDEF(WindowXObj,			kXObj,			200),	// D2
	XLIBDEF(XCMDGlueXObj,		kXObj,			200),	// D2
	XLIBDEF(XSoundXFCN,			kXObj,			400),	// D4
	XLIBDEF(XWINXObj,			kXObj,			300),	// D3
	XLIBDEF(XioXObj,			kXObj,			400),	// D3
	XLIBDEF(XPlayAnim,			kXObj,			300),	// D3
	XLIBDEF(Yasix,				kXObj,			300),	// D3
	{ nullptr, nullptr, nullptr, 0, 0 }
};

void Lingo::initXLibs() {
	for (const XLibProto *lib = xlibs; lib->names; lib++) {
		if (lib->version > _vm->getVersion())
			continue;

		for (uint i = 0; lib->names[i].name; i++) {
			// If this entry belongs to a specific game, skip it unless matched
			if (lib->names[i].gameId && strcmp(lib->names[i].gameId, g_director->getGameId()))
				continue;

			if (_xlibOpeners.contains(lib->names[i].name))
				warning("Lingo::initXLibs(): Duplicate entry for %s", lib->names[i].name);

			_xlibOpeners[lib->names[i].name] = lib->opener;
			_xlibClosers[lib->names[i].name] = lib->closer;
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
		closeXLib(it._key);
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
	g_lingo->printSTUBWithArglist("m_new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void LM::m_dispose(int nargs) {
	g_lingo->_state->me.u.obj->dispose();
}

/* ScriptContext */

ScriptContext::ScriptContext(Common::String name, ScriptType type, int id)
	: Object<ScriptContext>(name), _scriptType(type), _id(id) {
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

	_id = sc._id;
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

bool ScriptContext::setProp(const Common::String &propName, const Datum &value, bool force) {
	if (_disposed) {
		error("Property '%s' accessed on disposed object <%s>", propName.c_str(), Datum(this).asString(true).c_str());
	}
	if (_properties.contains(propName)) {
		_properties[propName] = value;
		return true;
	}
	if (force) {
		// used by e.g. the script compiler to add properties
		_propertyNames.push_back(propName);
		_properties[propName] = value;
		return true;
	} else if (_objType == kScriptObj) {
		if (_properties.contains("ancestor") && _properties["ancestor"].type == OBJECT
				&& (_properties["ancestor"].u.obj->getObjType() & (kScriptObj | kXtraObj))) {
			debugC(3, kDebugLingoExec, "Getting prop '%s' from ancestor: <%s>", propName.c_str(), _properties["ancestor"].asString(true).c_str());
			return _properties["ancestor"].u.obj->setProp(propName, value, force);
		}
	} else if (_objType == kFactoryObj) {
		// D3 style anonymous objects/factories, set whatever properties you like
		_propertyNames.push_back(propName);
		_properties[propName] = value;
		return true;
	}
	return false;
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
	Datum methodName = g_lingo->_stack.remove_at(g_lingo->_stack.size() - nargs); // Take method name out of stack
	Symbol funcSym = me->getMethod(*methodName.u.s);
	// Object methods expect the first argument to be the object
	g_lingo->_stack.insert_at(g_lingo->_stack.size() - nargs + 1, d);
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

bool Window::setProp(const Common::String &propName, const Datum &value, bool force) {
	Common::String fieldName = Common::String::format("%d%s", kTheWindow, propName.c_str());
	if (g_lingo->_theEntityFields.contains(fieldName)) {
		return setField(g_lingo->_theEntityFields[fieldName]->field, value);
	}

	warning("Window::setProp: unknown property '%s'", propName.c_str());
	return false;
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
		return getTitle();
	case kTheTitleVisible:
		return isTitleVisible();
	case kTheVisible:
		return isVisible();
	case kTheWindowType:
		return getWindowType();
	case kTheRect:
		return getStageRect();
	case kTheModal:
		return getModal();
	case kTheFileName:
		return getFileName();
	case kTheDrawRect:
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

bool Window::setField(int field, const Datum &value) {
	switch (field) {
	case kTheTitle:
		setTitle(value.asString());
		return true;
	case kTheTitleVisible:
		setTitleVisible((bool)value.asInt());
		return true;
	case kTheVisible:
		setVisible((bool)value.asInt());
		return true;
	case kTheWindowType:
		setWindowType(value.asInt());
		return true;
	case kTheRect:
		return setStageRect(value);
	case kTheModal:
		setModal((bool)value.asInt());
		return true;
	case kTheFileName:
		setFileName(value.asString());
		return true;
	default:
		warning("Window::setField: unhandled field '%s'", g_lingo->field2str(field));
		return false;
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

}

void LM::m_open(int nargs) {
	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->setVisible(true);
}

void LM::m_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LM::m_moveToFront(int nargs) {
	g_lingo->printSTUBWithArglist("m_moveToFront", nargs);

	Window *me = static_cast<Window *>(g_lingo->_state->me.u.obj);
	me->ensureMovieIsLoaded();
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
