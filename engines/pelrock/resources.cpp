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

#include "pelrock/resources.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"
#include "resources.h"
#include "room.h"

namespace Pelrock {

ResourceManager::ResourceManager(/* args */) {
	_inventoryIcons = new InventoryObject[69];
}

ResourceManager::~ResourceManager() {
	for (int i = 0; i < 5; i++) {
		delete[] _cursorMasks[i];
	}
	for (int i = 0; i < kNumVerbIcons; i++) {
		delete[] _verbIcons[i];
	}
	delete[] _popUpBalloon;
	for (int i = 0; i < 4; i++) {
		// free all frame buffers
		for (int j = 0; j < walkingAnimLengths[i]; j++) {
			delete[] alfredWalkFrames[i][j];
			delete[] alfredTalkFrames[i][j];
		}

		for (int j = 0; j < 2; j++) {
			delete[] alfredInteractFrames[i][j];
		}

		// free the array of pointers
		delete[] alfredWalkFrames[i];
		delete[] alfredTalkFrames[i];
		delete[] alfredInteractFrames[i];
		delete[] alfredIdle[i];
	}

	delete[] alfredCombFrames[0];
	delete[] alfredCombFrames[1];
	delete _mainMenu;
	delete[] _inventoryIcons;
}

void ResourceManager::loadCursors() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Couldnt find file ALFRED.7");
	}
	for (int i = 0; i < 5; i++) {
		uint32_t cursorOffset = cursor_offsets[i];
		alfred7File.seek(cursorOffset);
		_cursorMasks[i] = new byte[kCursorSize];
		alfred7File.read(_cursorMasks[i], kCursorSize);
	}
	alfred7File.close();
}

void ResourceManager::loadInteractionIcons() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Couldnt find file ALFRED.7");
	}

	alfred7File.seek(kBalloonFramesOffset, SEEK_SET);

	uint32_t totalBalloonSize = kBalloonWidth * kBalloonHeight * kBalloonFrames;
	_popUpBalloon = new byte[totalBalloonSize];

	uint32_t compressedSize = kBalloonFramesSize;

	byte *raw = new byte[compressedSize];
	alfred7File.read(raw, compressedSize);
	rleDecompress(raw, compressedSize, 0, totalBalloonSize, &_popUpBalloon);

	delete[] raw;

	alfred7File.close();
	Common::File alfred4File;
	if (!alfred4File.open("ALFRED.4")) {
		error("Couldnt find file ALFRED.4");
	}

	int iconSize = kVerbIconHeight * kVerbIconWidth;
	for (int i = 0; i < kNumVerbIcons; i++) {
		uint32_t iconOffset = i * iconSize;
		_verbIcons[i] = new byte[iconSize];
		alfred4File.read(_verbIcons[i], iconSize);
	}
	alfred4File.close();
}

void ResourceManager::loadAlfredAnims() {
	Common::File alfred3;
	if (!alfred3.open(Common::Path("ALFRED.3"))) {
		error("Could not open ALFRED.3");
		return;
	}
	int alfred3Size = alfred3.size();
	unsigned char *bufferFile = (unsigned char *)malloc(alfred3Size);
	alfred3.seek(0, SEEK_SET);
	alfred3.read(bufferFile, alfred3Size);
	alfred3.close();

	int index = 0;
	int index3 = 0;
	uint32_t capacity = 3060 * 102;
	unsigned char *pic = new unsigned char[capacity];
	rleDecompress(bufferFile, alfred3Size, 0, capacity, &pic);

	int frameSize = kAlfredFrameHeight * kAlfredFrameWidth;
	for (int i = 0; i < 4; i++) {
		alfredIdle[i] = new byte[frameSize];
		int talkingFramesOffset = walkingAnimLengths[0] + walkingAnimLengths[1] + walkingAnimLengths[2] + walkingAnimLengths[3] + 4;
		int interactingFramesOffset = talkingFramesOffset + talkingAnimLengths[0] + talkingAnimLengths[1] + talkingAnimLengths[2] + talkingAnimLengths[3];
		int prevWalkingFrames = 0;
		int prevTalkingFrames = 0;
		int prevInteractingFrames = 0;

		for (int j = 0; j < i; j++) {
			prevWalkingFrames += walkingAnimLengths[j] + 1;
			prevTalkingFrames += talkingAnimLengths[j];
			prevInteractingFrames += interactingAnimLength;
		}

		alfredWalkFrames[i] = new byte *[walkingAnimLengths[i]];

		int standingFrame = prevWalkingFrames;

		extractSingleFrame(pic, alfredIdle[i], standingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		for (int j = 0; j < walkingAnimLengths[i]; j++) {

			alfredWalkFrames[i][j] = new byte[frameSize];
			int walkingFrame = prevWalkingFrames + 1 + j;
			extractSingleFrame(pic, alfredWalkFrames[i][j], walkingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		}

		alfredTalkFrames[i] = new byte *[talkingAnimLengths[i]];

		int talkingStartFrame = talkingFramesOffset + prevTalkingFrames;
		for (int j = 0; j < talkingAnimLengths[i]; j++) {
			alfredTalkFrames[i][j] = new byte[frameSize];
			int talkingFrame = talkingStartFrame + j;
			extractSingleFrame(pic, alfredTalkFrames[i][j], talkingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		}

		alfredInteractFrames[i] = new byte *[interactingAnimLength];
		int interactingStartFrame = interactingFramesOffset + prevInteractingFrames;
		for (int j = 0; j < interactingAnimLength; j++) {
			alfredInteractFrames[i][j] = new byte[frameSize];
			int interactingFrame = interactingStartFrame + j;
			extractSingleFrame(pic, alfredInteractFrames[i][j], interactingFrame, kAlfredFrameWidth, kAlfredFrameHeight);
		}
	}

	free(bufferFile);

	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}
	int spriteMapSize = frameSize * 11;

	byte *alfredCombRightRaw;
	size_t alfredCombRightSize;

	readUntilBuda(&alfred7, ALFRED7_ALFRED_COMB_R, alfredCombRightRaw, alfredCombRightSize);
	byte *alfredCombRight = nullptr;
	rleDecompress(alfredCombRightRaw, alfredCombRightSize, 0, spriteMapSize, &alfredCombRight);

	alfredCombFrames[0] = new byte *[11];
	alfredCombFrames[1] = new byte *[11];

	for (int i = 0; i < 11; i++) {
		alfredCombFrames[0][i] = new byte[frameSize];
		extractSingleFrame(alfredCombRight, alfredCombFrames[0][i], i, kAlfredFrameWidth, kAlfredFrameHeight);
	}

	byte *alfredCombLeftRaw;
	size_t alfredCombLeftSize;
	readUntilBuda(&alfred7, ALFRED7_ALFRED_COMB_L, alfredCombLeftRaw, alfredCombLeftSize);
	byte *alfredCombLeft = nullptr;
	size_t outSize = rleDecompress(alfredCombLeftRaw, alfredCombLeftSize, 0, spriteMapSize, &alfredCombLeft);

	for (int i = 0; i < 11; i++) {
		alfredCombFrames[1][i] = new byte[frameSize];
		extractSingleFrame(alfredCombLeft, alfredCombFrames[1][i], i, kAlfredFrameWidth, kAlfredFrameHeight);
	}

	alfred7.close();
	free(alfredCombRightRaw);
	free(alfredCombLeftRaw);
}

void ResourceManager::loadInventoryItems() {
	loadInventoryDescriptions();
	Common::File alfred4File;
	if (!alfred4File.open("ALFRED.4")) {
		error("Couldnt find file ALFRED.4");
	}
	uint32 iconsSize = alfred4File.size() - 423656;
	byte *iconData = new byte[iconsSize];
	alfred4File.seek(42366, SEEK_SET);
	alfred4File.read(iconData, iconsSize);

	int iconSize = 60 * 60; // each icon has 30 bytes of header
	for (int i = 0; i < 69; i++) {
		_inventoryIcons[i].index = i;
		extractSingleFrame(iconData, _inventoryIcons[i].iconData, i, 60, 60);
		_inventoryIcons[i].description = _inventoryDescriptions[i];
	}
	delete[] iconData;
}

void ResourceManager::loadInventoryDescriptions() {
	Common::File exe;
	if (!exe.open("JUEGO.EXE")) {
		error("Couldnt find file JUEGO.EXE");
	}
	byte *descBuffer = new byte[kInventoryDescriptionsSize];
	exe.seek(kInventoryDescriptionsOffset, SEEK_SET);
	exe.read(descBuffer, kInventoryDescriptionsSize);
	int pos = 0;
	Common::String desc = "";
	while (pos < kInventoryDescriptionsSize) {
		if (descBuffer[pos] == 0xFD) {
			if (!desc.empty()) {
				_inventoryDescriptions.push_back(desc);
				desc = Common::String();
			}
			pos++;
			continue;
		}
		if (descBuffer[pos] == 0x00) {
			pos++;
			continue;
		}
		if (descBuffer[pos] == 0x08) {
			pos += 2;
			continue;
		}
		if(descBuffer[pos] == 0xC8) {
			desc.append(1, '\n');
			pos++;
			continue;
		}

		desc.append(1, decodeChar(descBuffer[pos]));
		if (pos + 1 == kInventoryDescriptionsSize) {
			_inventoryDescriptions.push_back(desc);
		}
		pos++;
	}
	delete[] descBuffer;
	exe.close();
}

InventoryObject ResourceManager::getInventoryObject(byte index) {
	return _inventoryIcons[index];
}

void ResourceManager::mergeRleBlocks(Common::SeekableReadStream *stream, uint32 offset, int numBlocks, byte *outputBuffer) {
	stream->seek(offset, SEEK_SET);
	// get screen
	size_t combined_size = 0;
	for (int i = 0; i < numBlocks; i++) {
		byte *thisBlock = nullptr;
		size_t blockSize = 0;
		readUntilBuda(stream, stream->pos(), thisBlock, blockSize);
		uint8_t *block_data = nullptr;
		size_t decompressedSize = rleDecompress(thisBlock, blockSize, 0, 640 * 400, &block_data, true);
		memcpy(outputBuffer + combined_size, block_data, decompressedSize);
		combined_size += decompressedSize;
		free(block_data);
		free(thisBlock);
	}
}

void ResourceManager::loadSettingsMenu() {

	bool alternateMenu = false;
	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}

	_mainMenu = new byte[640 * 400];

	if (!alternateMenu) {
		alfred7.seek(kSettingsPaletteOffset, SEEK_SET);
		alfred7.read(_mainMenuPalette, 768);
		for (int i = 0; i < 256; i++) {
			_mainMenuPalette[i * 3] = _mainMenuPalette[i * 3] << 2;
			_mainMenuPalette[i * 3 + 1] = _mainMenuPalette[i * 3 + 1] << 2;
			_mainMenuPalette[i * 3 + 2] = _mainMenuPalette[i * 3 + 2] << 2;
		}

		uint32 curPos = 0;
		alfred7.seek(2405266, SEEK_SET);
		alfred7.read(_mainMenu, 65536);

		curPos += 65536;

		byte *compressedPart1 = new byte[29418];
		alfred7.read(compressedPart1, 29418);
		byte *decompressedPart1 = nullptr;
		size_t decompressedSize = rleDecompress(compressedPart1, 29418, 0, 0, &decompressedPart1, true);

		memcpy(_mainMenu + curPos, decompressedPart1, decompressedSize);
		curPos += decompressedSize;

		delete[] compressedPart1;
		delete[] decompressedPart1;
		alfred7.seek(2500220, SEEK_SET);
		alfred7.read(_mainMenu + curPos, 32768);
		curPos += 32768;
		byte *compressedPart2 = new byte[30288];
		alfred7.read(compressedPart2, 30288);
		byte *decompressedPart2 = nullptr;
		decompressedSize = rleDecompress(compressedPart2, 30288, 0, 0, &decompressedPart2, true);

		memcpy(_mainMenu + curPos, decompressedPart2, decompressedSize);
		curPos += decompressedSize;
		debug("Settings menu size loaded: %d, with last block %d", curPos, curPos + 92160);
		delete[] compressedPart2;
		delete[] decompressedPart2;
		alfred7.seek(2563266, SEEK_SET);
		alfred7.read(_mainMenu + curPos, 92160);
		alfred7.close();
	} else {
		Common::File alfred7;
		if (!alfred7.open(Common::Path("ALFRED.7"))) {
			error("Could not open ALFRED.7");
			return;
		}

		_mainMenu = new byte[640 * 400];

		alfred7.seek(kAlternateSettingsPaletteOffset, SEEK_SET);
		alfred7.read(_mainMenuPalette, 768);
		for (int i = 0; i < 256; i++) {
			_mainMenuPalette[i * 3] = _mainMenuPalette[i * 3] << 2;
			_mainMenuPalette[i * 3 + 1] = _mainMenuPalette[i * 3 + 1] << 2;
			_mainMenuPalette[i * 3 + 2] = _mainMenuPalette[i * 3 + 2] << 2;
		}

		mergeRleBlocks(&alfred7, kAlternateSettingsMenuOffset, 8, _mainMenu);
		alfred7.close();
	}
}

} // End of namespace Pelrock
