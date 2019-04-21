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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_CRIMES_DATABASE_H
#define BLADERUNNER_CRIMES_DATABASE_H

#include "common/array.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;
class TextResource;

class CrimesDatabase {
	int                 _crimeCount;
	Common::Array<int>  _crimes;
	Common::Array<int>  _assetTypes;
	TextResource       *_cluesText;

public:
	CrimesDatabase(BladeRunnerEngine *vm, const Common::String &cluesResource, int crimeCount);
	~CrimesDatabase();

	void setCrime(int clueId, int crimeId);
	int getCrime(int clueId) const;

	void setAssetType(int clueId, int assetType);
	int getAssetType(int clueId) const;

	const char *getClueText(int clueId) const;

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);
};

} // End of namespace BladeRunner

#endif
