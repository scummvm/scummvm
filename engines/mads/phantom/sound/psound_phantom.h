/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT file.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef MADS_PHANTOM_SOUND_PSOUND_PHANTOM_H
#define MADS_PHANTOM_SOUND_PSOUND_PHANTOM_H

#include "mads/phantom/sound/psound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

class PhantomPSound : public PSound {
protected:
	PhantomPSound(Audio::Mixer *mixer, const PSoundDriverData &driverData);

	int dispatchBaseCommand(int commandId);
	void loadFixedChannels(const uint16 *sequences, uint count);
	void playSounds(const uint16 *sequences, uint count);
	void playMusic(const uint16 *sequences, uint count);
	bool loadFixedIfInactive(uint16 guard, const uint16 *sequences, uint count);
	void leaveMalformedChannelSilent(const char *filename, byte channel,
									 uint16 sequenceOffset);

	bool callFunction(uint16 targetOffset, Channel &channel) override;
	virtual int executeCommand(int commandId) = 0;

public:
	int command(int commandId, int param) override;
};

#define DECLARE_PHANTOM_PSOUND(_name)                  \
	class PSound##_name final : public PhantomPSound { \
	private:                                           \
		int executeCommand(int commandId) override;    \
                                                       \
	public:                                            \
		explicit PSound##_name(Audio::Mixer *mixer);   \
	}

class PSound1 final : public PhantomPSound {
private:
	uint16 _previousSelector;
	uint16 _olderSelector;
	int executeCommand(int commandId) override;
	void selectBackgroundMusic();

public:
	explicit PSound1(Audio::Mixer *mixer);
};

DECLARE_PHANTOM_PSOUND(2);
DECLARE_PHANTOM_PSOUND(3);

class PSound4 final : public PhantomPSound {
private:
	int executeCommand(int commandId) override;
	bool callFunction(uint16 targetOffset, Channel &channel) override;

public:
	explicit PSound4(Audio::Mixer *mixer);
};

DECLARE_PHANTOM_PSOUND(5);
DECLARE_PHANTOM_PSOUND(9);
DECLARE_PHANTOM_PSOUND(Demo);

#undef DECLARE_PHANTOM_PSOUND

/** Validate one exact retail section overlay or the separately built demo. */
bool validatePhantomPSoundFile(int section, bool isDemo,
							   Common::String *reason = nullptr);

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif // MADS_PHANTOM_SOUND_PSOUND_PHANTOM_H
