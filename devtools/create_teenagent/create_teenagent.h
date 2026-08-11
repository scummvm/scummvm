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

#ifndef CREATE_TEENAGENT_H
#define CREATE_TEENAGENT_H

#include "util.h"

#define TEENAGENT_DAT_VERSION 6

enum ResourceType {
	kResDialogStacks = 0,
	kResDialogs,
	kResItems,
	kResCredits,
	kResSceneObjects,
	kResMessages,
	kResCombinations,
};

struct ResourceInfo {
	byte _id;
	uint32 _offset;
	uint32 _size;
};

#define NUM_RESOURCES 7
#define NUM_LANGS 4

// If you are adding a new language here, make sure to sync with teenagent/resources.h
// enum in DataLanguage around line 1165

enum Language : byte {
	CS_CZE = 3,
	EN_ANY = 7,
	PL_POL = 27,
	RU_RUS = 30,
};

const Language supportedLanguages[NUM_LANGS] = {
	CS_CZE,
	EN_ANY,
	PL_POL,
	RU_RUS,
};

#endif
