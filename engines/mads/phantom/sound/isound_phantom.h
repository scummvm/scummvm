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

#ifndef MADS_PHANTOM_SOUND_ISOUND_PHANTOM_H
#define MADS_PHANTOM_SOUND_ISOUND_PHANTOM_H

#include "mads/phantom/sound/isound.h"

namespace MADS {
namespace Phantom {
namespace Sound {

class ISoundSection : public ISound {
private:
	enum CommandDisposition {
		kCommandUnhandled,
		kCommandPlaySequence,
		kCommandMalformed
	};

	int _sectionNumber;

	static CommandDisposition lookupCommand(int sectionNumber,
		byte commandId, uint16 &sequenceOffset, byte &priority);
	static bool validateSectionLayout(int sectionNumber,
		const OverlayLayout &layout, Common::String *reason);

protected:
	ISoundSection(Audio::Mixer *mixer, int sectionNumber);

public:
	static bool isOverlaySupported(int sectionNumber,
		Common::String *reason = nullptr);

	int command(int commandId, int param) override;
};

/** ISOUND.PH1: "Phantom  00105-26-93". */
class ISound1 : public ISoundSection {
public:
	ISound1(Audio::Mixer *mixer);
};

/** ISOUND.PH2: "Phantom  00205-26-93". */
class ISound2 : public ISoundSection {
public:
	ISound2(Audio::Mixer *mixer);
};

/** ISOUND.PH3: "Phantom  00406-02-93". */
class ISound3 : public ISoundSection {
public:
	ISound3(Audio::Mixer *mixer);
};

/** ISOUND.PH4: "Phantom  00405-26-93". */
class ISound4 : public ISoundSection {
public:
	ISound4(Audio::Mixer *mixer);
};

/** ISOUND.PH5: "Phantom  00505-26-93". */
class ISound5 : public ISoundSection {
public:
	ISound5(Audio::Mixer *mixer);
};

/** ISOUND.PH9: "Phantom  00905-26-93". */
class ISound9 : public ISoundSection {
public:
	ISound9(Audio::Mixer *mixer);
};

} // namespace Sound
} // namespace Phantom
} // namespace MADS

#endif
