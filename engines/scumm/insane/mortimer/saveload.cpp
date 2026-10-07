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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/insane/mortimer/mortimer.h"
#include "scumm/insane/mortimer/dialogs.h"
#include "scumm/insane/saveload.h"
#include "scumm/scumm.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "engines/metaengine.h"
#include "gui/message.h"

namespace Scumm {

const uint32 kMortimerPilotMagic = MKTAG('M', 'O', 'R', 'P');
const uint16 kMortimerPilotVersion = 1;

Common::String MortimerEngine::getSaveStateName(int slot) const {
	return getMetaEngine()->getSavegameFile(slot, _targetName.c_str());
}

bool MortimerEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return !_isDemo && _screen.getPixels();
}

bool MortimerEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	if (msg && !_isDemo)
		*msg = _("Progress is saved automatically when a riddle is solved.");
	return false;
}

Common::Error MortimerEngine::savePilot() {
	if (_isDemo || _pilotSlot < 0)
		return Common::kNoError;
	Common::ScopedPtr<Common::OutSaveFile> out(_saveFileMan->openForSaving(getSaveStateName(_pilotSlot), false));
	if (!out || !writeInsaneSavegameHeader(out.get(), _pilotName, getTotalPlayTime()))
		return Common::kWritingFailed;
	out->writeUint32BE(kMortimerPilotMagic);
	out->writeUint16LE(kMortimerPilotVersion);
	out->writeByte(_girl);
	out->writeByte(_flightDifficulty);
	out->writeByte(_riddleDifficulty);
	out->writeByte(_checkpoint);
	out->writeByte(ConfMan.getBool("subtitles"));
	out->writeByte(_controllerMode);
	out->finalize();
	return out->err() ? Common::kWritingFailed : Common::kNoError;
}

Common::Error MortimerEngine::loadGameState(int slot) {
	if (_isDemo || slot < 0 || slot > getMetaEngine()->getMaximumSaveSlot())
		return Common::kReadingFailed;
	Common::ScopedPtr<Common::InSaveFile> in(_saveFileMan->openForLoading(getSaveStateName(slot)));
	Common::String name;
	SaveStateMetaInfos infos;
	if (!in || !readInsaneSavegameHeader(in.get(), &name, infos) ||
		in->readUint32BE() != kMortimerPilotMagic || in->readUint16LE() != kMortimerPilotVersion)
		return Common::kReadingFailed;
	const byte girl = in->readByte();
	const byte flight = in->readByte(), riddle = in->readByte(), checkpoint = in->readByte();
	const byte subtitles = in->readByte(), controller = in->readByte();
	if (in->err() || in->eos() || name.empty() || girl > 1 || flight > 2 || riddle > 2 ||
		checkpoint > 5 || subtitles > 1 || controller > 1)
		return Common::kReadingFailed;

	_pilotName = name;
	_pilotSlot = slot;
	_checkpoint = checkpoint;
	_girl = girl;
	_controllerMode = controller;
	ConfMan.setBool("subtitles", subtitles);
	setDifficulty(flight, riddle);
	setTotalPlayTime(infos.playtime * 1000);
	// Leave the menu before switching movies, as in RA2's pilot loading.
	_pilotLoadRequested = true;
	return Common::kNoError;
}

void MortimerEngine::saveCheckpoint(uint checkpoint) {
	if (_isDemo || _pilotSlot < 0 || checkpoint <= _checkpoint)
		return;
	_checkpoint = checkpoint;
	if (savePilot().getCode() != Common::kNoError) {
		GUI::MessageDialog message(_("Could not save your pilot. Progress will be saved again when you finish another riddle or quit."));
		runDialog(message);
	}
}

bool MortimerEngine::selectPilot() {
	while (!shouldQuit()) {
		const SaveStateList pilots = getMetaEngine()->listSaves(_targetName.c_str());
		MortimerPilotDialog dialog(pilots);
		if (runDialog(dialog) != 1 || shouldQuit())
			return false;
		if (dialog.selectedPilot() != MortimerPilotDialog::kNewPilot) {
			const Common::Error result = loadGameState(dialog.selectedPilot());
			if (result.getCode() == Common::kNoError)
				return true;
			GUI::MessageDialog message(result.getTranslatedDesc());
			runDialog(message);
			continue;
		}
		int slot = 1;
		while (slot <= getMetaEngine()->getMaximumSaveSlot() && _saveFileMan->exists(getSaveStateName(slot)))
			++slot;
		if (slot > getMetaEngine()->getMaximumSaveSlot()) {
			GUI::MessageDialog message(_("There are no free pilot slots. Delete a saved pilot from the launcher before creating another."));
			runDialog(message);
			continue;
		}
		MortimerDifficultyDialog options(false, 0, 0, false, true);
		if (runDialog(options) != 1 || shouldQuit())
			continue;
		_pilotName = options.pilotName();
		_pilotSlot = slot;
		_checkpoint = 0;
		_girl = options.girl();
		_controllerMode = false;
		setDifficulty(options.flightDifficulty(), options.riddleDifficulty());
		setTotalPlayTime();
		const Common::Error result = savePilot();
		if (result.getCode() == Common::kNoError) {
			_pilotLoadRequested = true;
			return true;
		}
		_saveFileMan->removeSavefile(getSaveStateName(slot));
		_pilotSlot = -1;
		GUI::MessageDialog message(result.getTranslatedDesc());
		runDialog(message);
	}
	return false;
}

bool MortimerEngine::startPilot() {
	_pilotLoadRequested = false;
	stopSounds();
	_loadError.clear();
	_loadedRegion = -1;
	_finished = _interviewActive = _castleActive = _playingCached = false;
	_crashStage = kNoCrash;
	_needsRepair = _hitJolt = _invulnerable = false;
	_shockFrame = _crashCount = _shellShocks = 0;
	_carriedStones = _completedSpecies = _creditedRescues = 0;
	_uiFrame = _glowStones = _pendingGlowStones = 0;
	_shotFrame = 3;
	_glowFrame = 10;
	_pendingFlight.clear();
	_flightName.clear();
	memset(_steering, 0, sizeof(_steering));
	memset(_iconFlash, 0, sizeof(_iconFlash));
	for (uint i = 0; i < ARRAYSIZE(_steeringHistory); ++i)
		_steeringHistory[i] = Common::Point();
	_flight.clearProgress();
	_castle.clearProgress();
	_intro.context() = MortimerMovieContext();
	_intro.context().girl = _flight.context().girl = _girl;
	_interview.resetRiddle();
	_programmer = ConfMan.getBool("mortimer_programmer_easter_egg");
	// SHELL's region entry table resumes the castle at END_1. Completed pilots show credits.
	const int scenes[] = {24, 1, 2, 3, 34, 39};
	return _campaign.start(scenes[_checkpoint]) && enterCampaignScene();
}

} // End of namespace Scumm
