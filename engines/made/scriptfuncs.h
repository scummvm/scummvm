/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MADE_SCRIPTFUNCS_H
#define MADE_SCRIPTFUNCS_H

#include "common/util.h"
#include "common/file.h"
#include "common/func.h"
#include "common/stream.h"

namespace Made {

class MadeEngine;

typedef Common::Functor2<int16, int16*, int16> ExternalFunc;

class ScriptFunctions {
public:
	ScriptFunctions(MadeEngine *vm) : _vm(vm) {}
	virtual ~ScriptFunctions() {}
	int16 callFunction(uint16 index, int16 argc, int16 *argv)  {
		if (index >= _externalFuncs.size()) {
			// TODO: ERROR!
			return 0;
		}
		return (*_externalFuncs[index])(argc, argv);
	}
	virtual void setupExternalsTable() = 0;
protected:
	MadeEngine *_vm;
	Audio::SoundHandle _audioStreamHandle;
	Audio::SoundHandle _voiceStreamHandle;

	Common::Array<const ExternalFunc*> _externalFuncs;

};

class ScriptFunctionsLgop2 : public ScriptFunctions {
public:
	ScriptFunctionsLgop2(MadeEngine *vm) : ScriptFunctions(vm) {}
	~ScriptFunctionsLgop2() {}
	void setupExternalsTable();
protected:

	int16 o1_SYSTEM(int16 argc, int16 *argv);
	int16 o1_INITGRAF(int16 argc, int16 *argv);
	int16 o1_RESTOREGRAF(int16 argc, int16 *argv);
	int16 o1_DRAWPIC(int16 argc, int16 *argv);
	int16 o1_CLS(int16 argc, int16 *argv);
	int16 o1_SHOWPAGE(int16 argc, int16 *argv);
	int16 o1_EVENT(int16 argc, int16 *argv);
	int16 o1_EVENTX(int16 argc, int16 *argv);
	int16 o1_EVENTY(int16 argc, int16 *argv);
	int16 o1_EVENTKEY(int16 argc, int16 *argv);
	int16 o1_VISUALFX(int16 argc, int16 *argv);
	int16 o1_PLAYSND(int16 argc, int16 *argv);
	int16 o1_PLAYMUS(int16 argc, int16 *argv);
	int16 o1_STOPMUS(int16 argc, int16 *argv);
	int16 o1_ISMUS(int16 argc, int16 *argv);
	int16 o1_TEXTPOS(int16 argc, int16 *argv);
	int16 o1_FLASH(int16 argc, int16 *argv);
	int16 o1_PLAYNOTE(int16 argc, int16 *argv);
	int16 o1_STOPNOTE(int16 argc, int16 *argv);
	int16 o1_PLAYTELE(int16 argc, int16 *argv);
	int16 o1_STOPTELE(int16 argc, int16 *argv);
	int16 o1_HIDECURS(int16 argc, int16 *argv);
	int16 o1_SHOWCURS(int16 argc, int16 *argv);
	int16 o1_MUSICBEAT(int16 argc, int16 *argv);
	int16 o1_SCREENLOCK(int16 argc, int16 *argv);
	int16 o1_ADDSPRITE(int16 argc, int16 *argv);
	int16 o1_FREEANIM(int16 argc, int16 *argv);
	int16 o1_DRAWSPRITE(int16 argc, int16 *argv);
	int16 o1_ERASESPRITES(int16 argc, int16 *argv);
	int16 o1_UPDATESPRITES(int16 argc, int16 *argv);
	int16 o1_GETTIMER(int16 argc, int16 *argv);
	int16 o1_SETTIMER(int16 argc, int16 *argv);
	int16 o1_RESETTIMER(int16 argc, int16 *argv);
	int16 o1_ALLOCTIMER(int16 argc, int16 *argv);
	int16 o1_FREETIMER(int16 argc, int16 *argv);
	int16 o1_PALETTELOCK(int16 argc, int16 *argv);
	int16 o1_FONT(int16 argc, int16 *argv);
	int16 o1_DRAWTEXT(int16 argc, int16 *argv);
	int16 o1_HOMETEXT(int16 argc, int16 *argv);
	int16 o1_TEXTRECT(int16 argc, int16 *argv);
	int16 o1_TEXTXY(int16 argc, int16 *argv);
	int16 o1_DROPSHADOW(int16 argc, int16 *argv);
	int16 o1_TEXTCOLOR(int16 argc, int16 *argv);
	int16 o1_OUTLINE(int16 argc, int16 *argv);
	int16 o1_LOADCURSOR(int16 argc, int16 *argv);
	int16 o1_SETGROUND(int16 argc, int16 *argv);
	int16 o1_RESTEXT(int16 argc, int16 *argv);
	int16 o1_ADDMASK(int16 argc, int16 *argv);
	int16 o1_SETMASK(int16 argc, int16 *argv);
	int16 o1_ISSND(int16 argc, int16 *argv);
	int16 o1_STOPSND(int16 argc, int16 *argv);
	int16 o1_PLAYVOICE(int16 argc, int16 *argv);
};

class ScriptFunctionsRtz : public ScriptFunctions {
public:
	ScriptFunctionsRtz(MadeEngine *vm) : ScriptFunctions(vm) {}
	~ScriptFunctionsRtz() {}
	void setupExternalsTable();
protected:

	int16 o1_SYSTEM(int16 argc, int16 *argv);
	int16 o1_INITGRAF(int16 argc, int16 *argv);
	int16 o1_RESTOREGRAF(int16 argc, int16 *argv);
	int16 o1_DRAWPIC(int16 argc, int16 *argv);
	int16 o1_CLS(int16 argc, int16 *argv);
	int16 o1_SHOWPAGE(int16 argc, int16 *argv);
	int16 o1_EVENT(int16 argc, int16 *argv);
	int16 o1_EVENTX(int16 argc, int16 *argv);
	int16 o1_EVENTY(int16 argc, int16 *argv);
	int16 o1_EVENTKEY(int16 argc, int16 *argv);
	int16 o1_VISUALFX(int16 argc, int16 *argv);
	int16 o1_PLAYSND(int16 argc, int16 *argv);
	int16 o1_PLAYMUS(int16 argc, int16 *argv);
	int16 o1_STOPMUS(int16 argc, int16 *argv);
	int16 o1_ISMUS(int16 argc, int16 *argv);
	int16 o1_TEXTPOS(int16 argc, int16 *argv);
	int16 o1_FLASH(int16 argc, int16 *argv);
	int16 o1_PLAYNOTE(int16 argc, int16 *argv);
	int16 o1_STOPNOTE(int16 argc, int16 *argv);
	int16 o1_PLAYTELE(int16 argc, int16 *argv);
	int16 o1_STOPTELE(int16 argc, int16 *argv);
	int16 o1_HIDECURS(int16 argc, int16 *argv);
	int16 o1_SHOWCURS(int16 argc, int16 *argv);
	int16 o1_MUSICBEAT(int16 argc, int16 *argv);
	int16 o1_SCREENLOCK(int16 argc, int16 *argv);
	int16 o1_ADDSPRITE(int16 argc, int16 *argv);
	int16 o1_FREEANIM(int16 argc, int16 *argv);
	int16 o1_DRAWSPRITE(int16 argc, int16 *argv);
	int16 o1_ERASESPRITES(int16 argc, int16 *argv);
	int16 o1_UPDATESPRITES(int16 argc, int16 *argv);
	int16 o1_GETTIMER(int16 argc, int16 *argv);
	int16 o1_SETTIMER(int16 argc, int16 *argv);
	int16 o1_RESETTIMER(int16 argc, int16 *argv);
	int16 o1_ALLOCTIMER(int16 argc, int16 *argv);
	int16 o1_FREETIMER(int16 argc, int16 *argv);
	int16 o1_PALETTELOCK(int16 argc, int16 *argv);
	int16 o1_FONT(int16 argc, int16 *argv);
	int16 o1_DRAWTEXT(int16 argc, int16 *argv);
	int16 o1_HOMETEXT(int16 argc, int16 *argv);
	int16 o1_TEXTRECT(int16 argc, int16 *argv);
	int16 o1_TEXTXY(int16 argc, int16 *argv);
	int16 o1_DROPSHADOW(int16 argc, int16 *argv);
	int16 o1_TEXTCOLOR(int16 argc, int16 *argv);
	int16 o1_OUTLINE(int16 argc, int16 *argv);
	int16 o1_LOADCURSOR(int16 argc, int16 *argv);
	int16 o1_SETGROUND(int16 argc, int16 *argv);
	int16 o1_RESTEXT(int16 argc, int16 *argv);
	int16 o1_CLIPAREA(int16 argc, int16 *argv);
	int16 o1_SETCLIP(int16 argc, int16 *argv);
	int16 o1_ISSND(int16 argc, int16 *argv);
	int16 o1_STOPSND(int16 argc, int16 *argv);
	int16 o1_PLAYVOICE(int16 argc, int16 *argv);
	int16 o1_CDPLAY(int16 argc, int16 *argv);
	int16 o1_STOPCD(int16 argc, int16 *argv);
	int16 o1_CDSTATUS(int16 argc, int16 *argv);
	int16 o1_CDTIME(int16 argc, int16 *argv);
	int16 o1_CDPLAYSEG(int16 argc, int16 *argv);
	int16 o1_PRINTF(int16 argc, int16 *argv);
	int16 o1_MONOCLS(int16 argc, int16 *argv);
	int16 o1_SNDENERGY(int16 argc, int16 *argv);
	int16 o1_CLEARTEXT(int16 argc, int16 *argv);
	int16 o1_ANIMTEXT(int16 argc, int16 *argv);
	int16 o1_TEXTWIDTH(int16 argc, int16 *argv);
	int16 o1_PLAYMOVIE(int16 argc, int16 *argv);
	int16 o1_LOADSND(int16 argc, int16 *argv);
	int16 o1_LOADMUS(int16 argc, int16 *argv);
	int16 o1_LOADPIC(int16 argc, int16 *argv);
	int16 o1_MUSICVOL(int16 argc, int16 *argv);
	int16 o1_RESTARTEVENTS(int16 argc, int16 *argv);
	int16 o1_PLACESPRITE(int16 argc, int16 *argv);
	int16 o1_PLACETEXT(int16 argc, int16 *argv);
	int16 o1_DELETECHANNEL(int16 argc, int16 *argv);
	int16 o1_CHANNELTYPE(int16 argc, int16 *argv);
	int16 o1_SETSTATE(int16 argc, int16 *argv);
	int16 o1_SETLOCATION(int16 argc, int16 *argv);
	int16 o1_SETCONTENT(int16 argc, int16 *argv);
	int16 o1_EXCLUDEAREA(int16 argc, int16 *argv);
	int16 o1_SETEXCLUDE(int16 argc, int16 *argv);
	int16 o1_GETSTATE(int16 argc, int16 *argv);
	int16 o1_PLACEANIM(int16 argc, int16 *argv);
	int16 o1_SETFRAME(int16 argc, int16 *argv);
	int16 o1_GETFRAME(int16 argc, int16 *argv);
	int16 o1_GETFRAMECOUNT(int16 argc, int16 *argv);
	int16 o1_PICWIDTH(int16 argc, int16 *argv);
	int16 o1_PICHEIGHT(int16 argc, int16 *argv);
	int16 o1_SOUNDRATE(int16 argc, int16 *argv);
	int16 o1_DRAWANIMPIC(int16 argc, int16 *argv);
	int16 o1_LOADANIM(int16 argc, int16 *argv);
	int16 o1_READTEXT(int16 argc, int16 *argv);
	int16 o1_READMENU(int16 argc, int16 *argv);
	int16 o1_DRAWMENU(int16 argc, int16 *argv);
	int16 o1_MENUCOUNT(int16 argc, int16 *argv);
	int16 o1_SAVEGAME(int16 argc, int16 *argv);
	int16 o1_LOADGAME(int16 argc, int16 *argv);
	int16 o1_GAMENAME(int16 argc, int16 *argv);
	int16 o1_SHAKESCREEN(int16 argc, int16 *argv);
	int16 o1_PLACEMENU(int16 argc, int16 *argv);
	int16 o1_SETVOLUME(int16 argc, int16 *argv);
	int16 o1_WHATSYNTH(int16 argc, int16 *argv);
	int16 o1_SLOWSYSTEM(int16 argc, int16 *argv);
};

class ScriptFunctionsMhne : public ScriptFunctions {
public:
	ScriptFunctionsMhne(MadeEngine *vm) : ScriptFunctions(vm) {}
	~ScriptFunctionsMhne() {}
	void setupExternalsTable();
protected:

	int16 o1_SYSTEM(int16 argc, int16 *argv);
	int16 o1_INITGRAF(int16 argc, int16 *argv);
	int16 o1_RESTOREGRAF(int16 argc, int16 *argv);
	int16 o1_DRAWPIC(int16 argc, int16 *argv);
	int16 o1_CLS(int16 argc, int16 *argv);
	int16 o1_SHOWPAGE(int16 argc, int16 *argv);
	int16 o1_EVENT(int16 argc, int16 *argv);
	int16 o1_EVENTX(int16 argc, int16 *argv);
	int16 o1_EVENTY(int16 argc, int16 *argv);
	int16 o1_EVENTKEY(int16 argc, int16 *argv);
	int16 o1_VISUALFX(int16 argc, int16 *argv);
	int16 o1_PLAYSND(int16 argc, int16 *argv);
	int16 o1_PLAYMUS(int16 argc, int16 *argv);
	int16 o1_STOPMUS(int16 argc, int16 *argv);
	int16 o1_ISMUS(int16 argc, int16 *argv);
	int16 o1_TEXTPOS(int16 argc, int16 *argv);
	int16 o1_FLASH(int16 argc, int16 *argv);
	int16 o1_PLAYNOTE(int16 argc, int16 *argv);
	int16 o1_STOPNOTE(int16 argc, int16 *argv);
	int16 o1_PLAYTELE(int16 argc, int16 *argv);
	int16 o1_STOPTELE(int16 argc, int16 *argv);
	int16 o1_HIDECURS(int16 argc, int16 *argv);
	int16 o1_SHOWCURS(int16 argc, int16 *argv);
	int16 o1_MUSICBEAT(int16 argc, int16 *argv);
	int16 o1_SCREENLOCK(int16 argc, int16 *argv);
	int16 o1_ADDSPRITE(int16 argc, int16 *argv);
	int16 o1_FREEANIM(int16 argc, int16 *argv);
	int16 o1_DRAWSPRITE(int16 argc, int16 *argv);
	int16 o1_ERASESPRITES(int16 argc, int16 *argv);
	int16 o1_UPDATESPRITES(int16 argc, int16 *argv);
	int16 o1_GETTIMER(int16 argc, int16 *argv);
	int16 o1_SETTIMER(int16 argc, int16 *argv);
	int16 o1_RESETTIMER(int16 argc, int16 *argv);
	int16 o1_ALLOCTIMER(int16 argc, int16 *argv);
	int16 o1_FREETIMER(int16 argc, int16 *argv);
	int16 o1_PALETTELOCK(int16 argc, int16 *argv);
	int16 o1_FONT(int16 argc, int16 *argv);
	int16 o1_DRAWTEXT(int16 argc, int16 *argv);
	int16 o1_HOMETEXT(int16 argc, int16 *argv);
	int16 o1_TEXTRECT(int16 argc, int16 *argv);
	int16 o1_TEXTXY(int16 argc, int16 *argv);
	int16 o1_DROPSHADOW(int16 argc, int16 *argv);
	int16 o1_TEXTCOLOR(int16 argc, int16 *argv);
	int16 o1_OUTLINE(int16 argc, int16 *argv);
	int16 o1_LOADCURSOR(int16 argc, int16 *argv);
	int16 o1_SETGROUND(int16 argc, int16 *argv);
	int16 o1_RESTEXT(int16 argc, int16 *argv);
	int16 o1_ADDMASK(int16 argc, int16 *argv);
	int16 o1_SETMASK(int16 argc, int16 *argv);
	int16 o1_ISSND(int16 argc, int16 *argv);
	int16 o1_STOPSND(int16 argc, int16 *argv);
	int16 o1_PLAYVOICE(int16 argc, int16 *argv);
	int16 o1_CDPLAY(int16 argc, int16 *argv);
	int16 o1_STOPCD(int16 argc, int16 *argv);
	int16 o1_CDSTATUS(int16 argc, int16 *argv);
	int16 o1_CDTIME(int16 argc, int16 *argv);
	int16 o1_CDPLAYSEG(int16 argc, int16 *argv);
};

} // End of namespace Made

#endif /* MADE_H */
