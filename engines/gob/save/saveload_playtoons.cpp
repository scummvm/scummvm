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

#include "gob/save/saveload.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

SaveLoad_Playtoons::SaveFile SaveLoad_Playtoons::_saveFiles[] = {
	{  "disk.001", kSaveModeExists, 0, 0}, // Playtoons 1 identification file
	{  "disk.002", kSaveModeExists, 0, 0}, // Playtoons 2 identification file
	{  "disk.003", kSaveModeExists, 0, 0}, // Playtoons 3 identification file
	{  "disk.004", kSaveModeExists, 0, 0}, // Playtoons 4 identification file
	{  "disk.005", kSaveModeExists, 0, 0}, // Playtoons 5 identification file
	{  "disk.006", kSaveModeExists, 0, 0}, // Playtoons CK 1 identification file
	{  "disk.007", kSaveModeExists, 0, 0}, // Playtoons CK 2 identification file
	{  "disk.008", kSaveModeExists, 0, 0}, // Playtoons CK 3 identification file
	{  "titre.001", kSaveModeExists, 0, 0}, // Playtoons 1 titles
	{  "titre.002", kSaveModeExists, 0, 0}, // Playtoons 2 titles
	{  "titre.003", kSaveModeExists, 0, 0}, // Playtoons 3 titles
	{  "titre.004", kSaveModeExists, 0, 0}, // Playtoons 4 titles
	{  "titre.005", kSaveModeExists, 0, 0}, // Playtoons 5 titles
	{  "titre.006", kSaveModeExists, 0, 0}, // Playtoons CK 1 empty title (???)
	{  "titre.007", kSaveModeExists, 0, 0}, // Playtoons CK 2 empty title (???)
	{  "titre.008", kSaveModeExists, 0, 0}, // Playtoons CK 3 empty title (???)
	{  "mdo.def",  kSaveModeExists, 0, 0},
};

SaveLoad::SaveMode SaveLoad_Playtoons::getSaveMode(const char *fileName) const {
	const SaveFile *saveFile = getSaveFile(fileName);

	if (saveFile)
		return saveFile->mode;

	return kSaveModeNone;
}

SaveLoad_Playtoons::SaveLoad_Playtoons(GobEngine *vm) :
		SaveLoad(vm) {
}

SaveLoad_Playtoons::~SaveLoad_Playtoons() {
}

const SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) const {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

SaveLoad_Playtoons::SaveFile *SaveLoad_Playtoons::getSaveFile(const char *fileName) {
	fileName = stripPath(fileName);

	for (int i = 0; i < ARRAYSIZE(_saveFiles); i++)
		if (!scumm_stricmp(fileName, _saveFiles[i].sourceName))
			return &_saveFiles[i];

	return 0;
}

} // End of namespace Gob
