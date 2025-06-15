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


#include "snatcher/graphics.h"
#include "snatcher/saveload.h"
#include "snatcher/script.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "engines/engine.h"
#include "graphics/scaler.h"
#include "graphics/surface.h"
#include "graphics/thumbnail.h"


#define SNATCHER_SAVEFILE_VERSION		0

namespace Snatcher {

SaveLoadManager::SaveLoadManager(SnatcherEngine *vm) : _vm(vm), _pendingSaveLoad(0), _enableSaving(true), _tryLoadFromLauncher(true), _desc() {
	ConfMan.registerDefault("controller_conf", 0);
	ConfMan.registerDefault("disable_stereo", false);
	ConfMan.registerDefault("lightgun_usage", false);
	ConfMan.registerDefault("lightgun_biasX", 0);
	ConfMan.registerDefault("lightgun_biasY", 0);
}

SaveLoadManager::~SaveLoadManager() {
}

void SaveLoadManager::loadSettings(Config &config) {
	if (ConfMan.hasKey("controller_conf"))
		config.controllerSetup = ConfMan.getInt("controller_conf");
	if (ConfMan.hasKey("disable_stereo"))
		config.disableStereo = ConfMan.getBool("disable_stereo") ? 1 : 0;
	if (ConfMan.hasKey("lightgun_usage"))
		config.useLightGun = ConfMan.getBool("lightgun_usage") ? 1 : 0;
	if (ConfMan.hasKey("lightgun_biasX"))
		config.lightGunBias.x = ConfMan.getInt("lightgun_biasX");
	if (ConfMan.hasKey("lightgun_biasY"))
		config.lightGunBias.y = ConfMan.getInt("lightgun_biasY");
	if (_tryLoadFromLauncher && ConfMan.hasKey("save_slot")) {
		requestLoad(ConfMan.getInt("save_slot"));
		_tryLoadFromLauncher = false;
	}
}

void SaveLoadManager::saveSettings(const Config &config) {
	ConfMan.setInt("controller_conf", config.controllerSetup);
	ConfMan.setBool("disable_stereo", config.disableStereo != 0);
	ConfMan.setBool("lightgun_usage", config.useLightGun != 0);
	ConfMan.setInt("lightgun_biasX", config.lightGunBias.x);
	ConfMan.setInt("lightgun_biasY", config.lightGunBias.y);
}

void SaveLoadManager::updateSaveSlotsState(GameState &state) {
	state.saveSlotUsage = 0;
	for (int i = 0; i < 4; ++i) {
		if (isSaveSlotUsed(i + 1)) // Skip autosave slot 0
			state.saveSlotUsage |= (1 << i);
	}
}

void SaveLoadManager::requestLoad(int slot) {
	_pendingSaveLoad = slot + 1;
}

void SaveLoadManager::requestSave(int slot, const Common::String &desc) {
	_pendingSaveLoad = -(slot + 1);
	_desc = desc;
}

void SaveLoadManager::handleSaveLoad(GameState &state) {
	if (_pendingSaveLoad > 0)
		loadState(_pendingSaveLoad - 1, state);
	else if (_pendingSaveLoad < 0)
		saveState(-_pendingSaveLoad - 1, state);
	_pendingSaveLoad = 0;
}

bool SaveLoadManager::isSaveSlotUsed(int16 slot) const {
	if (slot < 0 || slot > 999)
		return false;

	Common::String fileName = getSavegameFilename(slot, _vm->_targetName);
	Common::SeekableReadStream *in = _vm->_saveFileMan->openForLoading(fileName);
	if (!in)
		return false;

	delete in;
	return true;
}

bool SaveLoadManager::isSavingEnabled() const {
	return _enableSaving;
}

void SaveLoadManager::enableSaving(bool enable) {
	_enableSaving = enable;
}

Common::String SaveLoadManager::getSavegameFilename(int slot, const Common::String target) {
	assert(slot >= 0 && slot <= 999);
	return target + Common::String::format(".%03d", slot);
}

bool SaveLoadManager::readSaveHeader(Common::SeekableReadStream *saveFile, SaveHeader &header) {
	if (saveFile == nullptr || saveFile->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		return false;

	header.version = saveFile->readUint32BE();
	if (header.version > SNATCHER_SAVEFILE_VERSION)
		return false;
	header.platform = saveFile->readUint32BE();
	header.lang = saveFile->readUint32BE();
	header.desc = saveFile->readString('\0', saveFile->readUint32BE());

	Graphics::Surface *srf = nullptr;
	if (!Graphics::loadThumbnail(*saveFile, srf))
		return false;
	header.thumbnail = Common::SharedPtr<Graphics::Surface>(srf, Graphics::SurfaceDeleter());

	header.td.tm_sec = saveFile->readUint32BE();
	header.td.tm_min = saveFile->readUint32BE();
	header.td.tm_hour = saveFile->readUint32BE();
	header.td.tm_mday = saveFile->readUint32BE();
	header.td.tm_mon = saveFile->readUint32BE();
	header.td.tm_year = saveFile->readUint32BE();
	header.td.tm_wday = saveFile->readUint32BE();

	header.totalPlayTime = saveFile->readUint32BE();
	header.saveCount = saveFile->readSint16BE();

	return true;
}

void SaveLoadManager::loadState(int slot, GameState &state) {
	SaveHeader header;
	Common::InSaveFile *in = openFileForLoading(slot, state, header);

	_vm->_keyRepeat = in->readByte();
	_vm->_enableLightGun = in->readByte();

	state.chapter = in->readSint16BE();
	state.chapterSub = in->readSint16BE();
	state.counter = in->readSint16BE();
	state.finish = in->readSint16BE();
	state.frameNo = in->readSint16BE();
	state.frameState = in->readSint16BE();
	state.menuSelect = in->readSint16BE();
	state.modIndex = in->readSint16BE();
	state.modProcessSub = in->readSint16BE();
	state.modProcessTop = in->readSint16BE();
	state.prologue = in->readSint16BE();

	_vm->_scriptEngine->loadState(in, state.script);
	_vm->_cmdQueue->loadState(in);
	_vm->gfx()->loadState(in);
	_vm->sound()->loadState(in);

	if (in->readUint32BE() != MKTAG('S', 'N', 'A', 'T'))
		error("%s(): Save file invalid or corrupt", __FUNCTION__);

	delete in;

	if (state.prologue == -1) {
		state.chapter = 0;
		state.chapterSub = 0;
		state.menuSelect = 1;
	}
}

void SaveLoadManager::saveState(int slot, GameState &state) {
	Common::OutSaveFile *out = openFileForSaving(slot, _desc, state);

	out->writeByte(_vm->_keyRepeat);
	out->writeByte(_vm->_enableLightGun);

	out->writeSint16BE(state.chapter);
	out->writeSint16BE(state.chapterSub);
	out->writeSint16BE(state.counter);
	out->writeSint16BE(state.finish);
	out->writeSint16BE(state.frameNo);
	out->writeSint16BE(state.frameState);
	out->writeSint16BE(state.menuSelect);
	out->writeSint16BE(state.modIndex);
	out->writeSint16BE(state.modProcessSub);
	out->writeSint16BE(state.modProcessTop);
	out->writeSint16BE(state.prologue);

	_vm->_scriptEngine->saveState(out, state.script);
	_vm->_cmdQueue->saveState(out);
	_vm->gfx()->saveState(out);
	_vm->sound()->saveState(out);

	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));

	out->finalize();
	delete out;

	updateSaveSlotsState(state);
}

Common::SeekableReadStream *SaveLoadManager::openFileForLoading(int slot, GameState &state, SaveHeader &header) {
	Common::String fileName = getSavegameFilename(slot, _vm->_targetName);
	Common::SeekableReadStream *in = _vm->_saveFileMan->openForLoading(fileName);
	if (!in)
		error("%s(): Unable to open savegame file", __FUNCTION__);

	readSaveHeader(in, header);
	state.totalPlayTime = header.totalPlayTime;
	state.saveCount = header.saveCount;

	return in;
}

Common::OutSaveFile *SaveLoadManager::openFileForSaving(int slot, const Common::String &desc, GameState &state) {
	Common::String fileName = getSavegameFilename(slot, _vm->_targetName);
	Common::OutSaveFile *out = _vm->_saveFileMan->openForSaving(fileName);
	if (!out)
		error("%s(): Unable to create savegame file", __FUNCTION__);

	out->writeUint32BE(MKTAG('S', 'N', 'A', 'T'));
	out->writeUint32BE(SNATCHER_SAVEFILE_VERSION);
	out->writeUint32BE(_vm->_game.platform);
	out->writeUint32BE(_vm->_game.lang);
	out->writeUint32BE(desc.size());
	out->write(desc.c_str(), desc.size());

	Graphics::Surface tn;
	if (!::createThumbnailFromScreen(&tn))
		error("%s(): Unable to create thumbnail", __FUNCTION__);

	Graphics::saveThumbnail(*out, tn);

	TimeDate td;
	_vm->_system->getTimeAndDate(td);

	out->writeSint32BE(td.tm_sec);
	out->writeSint32BE(td.tm_min);
	out->writeSint32BE(td.tm_hour);
	out->writeSint32BE(td.tm_mday);
	out->writeSint32BE(td.tm_mon);
	out->writeSint32BE(td.tm_year);
	out->writeSint32BE(td.tm_wday);

	out->writeUint32BE(state.totalPlayTime);
	out->writeSint16BE(state.saveCount);

	return out;
}

} // End of namespace Snatcher
