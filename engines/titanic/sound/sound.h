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

#ifndef TITANIC_SOUND_H
#define TITANIC_SOUND_H

#include "titanic/support/simple_file.h"
#include "titanic/sound/proximity.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/sound/wave_file.h"
#include "titanic/core/list.h"
#include "titanic/core/view_item.h"
#include "titanic/true_talk/dialogue_file.h"

namespace Titanic {

class CGameManager;

class CSoundItem : public ListItem {
public:
	CString _name;
	CWaveFile *_waveFile;
	File *_dialogueFileHandle;
	int _speechId;
	DisposeAfterUse::Flag _disposeAfterUse;
	bool _active;
public:
	CSoundItem() : ListItem(), _waveFile(nullptr), _dialogueFileHandle(nullptr),
		_speechId(0), _disposeAfterUse(DisposeAfterUse::NO), _active(false) {}
	CSoundItem(const CString &name) : ListItem(), _name(name), _waveFile(nullptr),
		_dialogueFileHandle(nullptr), _disposeAfterUse(DisposeAfterUse::NO),
		_speechId(0), _active(false) {}
	CSoundItem(File *dialogueFile, int speechId) : ListItem(), _waveFile(nullptr),
		_dialogueFileHandle(dialogueFile), _speechId(speechId), _active(false),
		_disposeAfterUse(DisposeAfterUse::NO) {}
	~CSoundItem() override;
};

class CSoundItemList : public List<CSoundItem> {
};

class CSound {
private:
	CGameManager *_gameManager;
	CSoundItemList _sounds;
private:
	/**
	 * Check whether any sounds are done and can be be removed
	 */
	void checkSounds();

	/**
	 * Removes the oldest sound from the sounds list that isn't
	 * currently playing
	 */
	void removeOldest();
public:
	QSoundManager _soundManager;
public:
	CSound(CGameManager *owner, Audio::Mixer *mixer);
	~CSound();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Called when a game is about to be loaded
	 */
	void preLoad();

	/**
	 * Called when loading a game is complete
	 */
	void postLoad() { _soundManager.postLoad(); }

	/**
	 * Called when a game is about to be saved
	 */
	void preSave() { _soundManager.preSave(); }

	/**
	 * Called when a game has finished being saved
	 */
	void postSave() { _soundManager.postSave(); }

	/**
	 * Called when the view has been changed
	 */
	void preEnterView(CViewItem *newView, bool isNewRoom);

	/**
	 * Returns true if a sound with the specified handle is active
	 */
	bool isActive(int handle);

	/**
	 * Sets the volume for a sound
	 * @param handle	Sound handle
	 * @param volume	Volume percentage (0 to 100)
	 * @param seconds	Number of seconds to transition to the new volume
	 */
	void setVolume(uint handle, uint volume, uint seconds);

	/**
	 * Flags a sound about to be played as activated
	 */
	void activateSound(CWaveFile *waveFile,
		DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::NO);

	/**
	 * Stops any sounds attached to a given channel
	 */
	void stopChannel(int channel);

	/**
	 * Loads a TrueTalk dialogue
	 * @param dialogueFile	Dialogue file reference
	 * @param speechId		Speech Id within dialogue
	 * @returns				Wave file instance
	 */
	CWaveFile *getTrueTalkSound(CDialogueFile *dialogueFile, int index);

	/**
	 * Load a speech resource
	 * @param dialogueFile	Dialogue file reference
	 * @param speechId		Speech Id within dialogue
	 * @returns				Wave file instance
	 */
	CWaveFile *loadSpeech(CDialogueFile *dialogueFile, int speechId);

	/**
	 * Play a speech
	 * @param dialogueFile	Dialogue file reference
	 * @param speechId		Speech Id within dialogue
	 * @param prox			Proximity instance
	 */
	int playSpeech(CDialogueFile *dialogueFile, int speechId, CProximity &prox);

	/**
	 * Load a sound
	 * @param name		Name of sound resource
	 * @returns			Sound item record
	 */
	CWaveFile *loadSound(const CString &name);

	/**
	 * Play a sound
	 */
	int playSound(const CString &name, CProximity &prox);

	/**
	 * Stop a sound
	 */
	void stopSound(uint handle);

	/**
	 * Flags that a sound can be freed if a timeout is set
	 */
	void setCanFree(int handle);

	/**
	 * Handles regularly updating the mixer
	 */
	void updateMixer();
};

} // End of namespace Titanic

#endif /* TITANIC_SOUND_H */
