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

#include "common/config-manager.h"
#include "common/system.h"

#include "petka/objects/object_cursor.h"
#include "petka/objects/object_case.h"
#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/flc.h"
#include "petka/video.h"
#include "petka/q_manager.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/save_load.h"
#include "petka/interfaces/panel.h"

namespace Petka {

// ПАНЕЛЬ УПРАВЛЕНИЯ
const char *const kPanelObjName = "\xCF\xC0\xCD\xC5\xCB\xDC\x20\xD3\xCF\xD0\xC0\xC2\xCB\xC5\xCD\xC8\xDF";

const uint kNewGameButtonIndex = 1;
const uint kLoadButtonIndex = 2;
const uint kContinueButtonIndex = 3;
const uint kExitButtonIndex = 4;
const uint kSaveButtonIndex = 5;
const uint kSfxLabelIndex = 7;
const uint kSubtitleButtonIndex = 8;
const uint kSfxVolumeSliderIndex = 9;
const uint kSpeedSliderIndex = 10;
const uint kMusicLabelIndex = 11;
const uint kSubtitleLabelIndex = 12;
const uint kSpeechLabelIndex = 13;
const uint kSpeedLabelIndex = 14;
const uint kSpeechVolumeSliderIndex = 15;
const uint kMusicVolumeSliderIndex = 16;
const uint kDecSpeechButtonIndex = 17;
const uint kIncSpeechButtonIndex = 18;
const uint kDecMusicButtonIndex = 19;
const uint kIncMusicButtonIndex = 20;
const uint kDecSfxButtonIndex = 21;
const uint kIncSfxButtonIndex = 22;
const uint kDecSpeedButtonIndex = 23;
const uint kIncSpeedButtonIndex = 24;

InterfacePanel::InterfacePanel() {
	_objectPoints[0] = Common::Point(0, 2);
	_objectPoints[1] = Common::Point(5, 70);
	_objectPoints[2] = Common::Point(5, 136);
	_objectPoints[3] = Common::Point(22, 328);
	_objectPoints[4] = Common::Point(87, 224);
	_objectPoints[5] = Common::Point(118, 395);
	_objectPoints[6] = Common::Point(467, 71);
	_objectPoints[7] = Common::Point(432, 144);
	_objectPoints[8] = Common::Point(428, 29);
	_objectPoints[9] = Common::Point(434, 170);
	_objectPoints[10] = Common::Point(297, 214);
	_objectPoints[11] = Common::Point(470, 139);
	_objectPoints[12] = Common::Point(318, 87);
	_objectPoints[13] = Common::Point(468, 172);
	_objectPoints[14] = Common::Point(262, 31);
	_objectPoints[15] = Common::Point(231, 137);
	_objectPoints[16] = Common::Point(0, 0);
	_objectPoints[17] = Common::Point(0, 0);
	_objectPoints[18] = Common::Point(0, 0);
	_objectPoints[19] = Common::Point(0, 0);
	_objectPoints[20] = Common::Point(0, 0);
	_objectPoints[21] = Common::Point(0, 0);
	_objectPoints[22] = Common::Point(0, 0);
	_objectPoints[23] = Common::Point(0, 0);
	readSettings();
}

void InterfacePanel::start(int id) {
	QSystem *sys = g_vm->getQSystem();
	readSettings();

	sys->getCase()->show(false);

	g_vm->videoSystem()->makeAllDirty();
	g_vm->videoSystem()->update();

	InterfaceSaveLoad::saveScreen();

	QObjectBG *bg = (QObjectBG *)sys->findObject(kPanelObjName);
	_objs.push_back(bg);

	const BGInfo *info = sys->_mainInterface->findBGInfo(bg->_id);
	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = sys->findObject(info->attachedObjIds[i]);
		FlicDecoder *flc = g_vm->resMgr()->getFlic(obj->_resourceId);
		flc->setFrame(1);
		obj->_z = 1;
		obj->_x = _objectPoints[i].x;
		obj->_y = _objectPoints[i].y;
		obj->_frame = 1;
		obj->_animate = false;
		obj->_isShown = true;
		_objs.push_back(obj);
	}

	SubInterface::start(id);

	updateSliders();
	updateSubtitles();

	sys->getCursor()->_animate = true;
}

void InterfacePanel::onLeftButtonDown(Common::Point p) {
	int i = 0;
	for (i = _objs.size() - 1; i > 0; --i) {
		if (_objs[i]->isInPoint(p)) {
			break;
		}
	}
	switch (i) {
	case kNewGameButtonIndex:
		g_vm->loadPart(1);
		break;
	case kLoadButtonIndex:
		stop();
		g_vm->getQSystem()->_saveLoadInterface->start(kLoadMode);
		break;
	case kContinueButtonIndex:
		stop();
		break;
	case kExitButtonIndex:
		g_system->quit();
		break;
	case kSaveButtonIndex:
		stop();
		g_vm->getQSystem()->_saveLoadInterface->start(kSaveMode);
		break;
	case kSubtitleButtonIndex:
		_subtitles = !_subtitles;
		updateSubtitles();
		break;
	case kDecSpeechButtonIndex:
		_speechFrame -= 5;
		updateSliders();
		break;
	case kIncSpeechButtonIndex:
		_speechFrame += 5;
		updateSliders();
		break;
	case kDecMusicButtonIndex:
		_musicFrame -= 5;
		updateSliders();
		break;
	case kIncMusicButtonIndex:
		_musicFrame += 5;
		updateSliders();
		break;
	case kDecSfxButtonIndex:
		_sfxFrame -= 5;
		updateSliders();
		break;
	case kIncSfxButtonIndex:
		_sfxFrame += 5;
		updateSliders();
		break;
	case kDecSpeedButtonIndex:
		_speedFrame -= 5;
		updateSliders();
		break;
	case kIncSpeedButtonIndex:
		_speedFrame += 5;
		updateSliders();
		break;
	default:
		break;
	}
}

void InterfacePanel::onMouseMove(Common::Point p) {
	bool found = false;
	for (uint i = _objs.size() - 1; i > 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		byte frame = 1;
		if (!found && obj->isInPoint(p)) {
			found = true;
			if ((i >= kNewGameButtonIndex && i <= kSaveButtonIndex) || (i >= kDecSpeechButtonIndex && i <= kIncSpeedButtonIndex)) {
				frame = 2;
			}
		}
		if (obj->_frame == frame)
			continue;
		obj->_frame = frame;

		int pointIndex;
		switch (i) {
		case kDecSpeechButtonIndex:
		case kIncSpeechButtonIndex:
			pointIndex = kSpeechLabelIndex - 1;
			obj = (QMessageObject *)_objs[kSpeechLabelIndex];
			break;
		case kDecMusicButtonIndex:
		case kIncMusicButtonIndex:
			pointIndex = kMusicLabelIndex - 1;
			obj = (QMessageObject *)_objs[kMusicLabelIndex];
			break;
		case kDecSfxButtonIndex:
		case kIncSfxButtonIndex:
			pointIndex = kSfxLabelIndex - 1;
			obj = (QMessageObject *)_objs[kSfxLabelIndex];
			break;
		case kIncSpeedButtonIndex:
		case kDecSpeedButtonIndex:
			pointIndex = kSpeedLabelIndex - 1;
			obj = (QMessageObject *)_objs[kSpeedLabelIndex];
			break;
		default:
			pointIndex = i - 1;
			break;
		}
		FlicDecoder *flc = g_vm->resMgr()->getFlic(obj->_resourceId);
		flc->setFrame(frame);
		g_vm->videoSystem()->addDirtyRect(_objectPoints[pointIndex], *flc);
	}
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_isShown = true;
	cursor->setPos(p, false);
}

void InterfacePanel::updateSliders() {
	applySettings();

	FlicDecoder *flc = g_vm->resMgr()->getFlic(_objs[kSpeechVolumeSliderIndex]->_resourceId);
	flc->setFrame(_speechFrame);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kSpeechVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->getFlic(_objs[kMusicVolumeSliderIndex]->_resourceId);
	flc->setFrame(_musicFrame);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kMusicVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->getFlic(_objs[kSfxVolumeSliderIndex]->_resourceId);
	flc->setFrame(_sfxFrame);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kSfxVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->getFlic(_objs[kSpeedSliderIndex]->_resourceId);
	flc->setFrame(_speedFrame);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kSpeedSliderIndex - 1], *flc);
}

void InterfacePanel::updateSubtitles() {
	applySettings();
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_objs[kSubtitleButtonIndex]->_resourceId);
	flc->setFrame(_subtitles == 0 ? 1 : 7);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kSubtitleButtonIndex - 1], *flc);

	flc = g_vm->resMgr()->getFlic(_objs[kSubtitleLabelIndex]->_resourceId);
	flc->setFrame(_subtitles == 0 ? 1 : 2);
	g_vm->videoSystem()->addDirtyRect(_objectPoints[kSubtitleLabelIndex - 1], *flc);
}

void InterfacePanel::readSettings() {
	_speechFrame = 1 + 30 * ConfMan.getInt("speech_volume") / 255;
	_musicFrame = 1 + 40 * ConfMan.getInt("music_volume") / 255;
	_sfxFrame = 1 + 30 * ConfMan.getInt("sfx_volume") / 255;
	_subtitles = ConfMan.getBool("subtitles");
	_speedFrame = 1 + ConfMan.getInt("petka_speed") / 4;
}

void InterfacePanel::applySettings() {
	_speechFrame = CLIP<int>(_speechFrame, 1, 31);
	_musicFrame = CLIP<int>(_musicFrame, 1, 41);
	_sfxFrame = CLIP<int>(_sfxFrame, 1, 31);
	_speedFrame = CLIP<int>(_speedFrame, 1, 26);

	ConfMan.setInt("speech_volume", 255 * (_speechFrame - 1) / (31 - 1));
	ConfMan.setInt("music_volume", 255 * (_musicFrame - 1) / (41 - 1));
	ConfMan.setInt("sfx_volume", 255 * (_sfxFrame - 1) / (31 - 1));
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.setInt("petka_speed", 4 * (_speedFrame - 1));
	ConfMan.flushToDisk();
	g_vm->syncSoundSettings();
}

void InterfacePanel::onRightButtonDown(Common::Point p) {
	stop();
}

int InterfacePanel::getHeroSpeed() {
	return (_speedFrame * 100 - 100) / 25;
}

} // End of namespace Petka
