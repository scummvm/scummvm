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

#ifndef MADS_NEBULAR_BONUS_EXE_DATA_H
#define MADS_NEBULAR_BONUS_EXE_DATA_H

#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace MADS {
namespace RexNebular {

struct BonusExeLayout {
	uint32 dataFileOffset;
	uint16 title[3];
	uint16 musicTitles[16];
	uint16 musicExit;
	uint16 musicMenuTitle;
	uint16 nowPlaying;
	uint16 mainMenu[6];
	uint16 mainMenuTitle;
	uint16 bonusTextFilename;
	uint16 continuePrompt;
	uint16 goodbye;
};

/** Text resources embedded in the known Rex Nebular BONUS.EXE layout. */
class BonusExeData {
public:
	bool load(const Common::Path &filename, Common::String &errorMessage);

	Common::String title[3];
	Common::String musicTitles[16];
	Common::String musicExit;
	Common::String musicMenuTitle;
	Common::String nowPlaying;
	Common::String mainMenu[6];
	Common::String mainMenuTitle;
	Common::String bonusTextFilename;
	Common::String continuePrompt;
	Common::String goodbye;

	static const BonusExeLayout &layout();

private:
	static bool readStringAt(Common::SeekableReadStream &stream, uint32 offset,
			Common::String &result, Common::String &errorMessage);
};

} // namespace RexNebular
} // namespace MADS

#endif // MADS_NEBULAR_BONUS_EXE_DATA_H
