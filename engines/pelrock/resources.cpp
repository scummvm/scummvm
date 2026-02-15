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
#include "pelrock/room.h"
#include "pelrock/util.h"

namespace Pelrock {

ResourceManager::ResourceManager(/* args */) {
	_inventoryIcons = new InventoryObject[69];
	for (int i = 0; i < 4; i++) {
		alfredIdle[i] = nullptr;
	}

}

const AlfredSpecialAnimOffset ResourceManager::alfredSpecialAnims[] = {
	{10, 51, 102, 1, 7, 559685, 1,}, // 0 - READ BOOK
	{10, 51, 102, 1, 7, 578943, 1}, // 1 - READ RECIPE
	{3, 45, 87, 0, 7, 37000, 1}, // 2 -  ELECTRIC SHOCK 1
	{2, 82, 58, 0, 7, 53106, 20}, // 3 - ELECTRIC SHOCK 3
	{3, 71, 110, 1, 2, 20724, 1, 62480}, // 4 - Throw
	{14, 171, 107, 1, 7, 1556540, 1} , // 5 - crocodile
	{12, 113, 103, 1, 7, 1583702, 1}, // 6 - exit through manhole
	{11, 33, 72, 1, 7, 1761234, 1}, // 7 - alfred climbs down
	{9, 33, 72, 1, 7, 1766378, 1}, // 8 - alfred climbs up
	{16, 158, 115, 0, 7, 1770196, 1}, // 9 - alfred exits tunnel
	{7, 208, 102, 0, 7, 1600956, 1} // 10 - alfred with workers
};

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

void ResourceManager::loadAlfredSpecialAnim(int numAnim, bool reverse) {
	AlfredSpecialAnimOffset anim = alfredSpecialAnims[numAnim];

	Common::String filename = Common::String::format("ALFRED.%d", anim.numAlfred);
	Common::File alfredFile;
	if (!alfredFile.open(Common::Path(filename))) {
		error("Could not open %s", filename.c_str());
		return;
	}

	alfredFile.seek(anim.offset, SEEK_SET);
	if (_currentSpecialAnim)
		delete _currentSpecialAnim;
	_currentSpecialAnim = new AlfredSpecialAnim(anim.numFrames, anim.w, anim.h, anim.numBudas, anim.offset, anim.loops, anim.size);
	uint32 size = anim.size == 0 ? anim.numFrames * anim.w * anim.h : anim.size;
	_currentSpecialAnim->animData = new byte[size];
	if (anim.numBudas > 0) {
		debug("Loading special anim with budas: numBudas=%d, totalSize %d", anim.numBudas, size);
		byte *thisBlock = nullptr;
		size_t blockSize = 0;
		readUntilBuda(&alfredFile, anim.offset, thisBlock, blockSize);
		rleDecompress(thisBlock, blockSize, 0, size, &_currentSpecialAnim->animData, false);
		delete[] thisBlock;
	} else {
		alfredFile.read(_currentSpecialAnim->animData, anim.numFrames * anim.w * anim.h);
	}
	if (reverse) {
		// reverse frames for testing
		byte *reversedData = new byte[anim.numFrames * anim.w * anim.h];
		for (int i = 0; i < anim.numFrames; i++) {
			extractSingleFrame(_currentSpecialAnim->animData,
							   &reversedData[i * anim.w * anim.h],
							   anim.numFrames - 1 - i,
							   anim.w,
							   anim.h);
		}
		delete[] _currentSpecialAnim->animData;
		_currentSpecialAnim->animData = reversedData;
	}

	_isSpecialAnimFinished = false;
	alfredFile.close();
}

void ResourceManager::clearSpecialAnim() {
	delete _currentSpecialAnim;
	_currentSpecialAnim = nullptr;
}

void ResourceManager::loadInventoryItems() {
	// loadInventoryDescriptions();
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
		// _inventoryIcons[i].description = _inventoryDescriptions[i];
	}
	delete[] iconData;
}

void ResourceManager::loadHardcodedText() {

	Common::File exe;
	if (!exe.open("JUEGO.EXE")) {
		error("Couldnt find file JUEGO.EXE");
	}
	byte *descBuffer = new byte[kAlfredResponsesSize];
	exe.seek(kAlfredResponsesOffset, SEEK_SET);
	exe.read(descBuffer, kAlfredResponsesSize);
	_ingameTexts = processTextData(descBuffer, kAlfredResponsesSize);
	byte *terminatorBuffer = new byte[39];
	exe.seek(kConversationTerminatorOffset, SEEK_SET);
	exe.read(terminatorBuffer, 39);
	_conversationTerminator = Common::String((const char *)terminatorBuffer, 39);
	delete[] descBuffer;
	exe.close();
}

void ResourceManager::getExtraScreen(int screenIndex, byte *screenBuf, byte *palette) {
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		error("Couldnt find file ALFRED.7");
	}
	ExtraImages screen = extraScreens[screenIndex];
	mergeRleBlocks(&alfred7, screen.offset, 8, screenBuf);
	alfred7.seek(screen.paletteOffset, SEEK_SET);
	alfred7.read(palette, 768);
	for (int i = 0; i < 256; i++) {
		palette[i * 3] = palette[i * 3] << 2;
		palette[i * 3 + 1] = palette[i * 3 + 1] << 2;
		palette[i * 3 + 2] = palette[i * 3 + 2] << 2;
	}
	alfred7.close();
}

Common::Array<Common::StringArray> ResourceManager::getCredits() {
	Common::File exe;
	if (!exe.open("JUEGO.EXE")) {
		error("Couldnt find file JUEGO.EXE");
	}
	byte *descBuffer = new byte[kCreditsSize];
	exe.seek(kCreditsOffset, SEEK_SET);
	exe.read(descBuffer, kCreditsSize);
	Common::Array<Common::StringArray> credits = processTextData(descBuffer, kCreditsSize);
	delete[] descBuffer;
	exe.close();
	return credits;
}

Common::Array<Common::StringArray> ResourceManager::processTextData(byte *data, size_t size, bool decode) {
	int pos = 0;
	Common::String desc = "";
	Common::StringArray lines;
	Common::Array<Common::StringArray> texts;
	while (pos < size) {
		if (data[pos] == CTRL_END_TEXT) {
			if (!desc.empty()) {

				lines.push_back(desc);
				texts.push_back(lines);
				lines.clear();
				desc = Common::String();
			}
			pos++;
			continue;
		}
		if (data[pos] == 0x00 || data[pos] == 0x78) {
			pos++;
			continue;
		}

		if (data[pos] == CTRL_SPEAKER_ID) {
			byte color = data[pos + 1];
			desc.append(1, '@');
			desc.append(1, color);
			pos += 2;

			continue;
		}
		if (data[pos] == 0xC8) {
			lines.push_back(desc);
			desc = Common::String();
			pos++;
			continue;
		}
		if (decode)
			desc.append(1, decodeChar(data[pos]));
		else
			desc.append(1, data[pos]);
		if (pos + 1 == size) {
			lines.push_back(desc);
			texts.push_back(lines);
		}
		pos++;
	}
	return texts;
}

Pelrock::Sticker ResourceManager::getSticker(int stickerIndex) {
	Common::File alfred6File;
	if (!alfred6File.open("ALFRED.6")) {
		error("Couldnt find file ALFRED.6");
	}

	uint32 stickerOffset = pegatina_offsets[stickerIndex];
	alfred6File.seek(stickerOffset, SEEK_SET);
	Sticker sticker;
	sticker.x = alfred6File.readUint16LE();
	sticker.y = alfred6File.readUint16LE();
	sticker.w = alfred6File.readByte();
	sticker.h = alfred6File.readByte();
	sticker.roomNumber = pegatina_rooms[stickerIndex];
	sticker.stickerIndex = stickerIndex;
	sticker.stickerData = new byte[sticker.w * sticker.h];
	alfred6File.read(sticker.stickerData, sticker.w * sticker.h);
	alfred6File.close();
	return sticker;
}

InventoryObject ResourceManager::getIconForObject(byte objectIndex) {
	byte iconIndex = 0;
	if (objectIndex < 59) {
		if (objectIndex >= 11 && objectIndex < 59) {
			iconIndex = ((objectIndex - 11) & 3) + 11; // Books cycle through icons 11-14
		} else {
			iconIndex = objectIndex; // Direct mapping for IDs 0-11
		}
	} else {
		iconIndex = objectIndex - 44; // Offset for high IDs (59+)
	}
	return _inventoryIcons[iconIndex];
}

void ResourceManager::mergeRleBlocks(Common::SeekableReadStream *stream, uint32 offset, int numBlocks, byte *outputBuffer) {
	stream->seek(offset, SEEK_SET);
	// get screen
	size_t combined_size = 0;
	for (int i = 0; i < numBlocks; i++) {
		byte *thisBlock = nullptr;
		size_t blockSize = 0;
		uint32 pos = stream->pos();
		readUntilBuda(stream, stream->pos(), thisBlock, blockSize);
		uint8_t *block_data = nullptr;
		size_t decompressedSize = rleDecompress(thisBlock, blockSize, 0, 640 * 400, &block_data, true);
		debug("Decompressed block %d: %zu bytes, total %zu", i, decompressedSize, combined_size + decompressedSize);
		if (combined_size + decompressedSize > 640 * 400) {
			debug("Warning: decompressed data exceeds output buffer size, truncating");
			decompressedSize = 640 * 400 - combined_size;
		}
		memcpy(outputBuffer + combined_size, block_data, decompressedSize);
		combined_size += decompressedSize;

		free(block_data);
		free(thisBlock);
	}
}

} // End of namespace Pelrock
