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

#ifndef MOHAWK_ZOOMBINI_SOUND_H
#define MOHAWK_ZOOMBINI_SOUND_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/queue.h"

#include "mohawk/resource.h"
#include "mohawk/sound.h"
#include "mohawk/zoombini_resource.h"

namespace Mohawk {

class MohawkEngine_Zoombini;
class ZoombiniPage;

/**
 * Zoombini sound owner and arbitration layer.
 *
 * There are two intentionally separate queue systems:
 *
 * - The SCRB/SCRS script-sound queue collects candidates during one render pass
 *   and selects one priority winner. The selected entry remains tracked until
 *   its mixer handle finishes, so a loop resource cannot become an orphaned
 *   stream when a later script sound wins.
 * - The handles in @ref _soundQueues are independent FIFO channels for explicit
 *   page sequences. Each channel plays its own entries in order and can run in
 *   parallel with other channels.
 *
 * Immediate playback through @ref playSound() is a third dispatch choice for
 * sounds that are not part of script arbitration. The base Mohawk sound decoder
 * still owns the MHWK/WAVE format rules, including embedded loop points; this
 * class decides which code path owns the resulting mixer handle.
 */
class ZoombiniSound : public Sound {
public:
	/** Create the page-aware Zoombini sound manager. */
	ZoombiniSound(MohawkEngine_Zoombini *vm);
	/** Stop playback and release sound queues. */
	~ZoombiniSound() override;

	/** Play a resolved sound at the default volume. */
	Audio::SoundHandle *playSound(ZmbResource resource, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType, bool loop = false);
	/** Play a resolved sound at an explicit volume. */
	Audio::SoundHandle *playSound(ZmbResource resource, Audio::Mixer::SoundType soundType, byte volume, bool loop);
	/** Return whether any mixer handle for one exact resolved resource is active. */
	bool isSoundPlaying(ZmbResource resource) const;
	/** Stop every active mixer handle for one exact resolved resource. */
	void stopSound(ZmbResource resource);
	/** Pause every active non-music SND instance except the standard button sound when a modal dialog opens. */
	void pauseActiveSoundsForDialog();
	/** Pause active PCM music while the credits dialog is visible. */
	void pauseActiveMusicSoundsForDialog();
	/** Resume the SND instances paused by the dialog that just closed. */
	void resumeActiveSoundsAfterDialog();
	/** Stop and discard every loaded or queued PCM sound. */
	void releaseAllLoadedSounds();

	/** Start one SCRB/SCRS sound collection pass while retaining its active winner. */
	void beginScriptSoundFrame();
	/**
	 * Add one SCRB/SCRS sound candidate to the current render-frame queue.
	 *
	 * MHWK embedded loops are a format-level playback rule and are independent
	 * of queue arbitration.
	 *
	 * @param resource Resolved page or system SND resource.
	 * @param soundType Mixer volume category for this candidate.
	 * @param forcePriority Select this candidate above every authored page range.
	 */
	void enqueueScriptSound(ZmbResource resource, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType, bool forcePriority = false);
	/** True when at least one new candidate awaits render-pass arbitration. */
	bool hasQueuedScriptSoundThisFrame() const { return _scriptSoundQueuedThisFrame; }
	/** Select and play the one winning SCRB/SCRS sound for this render frame. */
	void flushScriptSoundFrame(const ZoombiniPage &page);

	/**
	 * Suppress SFX playback.
	 * When true, @ref ZoombiniSound::playSound() with @ref Audio::Mixer::kSFXSoundType returns early
	 * without playing.
	 * Used by Smoke puzzle to mute ambient sounds during question phases.
	 */
	void setSfxMuted(bool muted) { _sfxMuted = muted; }
	bool isSfxMuted() const { return _sfxMuted; }

	/**
	 * Opaque handle returned by @ref ZoombiniSound::createSoundQueue().
	 * @ref ZoombiniSound::kInvalidSoundQueueHandle is the null or uninitialized value.
	 */
	typedef uint32 ZmbSoundQueueHandle;
	/** null or uninitialized value */
	static constexpr ZmbSoundQueueHandle kInvalidSoundQueueHandle = 0;

	/**
	 * Create a new independent sound queue and return its handle.
	 * Multiple queues run in parallel; each plays its entries sequentially.
	 */
	ZmbSoundQueueHandle createSoundQueue();

	/**
	 * Stop the queue's current sound, discard all pending entries, and
	 * release the handle. The handle must not be used after this call.
	 */
	void deleteSoundQueue(ZmbSoundQueueHandle handle);

	/** Append a sound to the end of the specified queue. */
	void queueSound(ZmbSoundQueueHandle handle, ZmbResource resource, Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType, bool loop = false);
	/** Append a sound with explicit volume to the specified queue. */
	void queueSound(ZmbSoundQueueHandle handle, ZmbResource resource, Audio::Mixer::SoundType soundType, byte volume, bool loop = false);

	/** Called every frame to advance all active queues. */
	void updateSoundQueue();

	/** Stop the current sound and discard all pending entries in the queue. */
	void stopSoundQueue(ZmbSoundQueueHandle handle);

	/** Stop and discard every queue that has been created. */
	void stopAllSoundQueues();

	/** Returns true while the queue has a sound actively playing. */
	bool isSoundQueuePlaying(ZmbSoundQueueHandle handle) const;

private:
	/**
	 * Numeric-ID-only Mohawk sound APIs cannot distinguish page and system archives.
	 * Keep them inaccessible through ZoombiniSound so callers use an exact
	 * @ref ZmbResource, an explicit queue, or @ref releaseAllLoadedSounds().
	 */
	using Sound::getNumSamplesPlayed;
	using Sound::isPlaying;
	using Sound::playSound;
	using Sound::stopSound;

	struct SoundQueueEntry {
		/** Resolved resource to play. */
		ZmbResource resource;
		/** Mixer channel used by this entry. */
		Audio::Mixer::SoundType soundType;
		/** Mixer volume for this entry. */
		byte volume;
		/** Whether playback should loop. */
		bool loop;
	};

	struct SoundQueueChannel {
		/** FIFO entries waiting on this channel. */
		Common::Queue<SoundQueueEntry> queue;
		/** Mixer handle for the channel's current entry. */
		Audio::SoundHandle currentHandle;
		/** Whether the channel currently has an active entry. */
		bool playing = false;
	};

	struct ScriptSoundQueueEntry {
		/** Resolved script sound candidate. */
		ZmbResource resource;
		/** Mixer volume category retained through script arbitration. */
		Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType;
		/** Mixer handle for the selected script sound. */
		Audio::SoundHandle handle;
		/** Whether this candidate has begun playback. */
		bool started = false;
		/** Whether @ref handle is valid. */
		bool hasHandle = false;
		/** Whether this candidate bypasses normal range priority. */
		bool forcePriority = false;
	};

	struct ActiveZmbSoundEntry {
		/** Exact resource represented by this mixer handle. */
		ZmbResource resource;
		/** Mixer channel used by the resource. */
		Audio::SoundHandle handle;
		/** Mixer channel classification used to select dialog pause behavior. */
		Audio::Mixer::SoundType soundType;
		/** Nested-dialog depth that owns this pause. */
		uint32 dialogPauseDepth = 0;
	};

	/** Engine instance that owns this sound manager. */
	MohawkEngine_Zoombini *_vm;

	/** Whether ordinary SFX playback is currently muted. */
	bool _sfxMuted = false;
	/** Exact resource addresses and mixer handles retained for playback ownership and dialog pauses. */
	Common::Array<ActiveZmbSoundEntry> _activeZmbSoundEntries;
	/** Current nested modal depth used to pair each paused handle with its owning dialog. */
	uint32 _dialogSoundPauseDepth = 0;
	/** Active script winner plus candidates waiting for the next render pass. */
	Common::Array<ScriptSoundQueueEntry> _scriptSoundQueue;
	bool _scriptSoundQueuedThisFrame = false;

	/** Independent FIFO channels for authored page sound sequences. */
	Common::HashMap<uint32, SoundQueueChannel> _soundQueues;
	/** Next independent queue handle; zero is reserved as invalid. */
	uint32 _nextQueueHandle = 1; // 0 is kInvalidSoundQueueHandle

	/** Advance a single channel: start next sound if the current one finished. */
	void updateChannel(SoundQueueChannel &ch);
	/** Internal helper: stop and drain a channel without erasing it from the map. */
	void stopChannel(SoundQueueChannel &ch);
	/** Discard exact-resource mixer entries for streams that have already finished. */
	void pruneInactiveSoundEntries();
};

/**
 * Play Zoombini MIDI resources through ScummVM's selected MIDI driver.
 *
 * When ScummVM's music-device scan reaches the Windows plugin, the first MIDI query after boot
 * can pause for about ten seconds while this player is constructed.
 * @ref MidiDriver::detectDevice() asks the Windows music plugin to enumerate WinMM outputs
 * through midiOutGetNumDevs().
 * On systems with Windows MIDI Services enabled, wdmaud2.drv forwards that legacy request
 * to the demand-started MidiSrv service.
 * If MidiSrv is still stopped after boot, this request starts it and enumerates its endpoints.
 * This can take several seconds, and more with larger device sets, even when the caller uses WinMM.
 * Detection runs on the engine's main thread, so event handling is also suspended during the wait.
 * The service and endpoint state remain warm outside the ScummVM process,
 * so later launches are normally fast.
 */
class ZoombiniMidiPlayer : public MidiPlayer {
public:
	/** Create the Zoombini MIDI player. */
	ZoombiniMidiPlayer(MohawkEngine_Zoombini *vm);
	/** Stop MIDI playback and release the player. */
	~ZoombiniMidiPlayer() override;

	/** Start playback of a resolved Zoombini MIDI resource. */
	void playMidi(ZmbResource resource);
	/** Apply the ScummVM music volume while respecting the game BGM gate. */
	void syncBgmVolume(bool enabled);
	/** Stop the current MIDI track. */
	void stopMidi() { stop(); }

private:
	/** Keep the numeric-ID base overload available internally but inaccessible to Zoombini callers. */
	using MidiPlayer::playMidi;

	/** Engine instance that owns this MIDI player. */
	MohawkEngine_Zoombini *_vm;
};

} // End of namespace Mohawk

#endif
