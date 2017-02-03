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

#include "bladerunner/crimes_database.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/text_resource.h"

namespace BladeRunner {

CrimesDatabase::CrimesDatabase(BladeRunnerEngine *vm, const char *cluesResource, int crimesCount) : _crimesCount(crimesCount) {
	// reset();

	_crimes     = new int[_crimesCount];
	_assetTypes = new int[_crimesCount];

	_cluesText = new TextResource(vm);
	_cluesText->open(cluesResource);

	for (int i = 0; i != _crimesCount; ++i) {
		_crimes[i] = -1;
		_assetTypes[i] = -1;
	}
}

CrimesDatabase::~CrimesDatabase() {
	delete   _cluesText;
	delete[] _assetTypes;
	delete[] _crimes;
}

void CrimesDatabase::setCrime(int crimeId, int value) {
	_crimes[crimeId] = value;
}

int CrimesDatabase::getCrime(int crimeId) {
	return _crimes[crimeId];
}

void CrimesDatabase::setAssetType(int assetId, int assetType) {
	_assetTypes[assetId] = assetType;
}

int CrimesDatabase::getAssetType(int assetId) {
	return _assetTypes[assetId];
}

const char *CrimesDatabase::getClueText(int id) {
	return _cluesText->getText(id);
}

} // End of namespace BladeRunner
