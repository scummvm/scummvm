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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/ringworld2/ringworld2_speakers.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_scenes1.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

VisualSpeaker::VisualSpeaker(): Speaker() {
	_delayAmount = 0;
	_voiceDelayAmount = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
	_color1 = 8;
	_color2 = 0;
	_displayMode = 0;
	_speakerMode = 0;

	_object2 = nullptr;
	_fieldF8 = 0;
	_soundId = 0;
	_removeObject = false;
	_numFrames = 0;
	_voiceFrameNumber = 0;
}

void VisualSpeaker::remove() {
	_numFrames = 0;
	_delayAmount = 0;
	R2_GLOBALS._playStream.stop();

	if (_object2) {
		if (_fieldF8) {
			_fieldF8 = 0;
			_object1.setStrip(_object1._strip - 1);
			_object1.setFrame(_object1.getFrameCount());
			_object1.animate(ANIM_MODE_6, (_speakerMode == 0xff) ? this : NULL);
		} else {
			_object1.animate(ANIM_MODE_6, (_speakerMode == 0xff) ? this : NULL);
		}
	}

	Speaker::remove();
}

void VisualSpeaker::signal() {
	// TODO: Handle _field18 if used in Action. It looks only used in the original in remove()
	// _action->_field18 = 1;
	if (_speakerMode == 0xff)
		stopSpeaking();

	_speakerMode = 0;
	if (_numFrames) {
		if (_object2) {
			_object1.setStrip(_object1._strip + 1);
			_object1.animate(ANIM_MODE_2, NULL);
			_fieldF8 = 1;
		}

		if ((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) || !_soundId)
			_sceneText.show();

		if ((R2_GLOBALS._speechSubtitles & SPEECH_VOICE) && _soundId) {
			setVoiceFrame(1);
		}
	} else if (_action && _object2) {
		_action->setDelay(1);
		_sceneText.remove();

		R2_GLOBALS._playStream.stop();
	}
}

void VisualSpeaker::dispatch() {
	uint32 frameNumber = R2_GLOBALS._events.getFrameNumber();
	assert(_action);

	// Delay check for character animation
	if (_delayAmount) {
		if (frameNumber >= _frameNumber) {
			_delayAmount = _delayAmount - (_frameNumber - frameNumber);
			_frameNumber = frameNumber;

			if (_delayAmount <= 0) {
				_delayAmount = 0;
				_object1.animate(ANIM_MODE_NONE, NULL);
				_object1.setFrame(1);
			}
		}
	}

	// Delay check for voice
	if (_voiceDelayAmount) {
		if (frameNumber >= _voiceFrameNumber) {
			_voiceDelayAmount = _voiceDelayAmount - (frameNumber - _voiceFrameNumber);
			_voiceFrameNumber = frameNumber;

			if (_voiceDelayAmount <= 0) {
				_voiceDelayAmount = 0;
				if (R2_GLOBALS._playStream.play(_soundId, NULL)) {
					_numFrames = 2;
					_soundId = 0;
				} else {
					_sceneText.show();
				}
			}
		}
	}

	if ((R2_GLOBALS._speechSubtitles & SPEECH_VOICE) && (_numFrames == 2) &&
			!R2_GLOBALS._playStream.isPlaying()) {
		_numFrames = 0;
		_object1.animate(ANIM_MODE_NONE);
		_object1.setFrame(1);

		if (!(R2_GLOBALS._speechSubtitles & SPEECH_TEXT)) {
			// Don't bother waiting for a mouse click to start the next speech segment
			_action->setDelay(1);
		}
	}
}

void VisualSpeaker::synchronize(Serializer &s) {
	Speaker::synchronize(s);

	SYNC_POINTER(_object2);
	s.syncAsSint16LE(_speakerMode);
	s.syncAsSint16LE(_fieldF8);
	s.syncAsSint16LE(_displayMode);
	s.syncAsSint16LE(_soundId);
	s.syncAsByte(_removeObject);
	s.syncAsSint16LE(_numFrames);
	s.syncAsSint16LE(_delayAmount);
	s.syncAsUint32LE(_frameNumber);
	s.syncAsSint16LE(_voiceDelayAmount);
	s.syncAsUint32LE(_voiceFrameNumber);
}

void VisualSpeaker::setText(const Common::String &msg) {
	_sceneText.remove();

	// Position the text depending on the specified display mode
	switch (_displayMode) {
	case 2:
		_textPos = Common::Point(60, 20);
		break;
	case 3:
		_textPos = Common::Point(110, 20);
		break;
	case 4:
		_textPos = Common::Point(10, 100);
		break;
	case 5:
		_textPos = Common::Point(60, 100);
		break;
	case 6:
		_textPos = Common::Point(110, 100);
		break;
	case 7:
		_textPos = Common::Point(170, 20);
		break;
	case 8:
		_textPos = Common::Point(170, 100);
		break;
	case 9:
		_textPos = Common::Point(330, 20);
		break;
	default:
		_textPos = Common::Point(10, 20);
		break;
	}

	// Check if the message starts with a '!'. If so, it indicates a speech resource Id to be played,
	// in which case extract the resource number from the message.
	_soundId = 0;
	Common::String s = msg;
	if (s.hasPrefix("!")) {
		s.deleteChar(0);
		_soundId = atoi(s.c_str());

		while (!s.empty() && (*s.c_str() >= '0' && *s.c_str() <= '9'))
			s.deleteChar(0);
	}

	// Set up the text details
	_sceneText._color1 = _color1;
	_sceneText._color2 = _color2;
	_sceneText._color3 = _color3;
	_sceneText._width = _textWidth;
	_sceneText._fontNumber = _fontNumber;
	_sceneText._textMode = _textMode;
	_sceneText.setup(s);

	_sceneText.setPosition(_textPos);
	_sceneText.fixPriority(256);

	// If subtitles are turned off, don't show the text
	if ((R2_GLOBALS._speechSubtitles & SPEECH_VOICE) &&
			!(R2_GLOBALS._speechSubtitles & SPEECH_TEXT)) {
		_sceneText.hide();
	}

	// Figure out the text delay if subtitles are turned on, or there's no speech resource specified
	if ((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) || !_soundId) {
		const char *msgP = s.c_str();
		int numWords = 0;
		while (*msgP != '\0') {
			if (*msgP++ == ' ')
				++numWords;
		}

		if (!numWords && !s.empty())
			++numWords;

		_numFrames = numWords * 30 + 120;
		setDelay(_numFrames);
	} else {
		_numFrames = 1;
	}

	// If the text is empty, no delay is needed
	if (s.empty())
		_numFrames = 0;

	if (_speakerMode) {
		if ((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) || !_soundId)
			_sceneText.hide();
	} else {
		if ((R2_GLOBALS._speechSubtitles & SPEECH_VOICE) && _soundId) {
			if (!R2_GLOBALS._playStream.play(_soundId, NULL)) {
				// Couldn't play voice, so fall back on showing text
				_sceneText.show();
			} else {
				_numFrames = 2;
				_soundId = 0;
			}
		}
	}
}

void VisualSpeaker::stopSpeaking() {
	R2_GLOBALS._playStream.stop();
	_speakerMode = 0;
	_object1.remove();

	if (_object2)
		_object2->show();
	_object2 = NULL;
	_fieldF8 = 0;
}

void VisualSpeaker::setFrame(int numFrames) {
	_delayAmount = numFrames;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

void VisualSpeaker::setVoiceFrame(int numFrames) {
	_voiceDelayAmount = numFrames;
	_voiceFrameNumber = R2_GLOBALS._events.getFrameNumber();
}

void VisualSpeaker::setDelay(int delay) {
	_delayAmount = delay;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

//----------------------------------------------------------------------------

SpeakerGameText::SpeakerGameText(): VisualSpeaker() {
	_speakerName = "GAMETEXT";
	_color1 = 8;
	_color2 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
}

//----------------------------------------------------------------------------
// Classes related to CAPTAIN
//----------------------------------------------------------------------------

SpeakerCaptain3210::SpeakerCaptain3210() {
	_speakerName = "Captain";
	_color1 = 5;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerCaptain3210::animateSpeaker() {
	int v = _speakerMode;
	Scene3210 *scene = (Scene3210 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_captain;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to CARETAKER
//----------------------------------------------------------------------------

SpeakerCaretaker2450::SpeakerCaretaker2450() {
	_speakerName = "CARETAKER";
	_color1 = 43;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

//----------------------------------------------------------------------------
// Classes related to CHIEF
//----------------------------------------------------------------------------

SpeakerChief1100::SpeakerChief1100() {
	_speakerName = "CHIEF";
	_color1 = 8;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerChief1100::animateSpeaker() {
	int v = _speakerMode;
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_chief;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4080, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4080, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 100:
		_numFrames = 0;
		((StripManager *)_action)->_useless = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to GUARD
//----------------------------------------------------------------------------

SpeakerGuard::SpeakerGuard() {
	_speakerName = "GUARD";
	_color1 = 5;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerGuard2800::animateSpeaker() {
	int v = _speakerMode;
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_guard;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setZoom(75);
		_object1.setup(4060, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to JOCKO
//----------------------------------------------------------------------------

SpeakerJocko::SpeakerJocko() {
	_speakerName = "Jocko";
	_color1 = 45;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerJocko3200::animateSpeaker() {
	int v = _speakerMode;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_jocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerJocko3220::animateSpeaker() {
	int v = _speakerMode;
	Scene3220 *scene = (Scene3220 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_jocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerJocko3230::animateSpeaker() {
	int v = _speakerMode;
	Scene3230 *scene = (Scene3230 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_jocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to MIRANDA
//----------------------------------------------------------------------------

SpeakerMiranda::SpeakerMiranda(): VisualSpeaker() {
	_speakerName = "MIRANDA";
	_color1 = 154;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerMiranda300::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_miranda;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((StripManager *)_action)->_useless = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((StripManager *)_action)->_useless = 0;

		if (v == 4) {
			_object1.setup(304, 5, 1);
		} else {
			_object1.setup(305, v * 2 - 1, 1);
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda1625::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_mirandaMouth;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(Common::Point(196, 65));

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1627, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda3255::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(3257, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda3375::animateSpeaker() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (scene->_companion1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3385::animateSpeaker() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3395::animateSpeaker() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3400::animateSpeaker() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4050, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3600::animateSpeaker() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_miranda;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);

		_object1.setPosition(_object2->_position);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4051, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4050, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3700::animateSpeaker() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_miranda;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		scene->_quinn.setup(10, 6, 1);
		scene->_seeker.setup(20, 5, 1);
		_object2->setup(30, 1, 1);
		scene->_webbster.setup(40, 1, 1);
		_object1.setup(4050, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		scene->_miranda.setup(30, 8, 1);
		_object1.setup(4052, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		scene->_seeker.setup(20, 1, 1);
		scene->_miranda.setup(30, 1, 1);
		_object1.setup(4051, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to NEJ
//----------------------------------------------------------------------------

SpeakerNej::SpeakerNej() {
	_speakerName = "NEJ";
	_color1 = 171;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerNej2700::animateSpeaker() {
	int v = _speakerMode;
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_nej;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 2701:
			_object1.setup(4022, 3, 1);
			_object1.setPosition(Common::Point(164, 163));
			_object2->setPosition(Common::Point(-10, -10));
			break;
		case 2705:
			_object1.setup(4022, 7, 1);
			_object1.fixPriority(162);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerNej2750::animateSpeaker() {
	int v = _speakerMode;
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_nej;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 2705:
			_object1.setup(4022, 7, 1);
			break;
		case 2752:
			_object1.setup(2752, 1, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerNej2800::animateSpeaker() {
	int v = _speakerMode;
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_nej;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4023, 3, 1);
		if (_object2->_visage == 2801)
			_object1.setPosition(Common::Point(_object2->_position.x - 12, _object2->_position.y));
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PHARISHA
//----------------------------------------------------------------------------

SpeakerPharisha::SpeakerPharisha(): VisualSpeaker() {
	_speakerName = "PHARISHA";
	_color1 = 151;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerPharisha2435::animateSpeaker() {
	int v = _speakerMode;
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_astor;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4098, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PRIVATE
//----------------------------------------------------------------------------

SpeakerPrivate3210::SpeakerPrivate3210() {
	_speakerName = "Private";
	_color1 = 45;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerPrivate3210::animateSpeaker() {
	int v = _speakerMode;
	Scene3210 *scene = (Scene3210 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_private;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PROTECTOR
//----------------------------------------------------------------------------

SpeakerProtector3600::SpeakerProtector3600() {
	_speakerName = "Protector";
	_color1 = 170;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 7;
	_numFrames = 0;
}

void SpeakerProtector3600::animateSpeaker() {
	int v = _speakerMode;
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_protector;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		if (scene->_sceneMode != 3324) {
			_object1.setup(4125, 3, 1);
			_object1.animate(ANIM_MODE_5, this);
		} else {
			_object1.setup(3258, 6, 1);
			_object1.animate(ANIM_MODE_2, NULL);
			_object1.hide();
			_object2->setup(3258, 6, 1);
			_object2->show();
		}
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to QUINN
//----------------------------------------------------------------------------

SpeakerQuinn::SpeakerQuinn(): VisualSpeaker() {
	_speakerName = "QUINN";
	_color1 = 60;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerQuinn300::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 300);
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_quinn;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((StripManager *)_action)->_useless = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 10:
			_object1.setup((v - 1) / 4 + 4010, ((v - ((v - 1) / 4 * 4) - 1) % 8) * 2 + 1, 1);
			break;
		case 302:
			_object1.setup(308, (v - 1) % 8 + 1, 1);
			break;
		case 308:
			_object1.setup(308, 5, 1);
			break;
		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn500::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 500);
			Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();

		_object1.postInit();
		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 10:
			_object1.setup(4021, (v == 1) ? 5 : 7, 1);
			break;

		case 1500:
			_object1.setup(4021, (v == 1) ? 1 : 3, 1);
			break;

		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn1100::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (v == 0)
			return;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 1100);
			Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1108, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1109, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1109, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn2435::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 2435);
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_companion;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object2->setStrip(7);
		_object1.setup(2020, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2450::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 2435);
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_companion;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		if (R2_GLOBALS.getFlag(61))
			_object1.setup(2020, 3, 1);
		else
			_object1.setup(2020, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2700::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 19:
			_object1.setup(4022, 5, 1);
			break;
		case 2701:
			_object1.setup(4022, 1, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2750::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 19:
			_object1.setup(4022, 5, 1);
			break;
		case 2752:
			_object1.setup(2752, 3, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2800::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 16:
			_object1.setZoom(75);
			_object1.setup(4023, 5, 1);
			break;
		case 19:
			_object1.setup(4023, 1, 1);
			break;
		case 3110:
			_object1.setZoom(75);
			if (_object2->_strip == 1)
				_object1.setup(4061	, 1, 1);
			else
				_object1.setup(4061	, 3, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn3255::animateSpeaker() {
	Scene3255 *scene = (Scene3255 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_quinn;
		_object2->hide();
		_object1.postInit();
		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(3257, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn3375::animateSpeaker() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &scene->_companion1;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (scene->_companion1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4010, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3385::animateSpeaker() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &scene->_companion1;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object1.setup(4010, 3, 1);
		else
			_object1.setup(4010, 5, 1);

		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3395::animateSpeaker() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &scene->_companion1;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object1.setup(4010, 3, 1);
		else
			_object1.setup(4010, 5, 1);

		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3400::animateSpeaker() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &scene->_companion1;
		else
			_object2 = &scene->_companion2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4010, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4010, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4012, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3600::animateSpeaker() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_quinn;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4021, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4010, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4012, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3700::setText(const Common::String &msg) {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	switch (_speakerMode) {
	case 2:
		scene->_miranda.setup(30, 1, 1);
		R2_GLOBALS._sound2.play(44);
		break;
	case 3:
		scene->_miranda.setup(30, 1, 1);
		break;
	default:
		scene->_miranda.setup(30, 7, 1);
		break;
	}
	VisualSpeaker::setText(msg);
}

void SpeakerQuinn3700::animateSpeaker() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_quinn;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		R2_GLOBALS._sound2.stop();
		scene->_quinn.setup(10, 4, 1);
		scene->_miranda.setup(30, 7, 1);
		_object1.setup(3701, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		scene->_seeker.setup(20, 1, 1);
		scene->_miranda.setup(30, 1, 1);
		_object1.setup(3702, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		scene->_quinn.setup(10, 2, 1);
		scene->_miranda.setup(30, 1, 1);
		_object1.setup(4011, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to QUINNL
//----------------------------------------------------------------------------

SpeakerQuinnL::SpeakerQuinnL(): SpeakerQuinn() {
	_speakerName = "QUINNL";
}

//----------------------------------------------------------------------------
// Classes related to RALF
//----------------------------------------------------------------------------

SpeakerRalf3245::SpeakerRalf3245() {
	_speakerName = "Ralf";
	_color1 = 5;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerRalf3245::animateSpeaker() {
	int v = _speakerMode;
	Scene3245 *scene = (Scene3245 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_ralf;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 3100:
			_object1.setup(4105, (_object2->_strip * 2) - 1, 1);
			break;
		case 3101:
			_object1.setup(4108, (_object2->_strip * 2) - 1, 1);
			break;
		case 3102:
			_object1.setup(4109, (_object2->_strip * 2) - 1, 1);
			break;
		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to ROCKO
//----------------------------------------------------------------------------

SpeakerRocko::SpeakerRocko() {
	_speakerName = "Rocko";
	_color1 = 5;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerRocko3200::animateSpeaker() {
	int v = _speakerMode;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_rocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerRocko3220::animateSpeaker() {
	int v = _speakerMode;
	Scene3220 *scene = (Scene3220 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_rocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerRocko3230::animateSpeaker() {
	int v = _speakerMode;
	Scene3230 *scene = (Scene3230 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_rocko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4111, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to SEEKER
//----------------------------------------------------------------------------

SpeakerSeeker::SpeakerSeeker(): VisualSpeaker() {
	_speakerName = "SEEKER";
	_color1 = 35;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSeeker300::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {assert(R2_GLOBALS._sceneManager._sceneNumber == 300);
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();
		_object1.postInit();
		_object1.fixPriority(140);
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((StripManager *)_action)->_useless = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(306, v * 2 - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker500::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 500);
			Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();
		_object1.postInit();

		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		if (v == 1)
			_object1.setup(4041, 3, 1);
		else
			_object1.setup(4041, 1, 1);

		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker1100::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (v == 0)
			return;

		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 1100);
			Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1108, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1108, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		_object1.setPosition(Common::Point(197, 134));
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1108, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1109, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 5:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1109, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker1900::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 1900);
			Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_companion;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 1) {
		_object1.setup(4032, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	} else {
		signal();
	}
}

void SpeakerSeeker2435::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 2435);
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_companion;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object2->setStrip(7);
		_object1.setup(4099, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker2450::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			_object2 = &R2_GLOBALS._player;
		} else {
			assert(R2_GLOBALS._sceneManager._sceneNumber == 2450);
			Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_companion;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4099, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker3375::animateSpeaker() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (scene->_companion1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3385::animateSpeaker() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3395::animateSpeaker() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3400::animateSpeaker() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_companion1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4030, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 5:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4033, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3600::animateSpeaker() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_seeker;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);

		_object1.setPosition(_object2->_position);

	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4031, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4030, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3700::setText(const Common::String &msg) {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	if (_speakerMode == 1) {
		R2_GLOBALS._sound2.play(44);
		scene->_miranda.setup(30, 8, 1);
	} else {
		scene->_miranda.setup(30, 2, 1);
	}

	VisualSpeaker::setText(msg);
}

void SpeakerSeeker3700::animateSpeaker() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_seeker;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		R2_GLOBALS._sound2.stop();
		scene->_quinn.setup(10, 8, 1);
		scene->_seeker.setup(20, 7, 1);
		scene->_miranda.setup(30, 8, 1);
		_object1.setup(3701, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		scene->_quinn.setup(10, 2, 1);
		scene->_seeker.setup(20, 1, 1);
		scene->_miranda.setup(30, 1, 1);
		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to SEEKERL
//----------------------------------------------------------------------------

SpeakerSeekerL::SpeakerSeekerL(): SpeakerSeeker() {
	_speakerName = "SEEKERL";
}

//----------------------------------------------------------------------------
// Classes related to SOCKO
//----------------------------------------------------------------------------

SpeakerSocko3200::SpeakerSocko3200() {
	_speakerName = "Socko";
	_color1 = 10;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSocko3200::animateSpeaker() {
	int v = _speakerMode;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_socko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to SOLDIER
//----------------------------------------------------------------------------

SpeakerSoldier::SpeakerSoldier(int color) {
	_speakerName = "SOLDIER";
	_color1 = color;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSoldier300::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_teal;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(303, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to TEAL
//----------------------------------------------------------------------------

SpeakerTeal::SpeakerTeal(): VisualSpeaker() {
	_speakerName = "TEAL";
	_color1 = 22;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

SpeakerTealMode7::SpeakerTealMode7(): SpeakerTeal() {
	_displayMode = 7;
}

void SpeakerTeal180::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_teal;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(75, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(77, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerTeal300::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_teal;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(303, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal1625::animateSpeaker() {
	int v = _speakerMode;

	if (!_object2) {
		Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_tealHead;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(Common::Point(68, 68));

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(1627, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal3240::animateSpeaker() {
	int v = _speakerMode;
	Scene3240 *scene = (Scene3240 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_teal;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4070, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal3400::animateSpeaker() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_teal;
		_object2->hide();
		_object1.postInit();
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}
	_object1.setPosition(_object2->_position);
	_object1.show();

	if (scene ->_sceneMode == 3305) {
		R2_GLOBALS._player.setStrip(6);
		scene->_companion1.setStrip(6);
		scene->_companion2.setStrip(6);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerTeal3600::animateSpeaker() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_teal;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}
	_object1.setPosition(_object2->_position);

	if (scene ->_sceneMode == 3323) {
		_object1.hide();
		_object2->show();
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4107, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to TOMKO
//----------------------------------------------------------------------------

SpeakerTomko3245::SpeakerTomko3245() {
	_speakerName = "Tomko";
	_color1 = 10;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerTomko3245::animateSpeaker() {
	int v = _speakerMode;
	Scene3245 *scene = (Scene3245 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_tomko;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		switch (_object2->_visage) {
		case 3100:
			_object1.setup(4105, (_object2->_strip * 2) - 1, 1);
			break;
		case 3101:
			_object1.setup(4108, (_object2->_strip * 2) - 1, 1);
			break;
		case 3102:
			_object1.setup(4109, (_object2->_strip * 2) - 1, 1);
			break;
		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to WEBBSTER
//----------------------------------------------------------------------------

SpeakerWebbster::SpeakerWebbster(int color) {
	_speakerName = "WEBBSTER";
	_color1 = color;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerWebbster180::animateSpeaker() {
	Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 6;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(75, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(76, 4, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(76, 6, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3240::animateSpeaker() {
	int v = _speakerMode;
	Scene3240 *scene = (Scene3240 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerWebbster3375::animateSpeaker() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (scene->_companion1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3385::animateSpeaker() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3395::animateSpeaker() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3400::animateSpeaker() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_webbster;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = EFFECT_SHADED;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((StripManager *)_action)->_useless = 0;

		_object1.setup(4110, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------

SpeakerDutyOfficer180::SpeakerDutyOfficer180(): VisualSpeaker() {
	_speakerName = "DUTYOFFICER";
	_color1 = 5;
	_color2 = 0;
	_speakerMode = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerDutyOfficer180::animateSpeaker() {
	Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;

	int v = _speakerMode;

	if (!_object2) {
		_object2 = &scene->_dutyOfficer;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		_action->_action = NULL;
		_object1.setup(76, 2, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
