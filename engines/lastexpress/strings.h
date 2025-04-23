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

#ifndef LASTEXPRESS_STRING_H
#define LASTEXPRESS_STRING_Hd

namespace LastExpress {

static const char gameStrings[46][256]{
	/*  0 */ "The Last Express",
	/*  1 */ "Invalid string id",
	/*  2 */ "Error:\n%s",
	/*  3 */ "Error:\n%s\n\nFile \"%s\", line %ld",
	/*  4 */ "Hard drive cache not found (please reinstall)",
	/*  5 */ "Out of memory",
	/*  6 */ "Could not open \"%s\".  Please reinstall.",
	/*  7 */ "%s has more than %ld nodes in it!",
	/*  8 */ "Error opening file \"%s\".  It probably doesn\'t exist or is write protected.",
	/*  9 */ "Error reading from file \"%s\"",
	/* 10 */ "Error writing to file \"%s\".  Your disk is probably full.",
	/* 11 */ "Error seeking in file \"%s\"",
	/* 12 */ "Error telling in file \"%s\"",
	/* 13 */ "Error flushing file \"%s\".  Your disk is probably full.",
	/* 14 */ "Error closing file \"%s\"",
	/* 15 */ "Error renaming file \"%s\" to \"%s\"",
	/* 16 */ "Error deleting file \"%s\"",
	/* 17 */ "Save game file \"%s\" is not named correctly (blue.egg, red.egg, etc.) or is not located in the same directory as the game.",
	/* 18 */ "Save game file \"%s\" is corrupt",
	/* 19 */ "Save game file \"%s\" is incompatible with this version of the game",
	/* 20 */ "Attempting to salvage corrupt save game file \"%s\"",
	/* 21 */ "Could not detect sound card - ",
	/* 22 */ "%s no device driver installed",
	/* 23 */ "Failed to synchronize sound card.\nProbaly due to an incorrect IRQ (%ld) or high DMA (%ld) in BLASTER setting.",
	/* 24 */ "%s initialization failed.\nProbably due to an incorrect port address (%x) in your BLASTER setting.",
	/* 25 */ "%s invalid BLASTER settings.\nPort=%x IRQ=%x high DMA=%x\nMake sure sound card is SoundBlaster 16 compatible and is correctly installed.",
	/* 26 */ "%s no BLASTER environment settings.\nMake sure there is a BLASTER setting in your AUTOEXEC.BAT",
	/* 27 */ "%s invalid IRQ (%ld) in BLASTER setting.\nOnly IRQ settings below 10 are supported.",
	/* 28 */ "Could not change display mode to 640x480 HiColor (16 bit)",
	/* 29 */ "Sound too large for buffer",
	/* 30 */ "DirectSound error",
	/* 31 */ "DirectSound error.  Code %ld",
	/* 32 */ "DirectDraw error",
	/* 33 */ "DirectDraw error.  Code %ld",
	/* 34 */ "Could not start timer",
	/* 35 */ "Another copy of The Last Express is already running",
	/* 36 */ "Could not detect a mouse.  Please make sure your mouse is correctly installed and configured.",
	/* 37 */ "%s\nVESA mode switch failed",
	/* 38 */ "%s\nVESA window size too small",
	/* 39 */ "%s\nVESA granularity unrecognized",
	/* 40 */ "%s\nVESA read window not found",
	/* 41 */ "%s\nVESA write window not found",
	/* 42 */ "%s\nVESA read/write window not found",
	/* 43 */ "%s\nVESA can\'t use window",
	/* 44 */ "Could not detect a CD-ROM drive.  Please make sure your CD-ROM drive is correctly installed and configured.",
	/* 45 */ "To play The Last Express you must change your screen mode to High Color (16 bit).",
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_STRING_H
