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

#include "common/language.h"
#include "util.h"

#define TEENAGENT_DAT_VERSION 5

enum ResourceType {
	kResDialogs = 0,
	kResDialogStacks,
	kResItems,
	kResCredits,
	kResSceneObjects,
	kResMessages,
	kResCombinations,
};

#define NUM_RESOURCES 7
#define NUM_LANGS 4

const Common::Language supportedLanguages[NUM_LANGS] = {
	CS_CZE,
	EN_ANY,
	PL_POL,
	RU_RUS,
};

#endif
