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

#include "cif.h"
#include "common/endian.h"

// Severely slimmed down version of the cif/ciftree code that's in the engine
// Can only read .cif files, and only write ciftrees into memory
byte *createCifTree(uint16 gameVersion, Common::Array<const char *> filenames, const char *folderName, uint32 &outSize) {
	Common::Array<uint32> fileSizes(filenames.size(), 0);
	Common::Array<byte *> cifInfos(filenames.size(), nullptr);
	Common::Array<byte *> fileData(filenames.size(), nullptr);
	Common::Array<bool> isCif(filenames.size(), false);

	uint32 headerSize = 1024 * 2;
	uint32 infoSizeTree = 0;
	uint32 infoSizeCif = 0;
	if (gameVersion <= 2) { // nancy1
		headerSize += 30;
		infoSizeTree = 38;
		infoSizeCif = 21;
	} else {
		headerSize += 32;
		if (gameVersion <= 3) {
			// Format 1, short filenames
			infoSizeTree = 70;
			infoSizeCif = 53;
		} else {
			// Format 1 or 2*, with long filenames
			infoSizeTree = 94;
			infoSizeCif = 53;
		}
	}

	// Read the .cif files
	for (uint i = 0; i < filenames.size(); ++i) {
		File f;
		char *path = new char[256];
		strcpy(path, folderName);
		strcat(path, "/");
		strcat(path, filenames[i]);
		f.open(path, kFileReadMode);
		delete[] path;

		const char *c = filenames[i];
		uint nameSize = 0;
		while (*c != '\0') {
			++nameSize;
			++c;
		}

		if (nameSize > 4) {
			// Check if file extension is .cif or not
			if (tolower(*(c - 4)) == '.' && tolower(*(c - 3)) == 'c' && tolower(*(c - 2)) == 'i' && tolower(*(c - 1)) == 'f') {
				isCif[i] = true;
			}
		}

		if (isCif[i]) {
			f.skip(20); // Skip header
			f.skip(4); // Skip empty
			f.skip(4); // Skip version

			cifInfos[i] = new byte[infoSizeCif];
			f.read(cifInfos[i], infoSizeCif);
		}

		fileSizes[i] = f.size() - f.pos(); // Assumes .cif files are well-behaved and don't have extra data at end
		fileData[i] = new byte[fileSizes[i]];
		f.read(fileData[i], fileSizes[i]);


		f.close();
	}

	uint32 retSize = headerSize + filenames.size() * infoSizeTree;
	for (uint i = 0; i < fileSizes.size(); ++i) {
		retSize += fileSizes[i];
	}

	uint32 retPos = 0;
	byte *ret = new byte[retSize];
	memset(ret, 0, retSize);
	
	memcpy(ret, "CIF TREE WayneSikes", 20);
	retPos += 24;

	WRITE_LE_UINT16(ret + retPos, 2);
	retPos += 2;
	if (gameVersion <= 2) { // nancy1
		WRITE_LE_UINT16(ret + retPos, 0);
	} else {
		WRITE_LE_UINT16(ret + retPos, 1);
	}
	retPos += 2;

	WRITE_LE_UINT16(ret + retPos, filenames.size());
	retPos += 2;

	if (gameVersion >= 3) { // nancy2
		retPos += 2;
	}

	retPos += 1024 * 2; // Skip hash table

	// Write the cif info structs
	const uint nameSize = gameVersion <= 3 ? 9 : 33;
	for (uint i = 0; i < filenames.size(); ++i) {
		uint thisNameSize = 0;
		char name[34];
		memset(name, 0, 34);

		const char *c = filenames[i];
		while (*c != '\0' && thisNameSize <= nameSize && (isCif[i] ? *c != '.' : true)) { // Do not store the extension for .cif files
			name[thisNameSize] = *c;
			++thisNameSize;
			++c;
		}
		name[nameSize] = '\0';

		memcpy(ret + retPos, name, nameSize);
		retPos += nameSize;

		retPos += 2; // block index

		uint32 dataOffset = headerSize + filenames.size() * infoSizeTree;
		for (uint j = 0; j < i; ++j) {
			dataOffset += fileSizes[j];
		}

		if (gameVersion >= 7) { // nancy6
			WRITE_LE_UINT32(ret + retPos, dataOffset);
			retPos += 6;
		}

		if (cifInfos[i]) {
			// Write rects
			uint32 pos = 0;
			if (gameVersion >= 3) { // nancy2
				memcpy(ret + retPos, cifInfos[i], 32);
				retPos += 32;
				pos += 32;
			}

			// width, pitch, height, depth, comp
			memcpy(ret + retPos, cifInfos[i] + pos, 8);
			retPos += 8;
			pos += 8;

			if (gameVersion <= 6) { // nancy5
				WRITE_LE_UINT32(ret + retPos, dataOffset);
				retPos += 4;
			}

			// uncompressed size, obsolete field, compressed size, data type
			memcpy(ret + retPos, cifInfos[i] + pos, 13);
			retPos += 13;
		} else {
			// Non-cif file

			// Fill rects with zeroes
			if (gameVersion >= 3) {
				retPos += 32;
			}

			// width, pitch, height, depth
			retPos += 7;

			*(ret + retPos) = 1; // No compression
			++retPos;

			if (gameVersion <= 6) { // nancy5
				WRITE_LE_UINT32(ret + retPos, dataOffset);
				retPos += 4;
			}

			WRITE_LE_UINT32(ret + retPos, fileSizes[i]);
			WRITE_LE_UINT32(ret + retPos + 4, 0);
			WRITE_LE_UINT32(ret + retPos + 8, fileSizes[i]);
			retPos += 12;

			*(ret + retPos) = 3; // data type
			++retPos;
		}


		if (gameVersion <= 6) { // nancy5
			retPos += 2; // next id in chain
		}
	}

	// Write the file datas
	for (uint i = 0; i < fileData.size(); ++i) {
		memcpy(ret + retPos, fileData[i], fileSizes[i]);
		retPos += fileSizes[i];
	}

	for (uint i = 0; i < filenames.size(); ++i) {
		delete[] fileData[i];
		delete[] cifInfos[i];
	}

	outSize = retPos;
	return ret;
}
