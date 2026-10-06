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

#include "mads/nebular/bonus/bonus_exe_data.h"

#include "common/file.h"
#include "common/stream.h"
#include "common/util.h"

namespace MADS {
namespace RexNebular {

namespace {

static const BonusExeLayout kBonusExeLayout = {
	0xE270,
	{ 0x0072, 0x00C2, 0x0112 },
	{
		0x0309, 0x0324, 0x033E, 0x0352,
		0x036A, 0x037E, 0x0392, 0x03AC,
		0x03C6, 0x03E1, 0x03F7, 0x040E,
		0x0427, 0x043F, 0x0452, 0x046B
	},
	0x0480,
	0x0493,
	0x04AD,
	{ 0x04C6, 0x04E2, 0x04FB, 0x0512, 0x052C, 0x0544 },
	0x0551,
	0x0565,
	0x0045,
	0x0572
};

} // namespace

const BonusExeLayout &BonusExeData::layout() {
	return kBonusExeLayout;
}

bool BonusExeData::readStringAt(Common::SeekableReadStream &stream, uint32 offset,
		Common::String &result, Common::String &errorMessage) {
	if (!stream.seek(offset)) {
		errorMessage = Common::String::format(
				"Unable to read BONUS.EXE string at 0x%X", offset);
		return false;
	}

	result = stream.readString();
	if (stream.err()) {
		errorMessage = Common::String::format(
				"Unable to read BONUS.EXE string at 0x%X", offset);
		return false;
	}
	return true;
}

bool BonusExeData::load(const Common::Path &filename, Common::String &errorMessage) {
	Common::File executable;
	if (!executable.open(filename)) {
		errorMessage = Common::String::format("Unable to open %s",
				filename.toString().c_str());
		return false;
	}

	const BonusExeLayout &exeLayout = layout();
	for (uint i = 0; i < ARRAYSIZE(title); ++i) {
		if (!readStringAt(executable,
				exeLayout.dataFileOffset + exeLayout.title[i], title[i], errorMessage))
			return false;
	}
	for (uint i = 0; i < ARRAYSIZE(musicTitles); ++i) {
		if (!readStringAt(executable,
				exeLayout.dataFileOffset + exeLayout.musicTitles[i],
				musicTitles[i], errorMessage))
			return false;
	}
	if (!readStringAt(executable, exeLayout.dataFileOffset + exeLayout.musicExit,
			musicExit, errorMessage))
		return false;
	if (!readStringAt(executable,
			exeLayout.dataFileOffset + exeLayout.musicMenuTitle,
			musicMenuTitle, errorMessage))
		return false;
	if (!readStringAt(executable, exeLayout.dataFileOffset + exeLayout.nowPlaying,
			nowPlaying, errorMessage))
		return false;
	for (uint i = 0; i < ARRAYSIZE(mainMenu); ++i) {
		if (!readStringAt(executable,
				exeLayout.dataFileOffset + exeLayout.mainMenu[i],
				mainMenu[i], errorMessage))
			return false;
	}
	if (!readStringAt(executable,
			exeLayout.dataFileOffset + exeLayout.mainMenuTitle,
			mainMenuTitle, errorMessage))
		return false;
	if (!readStringAt(executable,
			exeLayout.dataFileOffset + exeLayout.bonusTextFilename,
			bonusTextFilename, errorMessage))
		return false;
	if (!readStringAt(executable,
			exeLayout.dataFileOffset + exeLayout.continuePrompt,
			continuePrompt, errorMessage))
		return false;
	if (!readStringAt(executable, exeLayout.dataFileOffset + exeLayout.goodbye,
			goodbye, errorMessage))
		return false;

	goodbye.trim();
	return true;
}

} // namespace RexNebular
} // namespace MADS
