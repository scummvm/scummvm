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

#include "scumm/file_nes.h"
#include "scumm/scumm.h"
#include "scumm/players/player_nes.h"

namespace Scumm {

static bool nesTitleWaitOrSkip(OSystem *system, uint32 timeoutMs) {
	const uint32 startTimeMs = system->getMillis();

	Common::EventManager *eventMan = system->getEventManager();
	while (true) {
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				return true;

			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN)
				return true;
		}

		if (timeoutMs && (system->getMillis() - startTimeMs) >= timeoutMs)
			return true;

		system->delayMillis(10);
	}
}

#ifndef DISABLE_NES_APU
static void nesTitle2TwinkleStep(
		Scumm::Player_NES *player,
		const byte (*twinkleGroups)[4][6],
		byte group,
		byte step) {
		if (!player)
			return;

		if (group >= 8 || step >= 4)
			return;

		if (step == 0)
			player->startTitleTwinkleGroup(twinkleGroups[group]);
	}
#endif

static bool nesTitle2WaitOrSkipWithSpriteAnim(
	OSystem *system,
	Scumm::ScummEngine *vm,
	const Scumm::ScummNESFile::NESTitleScreen &t,
	const byte *backgroundFrame,
	uint32 waitMs,
	Scumm::ScummNESFile *nesFile,
	Scumm::Player_NES *player) {
	const uint32 startMs = system->getMillis();

	struct ActiveSparkle {
		byte x;
		byte y;
		byte frameIndex;
		byte positionIndex;
	};

	struct PendingChirp {
		byte groupIndex;
		byte stepIndex;
		int16 framesUntilNext;
	};

	static bool sTitle2SparklesInitialized = false;
	static uint32 sTitle2LastTickMs = 0;
	static int16 sTitle2AnimFramesUntilAdvance = 0;
	static int16 sTitle2SpawnFramesUntilNext = 0;
	static uint sTitle2NextPositionIndex = 0;
	static uint sTitle2SpawnedCount = 0;
	static bool sTitle2AllSpawned = false;
	static Common::Array<ActiveSparkle> sTitle2ActiveSparkles;
	static Common::Array<PendingChirp> sTitle2PendingChirps;

#ifndef DISABLE_NES_APU
	static const byte kTwinkleGroups[8][4][6] = {
		{
			{ 0x03, 0x05, 0x00, 0x0A, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0E, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x0B, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x10, 0x40, 0x04 }
		},
		{
			{ 0x03, 0x05, 0x00, 0x0B, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x10, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x12, 0x40, 0x04 }
		},
		{
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x12, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x0A, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0F, 0x40, 0x04 }
		},
		{
			{ 0x03, 0x05, 0x00, 0x08, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x11, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0E, 0x40, 0x04 }
		},
		{
			{ 0x07, 0x05, 0x00, 0x0E, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x0C, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0D, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x08, 0x40, 0x04 }
		},
		{
			{ 0x07, 0x05, 0x00, 0x0F, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x08, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x11, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 }
		},
		{
			{ 0x07, 0x05, 0x00, 0x0B, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0D, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x0A, 0x40, 0x04 }
		},
		{
			{ 0x07, 0x05, 0x00, 0x11, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x09, 0x40, 0x04 },
			{ 0x07, 0x05, 0x00, 0x0E, 0x40, 0x04 },
			{ 0x03, 0x05, 0x00, 0x0C, 0x40, 0x04 }
		}
	};
#endif

	Common::Array<byte> sparkleChr;
	if (!nesFile || !nesFile->readTitle2SparkleChr(sparkleChr))
		return nesTitleWaitOrSkip(system, waitMs);

	Common::Array<byte> sparklePalette;
	if (!nesFile->readTitle2SparklePalette(sparklePalette) || sparklePalette.size() < 16)
		return nesTitleWaitOrSkip(system, waitMs);

	const byte sparkleSubPaletteIndex = 0;

	const byte sparklePosX[8] = { 0x25, 0x3E, 0x5D, 0x6B, 0x92, 0xA4, 0xCC, 0xE5 };
	const byte sparklePosY[8] = { 0x68, 0x48, 0x60, 0x3E, 0x6D, 0x5E, 0x50, 0x3D };
	const int16 animFramesPerStep = 10;
	const int16 spawnFramesPerSparkle = 24;

		if (!sTitle2SparklesInitialized) {
			sTitle2SparklesInitialized = true;
			sTitle2LastTickMs = system->getMillis();
			sTitle2AnimFramesUntilAdvance = animFramesPerStep;
			sTitle2SpawnFramesUntilNext = spawnFramesPerSparkle;
			sTitle2NextPositionIndex = 0;
			sTitle2SpawnedCount = 0;
			sTitle2AllSpawned = false;
			sTitle2ActiveSparkles.clear();
			sTitle2PendingChirps.clear();
		}

	Common::EventManager *evm = system->getEventManager();
	Common::Array<byte> framePixels;
	framePixels.resize(256u * 240u);

	while (true) {
		Common::Event ev;
		while (evm->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN && ev.kbd.keycode == Common::KEYCODE_ESCAPE)
				return true;

			if (ev.type == Common::EVENT_LBUTTONDOWN || ev.type == Common::EVENT_RBUTTONDOWN)
				return true;
		}

		if (waitMs && (system->getMillis() - startMs) >= waitMs)
			return true;

		const uint32 nowMs = system->getMillis();
		const uint32 elapsedMs = nowMs - sTitle2LastTickMs;
		uint32 tickCount = elapsedMs / 16;
		if (tickCount == 0) {
			system->delayMillis(1);
			continue;
		}
		if (tickCount > 5)
			tickCount = 5;
		for (uint32 tickIndex = 0; tickIndex < tickCount; ++tickIndex) {
			sTitle2AnimFramesUntilAdvance--;
			if (sTitle2AnimFramesUntilAdvance < 0) {
				sTitle2AnimFramesUntilAdvance = animFramesPerStep;

				for (int i = (int)sTitle2ActiveSparkles.size() - 1; i >= 0; --i) {
					sTitle2ActiveSparkles[i].frameIndex++;
					if (sTitle2ActiveSparkles[i].frameIndex >= 6)
						sTitle2ActiveSparkles.remove_at(i);
				}
			}

			if (!sTitle2AllSpawned) {
				sTitle2SpawnFramesUntilNext--;
				if (sTitle2SpawnFramesUntilNext < 0) {
					sTitle2SpawnFramesUntilNext = spawnFramesPerSparkle;

					ActiveSparkle sp;
					sp.x = sparklePosX[sTitle2NextPositionIndex];
					sp.y = sparklePosY[sTitle2NextPositionIndex];
					sp.frameIndex = 0;
					sp.positionIndex = (byte)sTitle2NextPositionIndex;
					sTitle2ActiveSparkles.push_back(sp);

					PendingChirp pc;
					pc.groupIndex = (byte)sTitle2NextPositionIndex;
					pc.stepIndex = 0;
					pc.framesUntilNext = 1;
					sTitle2PendingChirps.push_back(pc);

									sTitle2NextPositionIndex = (sTitle2NextPositionIndex + 1) & 7;
				sTitle2SpawnedCount++;
				if (sTitle2SpawnedCount >= 8)
					sTitle2AllSpawned = true;
				}
			}

			for (int i = (int)sTitle2PendingChirps.size() - 1; i >= 0; --i) {
				PendingChirp &pc = sTitle2PendingChirps[i];
				pc.framesUntilNext--;

				if (pc.framesUntilNext <= 0) {
#ifndef DISABLE_NES_APU
					nesTitle2TwinkleStep(player, kTwinkleGroups, pc.groupIndex, pc.stepIndex);
#endif
					sTitle2PendingChirps.remove_at(i);
					break;
				}
			}

		}
		sTitle2LastTickMs += tickCount * 16;

		memcpy(framePixels.begin(), backgroundFrame, 256u * 240u);

		for (uint sparkleIndex = 0; sparkleIndex < sTitle2ActiveSparkles.size(); ++sparkleIndex) {
			const ActiveSparkle &sp = sTitle2ActiveSparkles[sparkleIndex];
			const uint32 chrTileOffset = (uint32)sp.frameIndex * 16u;
			if (chrTileOffset + 16u > sparkleChr.size())
				continue;

			for (uint pixelY = 0; pixelY < 8; ++pixelY) {
				const byte p0 = sparkleChr[chrTileOffset + pixelY];
				const byte p1 = sparkleChr[chrTileOffset + 8u + pixelY];

				for (uint pixelX = 0; pixelX < 8; ++pixelX) {
					const uint shift = 7u - pixelX;
					const byte ci = (byte)(((p0 >> shift) & 1u) | (((p1 >> shift) & 1u) << 1u));
					if (ci == 0)
						continue;

					const int screenX = (int)sp.x + (int)pixelX;
					const int screenY = (int)sp.y + (int)pixelY;
					if (screenX < 0 || screenX >= 256 || screenY < 0 || screenY >= 240)
						continue;

					const uint32 dstIndex = (uint32)screenY * 256u + (uint32)screenX;

					const uint32 paletteBase = (uint32)sparkleSubPaletteIndex * 4u;
					framePixels[dstIndex] = (byte)(sparklePalette[paletteBase + (uint32)ci] & 0x3Fu);
				}
			}
		}

		system->copyRectToScreen(framePixels.begin(), 256, 0, 0, 256, 240);
		system->updateScreen();
	}
}



void ScummEngine::playNESTitleScreens() {

	ScummNESFile *nesFile = dynamic_cast<ScummNESFile *>(_fileHandle);
	if (!nesFile)
		return;

	Player_NES *player = dynamic_cast<Player_NES *>(_musicEngine);

	resetPalette(true);

	Common::Array<byte> framePixels;
	framePixels.resize(256u * 240u);

	for (uint titleIndex = 0; titleIndex < 2; ++titleIndex) {
		ScummNESFile::NESTitleScreen title;
		if (!nesFile->decodeTitleScreen(titleIndex, title))
			return;

		memset(framePixels.begin(), 0, 256u * 240u);

		const byte backgroundColorIndex = (title.palette.size() >= 1) ? (byte)(title.palette[0] & 0x3Fu) : 0;

		for (int tileY = 0; tileY < 30; ++tileY) {
			for (int tileX = 0; tileX < 32; ++tileX) {
				const uint32 nametableIndex = (uint32)tileY * 32u + (uint32)tileX;
				const byte tileIndex = (nametableIndex < title.nametable.size()) ? title.nametable[nametableIndex] : 0;
				const uint32 tileOff = (uint32)tileIndex * 16u;

				if (tileOff + 16u > title.gfx.size())
					continue;

				const int attrX = tileX / 4;
				const int attrY = tileY / 4;
				const uint32 attrIndex = (uint32)attrY * 8u + (uint32)attrX;
				const byte attrByte = (attrIndex < title.attributes.size()) ? title.attributes[attrIndex] : 0;
				const int quadX = (tileX % 4) / 2;
				const int quadY = (tileY % 4) / 2;
				const int shift = (quadY * 2 + quadX) * 2;
				const byte subPaletteIndex = (byte)((attrByte >> shift) & 0x03u);

				for (int py = 0; py < 8; ++py) {
					const byte plane0 = title.gfx[tileOff + (uint32)py];
					const byte plane1 = title.gfx[tileOff + 8u + (uint32)py];
					const int y = tileY * 8 + py;
					byte *dstRow = framePixels.begin() + y * 256 + tileX * 8;

					for (int px = 0; px < 8; ++px) {
						const int bitIndex = 7 - px;
						const byte lowBit = (byte)((plane0 >> bitIndex) & 1u);
						const byte highBit = (byte)((plane1 >> bitIndex) & 1u);
						const byte colorIndex = (byte)(lowBit | (highBit << 1u));

						byte nesColor = backgroundColorIndex;
						if (colorIndex != 0 && title.palette.size() >= 16) {
							const int paletteBase = (int)subPaletteIndex * 4;
							nesColor = (byte)(title.palette[paletteBase + (int)colorIndex] & 0x3Fu);
						}
						dstRow[px] = nesColor;
					}
				}
			}
		}

		_system->copyRectToScreen(framePixels.begin(), 256, 0, 0, 256, 240);
		_system->updateScreen();

		if (titleIndex == 0) {
			nesTitleWaitOrSkip(_system, 5000u);
		} else {
			nesTitle2WaitOrSkipWithSpriteAnim(_system, this, title, framePixels.begin(), 5500u, nesFile, player);
		}
	}
}

} // End of namespace Scumm
