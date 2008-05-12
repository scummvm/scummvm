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

/* ScriptFunctionsLgop2 */

typedef Common::Functor2Mem<int16, int16*, int16, ScriptFunctionsLgop2> ExternalFuncLgop2;
#define External(x) \
	_externalFuncs.push_back(new ExternalFuncLgop2(this, &ScriptFunctionsLgop2::x));  \
	_externalFuncNames.push_back(#x);
void ScriptFunctionsLgop2::setupExternalsTable() {

	External(o1_SYSTEM);
	External(o1_INITGRAF);
	External(o1_RESTOREGRAF);
	External(o1_DRAWPIC);
	External(o1_CLS);
	External(o1_SHOWPAGE);
	External(o1_EVENT);
	External(o1_EVENTX);
	External(o1_EVENTY);
	External(o1_EVENTKEY);
	External(o1_VISUALFX);
	External(o1_PLAYSND);
	External(o1_PLAYMUS);
	External(o1_STOPMUS);
	External(o1_ISMUS);
	External(o1_TEXTPOS);
	External(o1_FLASH);
	External(o1_PLAYNOTE);
	External(o1_STOPNOTE);
	External(o1_PLAYTELE);
	External(o1_STOPTELE);
	External(o1_HIDECURS);
	External(o1_SHOWCURS);
	External(o1_MUSICBEAT);
	External(o1_SCREENLOCK);
	External(o1_ADDSPRITE);
	External(o1_FREEANIM);
	External(o1_DRAWSPRITE);
	External(o1_ERASESPRITES);
	External(o1_UPDATESPRITES);
	External(o1_GETTIMER);
	External(o1_SETTIMER);
	External(o1_RESETTIMER);
	External(o1_ALLOCTIMER);
	External(o1_FREETIMER);
	External(o1_PALETTELOCK);
	External(o1_FONT);
	External(o1_DRAWTEXT);
	External(o1_HOMETEXT);
	External(o1_TEXTRECT);
	External(o1_TEXTXY);
	External(o1_DROPSHADOW);
	External(o1_TEXTCOLOR);
	External(o1_OUTLINE);
	External(o1_LOADCURSOR);
	External(o1_SETGROUND);
	External(o1_RESTEXT);
	External(o1_ADDMASK);
	External(o1_SETMASK);
	External(o1_ISSND);
	External(o1_STOPSND);
	External(o1_PLAYVOICE);
	
}
#undef External

int16 ScriptFunctionsLgop2::o1_SYSTEM(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsLgop2::o1_INITGRAF(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsLgop2::o1_RESTOREGRAF(int16 argc, int16 *argv) {
	// This opcode is empty.
	return 0;
}

int16 ScriptFunctionsLgop2::o1_DRAWPIC(int16 argc, int16 *argv) {
	return _vm->_screen->drawPic(argv[4], argv[3], argv[2], argv[1], argv[0]);
}

int16 ScriptFunctionsLgop2::o1_CLS(int16 argc, int16 *argv) {
 	_vm->_screen->clearScreen();
	return 0;
}

int16 ScriptFunctionsLgop2::o1_SHOWPAGE(int16 argc, int16 *argv) {
	_vm->_mixer->stopHandle(_audioStreamHandle);
	_vm->_screen->show();
	return 0;
}

int16 ScriptFunctionsLgop2::o1_EVENT(int16 argc, int16 *argv) {

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

		/*
		case Common::EVENT_LBUTTONUP:
			eventNum = 2; // TODO: Is this correct?
			break;
		*/

		case Common::EVENT_RBUTTONDOWN:
			eventNum = 3;
			break;

		/*
		case Common::EVENT_RBUTTONUP:
			eventNum = 4; // TODO: Is this correct?
			break;
		*/

		case Common::EVENT_KEYDOWN:
			_vm->_eventKey = event.kbd.ascii;
			// For unknown reasons, the game accepts ASCII code
			// 9 as backspace
			if (_vm->_eventKey == Common::KEYCODE_BACKSPACE)
				_vm->_eventKey = 9;
			eventNum = 5;
			break;

		case Common::EVENT_QUIT:
			_vm->_quit = true;
			break;

		default:
			break;

		}
	}

	_vm->_system->updateScreen();

	return eventNum;
}

int16 ScriptFunctionsLgop2::o1_EVENTX(int16 argc, int16 *argv) {
	return _vm->_eventMouseX;
}

int16 ScriptFunctionsLgop2::o1_EVENTY(int16 argc, int16 *argv) {
	return _vm->_eventMouseY;
}

int16 ScriptFunctionsLgop2::o1_EVENTKEY(int16 argc, int16 *argv) {
	return _vm->_eventKey;
}

int16 ScriptFunctionsLgop2::o1_VISUALFX(int16 argc, int16 *argv) {
	_vm->_screen->setVisualEffectNum(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_PLAYSND(int16 argc, int16 *argv) {
	int soundNum = argv[0];
	bool loop = false;

	if (argc > 1) {
		soundNum = argv[1];
		loop = (argv[0] == 1);
	}

	if (soundNum > 0) {
		if (!_vm->_mixer->isSoundHandleActive(_audioStreamHandle)) {
			_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, 
										 _vm->_res->getSound(soundNum)->getAudioStream(_vm->_soundRate, loop));
		}
	}

	return 0;
}

int16 ScriptFunctionsLgop2::o1_PLAYMUS(int16 argc, int16 *argv) {
	/*
	
	Disabled for now since MIDI player doesn't support all commands
	which results in strange behavior.
	
	int16 musicNum = argv[0];
	if (musicNum > 0) {
		_xmidiRes = _vm->_res->getXmidi(musicNum);
		_vm->_music->playXMIDI(_xmidiRes);
	}
	*/
	return 0;
}

int16 ScriptFunctionsLgop2::o1_STOPMUS(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying()) {
		_vm->_music->stop();
		_vm->_res->freeResource(_xmidiRes);
	}
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ISMUS(int16 argc, int16 *argv) {
	if (_vm->_music->isPlaying())
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsLgop2::o1_TEXTPOS(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_TEXTPOS");
	// This seems to be some kind of low-level opcode.
	// The original engine calls int 10h to set the VGA cursor position.
	return 0;
}

int16 ScriptFunctionsLgop2::o1_FLASH(int16 argc, int16 *argv) {
	_vm->_screen->flash(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_PLAYNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYNOTE");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_STOPNOTE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPNOTE");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_PLAYTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_PLAYTELE");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_STOPTELE(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_STOPTELE");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_HIDECURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(false);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_SHOWCURS(int16 argc, int16 *argv) {
	_vm->_system->showMouse(true);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_MUSICBEAT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_MUSICBEAT");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_SCREENLOCK(int16 argc, int16 *argv) {
	_vm->_screen->setScreenLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ADDSPRITE(int16 argc, int16 *argv) {
	return argv[2];
}

int16 ScriptFunctionsLgop2::o1_FREEANIM(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsLgop2::o1_DRAWSPRITE(int16 argc, int16 *argv) {
	int16 channel = _vm->_screen->drawSprite(argv[2], argv[1], argv[0]);
	_vm->_screen->setChannelUseMask(channel);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ERASESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->clearChannels();
	return 0;
}

int16 ScriptFunctionsLgop2::o1_UPDATESPRITES(int16 argc, int16 *argv) {
	_vm->_screen->updateSprites();
	return 0;
}

int16 ScriptFunctionsLgop2::o1_GETTIMER(int16 argc, int16 *argv) {
	return _vm->getTimer(argv[0]);
}

int16 ScriptFunctionsLgop2::o1_SETTIMER(int16 argc, int16 *argv) {
	_vm->setTimer(argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_RESETTIMER(int16 argc, int16 *argv) {
	_vm->resetTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ALLOCTIMER(int16 argc, int16 *argv) {
	return _vm->allocTimer();
}

int16 ScriptFunctionsLgop2::o1_FREETIMER(int16 argc, int16 *argv) {
	_vm->freeTimer(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_PALETTELOCK(int16 argc, int16 *argv) {
	_vm->_screen->setPaletteLock(argv[0] != 0);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_FONT(int16 argc, int16 *argv) {
	_vm->_screen->setFont(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_DRAWTEXT(int16 argc, int16 *argv) {

	const char *text = _vm->_dat->getString(argv[argc - 1]);
	
	char finalText[1024];
	switch (argc) {
	case 1:
		snprintf(finalText, 1024, "%s", text);
		break;
	case 2:
		snprintf(finalText, 1024, text, argv[0]);
		break;
	case 3:
		snprintf(finalText, 1024, text, argv[1], argv[0]);
		break;
	case 4:
		snprintf(finalText, 1024, text, argv[2], argv[1], argv[0]);
		break;
	case 5:
		snprintf(finalText, 1024, text, argv[3], argv[2], argv[1], argv[0]);
		break;
	default:
		finalText[0] = '\0';
		break;
	}
	
	_vm->_screen->printText(finalText);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_HOMETEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_HOMETEXT");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_TEXTRECT(int16 argc, int16 *argv) {
	int16 x1 = CLIP<int16>(argv[4], 1, 318);
	int16 y1 = CLIP<int16>(argv[3], 1, 198);
	int16 x2 = CLIP<int16>(argv[2], 1, 318);
	int16 y2 = CLIP<int16>(argv[1], 1, 198);
	//int16 textValue = argv[0];
	// TODO: textValue
	_vm->_screen->setTextRect(Common::Rect(x1, y1, x2, y2));
	return 0;
}

int16 ScriptFunctionsLgop2::o1_TEXTXY(int16 argc, int16 *argv) {
	int16 x = CLIP<int16>(argv[1], 1, 318);
	int16 y = CLIP<int16>(argv[0], 1, 198);
	_vm->_screen->setTextXY(x, y);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_DROPSHADOW(int16 argc, int16 *argv) {
	// if the drop shadow color is -1, then text drop shadow is disabled
	// when font drop shadow is enabled, outline is disabled
	_vm->_screen->setDropShadowColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_TEXTCOLOR(int16 argc, int16 *argv) {
	_vm->_screen->setTextColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_OUTLINE(int16 argc, int16 *argv) {
	// if the outline color is -1, then text outline is disabled
	// when font outline is enabled, drop shadow is disabled
	_vm->_screen->setOutlineColor(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_LOADCURSOR(int16 argc, int16 *argv) {
	PictureResource *flex = _vm->_res->getPicture(argv[2]);
	Graphics::Surface *surf = flex->getPicture();
	CursorMan.replaceCursor((const byte *)surf->pixels, surf->w, surf->h, argv[1], argv[0], 0);
	CursorMan.showMouse(true);
	_vm->_res->freeResource(flex);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_SETGROUND(int16 argc, int16 *argv) {
	_vm->_screen->setGround(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_RESTEXT(int16 argc, int16 *argv) {
	warning("Unimplemented opcode: o1_RESTEXT");
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ADDMASK(int16 argc, int16 *argv) {
	_vm->_screen->drawMask(argv[2], argv[1], argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_SETMASK(int16 argc, int16 *argv) {
	_vm->_screen->setMask(argv[0]);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_ISSND(int16 argc, int16 *argv) {
	if (_vm->_mixer->isSoundHandleActive(_audioStreamHandle))
		return 1;
	else
		return 0;
}

int16 ScriptFunctionsLgop2::o1_STOPSND(int16 argc, int16 *argv) {
	_vm->_mixer->stopHandle(_audioStreamHandle);
	return 0;
}

int16 ScriptFunctionsLgop2::o1_PLAYVOICE(int16 argc, int16 *argv) {
	if (argv[0] > 0) {
		_vm->_mixer->stopHandle(_audioStreamHandle);
		_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle,
			_vm->_res->getSound(argv[0])->getAudioStream(_vm->_soundRate, false));
	}
	return 0;
}

} // End of namespace Made
