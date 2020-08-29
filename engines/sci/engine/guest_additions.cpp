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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/gui_options.h"
#include "common/savefile.h"
#include "sci/engine/features.h"
#include "sci/engine/file.h"
#include "sci/engine/guest_additions.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#ifdef ENABLE_SCI32
#include "common/translation.h"
#include "gui/saveload.h"
#include "sci/graphics/frameout.h"
#endif
#include "sci/sound/music.h"
#include "sci/sci.h"

namespace Sci {

enum {
	kSoundsMusicType = 0,
	kSoundsSoundType = 1
};

enum {
	kMessageTypeSubtitles = 1,
	kMessageTypeSpeech    = 2
};

GuestAdditions::GuestAdditions(EngineState *state, GameFeatures *features, Kernel *kernel) :
	_state(state),
	_features(features),
	_kernel(kernel),
	_segMan(state->_segMan),
#ifdef ENABLE_SCI32
	_restoring(false),
#endif
	_messageTypeSynced(false) {}

#pragma mark -

void GuestAdditions::syncSoundSettingsFromScummVM() const {
	if (_features->audioVolumeSyncUsesGlobals())
		syncAudioVolumeGlobalsFromScummVM();
	else
		syncMasterVolumeFromScummVM();
}

void GuestAdditions::syncAudioOptionsFromScummVM() const {
#ifdef ENABLE_SCI32
	if (_features->supportsTextSpeed()) {
		syncTextSpeedFromScummVM();
	}
#endif
	syncMessageTypeFromScummVM();
}

void GuestAdditions::reset() {
	_messageTypeSynced = false;
}

void GuestAdditions::invokeSelector(const reg_t objId, const Selector selector, const int argc, const StackPtr argv) const {
	::Sci::invokeSelector(_state, objId, selector, 0, _state->_executionStack.back().sp, argc, argv);
}

bool GuestAdditions::shouldSyncAudioToScummVM() const {
	const SciGameId gameId = g_sci->getGameId();
	Common::List<ExecStack>::const_iterator it;
	for (it = _state->_executionStack.begin(); it != _state->_executionStack.end(); ++it) {
		const ExecStack &call = *it;
		const Common::String objName = _segMan->getObjectName(call.sendp);

		if (getSciVersion() < SCI_VERSION_2 && (objName == "TheMenuBar" ||
												objName == "MenuBar")) {
			// SCI16 with menu bar
			return true;
		} else if (objName == "volumeSlider") {
			// SCI16 with icon bar, QFG4, Hoyle5, RAMA
			return true;
		} else if (gameId == GID_MOTHERGOOSE256 && objName == "soundBut") {
			return true;
		} else if (gameId == GID_SLATER && objName == "volButton") {
			return true;
		} else if (gameId == GID_LSL6 && (objName == "menuBar" ||
										  objName == "volumeDial")) {
			return true;
#ifdef ENABLE_SCI32
		} else if ((gameId == GID_GK1 || gameId == GID_SQ6) && (objName == "musicBar" ||
																objName == "soundBar")) {
			return true;
		} else if (gameId == GID_GK2 && objName == "soundSlider") {
			return true;
		} else if (gameId == GID_HOYLE5 && objName == "volumeSliderF") {
			// Hoyle5 has a second control panel with a different slider name
			return true;
		} else if (gameId == GID_KQ7 && (objName == "volumeUp" ||
										 objName == "volumeDown")) {
			return true;
		} else if (gameId == GID_LSL6HIRES && (objName == "hiResMenu" ||
											   objName == "volumeDial")) {
			return true;
		} else if ((gameId == GID_LSL7 || gameId == GID_TORIN) && (objName == "oMusicScroll" ||
																   objName == "oSFXScroll" ||
																   objName == "oAudioScroll")) {
			return true;
		} else if (gameId == GID_MOTHERGOOSEHIRES && objName == "MgButtonBar") {
			return true;
		} else if (gameId == GID_PHANTASMAGORIA && (objName == "midiVolDown" ||
													objName == "midiVolUp" ||
													objName == "dacVolDown" ||
													objName == "dacVolUp")) {
			return true;
		} else if (gameId == GID_PHANTASMAGORIA2 && objName == "foo2") {
			return true;
		} else if (gameId == GID_PQ4 && (objName == "increaseVolume" ||
										 objName == "decreaseVolume")) {
			return true;
		} else if (gameId == GID_PQSWAT && (objName == "volumeDownButn" ||
											objName == "volumeUpButn")) {
			return true;
		} else if (gameId == GID_SHIVERS && objName == "spVolume") {
			return true;
#endif
		}
	}

	return false;
}

#pragma mark -
#pragma mark Hooks

void GuestAdditions::sciEngineRunGameHook() {
	_messageTypeSynced = true;
}

void GuestAdditions::writeVarHook(const int type, const int index, const reg_t value) {
	if (type == VAR_GLOBAL) {
		if (_features->audioVolumeSyncUsesGlobals() && shouldSyncAudioToScummVM()) {
			syncAudioVolumeGlobalsToScummVM(index, value);
#ifdef ENABLE_SCI32
		} else if (g_sci->getGameId() == GID_GK1) {
			syncGK1StartupVolumeFromScummVM(index, value);
		} else if (g_sci->getGameId() == GID_HOYLE5 && index == kGlobalVarHoyle5MusicVolume) {
			syncHoyle5VolumeFromScummVM((ConfMan.getInt("music_volume") + 1) * kHoyle5VolumeMax / Audio::Mixer::kMaxMixerVolume);
		} else if (g_sci->getGameId() == GID_HOYLE5 && index == kGlobalVarHoyle5ResponseTime && value.getOffset() == 0) {
			// WORKAROUND: Global 899 contains the response time value,
			// which may have values between 1 and 15. There is a script
			// bug when loading values from game.opt, where this variable
			// may be incorrectly set to 0. This makes the opponent freeze
			// while playing Backgammon and Bridge. Fix this case here, by
			// setting the correct minimum value, 1.
			// TODO: Either make this a script patch, or find out if it's
			// a bug with ScummVM when reading values from text files.
			_state->variables[VAR_GLOBAL][index].setOffset(1);
		} else if (g_sci->getGameId() == GID_RAMA && !g_sci->isDemo() && index == kGlobalVarRamaMusicVolume) {
			syncRamaVolumeFromScummVM((ConfMan.getInt("music_volume") + 1) * kRamaVolumeMax / Audio::Mixer::kMaxMixerVolume);
		}

		if (_features->supportsTextSpeed()) {
			syncTextSpeedToScummVM(index, value);
#endif
		}

		syncMessageTypeToScummVM(index, value);
	}
}

bool GuestAdditions::kDoSoundMasterVolumeHook(const int volume) const {
	if (!_features->audioVolumeSyncUsesGlobals() && shouldSyncAudioToScummVM()) {
		syncMasterVolumeToScummVM(volume);
		return true;
	}
	return false;
}

#ifdef ENABLE_SCI32
void GuestAdditions::sciEngineInitGameHook() {
	if (g_sci->getGameId() == GID_PHANTASMAGORIA2 && Common::checkGameGUIOption(GAMEOPTION_ENABLE_CENSORING, ConfMan.get("guioptions"))) {
		_state->variables[VAR_GLOBAL][kGlobalVarPhant2CensorshipFlag] = make_reg(0, ConfMan.getBool("enable_censoring"));
	}

	if (g_sci->getGameId() == GID_KQ7 && Common::checkGameGUIOption(GAMEOPTION_UPSCALE_VIDEOS, ConfMan.get("guioptions"))) {
		uint16 value = ConfMan.getBool("enable_video_upscale") ? 32 : 0;
		_state->variables[VAR_GLOBAL][kGlobalVarKQ7UpscaleVideos] = make_reg(0, value);
	}
}

void GuestAdditions::sendSelectorHook(const reg_t sendObj, Selector &selector, reg_t *argp) {
	if (_features->getMessageTypeSyncStrategy() == kMessageTypeSyncStrategyLSL6Hires) {
		syncMessageTypeToScummVMUsingLSL6HiresStrategy(sendObj, selector, argp);
	}
}

bool GuestAdditions::audio32SetVolumeHook(const int16 channelIndex, int16 volume) const {
	if (!_features->audioVolumeSyncUsesGlobals() && shouldSyncAudioToScummVM()) {
		volume = volume * Audio::Mixer::kMaxMixerVolume / Audio32::kMaxVolume;
		if (Common::checkGameGUIOption(GUIO_LINKMUSICTOSFX, ConfMan.get("guioptions"))) {
			ConfMan.setInt("music_volume", volume);
		}
		ConfMan.setInt("sfx_volume", volume);
		ConfMan.setInt("speech_volume", volume);
		g_sci->updateSoundMixerVolumes();
		return true;
	}

	return false;
}

void GuestAdditions::kDoSoundSetVolumeHook(const reg_t soundObj, const int16 volume) const {
	if (g_sci->getGameId() == GID_GK1 && shouldSyncAudioToScummVM()) {
		syncGK1AudioVolumeToScummVM(soundObj, volume);
	}
}

void GuestAdditions::instantiateScriptHook(Script &script, const bool ignoreDelayedRestore) const {
	if (getSciVersion() < SCI_VERSION_2) {
		return;
	}

	// If there is a delayed restore, we still want to patch the script so
	// that the automatic return of the game ID works, but we do not want to
	// patch the scripts that get restored
	if (ConfMan.getBool("originalsaveload") &&
		(ignoreDelayedRestore || _state->_delayedRestoreGameId == -1)) {
		return;
	}

	if ((g_sci->getGameId() == GID_LSL7 || g_sci->getGameId() == GID_TORIN) &&
		script.getScriptNumber() == 64866) {

		patchGameSaveRestoreTorin(script);
	} else if (g_sci->getGameId() == GID_PHANTASMAGORIA2 && script.getScriptNumber() == 64978) {
		patchGameSaveRestorePhant2(script);
	} else if (script.getScriptNumber() == 64990) {
		// 64990 is the system script containing SRDialog. This script is used
		// by the main Game object, but it is not loaded immediately, so we wait
		// for it to be loaded before patching it. Attempting to preload this
		// script early for patching will cause the order of entries in the
		// segment table to change (versus save games that are not patched),
		// breaking persistent objects (like the control panel in SQ6) which
		// require reg_ts created during game startup to always be the same

		if (g_sci->getGameId() == GID_RAMA) {
			patchGameSaveRestoreRama(script);
		} else {
			patchGameSaveRestoreSCI32(script);
		}
	}
}

void GuestAdditions::segManSaveLoadScriptHook(Script &script) const {
	instantiateScriptHook(script, true);
}

#endif

bool GuestAdditions::kGetEventHook() const {
	if (_state->_delayedRestoreGameId == -1) {
		return false;
	}

#ifdef ENABLE_SCI32
	// Loading a save game while Lighthouse is still initializing itself will
	// cause loading to fail if the save game contains a saved Robot state,
	// because the Robot will try to restore itself into a game plane which does
	// not exist yet
	if (g_sci->getGameId() == GID_LIGHTHOUSE && _state->callInStack(g_sci->getGameObject(), SELECTOR(init))) {
		return false;
	}
#endif

	return g_sci->_guestAdditions->restoreFromLauncher();
}

bool GuestAdditions::kWaitHook() const {
	if (_state->_delayedRestoreGameId == -1) {
		return false;
	}

	// kWait cannot be used in Phant2 for delayed restore because it is
	// called during the fade-in of music in the intro room, before graphics
	// are fully initialized, which causes "Click to continue" text to be
	// brokenly drawn over the game and then crashes the engine on the next
	// room transition
	if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
		return false;
	}

	return g_sci->_guestAdditions->restoreFromLauncher();
}

#ifdef ENABLE_SCI32
bool GuestAdditions::kPlayDuckPlayVMDHook() const {
	return _state->_delayedRestoreGameId != -1;
}
#endif

#pragma mark -
#pragma mark Integrated save & restore

void GuestAdditions::patchGameSaveRestore() const {
	if (ConfMan.getBool("originalsaveload") || getSciVersion() >= SCI_VERSION_2)
		return;

	patchGameSaveRestoreSCI16();
}

static const byte kSaveRestorePatch[] = {
	0x39, 0x03,        // pushi 03
	0x76,              // push0
	0x38, 0xff, 0xff,  // pushi -1
	0x76,              // push0
	0x43, 0xff, 0x06,  // callk kRestoreGame/kSaveGame (will get changed afterwards)
	0x48               // ret
};

static void patchKSaveRestore(SegManager *segMan, reg_t methodAddress, byte id) {
	Script *script = segMan->getScript(methodAddress.getSegment());
	byte *patchPtr = const_cast<byte *>(script->getBuf(methodAddress.getOffset()));
	memcpy(patchPtr, kSaveRestorePatch, sizeof(kSaveRestorePatch));
	patchPtr[8] = id;
}

void GuestAdditions::patchGameSaveRestoreSCI16() const {
	const Object *gameObject = _segMan->getObject(g_sci->getGameObject());
	const Object *gameSuperObject = _segMan->getObject(gameObject->getSuperClassSelector());
	if (!gameSuperObject)
		gameSuperObject = gameObject;	// happens in KQ5CD, when loading saved games before r54510
	byte kernelIdRestore = 0;
	byte kernelIdSave = 0;

	switch (g_sci->getGameId()) {
	case GID_HOYLE1: // gets confused, although the game doesn't support saving/restoring at all
	case GID_HOYLE2: // gets confused, see hoyle1
	case GID_JONES: // gets confused, when we patch us in, the game is only able to save to 1 slot, so hooking is not required
	case GID_MOTHERGOOSE: // mother goose EGA saves/restores directly and has no save/restore dialogs
	case GID_MOTHERGOOSE256: // mother goose saves/restores directly and has no save/restore dialogs
		return;
	default:
		break;
	}

	uint16 kernelNamesSize = _kernel->getKernelNamesSize();
	for (uint16 kernelNr = 0; kernelNr < kernelNamesSize; kernelNr++) {
		Common::String kernelName = _kernel->getKernelName(kernelNr);
		if (kernelName == "RestoreGame")
			kernelIdRestore = kernelNr;
		if (kernelName == "SaveGame")
			kernelIdSave = kernelNr;
		if (kernelName == "Save")
			kernelIdSave = kernelIdRestore = kernelNr;
	}

	// Search for gameobject superclass ::restore
	uint16 gameSuperObjectMethodCount = gameSuperObject->getMethodCount();
	for (uint16 methodNr = 0; methodNr < gameSuperObjectMethodCount; methodNr++) {
		uint16 selectorId = gameSuperObject->getFuncSelector(methodNr);
		Common::String methodName = _kernel->getSelectorName(selectorId);
		if (methodName == "restore") {
				patchKSaveRestore(_segMan, gameSuperObject->getFunction(methodNr), kernelIdRestore);
		} else if (methodName == "save") {
			if (g_sci->getGameId() != GID_FAIRYTALES) {	// Fairy Tales saves automatically without a dialog
					patchKSaveRestore(_segMan, gameSuperObject->getFunction(methodNr), kernelIdSave);
			}
		}
	}

	// Patch gameobject ::save for now for SCI0 - SCI1.1
	// TODO: It seems this was never adjusted to superclass, but adjusting it now may cause
	// issues with some game. Needs to get checked and then possibly changed.
	const Object *patchObjectSave = gameObject;

	// Search for gameobject ::save, if there is one patch that one too
	uint16 patchObjectMethodCount = patchObjectSave->getMethodCount();
	for (uint16 methodNr = 0; methodNr < patchObjectMethodCount; methodNr++) {
		uint16 selectorId = patchObjectSave->getFuncSelector(methodNr);
		Common::String methodName = _kernel->getSelectorName(selectorId);
		if (methodName == "save") {
			if (g_sci->getGameId() != GID_FAIRYTALES &&  // Fairy Tales saves automatically without a dialog
				g_sci->getGameId() != GID_QFG3) { // QFG3 does automatic saving in Glory:save
					patchKSaveRestore(_segMan, patchObjectSave->getFunction(methodNr), kernelIdSave);
			}
			break;
		}
	}
}

#ifdef ENABLE_SCI32
static const byte SRDialogPatch[] = {
	0x76,                                 // push0
	0x59, 0x01,                           // &rest 1
	0x43, kScummVMSaveLoadId, 0x00, 0x00, // callk kScummVMSaveLoad, 0
	0x48                                  // ret
};

void GuestAdditions::patchGameSaveRestoreSCI32(Script &script) const {
	patchSRDialogDoit(script, "SRDialog", SRDialogPatch, sizeof(SRDialogPatch));
}

static const byte SRTorinPatch[] = {
	0x38, 0xFF, 0xFF,                     // pushi new
	0x76,                                 // push0
	0x51, 0x0f,                           // class Str
	0x4a, 0x04, 0x00,                     // send 4
	0xa3, 0x01,                           // sal 1
	0x76,                                 // push0
	0x59, 0x01,                           // &rest 1
	0x43, kScummVMSaveLoadId, 0x00, 0x00, // callk kScummVMSaveLoad, 0
	0x48                                  // ret
};

void GuestAdditions::patchGameSaveRestoreTorin(Script &script) const {
	const uint32 address = script.validateExportFunc(2, true);
	byte *patchPtr = const_cast<byte *>(script.getBuf(address));
	memcpy(patchPtr, SRTorinPatch, sizeof(SRTorinPatch));

	const Selector newSelector = SELECTOR(new_);
	assert(newSelector != -1);
	patchPtr[1] = newSelector & 0xFF;
	patchPtr[2] = (newSelector >> 8) & 0xFF;

	if (g_sci->isBE()) {
		SWAP(patchPtr[1], patchPtr[2]);
		SWAP(patchPtr[7], patchPtr[8]);
	}
}

void GuestAdditions::patchGameSaveRestorePhant2(Script &script) const {
	const ObjMap &objects = script.getObjectMap();
	for (ObjMap::const_iterator it = objects.begin(); it != objects.end(); ++it) {
		const Object &obj = it->_value;

		if (strcmp(_segMan->derefString(obj.getNameSelector()), "srGetGame") != 0) {
			continue;
		}

		int methodIndex = obj.funcSelectorPosition(SELECTOR(init));
		if (methodIndex == -1) {
			continue;
		}

		byte *scriptData = const_cast<byte *>(script.getBuf(obj.getFunction(methodIndex).getOffset()));
		memcpy(scriptData, SRDialogPatch, sizeof(SRDialogPatch));
		break;
	}
}

static const byte RamaSRDialogPatch[] = {
	0x78,                                 // push1
	0x7c,                                 // pushSelf
	0x43, kScummVMSaveLoadId, 0x02, 0x00, // callk kScummVMSaveLoad, 0
	0x48                                  // ret
};

static const int RamaSRDialogUint16Offsets[] = { 4 };

void GuestAdditions::patchGameSaveRestoreRama(Script &script) const {
	patchSRDialogDoit(script, "Save", RamaSRDialogPatch, sizeof(RamaSRDialogPatch), RamaSRDialogUint16Offsets, ARRAYSIZE(RamaSRDialogUint16Offsets));
	patchSRDialogDoit(script, "Restore", RamaSRDialogPatch, sizeof(RamaSRDialogPatch), RamaSRDialogUint16Offsets, ARRAYSIZE(RamaSRDialogUint16Offsets));
}

void GuestAdditions::patchSRDialogDoit(Script &script, const char *const objectName, const byte *patchData, const int patchSize, const int *uint16Offsets, const uint numOffsets) const {
	const ObjMap &objMap = script.getObjectMap();
	for (ObjMap::const_iterator it = objMap.begin(); it != objMap.end(); ++it) {
		const Object &obj = it->_value;
		if (strcmp(_segMan->getObjectName(obj.getPos()), objectName) != 0) {
			continue;
		}

		const uint16 methodCount = obj.getMethodCount();
		for (uint16 methodNr = 0; methodNr < methodCount; ++methodNr) {
			const uint16 selectorId = obj.getFuncSelector(methodNr);
			const Common::String methodName = _kernel->getSelectorName(selectorId);
			if (methodName == "doit") {
				const reg_t methodAddress = obj.getFunction(methodNr);
				byte *patchPtr = const_cast<byte *>(script.getBuf(methodAddress.getOffset()));
				memcpy(patchPtr, patchData, patchSize);

				if (g_sci->isBE()) {
					for (uint i = 0; i < numOffsets; ++i) {
						const int offset = uint16Offsets[i];
						SWAP(patchPtr[offset], patchPtr[offset + 1]);
					}
				}

				return;
			}
		}
	}
}

reg_t GuestAdditions::kScummVMSaveLoad(EngineState *s, int argc, reg_t *argv) const {
	if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
		return promptSaveRestorePhant2(s, argc, argv);
	}

	if (g_sci->getGameId() == GID_LSL7 || g_sci->getGameId() == GID_TORIN) {
		return promptSaveRestoreTorin(s, argc, argv);
	}

	if (g_sci->getGameId() == GID_RAMA) {
		return promptSaveRestoreRama(s, argc, argv);
	}

	if (g_sci->getGameId() == GID_HOYLE5) {
		return promptSaveRestoreHoyle5(s, argc, argv);
	}

	return promptSaveRestoreDefault(s, argc, argv);
}

reg_t GuestAdditions::promptSaveRestoreDefault(EngineState *s, int argc, reg_t *argv) const {
	return make_reg(0, runSaveRestore(argc > 0, argc > 0 ? argv[0] : NULL_REG, s->_delayedRestoreGameId));
}

reg_t GuestAdditions::promptSaveRestoreTorin(EngineState *s, int argc, reg_t *argv) const {
	const bool isSave = (argc > 0 && (bool)argv[0].toSint16());

	reg_t descriptionId = NULL_REG;
	if (isSave) {
		_segMan->allocateArray(kArrayTypeString, 0, &descriptionId);
	}

	const int saveNo = runSaveRestore(isSave, descriptionId, s->_delayedRestoreGameId);

	if (saveNo != -1) {
		assert(s->variablesMax[VAR_LOCAL] > 2);
		writeSelector(_segMan, s->variables[VAR_LOCAL][1], SELECTOR(data), descriptionId);
		s->variables[VAR_LOCAL][2] = make_reg(0, saveNo);
		s->variables[VAR_LOCAL][3] = make_reg(0, isSave ? 1 : 0);
	} else if (isSave) {
		_segMan->freeArray(descriptionId);
	}

	return make_reg(0, saveNo != -1);
}

reg_t GuestAdditions::promptSaveRestorePhant2(EngineState *s, int argc, reg_t *argv) const {
	assert(argc == 2);
	const bool isSave = argv[1].toSint16() == 0;
	const int saveNo = runSaveRestore(isSave, argv[0], s->_delayedRestoreGameId);

	// Clear the highlighted state of the button so if the same control panel is
	// opened again it does not appear to be opened to the save/load panels
	reg_t button;
	if (isSave) {
		button = _segMan->findObjectByName("saveButton");
	} else {
		button = _segMan->findObjectByName("loadButton");
	}
	writeSelectorValue(_segMan, button, SELECTOR(cel), 0);

	// This causes the control panel to quit its internal event loop and hide
	// itself
	const reg_t controlPanel = s->variables[VAR_GLOBAL][kGlobalVarPhant2ControlPanel];
	writeSelector(_segMan, controlPanel, SELECTOR(scratch), TRUE_REG);

	return make_reg(0, saveNo);
}

reg_t GuestAdditions::promptSaveRestoreRama(EngineState *s, int argc, reg_t *argv) const {
	assert(argc == 1);
	const bool isSave = (strcmp(_segMan->getObjectName(argv[0]), "Save") == 0);

	const reg_t editor = _segMan->findObjectByName("editI");
	reg_t outDescription = readSelector(_segMan, editor, SELECTOR(text));
	if (!_segMan->isValidAddr(outDescription, SEG_TYPE_ARRAY)) {
		_segMan->allocateArray(kArrayTypeString, 0, &outDescription);
		writeSelector(_segMan, editor, SELECTOR(text), outDescription);
	}

	int saveNo = runSaveRestore(isSave, outDescription, s->_delayedRestoreGameId);
	int saveIndex = -1;
	if (saveNo != -1) {
		// The save number returned by runSaveRestore is a SCI save number
		// because normally SRDialogs return the save ID, but RAMA returns the
		// save game's index in the save game list instead, so we need to
		// convert back to the ScummVM save number here to find the correct
		// index
		saveNo += kSaveIdShift;

		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		saveIndex = findSavegame(saves, saveNo);

		if (isSave) {
			bool resetCatalogFile = false;
			const Common::String saveGameName = _segMan->getString(outDescription);

			// The original game save/restore code returns index 0 when a game
			// is created that does not already exist and then the scripts find
			// the next hole and insert there, but the ScummVM GUI works
			// differently and allows users to insert a game wherever they want,
			// so we need to force the save game to exist in advance so RAMA's
			// save code will successfully put it where we want it
			if (saveIndex == -1) {
				// We need to touch the save file just so it exists here, since
				// otherwise the game will not let us save to the new save slot
				// (it will try to come up with a brand new slot instead)
				Common::OutSaveFile *out = g_sci->getSaveFileManager()->openForSaving(g_sci->getSavegameName(saveNo));
				set_savegame_metadata(out, saveGameName, "");

				// Make sure the save file is fully written before we try to
				// re-retrieve the list of saves, since otherwise it may not
				// show up in the list
				delete out;

				// We have to re-retrieve saves and find the index instead of
				// assuming the newest save will be in index 0 because save game
				// times are not guaranteed to be steady
				saves.clear();
				listSavegames(saves);
				saveIndex = findSavegame(saves, saveNo);
				if (saveIndex == -1) {
					warning("Stub save not found when trying to save a new game to slot %d", saveNo);
				} else {
					// Kick the CatalogFile into believing that this new save
					// game exists already, otherwise it the game will not
					// actually save into the new save
					resetCatalogFile = true;
				}
			} else if (strncmp(saveGameName.c_str(), saves[saveIndex].name, kMaxSaveNameLength) != 0) {
				// The game doesn't let the save game name change for the same
				// slot, but ScummVM's GUI does, so force the new name into the
				// save file metadata if it has changed so it actually makes it
				// into the save game
				Common::ScopedPtr<Common::OutSaveFile> out(g_sci->getSaveFileManager()->openForSaving(g_sci->getSavegameName(saveNo)));
				set_savegame_metadata(out.get(), saveGameName, "");
				resetCatalogFile = true;
			}

			if (resetCatalogFile) {
				const reg_t catalogFileId = _state->variables[VAR_GLOBAL][kGlobalVarRamaCatalogFile];
				if (catalogFileId.isNull()) {
					warning("Could not find CatalogFile when saving from launcher");
				}
				reg_t args[] = { NULL_REG };
				invokeSelector(catalogFileId, SELECTOR(dispose));
				invokeSelector(catalogFileId, SELECTOR(init), ARRAYSIZE(args), args);
			}
		}
	}

	return make_reg(0, saveIndex);
}

int GuestAdditions::runSaveRestore(const bool isSave, reg_t outDescription, const int forcedSaveId) const {
	assert(!(isSave && outDescription.isNull()));

	Common::String descriptionString;
	int saveId = runSaveRestore(isSave, descriptionString, forcedSaveId);

	if (!outDescription.isNull()) {
		if (_segMan->isObject(outDescription)) {
			outDescription = readSelector(_segMan, outDescription, SELECTOR(data));
		}
		SciArray &description = *_segMan->lookupArray(outDescription);
		description.fromString(descriptionString);
	}

	return saveId;
}

int GuestAdditions::runSaveRestore(const bool isSave, Common::String &outDescription, const int forcedSaveId) const {
	int saveId;

	if (!isSave && forcedSaveId != -1) {
		saveId = forcedSaveId;
	} else {
		Common::U32String title;
		Common::U32String action;
		if (isSave) {
			title = _("Save game:");
			action = _("Save");
		} else {
			title = _("Restore game:");
			action = _("Restore");
		}

		GUI::SaveLoadChooser dialog(title, action, isSave);
		saveId = dialog.runModalWithCurrentTarget();
		if (saveId != -1) {
			outDescription = dialog.getResultString();
			if (outDescription.empty()) {
				outDescription = dialog.createDefaultSaveDescription(saveId - 1);
			}
		}
	}

	// The autosave slot in ScummVM takes up slot 0, but in SCI the first
	// non-autosave save game number needs to be 0, so reduce the save
	// number here to match what would come from the normal SCI save/restore
	// dialog. Wrap slot 0 around to kMaxShiftedSaveId so that it remains
	// a legal SCI value.
	saveId = shiftScummVMToSciSaveId(saveId);

	return saveId;
}

reg_t GuestAdditions::promptSaveRestoreHoyle5(EngineState *s, int argc, reg_t *argv) const {
	assert(argc == 2);
	Common::String callerName = s->_segMan->getObjectName(s->r_acc);
	const bool isSave = (callerName == "Save");
	return make_reg(0, runSaveRestore(isSave, argc > 0 ? argv[0] : NULL_REG, s->_delayedRestoreGameId));
}

#endif

#pragma mark -
#pragma mark Restore from launcher

bool GuestAdditions::restoreFromLauncher() const {
	assert(_state->_delayedRestoreGameId != -1);

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		if (_restoring) {
			// Recursion will occur if a restore fails, as
			// _delayedRestoreGameId will not be reset so the kernel will try
			// to keep restoring forever
			_state->_delayedRestoreGameId = -1;
			_restoring = false;
			return false;
		}

		// Delayed restore should not happen until after the benchmarking room.
		// In particular, in SQ6, delayed restore must not happen until room 100
		// (the Sierra logo & main menu room), otherwise the game scripts will
		// try to make calls to the subtitles ScrollWindow, which does not
		// exist. In other games, restoring early either breaks benchmarking,
		// or, when trying to load an invalid save game, makes the dialog
		// telling the user that the game is invalid impossible to read
		if (strcmp(_segMan->getObjectName(_state->variables[VAR_GLOBAL][kGlobalVarCurrentRoom]), "speedRoom") == 0) {
			return false;
		}

		// Delayed restore should not happen in LSL6 hires or PQ4 until the room number is set.
		//  LSL6:restore and pq4:restore assume the room number has already been set, but the
		//  Mac versions of these game add a call to kGetEvent in the games' init method before
		//  the initial call to newRoom. If the room number isn't set yet then LSL6 doesn't
		//  allow the restore and PQ4 sends a message to an invalid object.
		if ((g_sci->getGameId() == GID_LSL6HIRES || g_sci->getGameId() == GID_PQ4) &&
			_state->variables[VAR_GLOBAL][kGlobalVarCurrentRoomNo] == NULL_REG) {
			return false;
		}

		_restoring = true;

		// Any events queued up before the game restore can cause accidental
		// input into the game if they are not flushed (this is particularly
		// noticeable in Phant2, where the game will display "Click to continue"
		// for one frame if the user clicked during startup)
		g_sci->getEventManager()->flushEvents();

		if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
			// Phantasmagoria 2 moves the function that actually restores
			// a game, and uses a property of the main game object when picking
			// the save game to restore
			writeSelectorValue(_segMan, g_sci->getGameObject(), SELECTOR(num), _state->_delayedRestoreGameId - kSaveIdShift);
			invokeSelector(g_sci->getGameObject(), SELECTOR(reallyRestore));
		} else if (g_sci->getGameId() == GID_SHIVERS) {
			// Shivers accepts the save game number as a parameter to
			// `SHIVERS::restore`
			reg_t args[] = { make_reg(0, _state->_delayedRestoreGameId - kSaveIdShift) };
			invokeSelector(g_sci->getGameObject(), SELECTOR(restore), 1, args);
		} else {
			int saveId = _state->_delayedRestoreGameId;

			// When `Game::restore` is invoked, it will call to `Restore::doit`
			// which will automatically return the `_delayedRestoreGameId` instead
			// of prompting the user for a save game
			invokeSelector(g_sci->getGameObject(), SELECTOR(restore));

			// initialize KQ7 Mac's global save state by recording the save id
			//  and description. this is necessary for subsequent saves to work
			//  after restoring from launcher.
			if (g_sci->getGameId() == GID_KQ7 && g_sci->getPlatform() == Common::kPlatformMacintosh) {
				_state->_kq7MacSaveGameId = saveId;

				SavegameDesc savegameDesc;
				if (fillSavegameDesc(g_sci->getSavegameName(saveId), savegameDesc)) {
					_state->_kq7MacSaveGameDescription = savegameDesc.name;
				}
			}

			// The normal save game system resets _delayedRestoreGameId with a
			// call to `EngineState::reset`, but RAMA uses a custom save game
			// system which does not reset the engine, so we need to clear the
			// ID here or the engine will just try to restore the game forever
			if (g_sci->getGameId() == GID_RAMA) {
				_state->_delayedRestoreGameId = -1;
			}
		}

		_restoring = false;

		return true;
	} else {
#else
	{
#endif
		int savegameId = _state->_delayedRestoreGameId; // delayedRestoreGameId gets destroyed within gamestate_restore()!
		Common::String fileName = g_sci->getSavegameName(savegameId);
		Common::SeekableReadStream *in = g_sci->getSaveFileManager()->openForLoading(fileName);

		if (in) {
			// found a savegame file
			gamestate_restore(_state, in);
			delete in;
			if (_state->r_acc != make_reg(0, 1)) {
				gamestate_afterRestoreFixUp(_state, savegameId);
				return true;
			}
		}

		error("Restoring gamestate '%s' failed", fileName.c_str());
	}
}

#pragma mark -
#pragma mark Message type sync

void GuestAdditions::syncMessageTypeFromScummVM() const {
	switch (_features->getMessageTypeSyncStrategy()) {
	case kMessageTypeSyncStrategyDefault:
		syncMessageTypeFromScummVMUsingDefaultStrategy();
		break;

#ifdef ENABLE_SCI32
	case kMessageTypeSyncStrategyShivers:
		syncMessageTypeFromScummVMUsingShiversStrategy();
		break;

	case kMessageTypeSyncStrategyLSL6Hires:
		syncMessageTypeFromScummVMUsingLSL6HiresStrategy();
		break;
#endif
	case kMessageTypeSyncStrategyNone:
	default:
		break;
	}
}

void GuestAdditions::syncMessageTypeFromScummVMUsingDefaultStrategy() const {
	uint8 value = 0;
	if (ConfMan.getBool("subtitles")) {
		value |= kMessageTypeSubtitles;
	}
	if (!ConfMan.getBool(("speech_mute"))) {
		value |= kMessageTypeSpeech;
	}

	if (value == kMessageTypeSubtitles + kMessageTypeSpeech && !_features->supportsSpeechWithSubtitles()) {
		value &= ~kMessageTypeSubtitles;
	}

	if (value) {
		_state->variables[VAR_GLOBAL][kGlobalVarMessageType] = make_reg(0, value);
	}

#ifdef ENABLE_SCI32
	if (g_sci->getGameId() == GID_GK1 && value == kMessageTypeSubtitles) {
		// The narrator speech needs to be forced off if speech has been
		// disabled in ScummVM, but otherwise the narrator toggle should just
		// be allowed to persist to whatever the user chose previously, since
		// it is controlled independently of other speech in the game and there
		// is no equivalent option in the ScummVM GUI
		_state->variables[VAR_GLOBAL][kGlobalVarGK1NarratorMode] = NULL_REG;
	}

	if (g_sci->getGameId() == GID_QFG4) {
		// QFG4 uses a game flag to control the Audio button's state in the control panel.
		//  This flag must be kept in sync with the standard global 90 speech bit.
		uint flagNumber = 400;
		uint globalNumber = kGlobalVarQFG4Flags + (flagNumber / 16);
		if (value & kMessageTypeSpeech) {
			_state->variables[VAR_GLOBAL][globalNumber] |= (int16)0x8000;
		} else {
			_state->variables[VAR_GLOBAL][globalNumber] &= (int16)~0x8000;
		}
	}

	if (g_sci->getGameId() == GID_SQ6) {
		// The SQ6 control panel buttons for speech and text only update
		//  their states when clicked so they need synchronization.
		const reg_t iconSpeech = _segMan->findObjectByName("iconSpeech");
		if (!iconSpeech.isNull()) {
			const reg_t iconCel = make_reg(0, (value & kMessageTypeSpeech) ? 2 : 0);
			writeSelector(_segMan, iconSpeech, SELECTOR(mainCel), iconCel);
		}
		const reg_t iconText = _segMan->findObjectByName("iconText");
		if (!iconText.isNull()) {
			const reg_t iconCel = make_reg(0, (value & kMessageTypeSubtitles) ? 2 : 0);
			writeSelector(_segMan, iconText, SELECTOR(mainCel), iconCel);
		}
	}
#endif
}

#ifdef ENABLE_SCI32
void GuestAdditions::syncMessageTypeFromScummVMUsingShiversStrategy() const {
	if (ConfMan.getBool("subtitles")) {
		_state->variables[VAR_GLOBAL][kGlobalVarShiversFlags] |= 256;
	} else {
		_state->variables[VAR_GLOBAL][kGlobalVarShiversFlags] &= ~256;
	}
}

void GuestAdditions::syncMessageTypeFromScummVMUsingLSL6HiresStrategy() const {
	// LSL6hires synchronisation happens in send_selector, except when
	// restoring a game, where it happens here
	if (_state->variables[VAR_GLOBAL][kGlobalVarLSL6HiresGameFlags].isNull()) {
		return;
	}

	reg_t params[] = { make_reg(0, kLSL6HiresSubtitleFlag) };
	Selector selector;
	reg_t restore;

	if (ConfMan.getBool("subtitles")) {
		restore = TRUE_REG;
		selector = SELECTOR(clear);
	} else {
		restore = NULL_REG;
		selector = SELECTOR(set);
	}

	// Attempting to show or hide the ScrollWindow used for subtitles
	// directly (by invoking `show` or `hide`) causes the game to crash with
	// an error about passing an invalid ScrollWindow ID. Fortunately, the
	// game scripts store a flag that restores the window when a game is
	// restored
	_state->variables[VAR_GLOBAL][kGlobalVarLSL6HiresRestoreTextWindow] = restore;
	invokeSelector(_state->variables[VAR_GLOBAL][kGlobalVarLSL6HiresGameFlags], selector, 1, params);
}
#endif

void GuestAdditions::syncMessageTypeToScummVM(const int index, const reg_t value) {
	switch (_features->getMessageTypeSyncStrategy()) {
	case kMessageTypeSyncStrategyDefault:
		syncMessageTypeToScummVMUsingDefaultStrategy(index, value);
		break;

#ifdef ENABLE_SCI32
	case kMessageTypeSyncStrategyShivers:
		syncMessageTypeToScummVMUsingShiversStrategy(index, value);
		break;

	case kMessageTypeSyncStrategyLSL6Hires:
		// LSL6hires synchronisation happens via send_selector
#endif
	case kMessageTypeSyncStrategyNone:
	default:
		break;
	}
}

void GuestAdditions::syncMessageTypeToScummVMUsingDefaultStrategy(const int index, const reg_t value) {
	if (index == kGlobalVarMessageType) {
		// ScummVM audio options haven't been applied yet. Use this set call
		// as a trigger to apply defaults from ScummVM, ignoring the default
		// value that was just received from the game scripts
		if (!_messageTypeSynced || _state->variables[VAR_GLOBAL][kGlobalVarQuit] == TRUE_REG) {
			_messageTypeSynced = true;
			syncAudioOptionsFromScummVM();
			return;
		}

		ConfMan.setBool("subtitles", value.toSint16() & kMessageTypeSubtitles);
		ConfMan.setBool("speech_mute", !(value.toSint16() & kMessageTypeSpeech));

		// need to update sound mixer volumes so that speech_mute will take effect
		g_sci->updateSoundMixerVolumes();
	}
}

#ifdef ENABLE_SCI32
void GuestAdditions::syncMessageTypeToScummVMUsingShiversStrategy(const int index, const reg_t value) {
	if (index == kGlobalVarShiversFlags) {
		// ScummVM audio options haven't been applied yet, so apply them
		// and ignore the default value that was just received from the
		// game scripts
		if (!_messageTypeSynced || _state->variables[VAR_GLOBAL][kGlobalVarQuit] == TRUE_REG) {
			_messageTypeSynced = true;
			syncAudioOptionsFromScummVM();
			return;
		}

		ConfMan.setBool("subtitles", value.toUint16() & 256);
	}
}

void GuestAdditions::syncMessageTypeToScummVMUsingLSL6HiresStrategy(const reg_t sendObj, Selector &selector, reg_t *argp) {
	if (_state->variables[VAR_GLOBAL][kGlobalVarLSL6HiresGameFlags] == sendObj &&
		(selector == SELECTOR(clear) || selector == SELECTOR(set))) {

		if (argp[1].toUint16() == kLSL6HiresSubtitleFlag) {
			if (_messageTypeSynced) {
				ConfMan.setBool("subtitles", selector == SELECTOR(clear));
			} else if (ConfMan.getBool("subtitles")) {
				selector = SELECTOR(clear);
				argp[-1].setOffset(selector);
				_messageTypeSynced = true;
			} else {
				selector = SELECTOR(set);
				argp[-1].setOffset(selector);
				_messageTypeSynced = true;
			}
		}
	}
}
#endif

#pragma mark -
#pragma mark Master volume sync

void GuestAdditions::syncMasterVolumeFromScummVM() const {
#ifdef ENABLE_SCI32
	const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * MUSIC_MASTERVOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
	const int16 sfxVolume = (ConfMan.getInt("sfx_volume") + 1) * Audio32::kMaxVolume / Audio::Mixer::kMaxMixerVolume;

	// Volume was changed from ScummVM during the game, so resync the
	// in-game UI
	syncInGameUI(musicVolume, sfxVolume);
#endif
}

void GuestAdditions::syncMasterVolumeToScummVM(const int16 masterVolume) const {
	const int scummVMVolume = masterVolume * Audio::Mixer::kMaxMixerVolume / MUSIC_MASTERVOLUME_MAX;
	ConfMan.setInt("music_volume", scummVMVolume);

	if (Common::checkGameGUIOption(GUIO_LINKMUSICTOSFX, ConfMan.get("guioptions"))) {
		ConfMan.setInt("sfx_volume", scummVMVolume);
		if (Common::checkGameGUIOption(GUIO_LINKSPEECHTOSFX, ConfMan.get("guioptions"))) {
			ConfMan.setInt("speech_volume", scummVMVolume);
		}
	}

	// In SCI32, digital audio volume is controlled separately by
	// kDoAudioVolume
	// TODO: In SCI16, the volume slider only changed the music volume.
	// Is this non-standard behavior better, or just wrong?
	if (getSciVersion() < SCI_VERSION_2) {
		ConfMan.setInt("sfx_volume", scummVMVolume);
		ConfMan.setInt("speech_volume", scummVMVolume);
	}
	g_sci->updateSoundMixerVolumes();
}

#pragma mark -
#pragma mark Globals volume sync

void GuestAdditions::syncAudioVolumeGlobalsFromScummVM() const {
	// On muting: Setting the music volume to zero when mute is enabled is done
	// only for the games that use MIDI for music playback, since MIDI playback
	// does not always run through the ScummVM mixer. Games that use digital
	// audio for music do not need any extra code since that always runs
	// straight through the audio mixer, which gets muted directly
	switch (g_sci->getGameId()) {
#ifdef ENABLE_SCI32
	case GID_GK1: {
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * MUSIC_VOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
		const int16 dacVolume = (ConfMan.getInt("sfx_volume") + 1) * Audio32::kMaxVolume / Audio::Mixer::kMaxMixerVolume;
		syncGK1VolumeFromScummVM(musicVolume, dacVolume);
		syncGK1UI();
		break;
	}

	case GID_GK2: {
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * Audio32::kMaxVolume / Audio::Mixer::kMaxMixerVolume;
		syncGK2VolumeFromScummVM(musicVolume);
		syncGK2UI();
		break;
	}

	case GID_HOYLE5: {
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * kHoyle5VolumeMax / Audio::Mixer::kMaxMixerVolume;
		syncHoyle5VolumeFromScummVM(musicVolume);
		syncHoyle5UI(musicVolume);
		break;
	}

	case GID_PHANTASMAGORIA: {
		reg_t &musicGlobal = _state->variables[VAR_GLOBAL][kGlobalVarPhant1MusicVolume];
		reg_t &dacGlobal   = _state->variables[VAR_GLOBAL][kGlobalVarPhant1DACVolume];

		const int16 oldMusicVolume = musicGlobal.toSint16();
		const int16 oldDacVolume   = dacGlobal.toSint16();

		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * MUSIC_MASTERVOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
		const int16 dacVolume   = (ConfMan.getInt("sfx_volume") + 1)   * Audio32::kMaxVolume / Audio::Mixer::kMaxMixerVolume;

		g_sci->_soundCmd->setMasterVolume(ConfMan.getBool("mute") ? 0 : musicVolume);

		// Phant1 has a fragile volume UI. Global volumes need to be set during
		// UI updates to move the volume bars to the correct position
		syncPhant1UI(oldMusicVolume, musicVolume, musicGlobal, oldDacVolume, dacVolume, dacGlobal);
		break;
	}

	case GID_PHANTASMAGORIA2: {
		const int16 masterVolume = (ConfMan.getInt("sfx_volume") + 1) * kPhant2VolumeMax / Audio::Mixer::kMaxMixerVolume;
		syncPhant2VolumeFromScummVM(masterVolume);
		syncPhant2UI(masterVolume);
		break;
	}

	case GID_RAMA: {
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * kRamaVolumeMax / Audio::Mixer::kMaxMixerVolume;
		syncRamaVolumeFromScummVM(musicVolume);
		syncRamaUI(musicVolume);
		break;
	}

	case GID_LSL7:
	case GID_TORIN: {
		const int16 musicVolume  = (ConfMan.getInt("music_volume") + 1)  * 100 / Audio::Mixer::kMaxMixerVolume;
		const int16 sfxVolume    = (ConfMan.getInt("sfx_volume") + 1)    * 100 / Audio::Mixer::kMaxMixerVolume;
		const int16 speechVolume = (ConfMan.getInt("speech_volume") + 1) * 100 / Audio::Mixer::kMaxMixerVolume;
		syncTorinVolumeFromScummVM(musicVolume, sfxVolume, speechVolume);
		syncTorinUI(musicVolume, sfxVolume, speechVolume);
		break;
	}
#endif

	case GID_LSL6:
	case GID_LSL6HIRES:	{
		const int16 musicVolume = (ConfMan.getInt("music_volume") + 1) * kLSL6UIVolumeMax / Audio::Mixer::kMaxMixerVolume;
		syncLSL6VolumeFromScummVM(musicVolume);
		syncLSL6UI(musicVolume);
		break;
	}

	default:
		error("Trying to sync audio volume globals in a game with no implementation");
	}
}

void GuestAdditions::syncLSL6VolumeFromScummVM(const int16 musicVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarLSL6MusicVolume] = make_reg(0, musicVolume);
	g_sci->_soundCmd->setMasterVolume(ConfMan.getBool("mute") ? 0 : (musicVolume * MUSIC_MASTERVOLUME_MAX / kLSL6UIVolumeMax));
}

#ifdef ENABLE_SCI32
void GuestAdditions::syncGK1StartupVolumeFromScummVM(const int index, const reg_t value) const {
	if (index == kGlobalVarGK1Music1 || index == kGlobalVarGK1Music2 ||
		index == kGlobalVarGK1DAC1 || index == kGlobalVarGK1DAC2 ||
		index == kGlobalVarGK1DAC3) {

		int16 volume;
		Selector selector;

		switch (readSelectorValue(_segMan, value, SELECTOR(type))) {
		case kSoundsMusicType: {
			volume = (ConfMan.getInt("music_volume") + 1) * MUSIC_VOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
			selector = SELECTOR(musicVolume);
			break;
		}

		case kSoundsSoundType: {
			volume = (ConfMan.getInt("sfx_volume") + 1) * MUSIC_VOLUME_MAX / Audio::Mixer::kMaxMixerVolume;
			selector = SELECTOR(soundVolume);
			break;
		}

		default:
			error("Unknown sound type");
		}

		writeSelectorValue(_segMan, value, selector, volume);
	}
}

void GuestAdditions::syncGK1VolumeFromScummVM(const int16 musicVolume, const int16 dacVolume) const {
	const reg_t soundsId = _state->variables[VAR_GLOBAL][kGlobalVarSounds];
	if (!soundsId.isNull()) {
		List *sounds = _segMan->lookupList(readSelector(_segMan, soundsId, SELECTOR(elements)));
		reg_t soundId = sounds->first;
		while (!soundId.isNull()) {
			Node *sound = _segMan->lookupNode(soundId);
			const int16 type = readSelectorValue(_segMan, sound->value, SELECTOR(type));
			int16 volume;

			if (type == kSoundsMusicType) {
				volume = ConfMan.getBool("mute") ? 0 : musicVolume;
				writeSelectorValue(_segMan, sound->value, SELECTOR(musicVolume), musicVolume);
			} else if (type == kSoundsSoundType) {
				volume = dacVolume;
				writeSelectorValue(_segMan, sound->value, SELECTOR(soundVolume), dacVolume);
			} else {
				error("Unknown sound type %d", type);
			}

			// `setVolume` will set the `vol` property on the sound object;
			// if it did not do this, an invocation of the `setVol` selector
			// would need to be here (though doing so would result in
			// recursion, so don't)
			g_sci->_soundCmd->setVolume(sound->value, volume);
			soundId = sound->succ;
		}
	}
}

void GuestAdditions::syncGK2VolumeFromScummVM(const int16 musicVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarGK2MusicVolume] = make_reg(0, musicVolume);

	// Calling `setVol` on all sounds is necessary to propagate the volume
	// change to existing sounds, and matches how game scripts propagate
	// volume changes when the in-game music slider is moved
	const reg_t soundsId = _state->variables[VAR_GLOBAL][kGlobalVarSounds];
	if (!soundsId.isNull()) {
		List *sounds = _segMan->lookupList(readSelector(_segMan, soundsId, SELECTOR(elements)));
		reg_t soundId = sounds->first;
		while (!soundId.isNull()) {
			Node *sound = _segMan->lookupNode(soundId);
			reg_t params[] = { make_reg(0, musicVolume) };
			invokeSelector(sound->value, SELECTOR(setVol), 1, params);
			soundId = sound->succ;
		}
	}
}

void GuestAdditions::syncHoyle5VolumeFromScummVM(const int16 musicVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarHoyle5MusicVolume] = make_reg(0, musicVolume);
	g_sci->_soundCmd->setMasterVolume(ConfMan.getBool("mute") ? 0 : (musicVolume * MUSIC_MASTERVOLUME_MAX / kHoyle5VolumeMax));
}

void GuestAdditions::syncPhant2VolumeFromScummVM(const int16 masterVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarPhant2MasterVolume] = make_reg(0, masterVolume);
	_state->variables[VAR_GLOBAL][kGlobalVarPhant2SecondaryVolume] = make_reg(0, masterVolume);

	const reg_t soundsId = _state->variables[VAR_GLOBAL][kGlobalVarSounds];
	if (!soundsId.isNull()) {
		reg_t params[] = { make_reg(0, SELECTOR(setVol)), make_reg(0, masterVolume) };
		invokeSelector(soundsId, SELECTOR(eachElementDo), 2, params);
	}
}

void GuestAdditions::syncRamaVolumeFromScummVM(const int16 musicVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarRamaMusicVolume] = make_reg(0, musicVolume);
	const reg_t gameId = _state->variables[VAR_GLOBAL][kGlobalVarGame];
	if (!gameId.isNull()) {
		reg_t args[] = { make_reg(0, musicVolume) };
		invokeSelector(gameId, SELECTOR(masterVolume), 1, args);
	}
}

void GuestAdditions::syncTorinVolumeFromScummVM(const int16 musicVolume, const int16 sfxVolume, const int16 speechVolume) const {
	_state->variables[VAR_GLOBAL][kGlobalVarTorinMusicVolume]  = make_reg(0, musicVolume);
	_state->variables[VAR_GLOBAL][kGlobalVarTorinSFXVolume]    = make_reg(0, sfxVolume);
	_state->variables[VAR_GLOBAL][kGlobalVarTorinSpeechVolume] = make_reg(0, speechVolume);

	// Calling `reSyncVol` on all sounds is necessary to propagate the
	// volume change to existing sounds, and matches how game scripts
	// propagate volume changes when the in-game volume sliders are moved
	const reg_t soundsId = _state->variables[VAR_GLOBAL][kGlobalVarSounds];
	if (!soundsId.isNull()) {
		const Selector selector = SELECTOR(reSyncVol);
		List *sounds = _segMan->lookupList(readSelector(_segMan, soundsId, SELECTOR(elements)));
		reg_t soundId = sounds->first;
		while (!soundId.isNull()) {
			Node *sound = _segMan->lookupNode(soundId);
			const reg_t &soundObj = sound->value;

			if (_segMan->isHeapObject(soundObj) && lookupSelector(_segMan, soundObj, selector, nullptr, nullptr) != kSelectorNone) {
				invokeSelector(sound->value, selector);
			}
			soundId = sound->succ;
		}
	}
}
#endif

void GuestAdditions::syncAudioVolumeGlobalsToScummVM(const int index, const reg_t value) const {
	switch (g_sci->getGameId()) {
#ifdef ENABLE_SCI32
	case GID_GK2:
		if (index == kGlobalVarGK2MusicVolume) {
			const int16 musicVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / Audio32::kMaxVolume;
			ConfMan.setInt("music_volume", musicVolume);
		}
		break;

	case GID_HOYLE5:
		if (index == kGlobalVarHoyle5MusicVolume) {
			const int16 masterVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / kHoyle5VolumeMax;
			ConfMan.setInt("music_volume", masterVolume);
			ConfMan.setInt("sfx_volume", masterVolume);
			ConfMan.setInt("speech_volume", masterVolume);
		}
		break;

	case GID_PHANTASMAGORIA:
		if (index == kGlobalVarPhant1MusicVolume) {
			const int16 musicVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / MUSIC_MASTERVOLUME_MAX;
			ConfMan.setInt("music_volume", musicVolume);
		} else if (index == kGlobalVarPhant1DACVolume) {
			const int16 dacVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / Audio32::kMaxVolume;
			ConfMan.setInt("sfx_volume", dacVolume);
			ConfMan.setInt("speech_volume", dacVolume);
		}
		break;

	case GID_PHANTASMAGORIA2:
		if (index == kGlobalVarPhant2MasterVolume) {
			const int16 masterVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / kPhant2VolumeMax;
			ConfMan.setInt("music_volume", masterVolume);
			ConfMan.setInt("sfx_volume", masterVolume);
			ConfMan.setInt("speech_volume", masterVolume);
		}
		break;

	case GID_RAMA:
		if (index == kGlobalVarRamaMusicVolume) {
			const int16 musicVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / kRamaVolumeMax;
			ConfMan.setInt("music_volume", musicVolume);
		}
		break;

	case GID_LSL7:
	case GID_TORIN:
		if (index == kGlobalVarTorinMusicVolume ||
			index == kGlobalVarTorinSFXVolume ||
			index == kGlobalVarTorinSpeechVolume) {

			const int16 volume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / 100;

			switch (index) {
			case kGlobalVarTorinMusicVolume:
				ConfMan.setInt("music_volume", volume);
				break;
			case kGlobalVarTorinSFXVolume:
				ConfMan.setInt("sfx_volume", volume);
				break;
			case kGlobalVarTorinSpeechVolume:
				ConfMan.setInt("speech_volume", volume);
				break;
			default:
				break;
			}
		}
		break;
#endif

	case GID_LSL6:
	case GID_LSL6HIRES:
		if (index == kGlobalVarLSL6MusicVolume) {
			const int16 musicVolume = value.toSint16() * Audio::Mixer::kMaxMixerVolume / kLSL6UIVolumeMax;
			ConfMan.setInt("music_volume", musicVolume);
		}
		break;

	default:
		break;
	}
}

#ifdef ENABLE_SCI32
void GuestAdditions::syncGK1AudioVolumeToScummVM(const reg_t soundObj, int16 volume) const {
	const Common::String objName = _segMan->getObjectName(soundObj);
	volume = volume * Audio::Mixer::kMaxMixerVolume / MUSIC_VOLUME_MAX;

	// Using highest-numbered sound objects to sync only after all slots
	// have been set by the volume slider
	if (objName == "gkMusic2") {
		ConfMan.setInt("music_volume", volume);
		g_sci->updateSoundMixerVolumes();
	} else if (objName == "gkSound3") {
		ConfMan.setInt("sfx_volume", volume);
		ConfMan.setInt("speech_volume", volume);
		g_sci->updateSoundMixerVolumes();
	}
}

#pragma mark -
#pragma mark Audio UI sync

void GuestAdditions::syncInGameUI(const int16 musicVolume, const int16 sfxVolume) const {
	if (_state->abortScriptProcessing != kAbortNone) {
		// Attempting to update a UI that is in the process of being destroyed
		// will result in a crash
		return;
	}

	switch (g_sci->getGameId()) {
	case GID_MOTHERGOOSEHIRES:
		syncMGDXUI(musicVolume);
		break;

	case GID_PQ4:
		syncPQ4UI(musicVolume);
		break;

	case GID_PQSWAT:
		syncPQSWATUI();
		break;

	case GID_QFG4:
		syncQFG4UI(musicVolume);
		break;

	case GID_HOYLE5:
		syncHoyle5UI(musicVolume);
		break;

	case GID_SHIVERS:
		syncShivers1UI(sfxVolume);
		break;

	case GID_SQ6:
		syncSQ6UI();
		break;

	default:
		break;
	}
}

void GuestAdditions::syncGK1UI() const {
	const reg_t bars[] = { _segMan->findObjectByName("musicBar"),
						   _segMan->findObjectByName("soundBar") };

	for (int i = 0; i < ARRAYSIZE(bars); ++i) {
		const reg_t barId = bars[i];
		if (!barId.isNull()) {
			// Resetting the position to 0 causes the bar to refresh its
			// position when it next draws
			writeSelectorValue(_segMan, barId, SELECTOR(position), 0);

			// The `signal` property indicates bar visibility (for some
			// reason, the normal `-info-` flag is not used)
			if (readSelectorValue(_segMan, barId, SELECTOR(signal)) & 0x20) {
				// `show` pulls a new value from the underlying sound object
				// and refreshes the bar rendering
				invokeSelector(barId, SELECTOR(show));
			}
		}
	}
}

void GuestAdditions::syncGK2UI() const {
	const reg_t sliderId = _segMan->findObjectByName("soundSlider");
	if (!sliderId.isNull() && _segMan->getObject(sliderId)->isInserted()) {
		const reg_t oldAcc = _state->r_acc;
		invokeSelector(sliderId, SELECTOR(initialOff));
		writeSelector(_segMan, sliderId, SELECTOR(x), _state->r_acc);
		_state->r_acc = oldAcc;
	}
}

void GuestAdditions::syncHoyle5UI(const int16 musicVolume) const {
	// Hoyle5 has two control panels with different slider names
	const reg_t sliders[] = { _segMan->findObjectByName("volumeSlider"),
							  _segMan->findObjectByName("volumeSliderF") };
	for (int i = 0; i < ARRAYSIZE(sliders); ++i) {
		const reg_t sliderId = sliders[i];
		if (!sliderId.isNull()) {
			const int16 yPosition = 167 - musicVolume * 145 / 10;
			writeSelectorValue(_segMan, sliderId, SELECTOR(y), yPosition);

			// There does not seem to be any good way to learn whether the
			// volume slider is visible (and thus eligible for
			// kUpdateScreenItem)
			const reg_t planeId = readSelector(_segMan, sliderId, SELECTOR(plane));
			if (g_sci->_gfxFrameout->getPlanes().findByObject(planeId) != nullptr) {
				g_sci->_gfxFrameout->kernelUpdateScreenItem(sliderId);
			}
		}
	}
}
#endif

void GuestAdditions::syncLSL6UI(const int16 musicVolume) const {
	const reg_t musicDialId = _segMan->findObjectByName("volumeDial");
	if (!musicDialId.isNull()) {
		writeSelectorValue(_segMan, musicDialId, SELECTOR(curPos), musicVolume);
		writeSelectorValue(_segMan, musicDialId, SELECTOR(cel), musicVolume);
		reg_t params[] = { make_reg(0, musicVolume) };
		// volumeDial's method is doit in the lo-res version and update in hi-res
		if (g_sci->getGameId() == GID_LSL6) {
			invokeSelector(musicDialId, SELECTOR(doit), 1, params);
		} else {
			invokeSelector(musicDialId, SELECTOR(update), 1, params);
		}
#ifdef ENABLE_SCI32
		if (g_sci->getGameId() == GID_LSL6HIRES) {
			if (_segMan->getObject(musicDialId)->isInserted()) {
				g_sci->_gfxFrameout->kernelUpdateScreenItem(musicDialId);
			}
		}
#endif
	}
}

#ifdef ENABLE_SCI32
void GuestAdditions::syncPhant1UI(const int16 oldMusicVolume, const int16 musicVolume, reg_t &musicGlobal, const int16 oldDacVolume, const int16 dacVolume, reg_t &dacGlobal) const {
	const reg_t buttonId = _segMan->findObjectByName("dacVolUp");
	if (buttonId.isNull() || !_segMan->getObject(buttonId)->isInserted()) {
		// No inserted dacVolUp button means the control panel with the
		// volume controls is not visible and we can just update the values
		// and leave
		musicGlobal.setOffset(musicVolume);
		dacGlobal.setOffset(dacVolume);
		return;
	}

	reg_t thermo = _segMan->findObjectByName("midiVolThermo");
	if (!thermo.isNull()) {
		int count = ABS(musicVolume - oldMusicVolume);
		const int stepSize = (musicVolume > oldMusicVolume ? 1 : -1);
		while (count--) {
			musicGlobal.incOffset(stepSize);
			invokeSelector(thermo, SELECTOR(doit));
		}
	}

	thermo = _segMan->findObjectByName("dacVolThermo");
	if (!thermo.isNull()) {
		int count = ABS(dacVolume - oldDacVolume) / 8;
		const int stepSize = (dacVolume > oldDacVolume ? 8 : -8);
		while (count--) {
			dacGlobal.incOffset(stepSize);
			invokeSelector(thermo, SELECTOR(doit));
		}
	}
}

void GuestAdditions::syncPhant2UI(const int16 masterVolume) const {
	const reg_t masterVolumeScript = _segMan->findObjectByName("foo2");
	Common::Array<reg_t> scrollBars = _segMan->findObjectsByName("P2ScrollBar");
	for (uint i = 0; i < scrollBars.size(); ++i) {
		if (readSelector(_segMan, scrollBars[i], SELECTOR(client)) == masterVolumeScript) {
			// P2ScrollBar objects may exist without actually being on-screen;
			// the easiest way to tell seems to be to look to see if it has
			// non-null pointers to subviews. (The game will correctly set the
			// position of the scrollbar when it first becomes visible, so this
			// is fine.)
			if (!readSelector(_segMan, scrollBars[i], SELECTOR(physicalBar)).isNull()) {
				reg_t params[] = { make_reg(0, masterVolume), make_reg(0, 1) };
				invokeSelector(scrollBars[i], SELECTOR(move), 2, params);
				break;
			}
		}
	}
}

void GuestAdditions::syncMGDXUI(const int16 musicVolume) const {
	const reg_t sliderId = _segMan->findObjectByName("icon1");
	if (!sliderId.isNull()) {
		const int16 celNo = 7 - (musicVolume * 8 / (MUSIC_MASTERVOLUME_MAX + 1));
		writeSelectorValue(_segMan, sliderId, SELECTOR(mainCel), celNo);
		writeSelectorValue(_segMan, sliderId, SELECTOR(cel), celNo);

		// There does not seem to be any good way to learn whether the
		// volume slider is visible (and thus eligible for
		// kUpdateScreenItem)
		const reg_t planeId = readSelector(_segMan, sliderId, SELECTOR(plane));
		if (g_sci->_gfxFrameout->getPlanes().findByObject(planeId) != nullptr) {
			g_sci->_gfxFrameout->kernelUpdateScreenItem(sliderId);
		}
	}
}

void GuestAdditions::syncPQ4UI(const int16 musicVolume) const {
	const SegmentId segment = _segMan->getScriptSegment(9, SCRIPT_GET_DONT_LOAD);
	if (segment != 0 && _segMan->getScript(segment)->getLocalsCount() > 2) {
		const reg_t barId = _segMan->getScript(segment)->getLocalsBegin()[2];
		if (!barId.isNull()) {
			reg_t params[] = { make_reg(0, musicVolume) };
			invokeSelector(barId, SELECTOR(setSize), 1, params);
		}
	}
}

void GuestAdditions::syncPQSWATUI() const {
	const reg_t barId = _segMan->findObjectByName("volumeLed");
	if (!barId.isNull() && _segMan->getObject(barId)->isInserted()) {
		invokeSelector(barId, SELECTOR(displayValue));
	}
}

void GuestAdditions::syncQFG4UI(const int16 musicVolume) const {
	const reg_t sliderId = _segMan->findObjectByName("volumeSlider");
	if (!sliderId.isNull()) {
		const int16 yPosition = 84 - musicVolume * 34 / 10;
		writeSelectorValue(_segMan, sliderId, SELECTOR(y), yPosition);

		// There does not seem to be any good way to learn whether the
		// volume slider is visible (and thus eligible for
		// kUpdateScreenItem)
		const reg_t planeId = readSelector(_segMan, sliderId, SELECTOR(plane));
		if (g_sci->_gfxFrameout->getPlanes().findByObject(planeId) != nullptr) {
			g_sci->_gfxFrameout->kernelUpdateScreenItem(sliderId);
		}
	}
}

void GuestAdditions::syncRamaUI(const int16 musicVolume) const {
	const reg_t sliderId = _segMan->findObjectByName("volumeSlider");
	if (!sliderId.isNull() && !readSelector(_segMan, sliderId, SELECTOR(plane)).isNull()) {
		reg_t args[] = { make_reg(0, musicVolume) };
		invokeSelector(sliderId, SELECTOR(setCel), 1, args);
	}
}

void GuestAdditions::syncShivers1UI(const int16 dacVolume) const {
	const reg_t sliderId = _segMan->findObjectByName("spVolume");
	if (!sliderId.isNull()) {
		const int16 xPosition = dacVolume * 78 / Audio32::kMaxVolume + 32;
		writeSelectorValue(_segMan, sliderId, SELECTOR(x), xPosition);
		if (_segMan->getObject(sliderId)->isInserted()) {
			g_sci->_gfxFrameout->kernelUpdateScreenItem(sliderId);
		}
	}
}

void GuestAdditions::syncSQ6UI() const {
	const reg_t bars[] = { _segMan->findObjectByName("musicBar"),
						   _segMan->findObjectByName("soundBar") };
	for (int i = 0; i < ARRAYSIZE(bars); ++i) {
		const reg_t barId = bars[i];
		if (!barId.isNull()) {
			invokeSelector(barId, SELECTOR(show));
		}
	}
}

void GuestAdditions::syncTorinUI(const int16 musicVolume, const int16 sfxVolume, const int16 speechVolume) const {
	const reg_t sliders[] = { _segMan->findObjectByName("oMusicScroll"),
							  _segMan->findObjectByName("oSFXScroll"),
							  _segMan->findObjectByName("oAudioScroll") };
	const int16 values[] = { musicVolume, sfxVolume, speechVolume };
	for (int i = 0; i < ARRAYSIZE(sliders); ++i) {
		const reg_t sliderId = sliders[i];
		if (!sliderId.isNull()) {
			reg_t params[] = { make_reg(0, values[i]) };
			invokeSelector(sliderId, SELECTOR(setPos), 1, params);
		}
	}
}

#pragma mark -
#pragma mark Talk speed sync

void GuestAdditions::syncTextSpeedFromScummVM() const {
	const int16 textSpeed = 8 - (ConfMan.getInt("talkspeed") + 1) * 8 / 255;

	_state->variables[VAR_GLOBAL][kGlobalVarTextSpeed] = make_reg(0, textSpeed);

	if (g_sci->getGameId() == GID_GK1) {
		const reg_t textBarId = _segMan->findObjectByName("textBar");
		if (!textBarId.isNull()) {
			// Resetting the bar position to 0 causes the game to retrieve the
			// new text speed value and re-render
			writeSelectorValue(_segMan, textBarId, SELECTOR(position), 0);
		}
	}
}

void GuestAdditions::syncTextSpeedToScummVM(const int index, const reg_t value) const {
	if (index == kGlobalVarTextSpeed) {
		ConfMan.setInt("talkspeed", (8 - value.toSint16()) * 255 / 8);
	}
}

#endif

} // End of namespace Sci
