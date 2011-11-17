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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

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

namespace TsAGE {

namespace Ringworld2 {

VisualSpeaker::VisualSpeaker(): Speaker() {
	_delayAmount = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
	_color1 = 8;
	_color2 = 0;
	_displayMode = 0;
	_fieldF6 = 0;
}

void VisualSpeaker::remove() {
	if (_object2) {
		if (_fieldF8) {
			_fieldF8 = 0;
			_object1.setStrip(_object1._strip - 1);
			_object1.setFrame(_object1.getFrameCount());
			_object1.animate(ANIM_MODE_6, (_fieldF6 == 0xff) ? this : NULL);
		} else {
			_object1.animate(ANIM_MODE_6, (_fieldF6 == 0xff) ? this : NULL);
		}
	}

	Speaker::remove();
}

void VisualSpeaker::synchronize(Serializer &s) {
	Speaker::synchronize(s);

	SYNC_POINTER(_object2);
	s.syncAsSint16LE(_fieldF6);
	s.syncAsSint16LE(_fieldF8);
	s.syncAsSint16LE(_displayMode);
	s.syncAsSint16LE(_soundId);
	s.syncAsSint16LE(_delayAmount);
	s.syncAsByte(_removeObject);
	s.syncAsSint32LE(_frameNumber);
	s.syncAsSint16LE(_numFrames);
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
		_soundId = atoi(msg.c_str());

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
	_sceneText.setup(msg);

	//_sceneText.clone();

	_sceneText.setPosition(_textPos);
	_sceneText.setPriority(0x100);

	// If subtitles are turned off, don't show the text
	if (!(R2_GLOBALS._speechSubtitles & 1)) {
		_sceneText.hide();
	}

	// Figure out the text delay if subtitles are turned on, or there's no speech resource specified
	if ((R2_GLOBALS._speechSubtitles & 1) || !_soundId) {
		const char *msgP = s.c_str();
		int numWords = 0;
		while (*msgP != '\0') {
			if (*msgP++ == ' ')
				++numWords;
		}

		if (!numWords && !s.empty())
			++numWords;

		_numFrames = numWords * 30 + 120;
		setFrame(_numFrames);
	} else {
		_numFrames = 1;
	}

	// If the text is empty, no delay is needed
	if (s.empty())
		_numFrames = 0;

	
	if (_fieldF6) {
		if ((R2_GLOBALS._speechSubtitles & 1) || !_soundId)
			_sceneText.hide();
	} else {
		if ((R2_GLOBALS._speechSubtitles & 2) && _soundId) {
			if (!R2_GLOBALS._playStream.play(_soundId, NULL))
				_sceneText.show();
		}
	}
}

void VisualSpeaker::proc16() {
	R2_GLOBALS._playStream.stop();
	_fieldF6 = 0;
	_object1.remove();

	assert(_object2);
	_object2->show();
	_object2 = NULL;
	_fieldF8 = 0;
}

void VisualSpeaker::setFrame(int numFrames) {
	_delayAmount = numFrames;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

/*--------------------------------------------------------------------------*/

SpeakerMiranda300::SpeakerMiranda300(): VisualSpeaker() {
	_speakerName = "MIRANDA";
	_color1 = 154;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerMiranda300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
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
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;

		if (v == 4) {
			_object1.setup(304, 5, 1);
		} else {
			_object1.setup(305, v * 2 - 1, 1);
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

/*--------------------------------------------------------------------------*/

SpeakerSeeker300::SpeakerSeeker300(): VisualSpeaker() {
	_speakerName = "SEEKER";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSeeker300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
			_object2 = &R2_GLOBALS._player;
		} else {
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
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(306, v * 2 - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

/*--------------------------------------------------------------------------*/

SpeakerSeekerL::SpeakerSeekerL(): VisualSpeaker() {
	_speakerName = "SEEKERL";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
	_fontNumber = 10;
};

/*--------------------------------------------------------------------------*/

SpeakerQuinnL::SpeakerQuinnL(): VisualSpeaker() {
	_speakerName = "QUINNL";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
	_fontNumber = 10;
};

/*--------------------------------------------------------------------------*/

SpeakerQuinn300::SpeakerQuinn300(): VisualSpeaker() {
	_speakerName = "QUINN";
	_color1 = 60;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerQuinn300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_quinn;
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
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		
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
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

/*--------------------------------------------------------------------------*/

SpeakerTeal300::SpeakerTeal300(): VisualSpeaker() {
	_speakerName = "TEAL";
	_color1 = 22;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerTeal300::proc15() {
	int v = _fieldF6;

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
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(303, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}


/*--------------------------------------------------------------------------*/

SpeakerSoldier300::SpeakerSoldier300(): VisualSpeaker() {
	_speakerName = "SOLDIER";
	_color1 = 60;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSoldier300::proc15() {
	int v = _fieldF6;

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
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(303, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}
} // End of namespace Ringworld2

} // End of namespace TsAGE
