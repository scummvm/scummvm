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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_SoundEngine
 * -------------
 * This is the sound engine interface that contains all the methods a sound
 * engine must implement.
 * Implementations of the sound engine have to be derived from this class.
 * It should be noted that a sound engine must maintain a list of all the
 * samples it uses, and that these samples should be released when the
 * destructor is called.
 *
 * Autor: Malte Thiesen
 */

#ifndef SWORD25_SOUNDENGINE_H
#define SWORD25_SOUNDENGINE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/resservice.h"
#include "sword25/kernel/persistable.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Class definitions
// -----------------------------------------------------------------------------

class SoundEngine : public ResourceService, public Persistable {
public:
	// -----------------------------------------------------------------------------
	// Enums and Types
	// -----------------------------------------------------------------------------

	enum SOUND_TYPES {
		MUSIC = 0,
		SPEECH = 1,
		SFX = 2
	};

	/**
	 * The callback function of PlayDynamicSoundEx
	 * @param UserData      User-specified pointer
	 * @param Data          Pointer to the data buffer
	 * @param DataLength    Length of the data to be written in bytes
	*/
	typedef void (*DynamicSoundReadCallback)(void *UserData, void *Data, unsigned int DataLength);

	// -----------------------------------------------------------------------------
	// Constructor / destructor
	// -----------------------------------------------------------------------------

	SoundEngine(Kernel *pKernel);
	virtual ~SoundEngine() {};

	// --------------------------------------------------------------
	// THIS METHOD MUST BE IMPLEMENTED BY THE SOUND ENGINE
	// --------------------------------------------------------------

	/**
	 * Initialises the sound engine
	 * @param SampleRate    Specifies the sample rate to use.
	 * @param Channels      The maximum number of channels. The default is 32.
	 * @return              Returns true on success, otherwise false.
	 * @remark              Calls to other methods may take place only if this
	 * method was called successfully.
	 */
	virtual bool Init(unsigned int SampleRate, unsigned int Channels = 32) = 0;

	/**
	 * Performs a "tick" of the sound engine
	 *
	 * This method should be called once per frame. It can be used by implementations
	 * of the sound engine that are not running in their own thread, or to perform
	 * additional administrative tasks that are needed.
	 */
	virtual void Update() = 0;

	/**
	 * Sets the default volume for the different sound types
	 * @param Volume        The default volume level (0 = off, 1 = full volume)
	 * @param Type          The SoundType whose volume is to be changed
	 */
	virtual void SetVolume(float Volume, SOUND_TYPES Type) = 0;

	/**
	 * Specifies the default volume of different sound types
	 * @param Type          The SoundType
	 * @return              Returns the standard sound volume for the given type
	 * (0 = off, 1 = full volume).
	*/
	virtual float GetVolume(SOUND_TYPES Type) = 0;

	/**
	 * Pauses all the sounds that are playing.
	 */
	virtual void PauseAll() = 0;

	/**
	 * Resumes all currently stopped sounds
	 */
	virtual void ResumeAll() = 0;

	/**
	 * Pauses all sounds of a given layer.
	 * @param Layer         The Sound Layer
	*/
	virtual void PauseLayer(unsigned int Layer) = 0;

	/**
	 * Resumes all the sounds in a layer that was previously stopped with PauseLayer()
	 * @param Layer         The Sound Layer
	*/
	virtual void ResumeLayer(unsigned int Layer) = 0;


	/**
	 * Plays a sound
	 * @param FileName      The filename of the sound to be played
	 * @param Type          The type of sound
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 * @param Loop          Indicates whether the sound should be looped
	 * @param LoopStart     Indicates the starting loop point. If a value less than 0 is passed, the start
	 * of the sound is used.
	 * @param LoopEnd       Indicates the ending loop point. If a avlue is passed less than 0, the end of
	 * the sound is used.
	 * @param Layer         The sound layer
	 * @return              Returns true if the playback of the sound was started successfully.
	 * @remark              If more control is needed over the playing, eg. changing the sound parameters
	 * for Volume and Panning, then PlaySoundEx should be used.
	*/
	virtual bool PlaySound(const Common::String &FileName, SOUND_TYPES Type, float Volume = 1.0f, float Pan = 0.0f, bool Loop = false, int LoopStart = -1, int LoopEnd = -1, unsigned int Layer = 0) = 0;

	/**
	 * Plays a sound
	 * @param Type          The type of sound
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 * @param Loop          Indicates whether the sound should be looped
	 * @param LoopStart     Indicates the starting loop point. If a value less than 0 is passed, the start
	 * of the sound is used.
	 * @param LoopEnd       Indicates the ending loop point. If a avlue is passed less than 0, the end of
	 * the sound is used.
	 * @param Layer         The sound layer
	 * @return              Returns a handle to the sound. With this handle, the sound can be manipulated during playback.
	 * @remark              If more control is needed over the playing, eg. changing the sound parameters
	 * for Volume and Panning, then PlaySoundEx should be used.
	 */
	virtual unsigned int PlaySoundEx(const Common::String &FileName, SOUND_TYPES Type, float Volume = 1.0f, float Pan = 0.0f, bool Loop = false, int LoopStart = -1, int LoopEnd = -1, unsigned int Layer = 0) = 0;

	/**
	 * Plays a sound generated at runtime
	 * @param ReadCallback  A pointer to a callback function that is called when sound data is needed.
	 * See the documentation for DynamicSoundReadCallback for more information.
	 * @param UserData      A pointer to the data. These are passed to the callback function each time.
	 * If no such data is needed, this parameter can be set to NULL.
	 * @param Type          The type of sound
	 * @param SampleRate    The sample rate for the sound
	 * @param BitsPerSample The size of the sample in bits. This statement is independant of the number of
	 * channels. Allowed values are 8, 16, 24, and 32.
	 * @param Channels      The number of channels. Allowed values are 1 and 2.
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 * @param Layer         The sound layer
	 * @return              Returns a handle to the sound. With this handle, the sound can be manipulated during playback.
	 * @remark              Dynamic sounds cannot be persisted.
	 */
	virtual unsigned int PlayDynamicSoundEx(DynamicSoundReadCallback ReadCallback, void *UserData, SOUND_TYPES Type, unsigned int SampleRate, unsigned int BitsPerSample, unsigned int Channels, float Volume = 1.0f, float Pan = 0.0f, unsigned int Layer = 0) = 0;

	/**
	 * Sets the volume of a playing sound
	 * @param Handle        The sound handle
	 * @param Volume        The volume of the sound (0 = off, 1 = full volume)
	 */
	virtual void SetSoundVolume(unsigned int Handle, float Volume) = 0;

	/**
	 * Sets the panning of a playing sound
	 * @param Handle        The sound handle
	 * @param Pan           Panning (-1 = full left, 1 = right)
	 */
	virtual void SetSoundPanning(unsigned int Handle, float Pan) = 0;

	/**
	 * Pauses a playing sound
	 * @param Handle        The sound handle
	 */
	virtual void PauseSound(unsigned int Handle) = 0;

	/**
	 * Resumes a paused sound
	 * @param Handle        The sound handle
	 */
	virtual void ResumeSound(unsigned int Handle) = 0;

	/**
	 * Stops a playing sound
	 * @param Handle        The sound handle
	 * @remark              Calling this method invalidates the passed handle; it can no longer be used.
	 */
	virtual void StopSound(unsigned int Handle) = 0;

	/**
	 * Returns whether a sound is paused
	 * @param Handle        The sound handle
	 * @return              Returns true if the sound is paused, false otherwise.
	 */
	virtual bool IsSoundPaused(unsigned int Handle) = 0;

	/**
	 * Returns whether a sound is still playing.
	 * @param Handle        The sound handle
	 * @return              Returns true if the sound is playing, false otherwise.
	*/
	virtual bool IsSoundPlaying(unsigned int Handle) = 0;

	/**
	 * Returns the volume of a playing sound (0 = off, 1 = full volume)
	 */
	virtual float GetSoundVolume(unsigned int Handle) = 0;

	/**
	 * Returns the panning of a playing sound (-1 = full left, 1 = right)
	 */
	virtual float GetSoundPanning(unsigned int Handle) = 0;

	/**
	 * Returns the position within a playing sound in seconds
	 */
	virtual float GetSoundTime(unsigned int Handle) = 0;

private:
	bool _RegisterScriptBindings();
};

} // End of namespace Sword25

#endif
