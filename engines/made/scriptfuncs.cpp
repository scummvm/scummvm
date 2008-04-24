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

#include "common/endian.h"
#include "common/util.h"
#include "common/events.h"

#include "graphics/cursorman.h"

#include "made/made.h"
#include "made/resource.h"
#include "made/database.h"
#include "made/screen.h"
#include "made/script.h"
#include "made/pmvplayer.h"
#include "made/scriptfuncs.h"
#include "made/music.h"

namespace Made {

int16 ScriptFunctions::callFunction(uint16 index, int16 argc, int16 *argv) {
	if (index >= _externalFuncs.size()) {
		// TODO: ERROR!
		return 0;
	}
	
	fflush(stdout);
	//g_system->delayMillis(2000);
	
	return (*_externalFuncs[index])(argc, argv);
}

typedef Common::Functor2Mem<int16, int16*, int16, ScriptFunctionsRtz> ExternalFuncRtz;
#define External(x) ExternalFuncRtz(this, &ScriptFunctionsRtz::x)
void ScriptFunctionsRtz::setupExternalsTable() {
	static const ExternalFuncRtz externalsTable[] = {
		External(o1_SYSTEM),
		External(o1_INITGRAF),
		External(o1_RESTOREGRAF),
		External(o1_DRAWPIC),
		External(o1_CLS),
		External(o1_SHOWPAGE),
		External(o1_EVENT),
		External(o1_EVENTX),
		External(o1_EVENTY),
		External(o1_EVENTKEY),
		External(o1_VISUALFX),
		External(o1_PLAYSND),
		External(o1_PLAYMUS),
		External(o1_STOPMUS),
		External(o1_ISMUS),
		External(o1_TEXTPOS),
		External(o1_FLASH),
		External(o1_PLAYNOTE),
		External(o1_STOPNOTE),
		External(o1_PLAYTELE),
		External(o1_STOPTELE),
		External(o1_HIDECURS),
		External(o1_SHOWCURS),
		External(o1_MUSICBEAT),
		External(o1_SCREENLOCK),
		External(o1_ADDSPRITE),
		External(o1_FREEANIM),
		External(o1_DRAWSPRITE),
		External(o1_ERASESPRITES),
		External(o1_UPDATESPRITES),
		External(o1_GETTIMER),
		External(o1_SETTIMER),
		External(o1_RESETTIMER),
		External(o1_ALLOCTIMER),
		External(o1_FREETIMER),
		External(o1_PALETTELOCK),
		External(o1_FONT),
		External(o1_DRAWTEXT),
		External(o1_HOMETEXT),
		External(o1_TEXTRECT),
		External(o1_TEXTXY),
		External(o1_DROPSHADOW),
		External(o1_TEXTCOLOR),
		External(o1_OUTLINE),
		External(o1_LOADCURSOR),
		External(o1_SETGROUND),
		External(o1_RESTEXT),
		External(o1_CLIPAREA),
		External(o1_SETCLIP),
		External(o1_ISSND),
		External(o1_STOPSND),
		External(o1_PLAYVOICE),
		External(o1_CDPLAY),
		External(o1_STOPCD),
		External(o1_CDSTATUS),
		External(o1_CDTIME),
		External(o1_CDPLAYSEG),
		External(o1_PRINTF),
		External(o1_MONOCLS),
		External(o1_SNDENERGY),
		External(o1_CLEARTEXT),
		External(o1_ANIMTEXT),
		External(o1_TEXTWIDTH),
		External(o1_PLAYMOVIE),
		External(o1_LOADSND),
		External(o1_LOADMUS),
		External(o1_LOADPIC),
		External(o1_MUSICVOL),
		External(o1_RESTARTEVENTS),
		External(o1_PLACESPRITE),
		External(o1_PLACETEXT),
		External(o1_DELETECHANNEL),
		External(o1_CHANNELTYPE),
		External(o1_SETSTATE),
		External(o1_SETLOCATION),
		External(o1_SETCONTENT),
		External(o1_EXCLUDEAREA),
		External(o1_SETEXCLUDE),
		External(o1_GETSTATE),
		External(o1_PLACEANIM),
		External(o1_SETFRAME),
		External(o1_GETFRAME),
		External(o1_GETFRAMECOUNT),
		External(o1_PICWIDTH),
		External(o1_PICHEIGHT),
		External(o1_SOUNDRATE),
		External(o1_DRAWANIMPIC),
		External(o1_LOADANIM),
		External(o1_READTEXT),
		External(o1_READMENU),
		External(o1_DRAWMENU),
		External(o1_MENUCOUNT),
		External(o1_SAVEGAME),
		External(o1_LOADGAME),
		External(o1_GAMENAME),
		External(o1_SHAKESCREEN),
		External(o1_PLACEMENU),
		External(o1_SETVOLUME),
		External(o1_WHATSYNTH),
		External(o1_SLOWSYSTEM)
	};

	for (int i = 0; i < ARRAYSIZE(externalsTable); ++i)
		_externalFuncs.push_back(&externalsTable[i]);

}
#undef External

int16 ScriptFunctionsRtz::o1_SYSTEM(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SYSTEM");
	return 0;
}

int16 ScriptFunctionsRtz::o1_INITGRAF(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_INITGRAF");
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTOREGRAF(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTOREGRAF");
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWPIC(int16 argc, int16 *argv) {
	int16 channel = _vm->_screen->drawPic(argv[4], argv[3], argv[2], argv[1], argv[0]);
	return channel;
}

int16 ScriptFunctionsRtz::o1_CLS(int16 argc, int16 *argv) {
 	_vm->_screen->clearScreen();
	return 0;
}

int16 ScriptFunctionsRtz::o1_SHOWPAGE(int16 argc, int16 *argv) {
	_vm->_screen->show();
	return 0;
}

int16 ScriptFunctionsRtz::o1_EVENT(int16 argc, int16 *argv) {

	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();

	int16 eventNum = 0;

	if (eventMan->pollEvent(event)) {
		switch (event.type) {

		case Common::EVENT_MOUSEMOVE:
			_vm->_eventMouseX = event.mouse.x;
			_vm->_eventMouseY = event.mouse.y;
			break;
			
		case Common::EVENT_LBUTTONDOWN:
			eventNum = 1;
			break;

		case Common::EVENT_RBUTTONDOWN:
			eventNum = 3;
			break;

		case Common::EVENT_KEYDOWN:
			_vm->_eventKey = event.kbd.ascii;
			switch (_vm->_eventKey) {
			case '1':
				eventNum = 1;
				break;
			case '2':
				eventNum = 2;
				break;
			case '3':
				eventNum = 3;
				break;
			case '4':
				eventNum = 4;
				break;
			case '5':
				eventNum = 5;
				break;
			default:
				break;
			}
			break;

		case Common::EVENT_QUIT:
			// TODO: Exit more gracefully.
			g_system->quit();
			break;

		default:
			break;

		}
	}

	_vm->_system->updateScreen();
	//g_system->delayMillis(10);

	return eventNum;
}

int16 ScriptFunctionsRtz::o1_EVENTX(int16 argc, int16 *argv) {
	return _vm->_eventMouseX;
}

int16 ScriptFunctionsRtz::o1_EVENTY(int16 argc, int16 *argv) {
	return _vm->_eventMouseY;
}

int16 ScriptFunctionsRtz::o1_EVENTKEY(int16 argc, int16 *argv) {
	return _vm->_eventKey;
}

int16 ScriptFunctionsRtz::o1_VISUALFX(int16 argc, int16 *argv) {
	_vm->_screen->setVisualEffectNum(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYSND(int16 argc, int16 *argv) {
	int soundId = argv[0];
	bool loop = false;

	if (argc > 1) {
		soundId = argv[1];
		loop = (argv[0] == 1);
	}

	if (soundId > 0) {
		if (!_vm->_mixer->isSoundHandleActive(_audioStreamHandle)) {
			_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, 
										 _vm->_res->getSound(soundId)->getAudioStream(_vm->_soundRate, loop));
		}
	}

	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYMUS(int16 argc, int16 *argv) {
	int16 musicId = argv[0];
	if (musicId > 0) {
		XmidiResource *xmidi = _vm->_res->getXmidi(musicId);
		_vm->_music->play(xmidi);
		_vm->_res->freeResource(xmidi);
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPMUS(int16 argc, int16 *argv) {
	_vm->_music->stop();
	return 0;
}

int16 ScriptFunctionsRtz::o1_ISMUS(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying())
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTPOS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTPOS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_FLASH(int16 argc, int16 *argv) {
	_vm->_screen->flash(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYNOTE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPNOTE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYTELE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPTELE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_HIDECURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(false);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SHOWCURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(true);
	return 0;
}

int16 ScriptFunctionsRtz::o1_MUSICBEAT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MUSICBEAT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SCREENLOCK(int16 argc, int16 *argv) {
	_vm->_screen->setScreenLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ADDSPRITE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_ADDSPRITE");
	//_vm->_screen->addSprite(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_FREEANIM(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWSPRITE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DRAWSPRITE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_ERASESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsRtz::o1_UPDATESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->updateSprites();
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETTIMER(int16 argc, int16 *argv) {
	return _vm->getTimer(argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETTIMER(int16 argc, int16 *argv) {
	_vm->setTimer(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESETTIMER(int16 argc, int16 *argv) {
	_vm->resetTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ALLOCTIMER(int16 argc, int16 *argv) {
	int16 timerNum = _vm->allocTimer();
	return timerNum;
}

int16 ScriptFunctionsRtz::o1_FREETIMER(int16 argc, int16 *argv) {
	_vm->freeTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PALETTELOCK(int16 argc, int16 *argv) {
	_vm->_screen->setPaletteLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsRtz::o1_FONT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_FONT");

	uint16 fontID = argv[0];
	printf("Set font to %i\n", fontID);
	_vm->_screen->setFont(fontID);
	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DRAWTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_HOMETEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_HOMETEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTRECT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTRECT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTXY(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTXY");

	int16 x = CLIP<int16>(argv[0], 1, 318);
	int16 y = CLIP<int16>(argv[1], 1, 198);

	printf("Text: x = %i, y = %i\n", x, y);
	return 0;
}

int16 ScriptFunctionsRtz::o1_DROPSHADOW(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DROPSHADOW");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTCOLOR(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTCOLOR");
	return 0;
}

int16 ScriptFunctionsRtz::o1_OUTLINE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_OUTLINE");
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADCURSOR(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[2]);
	Graphics::Surface *surf = flex->getPicture();
	CursorMan.replaceCursor((const byte *)surf->pixels, surf->w, surf->h, argv[1], argv[0], 0);
	CursorMan.showMouse(true);
	_vm->_res->freeResource(flex);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETGROUND(int16 argc, int16 *argv) {
	_vm->_screen->setGround(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CLIPAREA(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CLIPAREA");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETCLIP(int16 argc, int16 *argv) {
	_vm->_screen->setClip(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_ISSND(int16 argc, int16 *argv) {
	if (_vm->_mixer->isSoundHandleActive(_audioStreamHandle))
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsRtz::o1_STOPSND(int16 argc, int16 *argv) {
	_vm->_mixer->stopHandle(_audioStreamHandle);
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYVOICE(int16 argc, int16 *argv) {
	if (argv[0] > 0) {
		if (!_vm->_mixer->isSoundHandleActive(_voiceStreamHandle)) {
			_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_voiceStreamHandle, 
										 _vm->_res->getSound(argv[0])->getAudioStream(_vm->_soundRate, false));
		}
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDPLAY(int16 argc, int16 *argv) {
	// This one is called loads of times, so it has been commented out to reduce spam
	//warning("Unimplemented opcode: o1_CDPLAY");
	return 0;
}

int16 ScriptFunctionsRtz::o1_STOPCD(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPCD");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDSTATUS(int16 argc, int16 *argv) {
	// This one is called loads of times, so it has been commented out to reduce spam
	//warning("Unimplemented opcode: o1_CDSTATUS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDTIME(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CDTIME");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CDPLAYSEG(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CDPLAYSEG");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PRINTF(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PRINTF");
	return 0;
}

int16 ScriptFunctionsRtz::o1_MONOCLS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MONOCLS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SNDENERGY(int16 argc, int16 *argv) {
	// This is called while in-game voices are played
	// Not sure what it's used for
	// Commented out to reduce spam
	//warning("Unimplemented opcode: o1_SNDENERGY");
	return 0;
}

int16 ScriptFunctionsRtz::o1_CLEARTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_CLEARTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_ANIMTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_ANIMTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_TEXTWIDTH(int16 argc, int16 *argv) {
	Object *obj = _vm->_dat->getObject(argv[1]);
	const char *text = obj->getString();
	debug(4, "text = %s\n", text);
	// TODO
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLAYMOVIE(int16 argc, int16 *argv) {
	const char *movieName = _vm->_dat->getObject(argv[1])->getString();
	_vm->_pmvPlayer->play(movieName);
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADSND(int16 argc, int16 *argv) {
	SoundResource *sound = _vm->_res->getSound(argv[0]);
	if (sound) {
		_vm->_res->freeResource(sound);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADMUS(int16 argc, int16 *argv) {
	XmidiResource *xmidi = _vm->_res->getXmidi(argv[0]);
	if (xmidi) {
		_vm->_res->freeResource(xmidi);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADPIC(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		_vm->_res->freeResource(flex);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_MUSICVOL(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MUSICVOL");
	return 0;
}

int16 ScriptFunctionsRtz::o1_RESTARTEVENTS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTARTEVENTS");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLACESPRITE(int16 argc, int16 *argv) {
	return _vm->_screen->placeSprite(argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_PLACETEXT(int16 argc, int16 *argv) {
	Object *obj = _vm->_dat->getObject(argv[5]);
	const char *text = obj->getString();
	debug(4, "text = %s\n", text); fflush(stdout);
	return 0;
}

int16 ScriptFunctionsRtz::o1_DELETECHANNEL(int16 argc, int16 *argv) {
	_vm->_screen->deleteChannel(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_CHANNELTYPE(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelType(argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETSTATE(int16 argc, int16 *argv) {
	_vm->_screen->setChannelState(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETLOCATION(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SETLOCATION");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETCONTENT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SETCONTENT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_EXCLUDEAREA(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_EXCLUDEAREA");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETEXCLUDE(int16 argc, int16 *argv) {
	_vm->_screen->setExclude(argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETSTATE(int16 argc, int16 *argv) {
	return _vm->_screen->getChannelState(argv[0]);
}

int16 ScriptFunctionsRtz::o1_PLACEANIM(int16 argc, int16 *argv) {
	return _vm->_screen->placeAnim(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsRtz::o1_SETFRAME(int16 argc, int16 *argv) {
	_vm->_screen->setAnimFrame(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsRtz::o1_GETFRAME(int16 argc, int16 *argv) {
	return _vm->_screen->getAnimFrame(argv[0]);
}

int16 ScriptFunctionsRtz::o1_GETFRAMECOUNT(int16 argc, int16 *argv) {
	debug(4, "anim = %04X\n", argv[0]);
	int16 frameCount = _vm->_screen->getAnimFrameCount(argv[0]);
	debug(4, "frameCount = %04X\n", frameCount);
	return frameCount;
}

int16 ScriptFunctionsRtz::o1_PICWIDTH(int16 argc, int16 *argv) {
	int16 width = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		width = flex->getPicture()->w;
		_vm->_res->freeResource(flex);
	}
	return width;
}

int16 ScriptFunctionsRtz::o1_PICHEIGHT(int16 argc, int16 *argv) {
	int16 height = 0;
	PictureResource *flex = _vm->_res->getPicture(argv[0]);
	if (flex) {
		height = flex->getPicture()->h;
		_vm->_res->freeResource(flex);
	}
	return height;
}

int16 ScriptFunctionsRtz::o1_SOUNDRATE(int16 argc, int16 *argv) {
	_vm->_soundRate = argv[0];
	return 1;
}

int16 ScriptFunctionsRtz::o1_DRAWANIMPIC(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DRAWANIMPIC");
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADANIM(int16 argc, int16 *argv) {
	AnimationResource *anim = _vm->_res->getAnimation(argv[0]);
	if (anim) {
		_vm->_res->freeResource(anim);
		return 1;
	}
	return 0;
}

int16 ScriptFunctionsRtz::o1_READTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_READTEXT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_READMENU(int16 argc, int16 *argv) {
	int16 objectIndex = argv[2];
	int16 menuIndex = argv[1];
	int16 textIndex = argv[0];
	MenuResource *menu = _vm->_res->getMenu(menuIndex);
	if (menu) {
		const char *text = menu->getString(textIndex);
		debug(4, "text = %s\n", text); fflush(stdout);
		Object *obj = _vm->_dat->getObject(objectIndex);
		obj->setString(text);
		_vm->_res->freeResource(menu);
	}

	return 0;
}

int16 ScriptFunctionsRtz::o1_DRAWMENU(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_DRAWMENU");
	return 0;
}

int16 ScriptFunctionsRtz::o1_MENUCOUNT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MENUCOUNT");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SAVEGAME(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SAVEGAME");
	return 0;
}

int16 ScriptFunctionsRtz::o1_LOADGAME(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_LOADGAME");
	return 0;
}

int16 ScriptFunctionsRtz::o1_GAMENAME(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_GAMENAME");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SHAKESCREEN(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SHAKESCREEN");
	return 0;
}

int16 ScriptFunctionsRtz::o1_PLACEMENU(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLACEMENU");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SETVOLUME(int16 argc, int16 *argv) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, argv[0] * 25);
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, argv[0] * 25);
	return 0;
}

int16 ScriptFunctionsRtz::o1_WHATSYNTH(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_WHATSYNTH");
	return 0;
}

int16 ScriptFunctionsRtz::o1_SLOWSYSTEM(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_SLOWSYSTEM");
	return 0;
}

} // End of namespace Made
