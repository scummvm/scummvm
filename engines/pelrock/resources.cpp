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

namespace Pelrock {

ResourceManager::ResourceManager(/* args */) {
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
		}

		// free the array of pointers
		delete[] alfredWalkFrames[i];
	}
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
	rleDecompress(raw, compressedSize, 0, compressedSize, &_popUpBalloon);

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
	rleDecompress(bufferFile, alfred3Size, 0, alfred3Size, &pic);

	int frameSize = kAlfredFrameHeight * kAlfredFrameWidth;
	for (int i = 0; i < 4; i++) {
		alfredIdle[i] = new byte[frameSize];
		int talkingFramesOffset = walkingAnimLengths[0] + walkingAnimLengths[1] + walkingAnimLengths[2] + walkingAnimLengths[3] + 4;

		int prevWalkingFrames = 0;
		int prevTalkingFrames = 0;

		for (int j = 0; j < i; j++) {
			prevWalkingFrames += walkingAnimLengths[j] + 1;
			prevTalkingFrames += talkingAnimLengths[j];
		}

		alfredWalkFrames[i] = new byte *[walkingAnimLengths[i]];

		int standingFrame = prevWalkingFrames;
		debug("Loading standing frame %d at index %d", i, standingFrame);
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
	rleDecompress(alfredCombRightRaw, alfredCombRightSize, 0, alfredCombRightSize, &alfredCombRight);

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
	rleDecompress(alfredCombLeftRaw, alfredCombLeftSize, 0, spriteMapSize, &alfredCombLeft);

	for (int i = 0; i < 11; i++) {
		alfredCombFrames[1][i] = new byte[frameSize];
		extractSingleFrame(alfredCombLeft, alfredCombFrames[1][i], i, kAlfredFrameWidth, kAlfredFrameHeight);
	}

	alfred7.close();
	free(alfredCombRightRaw);
	free(alfredCombLeftRaw);
}

} // End of namespace Pelrock
