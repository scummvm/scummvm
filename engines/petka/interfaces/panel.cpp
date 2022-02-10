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

namespace {

// ПАНЕЛЬ УПРАВЛЕНИЯ
const char *const kPanelObjName = "\xCF\xC0\xCD\xC5\xCB\xDC\x20\xD3\xCF\xD0\xC0\xC2\xCB\xC5\xCD\xC8\xDF";

const uint kNewGameButtonIndex = 1;
const uint kLoadButtonIndex = 2;
const uint kContinueButtonIndex = 3;
const uint kExitButtonIndex = 4;
const uint kSaveButtonIndex = 5;
const uint kSafeObjectIndex = 6;
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

Common::Point getObjectPos(uint index)
{
	switch (index) {
	case kNewGameButtonIndex:
		return {0, 2};
	case kLoadButtonIndex:
		return {5, 70};
	case kContinueButtonIndex:
		return {5, 136};
	case kExitButtonIndex:
		return {22, 328};
	case kSaveButtonIndex:
		return {87, 224};
	case kSafeObjectIndex:
		return {118, 395};
	case kSfxLabelIndex:
		return {467, 71};
	case kSubtitleButtonIndex:
		return {432, 144};
	case kSfxVolumeSliderIndex:
		return {428, 29};
	case kSpeedSliderIndex:
		return {434, 170};
	case kMusicLabelIndex:
		return {297, 214};
	case kSubtitleLabelIndex:
		return {470, 139};
	case kSpeechLabelIndex:
		return {318, 87};
	case kSpeedLabelIndex:
		return {468, 172};
	case kSpeechVolumeSliderIndex:
		return {262, 31};
	case kMusicVolumeSliderIndex:
		return {231, 137};
	// case kDecSpeechButtonIndex:
	// case kIncSpeechButtonIndex:
	// case kDecMusicButtonIndex:
	// case kIncMusicButtonIndex:
	// case kDecSfxButtonIndex:
	// case kIncSfxButtonIndex:
	// case kDecSpeedButtonIndex:
	// case kIncSpeedButtonIndex:
	default:
		return {0, 0};
	}
}

}

namespace Petka {

InterfacePanel::InterfacePanel() {
	readSettings();
}

void InterfacePanel::start(int id) {
	QSystem *sys = g_vm->getQSystem();

	sys->getCase()->show(false);

	g_vm->videoSystem()->makeAllDirty();
	g_vm->videoSystem()->update();

	InterfaceSaveLoad::saveScreen();

	QObjectBG *bg = (QObjectBG *)sys->findObject(kPanelObjName);
	const BGInfo *info = sys->_mainInterface->findBGInfo(bg->_id);

	_objs.reserve(info->attachedObjIds.size() + 1);
	_objs.push_back(bg);

	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = sys->findObject(info->attachedObjIds[i]);
		FlicDecoder *flc = g_vm->resMgr()->getFlic(obj->_resourceId);
		flc->setFrame(1);

		const auto pos = getObjectPos(i + 1);
		obj->_x = pos.x;
		obj->_y = pos.y;
		obj->_z = 1;
		obj->_frame = 1;
		obj->_animate = false;
		_objs.push_back(obj);
	}

	SubInterface::start(id);
	onSettingsChanged();
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

		switch (i) {
		case kDecSpeechButtonIndex:
		case kIncSpeechButtonIndex:
			updateSprite(kSpeechLabelIndex, frame);
			break;
		case kDecMusicButtonIndex:
		case kIncMusicButtonIndex:
			updateSprite(kMusicLabelIndex, frame);
			break;
		case kDecSfxButtonIndex:
		case kIncSfxButtonIndex:
			updateSprite(kSfxLabelIndex, frame);
			break;
		case kIncSpeedButtonIndex:
		case kDecSpeedButtonIndex:
			updateSprite(kSpeedLabelIndex, frame);
			break;
		default:
			updateSprite(i, frame);
			break;
		}
	}
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_isShown = true;
	cursor->setPos(p, false);
}

void InterfacePanel::updateSliders() {
	applySettings();

	updateSprite(kSpeechVolumeSliderIndex, _speechFrame);
	updateSprite(kMusicVolumeSliderIndex, _musicFrame);
	updateSprite(kSfxVolumeSliderIndex, _sfxFrame);
	updateSprite(kSpeedSliderIndex, _speedFrame);
}

void InterfacePanel::updateSubtitles() {
	applySettings();

	updateSprite(kSubtitleButtonIndex, _subtitles == 0 ? 1 : 7);
	updateSprite(kSubtitleLabelIndex, _subtitles == 0 ? 1 : 2);
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

	const auto speechFrame = _speechFrame;
	const auto musicFrame = _musicFrame;
	const auto sfxFrame = _sfxFrame;
	const auto speedFrame = _speedFrame;
	const auto subtitles = _subtitles;

	readSettings();

	if (speechFrame != _speechFrame) {
		ConfMan.setInt("speech_volume", 255 * (speechFrame - 1) / (31 - 1));
	}
	if (musicFrame != _musicFrame) {
		ConfMan.setInt("music_volume", 255 * (musicFrame - 1) / (41 - 1));
	}
	if (sfxFrame != _sfxFrame) {
		ConfMan.setInt("sfx_volume", 255 * (sfxFrame - 1) / (31 - 1));
	}

	ConfMan.setBool("subtitles", subtitles);

	if (speedFrame != _speedFrame) {
		ConfMan.setInt("petka_speed", 4 * (speedFrame - 1));
	}

	readSettings();

	ConfMan.flushToDisk();
	g_vm->syncSoundSettings();
}

void InterfacePanel::onRightButtonDown(Common::Point p) {
	stop();
}

void InterfacePanel::updateSprite(uint index, uint frame) const {
	const auto *object = (QMessageObject *)(_objs[index]);
	FlicDecoder *flc = g_vm->resMgr()->getFlic(object->_resourceId);
	flc->setFrame(frame);
	g_vm->videoSystem()->addDirtyRect(Common::Point(object->_x, object->_y), *flc);
}

int InterfacePanel::getHeroSpeed() {
	return (_speedFrame * 100 - 100) / 25;
}

void InterfacePanel::onSettingsChanged() {
	readSettings();
	updateSliders();
	updateSubtitles();
}

} // End of namespace Petka
