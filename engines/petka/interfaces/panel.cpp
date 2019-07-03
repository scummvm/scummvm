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

#ifndef PETKA_STARTUP_H
#define PETKA_STARTUP_H

#include "common/config-manager.h"
#include "common/system.h"

#include "petka/objects/object_cursor.h"
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
const Common::Point kObjectsPoints[] = {{0, 2}, {5, 70}, {5, 136}, {22, 328},
									   {87, 224}, {118, 395}, {467, 71}, {432, 144},
									   {428, 29}, {434, 170}, {297, 214}, {470, 139},
									   {318, 87}, {468, 172}, {262, 31}, {231, 137},
									   {0, 0}, {0, 0}, {0, 0}, {0, 0},
									   {0, 0}, {0, 0}, {0, 0}, {0, 0}};

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

void InterfacePanel::start() {
	readSettings();

	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject(kPanelObjName);
	_objs.push_back(bg);
	g_vm->getQSystem()->update();
	const Common::Array<BGInfo> &infos = g_vm->getQSystem()->_mainInterface->_bgs;

	for (uint i = 0; i < infos.size(); ++i) {
		if (infos[i].objId != bg->_id) {
			continue;
		}
		for (uint j = 0; j < infos[i].attachedObjIds.size(); ++j) {
			QMessageObject *obj = g_vm->getQSystem()->findObject(infos[i].attachedObjIds[j]);
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			flc->setFrame(1);
			obj->_z = 1;
			obj->_x = kObjectsPoints[j].x;
			obj->_y = kObjectsPoints[j].y;
			obj->_field24 = 1;
			obj->_field20 = 1;
			obj->_field28 = 1;
			obj->_animate = 0;
			obj->_isShown = 1;
			_objs.push_back(obj);
		}
		break;
	}


	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	_objs.push_back(cursor);
	cursor->_resourceId = 4901;
	cursor->_isShown = 1;
	cursor->_animate = 1;
	cursor->setCursorPos(cursor->_x, cursor->_y, 0);

	updateSliders();
	updateSubtitles();

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->makeAllDirty();
}

void InterfacePanel::onLeftButtonDown(const Common::Point p) {
	int i = 0;
	for (i = _objs.size() - 1; i > 0; --i) {
		if (_objs[i]->isInPoint(p.x, p.y)) {
			break;
		}
	}
	switch (i) {
	case kNewGameButtonIndex:
		g_vm->loadPart(1);
		break;
	case kLoadButtonIndex:
		g_vm->getQSystem()->_saveLoadInterface->startSaveLoad(0);
		break;
	case kContinueButtonIndex:
		stop();
		break;
	case kExitButtonIndex:
		g_system->quit();
		break;
	case kSaveButtonIndex:
		g_vm->getQSystem()->_saveLoadInterface->startSaveLoad(1);
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

void InterfacePanel::onMouseMove(const Common::Point p) {
	bool found = false;
	for (uint i = _objs.size() - 1; i > 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		int frame = 1;
		if (!found && obj->isInPoint(p.x, p.y)) {
			found = true;
			if ((i >= kNewGameButtonIndex && i <= kSaveButtonIndex) || (i >= kDecSpeechButtonIndex && i <= kIncSpeedButtonIndex)) {
				frame = 2;
			}
		}
		if (obj->_field20 == frame)
			continue;
		obj->_field20 = frame;
		obj->_field24 = frame;

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
		FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
		flc->setFrame(frame);
		g_vm->videoSystem()->addDirtyRect(kObjectsPoints[pointIndex], *flc);
	}
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_isShown = 1;
	cursor->setCursorPos(p.x, p.y, 0);
}

void InterfacePanel::updateSliders() {
	applySettings();

	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_objs[kSpeechVolumeSliderIndex]->_resourceId);
	flc->setFrame(_speechFrame);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kSpeechVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->loadFlic(_objs[kMusicVolumeSliderIndex]->_resourceId);
	flc->setFrame(_musicFrame);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kMusicVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->loadFlic(_objs[kSfxVolumeSliderIndex]->_resourceId);
	flc->setFrame(_sfxFrame);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kSfxVolumeSliderIndex - 1], *flc);

	flc = g_vm->resMgr()->loadFlic(_objs[kSpeedSliderIndex]->_resourceId);
	flc->setFrame(_speedFrame);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kSpeedSliderIndex - 1], *flc);
}

void InterfacePanel::updateSubtitles() {
	applySettings();
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_objs[kSubtitleButtonIndex]->_resourceId);
	flc->setFrame(_subtitles == 0 ? 1 : 7);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kSubtitleButtonIndex - 1], *flc);

	flc = g_vm->resMgr()->loadFlic(_objs[kSubtitleLabelIndex]->_resourceId);
	flc->setFrame(_subtitles == 0 ? 1 : 2);
	g_vm->videoSystem()->addDirtyRect(kObjectsPoints[kSubtitleLabelIndex - 1], *flc);
}

void InterfacePanel::readSettings() {
	_speechFrame = 1 + 30 * ConfMan.getInt("speech_volume") / 255;
	_musicFrame = 1 + 40 * ConfMan.getInt("music_volume") / 255;
	_sfxFrame = 1 + 30 * ConfMan.getInt("sfx_volume") / 255;
	_subtitles = ConfMan.getBool("subtitles");
	//_speedFrame = 1 + ConfMan.getInt("petka_speed") / 4;
}

void InterfacePanel::applySettings() {
	_speechFrame = MIN(MAX(1, _speechFrame), 31);
	_musicFrame = MIN(MAX(1, _musicFrame), 41);
	_sfxFrame = MIN(MAX(1, _sfxFrame), 31);
	_speedFrame = MIN(MAX(1, _speedFrame), 26);

	ConfMan.setInt("speech_volume", 255 * (_speechFrame - 1) / (31 - 1));
	ConfMan.setInt("music_volume", 255 * (_musicFrame - 1) / (41 - 1));
	ConfMan.setInt("sfx_volume", 255 * (_sfxFrame - 1) / (31 - 1));
	ConfMan.setBool("subtitles", _subtitles);
	//ConfMan.setInt("petka_speed", 4 * (_speedFrame - 1));
	ConfMan.flushToDisk();
	g_vm->syncSoundSettings();
}

} // End of namespace Petka

#endif

