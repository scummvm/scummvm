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
 */

#ifndef MADS_DRAGONSPHERE_SOUND_PSOUND_DRAGONSPHERE_H
#define MADS_DRAGONSPHERE_SOUND_PSOUND_DRAGONSPHERE_H

#include "mads/dragonsphere/sound/psound.h"

namespace MADS {
namespace Dragonsphere {
namespace Sound {

/** Shared, game-local control layer used by the seven retail PSOUND overlays. */
class DragonspherePSound : public PSound {
protected:
	enum MusicReset {
		kKeepPlayback,
		kClearMusicIdentity,
		kStopMusic,
		kStopAll
	};

	struct ChannelLoad {
		byte channel;
		uint16 sequence;
	};
	struct MusicLoad {
		MusicReset reset;
		int musicIndex;
		int counter;
		int period;
		bool clearCallback;
		const ChannelLoad *channels;
		uint channelCount;
	};

	uint16 _callbackCounter;
	uint16 _callbackPeriod;
	int _pendingCommand;
	bool _pendingLoadOnly;
	int _musicIndex;
	byte _maxMusicCommand;
	bool _resetClearsCallback;

	DragonspherePSound(Audio::Mixer *mixer, const PSoundDriverData &driverData,
			byte maxMusicCommand, bool resetClearsCallback);

	int dispatchBaseCommand(int commandId);
	int resetSection();
	void playSounds(const uint16 *sequences, uint count, bool anyChannel = false);
	void loadChannels(const ChannelLoad *loads, uint count);
	bool musicChannelsActive(uint count = kMusicChannelCount) const;
	void applyMusicLoad(const MusicLoad &load);
	bool startOrDeferMusic(int commandId, uint16 guard,
			const MusicLoad &load, bool loadOnly,
			uint musicChannelCount = kMusicChannelCount);
	bool startOrDeferMusicWhenActive(int commandId, uint16 guard,
			const MusicLoad &load, bool loadOnly,
			uint musicChannelCount = kMusicChannelCount);
	void deferCommand(int commandId, bool loadOnly);
	void serviceCallbacks() override;
	virtual int executeCommand(int commandId, bool loadOnly) = 0;

public:
	int command(int commandId, int param) override;
};

/** Demo control layer: its exported dispatcher has no saved music command. */
class DragonspherePSoundDemo : public DragonspherePSound {
protected:
	DragonspherePSoundDemo(Audio::Mixer *mixer,
			const PSoundDriverData &driverData, bool resetClearsCallback);

public:
	int command(int commandId, int param) override;
};

#define DECLARE_DRAGONSPHERE_PSOUND(_section, _count) \
	class PSound##_section final : public DragonspherePSound { \
	private: \
		static const uint16 _commandList[_count]; \
		int executeCommand(int commandId, bool loadOnly) override; \
		bool callFunction(uint16 targetOffset, Channel &channel) override; \
	public: \
		explicit PSound##_section(Audio::Mixer *mixer); \
	}

DECLARE_DRAGONSPHERE_PSOUND(1, 102);
DECLARE_DRAGONSPHERE_PSOUND(2, 73);
DECLARE_DRAGONSPHERE_PSOUND(3, 74);
DECLARE_DRAGONSPHERE_PSOUND(4, 79);
DECLARE_DRAGONSPHERE_PSOUND(6, 99);
DECLARE_DRAGONSPHERE_PSOUND(9, 64);

class PSound5 final : public DragonspherePSound {
private:
	static const uint16 _commandList[79];
	void loadAlternateMusic();
	int executeCommand(int commandId, bool loadOnly) override;
	bool callFunction(uint16 targetOffset, Channel &channel) override;
public:
	explicit PSound5(Audio::Mixer *mixer);
};

class PSoundDemo1 final : public DragonspherePSoundDemo {
private:
	static const uint16 _commandList[89];
	int executeCommand(int commandId, bool loadOnly) override;
	bool callFunction(uint16 targetOffset, Channel &channel) override;
public:
	explicit PSoundDemo1(Audio::Mixer *mixer);
};

class PSoundDemo9 final : public DragonspherePSoundDemo {
private:
	static const uint16 _commandList[51];
	int executeCommand(int commandId, bool loadOnly) override;
	bool callFunction(uint16 targetOffset, Channel &channel) override;
public:
	explicit PSoundDemo9(Audio::Mixer *mixer);
};

/** Validate one exact retail section overlay or separately built demo file. */
bool validateDragonspherePSoundFile(int section, bool isDemo,
		Common::String *reason = nullptr);

#undef DECLARE_DRAGONSPHERE_PSOUND

} // namespace Sound
} // namespace Dragonsphere
} // namespace MADS

#endif // MADS_DRAGONSPHERE_SOUND_PSOUND_DRAGONSPHERE_H
