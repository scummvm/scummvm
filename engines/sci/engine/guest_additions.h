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

#ifndef SCI_ENGINE_GUEST_ADDITIONS_H
#define SCI_ENGINE_GUEST_ADDITIONS_H

#include "sci/engine/vm_types.h"

namespace Sci {

struct EngineState;
class GameFeatures;
class Kernel;
class Script;
class SegManager;

enum {
	// The in-game volumes for Phant2 use a volume range smaller than the
	// actual master volume because movie volume needs to be controllable from
	// the normal ScummVM launcher volume controls, but movie dialogue cannot be
	// heard if the game audio is at the same level as movies. The game normally
	// sets defaults so that the in-game volume is 85 and movies are 127, so we
	// will just use 85 as the maximum volume.
	kPhant2VolumeMax       = 85,

	kRamaVolumeMax         = 16,
	kLSL6UIVolumeMax       = 13,
	kHoyle5VolumeMax       = 8,
	kLSL6HiresSubtitleFlag = 105
};

/**
 * The GuestAdditions class hooks into the SCI virtual machine to provide
 * enhanced interactions between the ScummVM GUI and the game engine. Currently,
 * this enhanced functionality encompasses synchronisation of audio volumes and
 * other audio-related settings, and integration of the ScummVM GUI when saving
 * and loading game states.
 *
 * NOTE: Some parts of the code used to manage audio sync are applied as script
 * patches using the normal ScriptPatcher mechanism. These patches prevent the
 * game from resetting audio volumes to defaults when starting up, and prevent
 * the game from restoring audio volumes stored inside of a save game.
 */
class GuestAdditions {
public:
	GuestAdditions(EngineState *state, GameFeatures *features, Kernel *kernel);

#pragma mark -

	/**
	 * Synchronises audio volume settings from ScummVM to the game. Called
	 * whenever the ScummVM global menu is dismissed.
	 */
	void syncSoundSettingsFromScummVM() const;

	/**
	 * Synchronises all audio settings from ScummVM to the game. Called when the
	 * game is first started, and when save games are loaded.
	 */
	void syncAudioOptionsFromScummVM() const;

	/**
	 * Clears audio settings synchronisation state.
	 */
	void reset();

private:
	EngineState *_state;
	GameFeatures *_features;
	Kernel *_kernel;
	SegManager *_segMan;

	/**
	 * Convenience function for invoking selectors that reduces boilerplate code
	 * required by Sci::invokeSelector.
	 */
	void invokeSelector(const reg_t objId, const Selector selector, const int argc = 0, const StackPtr argv = nullptr) const;

	/**
	 * Determines whether the current stack contains calls from audio controls
	 * that indicate a user-initiated change of audio settings.
	 */
	bool shouldSyncAudioToScummVM() const;

#pragma mark -
#pragma mark Hooks

public:
	/**
	 * Guest additions hook for SciEngine::runGame.
	 */
	void sciEngineRunGameHook();

	/**
	 * Guest additions hook for write_var.
	 */
	void writeVarHook(const int type, const int index, const reg_t value);

	/**
	 * Guest additions hook for kDoSoundMasterVolume.
	 *
	 * @returns true if the default action should be prevented
	 */
	bool kDoSoundMasterVolumeHook(const int volume) const;

#ifdef ENABLE_SCI32
	/**
	 * Guest additions hook for SciEngine::initGame.
	 */
	void sciEngineInitGameHook();

	/**
	 * Guest additions hook for send_selector.
	 */
	void sendSelectorHook(const reg_t sendObj, Selector &selector, reg_t *argp);

	/**
	 * Guest additions hook for Audio32::setVolume.
	 *
	 * @returns true if the default action should be prevented
	 */
	bool audio32SetVolumeHook(const int16 channelIndex, const int16 volume) const;

	/**
	 * Guest additions hook for kDoSoundSetVolume.
	 */
	void kDoSoundSetVolumeHook(const reg_t soundObj, const int16 volume) const;

	/**
	 * Guest additions hook for SegManager::instantiateScript.
	 */
	void instantiateScriptHook(Script &script, const bool ignoreDelayedRestore = false) const;

	/**
	 * Guest additions hook for SegManager::saveLoadWithSerializer.
	 */
	void segManSaveLoadScriptHook(Script &script) const;
#endif

	/**
	 * Guest additions hook for kGetEvent.
	 */
	bool kGetEventHook() const;

	/**
	 * Guest additions hook for kWait.
	 */
	bool kWaitHook() const;

#ifdef ENABLE_SCI32
	/**
	 * Guest additions hook for kPlayDuck(Play) and kPlayVMD(PlayUntilEvent).
	 */
	bool kPlayDuckPlayVMDHook() const;
#endif

#pragma mark -
#pragma mark Integrated save & restore

public:
	/**
	 * Patches game scripts to use the ScummVM save/load dialogue instead of the
	 * game's native save/load dialogue when a user tries to save or restore a
	 * game from inside the game.
	 */
	void patchGameSaveRestore() const;

private:
	/**
	 * Patches the ScummVM save/load dialogue into the game for SCI16 games that
	 * use Game::save and Game::restore.
	 */
	void patchGameSaveRestoreSCI16() const;

#ifdef ENABLE_SCI32
public:
	/**
	 * Finds the correct save file number and description to save or load and
	 * returns it to the VM. For user-interactive save file lookup, this method
	 * displays the ScummVM save/load dialogue. For delayed restores, it returns
	 * the save game number sent by the ScummVM launcher without prompting the
	 * user.
	 */
	reg_t kScummVMSaveLoad(EngineState *s, int argc, reg_t *argv) const;

private:
	/**
	 * Patches the ScummVM save/load dialogue into SCI32 games that use
	 * SRDialog.
	 */
	void patchGameSaveRestoreSCI32(Script &script) const;

	/**
	 * Patches the ScummVM save/load dialogue into Torin/LSL7.
	 */
	void patchGameSaveRestoreTorin(Script &script) const;

	/**
	 * Patches the ScummVM save/load dialogue into Phant2.
	 */
	void patchGameSaveRestorePhant2(Script &script) const;

	/**
	 * Patches the ScummVM save/load dialogue into RAMA.
	 */
	void patchGameSaveRestoreRama(Script &script) const;

	/**
	 * Patches the `doit` method of an SRDialog object with the given name
	 * using the given patch data.
	 */
	void patchSRDialogDoit(Script &script, const char *const objectName, const byte *patchData, const int patchSize, const int *uint16Offsets = nullptr, const uint numOffsets = 0) const;

	/**
	 * Prompts for a save game and returns it to game scripts using default
	 * SRDialog game class semantics.
	 */
	reg_t promptSaveRestoreDefault(EngineState *s, int argc, reg_t *argv) const;

	/**
	 * Prompts for a save game and returns it to game scripts using Torin/LSL7's
	 * custom NewGame class semantics.
	 */
	reg_t promptSaveRestoreTorin(EngineState *s, int argc, reg_t *argv) const;

	/**
	 * Prompts for a save game and returns it to game scripts using Phant2's
	 * custom ControlPanel class semantics.
	 */
	reg_t promptSaveRestorePhant2(EngineState *s, int argc, reg_t *argv) const;

	/**
	 * Prompts for a save game and returns it to game scripts using RAMA's
	 * custom SRDialog class semantics.
	 */
	reg_t promptSaveRestoreRama(EngineState *s, int argc, reg_t *argv) const;

	/**
	 * Prompts for a save game and returns it to game scripts using Hoyle 5's
	 * custom SRDialog class semantics.
	 */
	reg_t promptSaveRestoreHoyle5(EngineState *s, int argc, reg_t *argv) const;

public:
	/**
	 * Prompts the user to save or load a game.
	 *
	 * @param isSave If true, the prompt is for saving.
	 * @param outDescription Will be filled with the save game description.
	 * Optional for loads, required for saves.
	 * @param forcedSaveId During delayed restore, force the returned save game
	 * id to this value.
	 */
	int runSaveRestore(const bool isSave, const reg_t outDescription, const int forcedSaveId = -1) const;
	int runSaveRestore(const bool isSave, Common::String &outDescription, const int forcedSaveId = -1) const;
#endif

#pragma mark -
#pragma mark Restore from launcher

private:
	/**
	 * Invokes the game's save restore mechanism to load a save game that was
	 * selected in the ScummVM launcher when the game was started.
	 */
	bool restoreFromLauncher() const;

#ifdef ENABLE_SCI32
	/**
	 * If true, GuestAdditions is in the process of handling a delayed game
	 * restore from the ScummVM launcher or global menu.
	 */
	mutable bool _restoring;
#endif

#pragma mark -
#pragma mark Message type sync

private:
	/**
	 * True if the message type (text/speech/text+speech) has been synchronised
	 * from ScummVM to the game.
	 */
	bool _messageTypeSynced;

	/**
	 * Synchronises the message type (speech/text/speech+text) from ScummVM to
	 * a game.
	 */
	void syncMessageTypeFromScummVM() const;

	/**
	 * Synchronises the message type from ScummVM using the default strategy
	 * (global90).
	 */
	void syncMessageTypeFromScummVMUsingDefaultStrategy() const;

#ifdef ENABLE_SCI32
	/**
	 * Synchronises the message type from ScummVM using the strategy used by
	 * Shivers (global211).
	 */
	void syncMessageTypeFromScummVMUsingShiversStrategy() const;

	/**
	 * Synchronises the message type from ScummVM using the strategy used by
	 * LSL6hires (gameFlags).
	 */
	void syncMessageTypeFromScummVMUsingLSL6HiresStrategy() const;
#endif

	/**
	 * Synchronises the message type (speech/text/speech+text) from a game to
	 * ScummVM.
	 */
	void syncMessageTypeToScummVM(const int index, const reg_t value);

	/**
	 * Synchronises the message type to ScummVM using the default strategy
	 * (global90).
	 */
	void syncMessageTypeToScummVMUsingDefaultStrategy(const int index, const reg_t value);

#ifdef ENABLE_SCI32
	/**
	 * Synchronises the message type to ScummVM using the strategy used by
	 * Shivers (global211).
	 */
	void syncMessageTypeToScummVMUsingShiversStrategy(const int index, const reg_t value);

	/**
	 * Synchronises the message type to ScummVM using the strategy used by
	 * LSL6hires (gameFlags).
	 */
	void syncMessageTypeToScummVMUsingLSL6HiresStrategy(const reg_t sendObj, Selector &selector, reg_t *argp);
#endif

#pragma mark -
#pragma mark Master volume sync

private:
	/**
	 * Synchronises audio volume settings from ScummVM to the game, for games
	 * that do not store volume themselves and just call to the kernel.
	 */
	void syncMasterVolumeFromScummVM() const;

	/**
	 * Synchronises audio volume settings from the game to ScummVM, for games
	 * that do not store volume themselves and just call to the kernel.
	 */
	void syncMasterVolumeToScummVM(const int16 masterVolume) const;

#pragma mark -
#pragma mark Globals volume sync

private:
	/**
	 * Synchronises audio volume settings from ScummVM to the game, for games
	 * that store volumes in globals.
	 */
	void syncAudioVolumeGlobalsFromScummVM() const;

	void syncLSL6VolumeFromScummVM(const int16 musicVolume) const;

#ifdef ENABLE_SCI32
	/**
	 * Synchronises audio volume settings from ScummVM to GK1 at game startup
	 * time.
	 */
	void syncGK1StartupVolumeFromScummVM(const int index, const reg_t value) const;

	/**
	 * Synchronises audio volume settings from ScummVM to GK1 when the game is
	 * running.
	 */
	void syncGK1VolumeFromScummVM(const int16 musicVolume, const int16 dacVolume) const;

	void syncGK2VolumeFromScummVM(const int16 musicVolume) const;
	void syncHoyle5VolumeFromScummVM(const int16 musicVolume) const;
	void syncPhant2VolumeFromScummVM(const int16 masterVolume) const;
	void syncRamaVolumeFromScummVM(const int16 musicVolume) const;
	void syncTorinVolumeFromScummVM(const int16 musicVolume, const int16 sfxVolume, const int16 speechVolume) const;
#endif

	/**
	 * Synchronises audio volume settings from a game to ScummVM, for games
	 * that store volumes in globals.
	 */
	void syncAudioVolumeGlobalsToScummVM(const int index, const reg_t value) const;

#ifdef ENABLE_SCI32
	/**
	 * Synchronises audio volume settings from GK1 to ScummVM.
	 */
	void syncGK1AudioVolumeToScummVM(const reg_t soundObj, const int16 volume) const;

#pragma mark -
#pragma mark Audio UI sync

private:
	/**
	 * Synchronises the in-game control panel UI in response to a change of
	 * volume from the ScummVM GUI. The values of the volume parameters passed
	 * to this function are game-specific.
	 */
	void syncInGameUI(const int16 musicVolume, const int16 sfxVolume) const;

	void syncGK1UI() const;
	void syncGK2UI() const;
	void syncHoyle5UI(const int16 musicVolume) const;
#endif
	void syncLSL6UI(const int16 musicVolume) const;
#ifdef ENABLE_SCI32
	void syncMGDXUI(const int16 musicVolume) const;
	void syncPhant1UI(const int16 oldMusicVolume, const int16 musicVolume, reg_t &musicGlobal, const int16 oldDacVolume, const int16 dacVolume, reg_t &dacGlobal) const;
	void syncPhant2UI(const int16 masterVolume) const;
	void syncPQ4UI(const int16 musicVolume) const;
	void syncPQSWATUI() const;
	void syncQFG4UI(const int16 musicVolume) const;
	void syncRamaUI(const int16 musicVolume) const;
	void syncShivers1UI(const int16 dacVolume) const;
	void syncSQ6UI() const;
	void syncTorinUI(const int16 musicVolume, const int16 sfxVolume, const int16 speechVolume) const;

#pragma mark -
#pragma mark Talk speed sync

private:
	/**
	 * Synchronises text speed settings from ScummVM to a game.
	 */
	void syncTextSpeedFromScummVM() const;

	/**
	 * Synchronises text speed settings from a game to ScummVM.
	 */
	void syncTextSpeedToScummVM(const int index, const reg_t value) const;
#endif
};

} // End of namespace Sci

#endif // SCI_ENGINE_GUEST_ADDITIONS_H
