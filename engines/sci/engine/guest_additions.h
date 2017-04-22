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

/**
 * The GuestAdditions class hooks into the SCI virtual machine to provide
 * enhanced interactions between the ScummVM GUI and the game engine. Currently,
 * this enhanced functionality encompasses synchronisation of audio volumes and
 * other audio-related settings.
 *
 * Some parts of the audio sync are applied as script patches using the normal
 * ScriptPatcher mechanism. These patches are designed to prevent the game from
 * resetting to a default volume when starting up or loading a save.
 */
class GuestAdditions {
public:
	GuestAdditions(EngineState *state, GameFeatures *features, Kernel *kernel);

#pragma mark -

	/**
	 * Synchronises audio volume settings from ScummVM to the game. Called
	 * whenever the ScummVM launcher is dismissed.
	 */
	void syncSoundSettings() const;

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
	bool shouldSyncAudio() const;

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
	void instantiateScriptHook(Script &script) const;
#endif

#pragma mark -
#pragma mark Save & restore

public:
	/**
	 * Patches game scripts to hook into the ScummVM launcher UI when a user
	 * tries to save or restore a game from inside the game.
	 */
	void patchGameSaveRestore() const;

private:
	void patchGameSaveRestoreSCI16() const;

#ifdef ENABLE_SCI32
public:
	reg_t kScummVMSaveLoad(EngineState *s, int argc, reg_t *argv) const;

private:
	void patchGameSaveRestoreSCI32(Script &script) const;
#endif

#pragma mark -
#pragma mark Message type sync

private:
	/**
	 * true if the message type (text/speech/text+speech) has been synchronised
	 * from ScummVM to the game.
	 */
	bool _messageTypeSynced;

	/**
	 * Synchronises the message type (speech/text/speech+text) from a ScummVM to
	 * a game.
	 */
	void syncMessageTypeFromScummVM() const;

	void syncMessageTypeFromScummVMUsingDefaultStrategy() const;
#ifdef ENABLE_SCI32
	void syncMessageTypeFromScummVMUsingShiversStrategy() const;
	void syncMessageTypeFromScummVMUsingLSL6HiresStrategy() const;
#endif

	/**
	 * Synchronises the message type (speech/text/speech+text) from a game to
	 * ScummVM.
	 */
	void syncMessageTypeToScummVM(const int index, const reg_t value);

	void syncMessageTypeToScummVMUsingDefaultStrategy(const int index, const reg_t value);
#ifdef ENABLE_SCI32
	void syncMessageTypeToScummVMUsingShiversStrategy(const int index, const reg_t value);
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

#ifdef ENABLE_SCI32
#pragma mark -
#pragma mark Globals volume sync

private:
	/**
	 * Synchronises audio volume settings from ScummVM to the game, for games
	 * that store volumes in globals.
	 */
	void syncAudioVolumeGlobalsFromScummVM() const;

	/**
	 * Synchronises audio volume settings from ScummVM to GK1 at game startup
	 * time.
	 */
	void syncGK1StartupVolumeFromScummVM(const int index, const reg_t value) const;

	void syncGK1VolumeFromScummVM(const int16 musicVolume, const int16 dacVolume) const;
	void syncGK2VolumeFromScummVM(const int16 musicVolume) const;
	void syncLSL6HiresVolumeFromScummVM(const int16 musicVolume) const;
	void syncTorinVolumeFromScummVM(const int16 musicVolume, const int16 sfxVolume, const int16 speechVolume) const;

	/**
	 * Synchronises audio volume settings from a game to ScummVM, for games
	 * that store volumes in globals.
	 */
	void syncAudioVolumeGlobalsToScummVM(const int index, const reg_t value) const;

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
	void syncLSL6HiresUI(const int16 musicVolume) const;
	void syncPhant1UI(const int16 oldMusicVolume, const int16 musicVolume, reg_t &musicGlobal, const int16 oldDacVolume, const int16 dacVolume, reg_t &dacGlobal) const;
	void syncPQ4UI(const int16 musicVolume) const;
	void syncPQSWATUI() const;
	void syncQFG4UI(const int16 musicVolume) const;
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
