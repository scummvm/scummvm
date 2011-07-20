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
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/eobcommon.h"
#include "kyra/resource.h"
#include "kyra/script_eob.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void LolEobBaseEngine::setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim) {
	if (_lvlShapeLeftRight[index << 1] == -1) {
		x1 = 0;
		x2 = 22;

		int16 y1 = 0;
		int16 y2 = 120;

		int m = index * 18;

		for (int i = 0; i < 18; i++) {
			uint8 d = _visibleBlocks[i]->walls[_sceneDrawVarDown];
			uint8 a = _wllWallFlags[d];

			if (a & 8) {
				int t = _dscDim2[(m + i) << 1];

				if (t > x1) {
					x1 = t;
					if (!(a & 0x10))
						scaleLevelShapesDim(index, y1, y2, -1);
				}

				t = _dscDim2[((m + i) << 1) + 1];

				if (t < x2) {
					x2 = t;
					if (!(a & 0x10))
						scaleLevelShapesDim(index, y1, y2, -1);
				}
			} else {
				int t = _dscDim1[m + i];

				if (!_wllVmpMap[d] || t == -40)
					continue;

				if (t == -41) {
					x1 = 22;
					x2 = 0;
					break;
				}

				if (t > 0 && x2 > t)
					x2 = t;

				if (t < 0 && x1 < -t)
					x1 = -t;
			}

			if (x2 < x1)
				break;
		}

		x1 += (_sceneXoffset >> 3);
		x2 += (_sceneXoffset >> 3);


		_lvlShapeTop[index] = y1;
		_lvlShapeBottom[index] = y2;
		_lvlShapeLeftRight[index << 1] = x1;
		_lvlShapeLeftRight[(index << 1) + 1] = x2;
	} else {
		x1 = _lvlShapeLeftRight[index << 1];
		x2 = _lvlShapeLeftRight[(index << 1) + 1];
	}

	drawLevelModifyScreenDim(dim, x1, 0, x2, 15);
}

void LolEobBaseEngine::scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim) {
	static const int8 dscY1[] = { 0x1E, 0x18, 0x10, 0x00 };
	static const int8 dscY2[] = { 0x3B, 0x47, 0x56, 0x78 };

	uint8 a = _dscDimMap[index];

	if (dim == -1 && a != 3)
		a++;

	y1 = dscY1[a];
	y2 = dscY2[a];

	if (dim == -1)
		return;

	const ScreenDim *cDim = screen()->getScreenDim(dim);

	screen()->modifyScreenDim(dim, cDim->sx, y1, cDim->w, y2 - y1);
}

void LolEobBaseEngine::drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2) {
	screen()->modifyScreenDim(dim, x1, y1 << 3, x2 - x1, (y2 - y1) << 3);
}

void LolEobBaseEngine::generateBlockDrawingBuffer() {
	_sceneDrawVarDown = _dscBlockMap[_currentDirection];
	_sceneDrawVarRight = _dscBlockMap[_currentDirection + 4];
	_sceneDrawVarLeft = _dscBlockMap[_currentDirection + 8];

	/*******************************************
    *             _visibleBlocks map           *
	*                                          *
	*     |     |     |     |     |     |      *
	*  00 |  01 |  02 |  03 |  04 |  05 |  06  *
	* ____|_____|_____|_____|_____|_____|_____ *
	*     |     |     |     |     |     |      *
	*     |  07 |  08 |  09 |  10 |  11 |      *
	*     |_____|_____|_____|_____|_____|      *
	*           |     |     |     |            *
	*           |  12 |  13 |  14 |            *
	*           |_____|_____|_____|            *
	*                 |     |                  *
	*              15 |  16 |  17              *
	*                 | (P) |                  *
	********************************************/

	memset(_blockDrawingBuffer, 0, 660 * sizeof(uint16));

	_wllProcessFlag = ((_currentBlock >> 5) + (_currentBlock & 0x1f) + _currentDirection) & 1;

	if (_wllProcessFlag) // floor and ceiling
		generateVmpTileDataFlipped(0, 15, 1, -330, 22, 15);
	else
		generateVmpTileData(0, 15, 1, -330, 22, 15);

	assignVisibleBlocks(_currentBlock, _currentDirection);

	uint8 t = _visibleBlocks[0]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(-2, 3, t, 102, 3, 5);

	t = _visibleBlocks[6]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(21, 3, t, 102, 3, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarRight];
	uint8 t2 = _visibleBlocks[2]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileData(2, 3, t2, 102, 3, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarLeft];
	t2 = _visibleBlocks[4]->walls[_sceneDrawVarDown];

	if (hasWall(t) && !(_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t, 102, 3, 5);
	else if (t && (_wllWallFlags[t2] & 8))
		generateVmpTileDataFlipped(17, 3, t2, 102, 3, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(8, 3, t, 97, 1, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(13, 3, t, 97, 1, 5);

	t = _visibleBlocks[1]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 3, t, 129, 6, 5);

	t = _visibleBlocks[5]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(20, 3, t, 129, 6, 5);

	t = _visibleBlocks[2]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(2, 3, t, 129, 6, 5);

	t = _visibleBlocks[4]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(14, 3, t, 129, 6, 5);

	t = _visibleBlocks[3]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(8, 3, t, 129, 6, 5);

	t = _visibleBlocks[7]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(0, 3, t, 117, 2, 6);

	t = _visibleBlocks[11]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(20, 3, t, 117, 2, 6);

	t = _visibleBlocks[8]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(6, 2, t, 81, 2, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(14, 2, t, 81, 2, 8);

	t = _visibleBlocks[8]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(-4, 2, t, 159, 10, 8);

	t = _visibleBlocks[10]->walls[_sceneDrawVarDown];
	if (hasWall(t))
		generateVmpTileData(16, 2, t, 159, 10, 8);

	t = _visibleBlocks[9]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(6, 2, t, 159, 10, 8);

	t = _visibleBlocks[12]->walls[_sceneDrawVarRight];
	if (t)
		generateVmpTileData(3, 1, t, 45, 3, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileDataFlipped(16, 1, t, 45, 3, 12);

	t = _visibleBlocks[12]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(-13, 1, t, 239, 16, 12);

	t = _visibleBlocks[14]->walls[_sceneDrawVarDown];
	if (!(_wllWallFlags[t] & 8))
		generateVmpTileData(19, 1, t, 239, 16, 12);

	t = _visibleBlocks[13]->walls[_sceneDrawVarDown];
	if (t)
		generateVmpTileData(3, 1, t, 239, 16, 12);

	t = _visibleBlocks[15]->walls[_sceneDrawVarRight];
	t2 = _visibleBlocks[17]->walls[_sceneDrawVarLeft];
	if (t)
		generateVmpTileData(0, 0, t, 0, 3, 15);
	if (t2)
		generateVmpTileDataFlipped(19, 0, t2, 0, 3, 15);
}

void LolEobBaseEngine::generateVmpTileData(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		uint16 *bl = &_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX];
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii >= 0) && (startBlockX + ii < 22) && *vmp)
				*bl = *vmp;
			bl++;
			vmp++;
		}
	}
}

void LolEobBaseEngine::generateVmpTileDataFlipped(int16 startBlockX, uint8 startBlockY, uint8 vmpMapIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY) {
	if (!_wllVmpMap[vmpMapIndex])
		return;

	uint16 *vmp = &_vmpPtr[(_wllVmpMap[vmpMapIndex] - 1) * 431 + vmpOffset + 330];

	for (int i = 0; i < numBlocksY; i++) {
		for (int ii = 0; ii < numBlocksX; ii++) {
			if ((startBlockX + ii) < 0 || (startBlockX + ii) > 21)
				continue;

			uint16 v = vmp[i * numBlocksX + (numBlocksX - 1 - ii)];
			if (!v)
				continue;

			if (v & 0x4000)
				v -= 0x4000;
			else
				v |= 0x4000;

			_blockDrawingBuffer[(startBlockY + i) * 22 + startBlockX + ii] = v;
		}
	}
}

bool LolEobBaseEngine::hasWall(int index) {
	if (!index || (_wllWallFlags[index] & 8))
		return false;
	return true;
}

void LolEobBaseEngine::assignVisibleBlocks(int block, int direction) {
	for (int i = 0; i < 18; i++) {
		uint16 t = (block + _dscBlockIndex[direction * 18 + i]) & 0x3ff;
		_visibleBlockIndex[i] = t;

		_visibleBlocks[i] = &_levelBlockProperties[t];
		_lvlShapeLeftRight[i] = _lvlShapeLeftRight[18 + i] = -1;
	}
}

bool LolEobBaseEngine::checkSceneUpdateNeed(int block) {
	if (_sceneUpdateRequired)
		return true;

	for (int i = 0; i < 15; i++) {
		if (_visibleBlockIndex[i] == block) {
			_sceneUpdateRequired = true;
			return true;
		}
	}

	if (_currentBlock == block){
		_sceneUpdateRequired = true;
		return true;
	}

	return false;
}

void LolEobBaseEngine::drawVcnBlocks() {
	uint8 *d = _sceneWindowBuffer;
	uint16 *bdb = _blockDrawingBuffer;

	for (int y = 0; y < 15; y++) {
		for (int x = 0; x < 22; x++) {
			bool horizontalFlip = false;
			int remainder = 0;

			uint16 vcnOffset = *bdb++;
			int wllVcnOffset = 0;
			int wllVcnRmdOffset = 0;

			if (vcnOffset & 0x8000) {
				// this renders a wall block over the transparent pixels of a floor/ceiling block
				remainder = vcnOffset - 0x8000;
				vcnOffset = 0;
				wllVcnRmdOffset = _wllVcnOffset;
			}

			if (vcnOffset & 0x4000) {
				horizontalFlip = true;
				vcnOffset &= 0x3fff;
			}

			uint8 *src = 0;
			if (vcnOffset) {
				src = &_vcnBlocks[vcnOffset << 5];
				wllVcnOffset = _wllVcnOffset;
			} else {
				// floor/ceiling blocks
				vcnOffset = bdb[329];
				if (vcnOffset & 0x4000) {
					horizontalFlip = true;
					vcnOffset &= 0x3fff;
				}

				src = (_vcfBlocks ? _vcfBlocks : _vcnBlocks) + (vcnOffset << 5);
			}

			uint8 shift = _vcnShift ? _vcnShift[vcnOffset] : _blockBrightness;

			if (horizontalFlip) {
				for (int blockY = 0; blockY < 8; blockY++) {
					src += 3;
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 t = *src--;
						*d++ = _vcnExpTable[((t & 0x0f) + wllVcnOffset) | shift];
						*d++ = _vcnExpTable[((t >> 4) + wllVcnOffset) | shift];
					}
					src += 5;
					d += 168;
				}
			} else {
				for (int blockY = 0; blockY < 8; blockY++) {
					for (int blockX = 0; blockX < 4; blockX++) {
						uint8 t = *src++;
						*d++ = _vcnExpTable[((t >> 4) + wllVcnOffset) | shift];
						*d++ = _vcnExpTable[((t & 0x0f) + wllVcnOffset) | shift];
					}
					d += 168;
				}
			}
			d -= 1400;

			if (remainder) {
				d -= 8;
				horizontalFlip = false;

				if (remainder & 0x4000) {
					remainder &= 0x3fff;
					horizontalFlip = true;
				}

				shift = _vcnShift ? _vcnShift[remainder] : _blockBrightness;
				src = &_vcnBlocks[remainder << 5];

				if (horizontalFlip) {
					for (int blockY = 0; blockY < 8; blockY++) {
						src += 3;
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 t = *src--;
							uint8 h = _vcnExpTable[((t & 0x0f) + wllVcnRmdOffset) | shift];
							uint8 l = _vcnExpTable[((t >> 4) + wllVcnRmdOffset) | shift];
							if (h)
								*d = h;
							d++;
							if (l)
								*d = l;
							d++;
						}
						src += 5;
						d += 168;
					}
				} else {
					for (int blockY = 0; blockY < 8; blockY++) {
						for (int blockX = 0; blockX < 4; blockX++) {
							uint8 t = *src++;
							uint8 h = _vcnExpTable[((t >> 4) + wllVcnRmdOffset) | shift];
							uint8 l = _vcnExpTable[((t & 0x0f) + wllVcnRmdOffset) | shift];
							if (h)
								*d = h;
							d++;
							if (l)
								*d = l;
							d++;
						}
						d += 168;
					}
				}
				d -= 1400;
			}
		}
		d += 1232;
	}

	screen()->copyBlockToPage(_sceneDrawPage1, _sceneXoffset, 0, 176, 120, _sceneWindowBuffer);
}

uint16 LolEobBaseEngine::calcNewBlockPosition(uint16 curBlock, uint16 direction) {
	static const int16 blockPosTable[] = { -32, 1, 32, -1 };
	return (curBlock + blockPosTable[direction]) & 0x3ff;
}

int LolEobBaseEngine::clickedWallShape(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	snd_stopSpeech(true);
	runLevelScript(block, 0x40);

	return 1;
}

int LolEobBaseEngine::clickedLeverOn(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]++;
	_sceneUpdateRequired = true;

	if (_flags.gameID == GI_LOL)
		snd_playSoundEffect(30, -1);

	runLevelScript(block, _clickedSpecialFlag);

	return 1;
}

int LolEobBaseEngine::clickedLeverOff(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	_levelBlockProperties[block].walls[direction]--;
	_sceneUpdateRequired = true;

	if (_flags.gameID == GI_LOL)
		snd_playSoundEffect(29, -1);

	runLevelScript(block, _clickedSpecialFlag);
	return 1;
}

int LolEobBaseEngine::clickedWallOnlyScript(uint16 block) {
	runLevelScript(block, _clickedSpecialFlag);
	return 1;
}

void LolEobBaseEngine::processDoorSwitch(uint16 block, int openClose) {
	if (block == _currentBlock)
		return;

	if ((_flags.gameID == GI_LOL && (_levelBlockProperties[block].assignedObjects & 0x8000)) || (_flags.gameID != GI_LOL  && (_levelBlockProperties[block].flags & 7)))
		return;

	if (openClose == 0) {
		for (int i = 0; i < 3; i++) {
			if (_openDoorState[i].block != block)
				continue;
			openClose = -_openDoorState[i].state;
			break;
		}
	}

	if (openClose == 0) {
		openClose = (_wllWallFlags[_levelBlockProperties[block].walls[_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8 ? 0 : 1]] & 1) ? 1 : -1;
		if (_flags.gameID != GI_LOL)
			openClose *= -1;
	}

	openCloseDoor(block, openClose);
}

void LolEobBaseEngine::openCloseDoor(int block, int openClose) {
	int s1 = -1;
	int s2 = -1;

	int c = (_wllWallFlags[_levelBlockProperties[block].walls[0]] & 8) ? 0 : 1;
	int v = _levelBlockProperties[block].walls[c];
	int flg = (openClose == 1) ? 0x10 : (openClose == -1 ? 0x20 : 0);

	if (_wllWallFlags[v] & flg)
		return;

	for (int i = 0; i < 3; i++) {
		if (_openDoorState[i].block == block) {
			s1 = i;
			break;
		} else if (_openDoorState[i].block == 0 && s2 == -1) {
			s2 = i;
		}
	}

	if (s1 != -1 || s2 != -1) {
		if (s1 == -1)
			s1 = s2;

		_openDoorState[s1].block = block;
		_openDoorState[s1].state = openClose;
		_openDoorState[s1].wall = c;

		flg = (-openClose == 1) ? 0x10 : (-openClose == -1 ? 0x20 : 0);

		if (_wllWallFlags[v] & flg) {
			_levelBlockProperties[block].walls[c] += openClose;
			_levelBlockProperties[block].walls[c ^ 2] += openClose;

			int snd = (openClose == -1) ? 4 : 3;
			if (_flags.gameID == GI_LOL) {
				snd_processEnvironmentalSoundEffect(snd + 28, _currentBlock);
				if (!checkSceneUpdateNeed(block))
					updateEnvironmentalSfx(0);
			} else {
				updateEnvironmentalSfx(snd);
			}
		}

		enableTimer(_flags.gameID == GI_LOL ? 0 : 4);

	} else {
		while (!(flg & _wllWallFlags[v]))
			v += openClose;

		_levelBlockProperties[block].walls[c] = _levelBlockProperties[block].walls[c ^ 2] = v;
		checkSceneUpdateNeed(block);
	}
}

void LolEobBaseEngine::completeDoorOperations() {
	for (int i = 0; i < 3; i++) {
		if (!_openDoorState[i].block)
			continue;

		uint16 b = _openDoorState[i].block;

		do {
			_levelBlockProperties[b].walls[_openDoorState[i].wall] += _openDoorState[i].state;
			_levelBlockProperties[b].walls[_openDoorState[i].wall ^ 2] += _openDoorState[i].state;
		} while	(!(_wllWallFlags[_levelBlockProperties[b].walls[_openDoorState[i].wall]] & 0x30));

		_openDoorState[i].block = 0;
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL
#ifdef ENABLE_EOB

namespace Kyra {

void EobCoreEngine::loadLevel(int level, int sub) {
	_currentLevel = level;
	_currentSub = sub;

	Common::String file;
	Common::SeekableReadStream *s = 0;
	static const char *suffix[] = { "INF", "DRO", "ELO", 0 };
	
	for (const char *const *sf = suffix; *sf && !s; sf++) {
		file = Common::String::format("LEVEL%d.%s", level, *sf);
		s = _res->createReadStream(file);
	}

	if (!s)
		error("Failed to load level file LEVEL%d.INF/DRO/ELO", level);

	if (s->readUint16LE() + 2 == s->size()) {
		if (s->readUint16LE() == 4) {
			delete s;
			s = 0;
			_screen->loadBitmap(file.c_str(), 5, 5, 0);
		}
	}

	if (s) {
		s->seek(0);
		_screen->loadFileDataToPage(s, 5, 15000);
		delete s;
	}

	Common::String gfxFile = initLevelData(sub);

	const uint8 *data = _screen->getCPagePtr(5);
	const uint8 *pos = data + READ_LE_UINT16(data);
	uint16 len = READ_LE_UINT16(pos);
	uint16 len2 = len;
	pos += 2;

	if (_flags.gameID == GI_EOB2) {
		if (*pos++ == 0xEC)
			pos = loadActiveMonsterData(pos, level);
		else if (!(_hasTempDataFlags & (1 << (level - 1))))
			memset(_monsters, 0, 30 * sizeof(EobMonsterInPlay));

		len2 = len - (pos - data);
		_inf->loadData(pos, len2);
	} else {
		_inf->loadData(data, READ_LE_UINT16(data));
	}

	_screen->setCurPage(2);
	addLevelItems();

	if (_flags.gameID == GI_EOB2) {
		pos = data + len;
		len2 = READ_LE_UINT16(pos);
		pos += 2;
	}

	for (uint16 i = 0; i < len2; i++) {
		LevelBlockProperty *p = &_levelBlockProperties[READ_LE_UINT16(pos)];
		pos += 2;
		if (_flags.gameID == GI_EOB2) {
			p->flags |= READ_LE_UINT16(pos);
			pos += 2;
		} else {
			p->flags |= *pos++;
		}
		p->assignedObjects = READ_LE_UINT16(pos);
		pos += 2;
	}

	loadVcnData(gfxFile.c_str(), 0);
	_screen->loadEobCpsFileToPage("INVENT", 0, 5, 3, 2);

	enableSysTimer(2);
	_sceneDrawPage1 = 2;
	_sceneDrawPage2 = 1;
	_screen->setCurPage(0);
}

Common::String EobCoreEngine::initLevelData(int sub){
	const uint8 *data = _screen->getCPagePtr(5) + 2;
	const uint8 *pos = data;

	int slen = (_flags.gameID == GI_EOB1) ? 12 : 13;

	_sound->playTrack(0);

	for (int i = 0; i < sub; i++)
		pos = data + READ_LE_UINT16(pos);

	pos += 2;
	if (*pos++ == 0xEC || _flags.gameID == GI_EOB1) {
		if (_flags.gameID == GI_EOB1)
			pos -= 3;

		loadBlockProperties((const char*)pos);
		pos += slen;

		Common::SeekableReadStream *s = _res->createReadStream(Common::String::format("%s.VMP", (const char*)pos));
		uint16 size = s->readUint16LE();
		delete[] _vmpPtr;
		_vmpPtr = new uint16[size];
		for (int i = 0; i < size; i++)
			_vmpPtr[i] = s->readUint16LE();
		delete s;

		Common::String tmpStr = Common::String::format("%s.PAL", (const char*) pos);
		_curGfxFile = (const char*) pos;
		pos += slen;

		if (*pos++ != 0xff && _flags.gameID == GI_EOB2) {
			tmpStr = Common::String::format("%s.PAL", (const char*) pos);
			pos += 13;
		}

	////////	_screen->loadPalette(tmpStr, _screen->getPalette(0));

		if (_flags.gameID == GI_EOB1) {
			pos += 11;
			_screen->setShapeFadeMode(0, false);
			_screen->setShapeFadeMode(1, false);
		}
		else
			_screen->loadPalette(tmpStr.c_str(), _screen->getPalette(0));

		Palette backupPal(256);
		backupPal.copy(_screen->getPalette(0), 224, 32, 224);
		_screen->getPalette(0).fill(224, 32, 0x3f);
		uint8 *src = _screen->getPalette(0).getData();

		_screen->createFadeTable(src, _screen->getFadeTable(0), 4, 75);		// green
		_screen->createFadeTable(src, _screen->getFadeTable(1), 12, 200);	// black
		_screen->createFadeTable(src, _screen->getFadeTable(2), 10, 85);	// blue
		_screen->createFadeTable(src, _screen->getFadeTable(3), 11, 125);	// light blue

		_screen->getPalette(0).copy(backupPal, 224, 32, 224);
		_screen->createFadeTable(src, _screen->getFadeTable(4), 12, 85);	// grey (shadow)
		_screen->setFadeTableIndex(4);
	}

	if (_flags.gameID == GI_EOB2) {
		delay(_tickLength);
		_sound->loadSoundFile((const char*) pos);
		pos += 13;
	}

	releaseDoorShapes();
	releaseMonsterShapes(0, 36);
	releaseDecorations();

	if (_flags.gameID == GI_EOB1) {
		loadDoorShapes(pos[0], pos[1], pos[2], pos[3]);
		pos += 4;
		_scriptTimersMode = *pos++;
		_scriptTimers[0].ticks = READ_LE_UINT16(pos);
		_scriptTimers[0].func = 0;
		_scriptTimers[0].next = _system->getMillis() + _scriptTimers[0].ticks * _tickLength;
		pos+= 2;
	} else {
		for (int i = 0; i < 2; i++) {
			int a = (*pos == 0xEC) ? 0 : ((*pos == 0xEA) ? 1 : -1);
			pos++;
			if (a == -1)
				continue;
			toggleWallState(pos[13], a);
			_doorType[pos[13]] = pos[14];
			_noDoorSwitch[pos[13]] = pos[15];
			pos = loadDoorShapes((const char*)pos, pos[13], pos + 16);
		}
	}

	_stepsUntilScriptCall = READ_LE_UINT16(pos);
	pos+= 2;
	_stepCounter = 0;

	for (int i = 0; i < 2; i++) {
		if (_flags.gameID == GI_EOB1) {
			if (*pos == 0xFF)
				continue;
			loadMonsterShapes((const char *)(pos + 1), i * 18, false, *pos * 18);
			pos += 13;
		} else {
			if (*pos++ != 0xEC)
				continue;
			loadMonsterShapes((const char *)(pos + 2), pos[1] * 18, pos[15] ? true : false, *pos * 18);
			pos += 16;
		}
	}

	if (_flags.gameID == GI_EOB1)
		pos = loadActiveMonsterData(pos, _currentLevel) - 1;
	else
		pos = loadMonsterProperties(pos);

	if (*pos++ == 0xEC || _flags.gameID == GI_EOB1) {
		int num = READ_LE_UINT16(pos);
		pos += 2;

		for (int i = 0; i < num; i++) {
			if (*pos++ == 0xEC) {
				loadDecorations((const char*)pos, (const char*)(pos + slen));
				pos += (slen << 1);
			} else {
				assignWallsAndDecorations(pos[0], pos[1], (int8)pos[2], pos[3], pos[4]);
				pos += 5;
			}
		}
	}

	if (_flags.gameID == GI_EOB2)
		pos = initScriptTimers(pos);

	return _curGfxFile;
}

void EobCoreEngine::addLevelItems() {
	for (int i = 0; i < 1024; i++)
		_levelBlockProperties[i].drawObjects = 0;

	for (int i = 0; i < 600; i++) {
		if (_items[i].level != _currentLevel || _items[i].block <= 0)
			continue;
		setItemPosition((Item*)&_levelBlockProperties[_items[i].block & 0x3ff].drawObjects, _items[i].block, i, _items[i].pos);
	}
}

void EobCoreEngine::loadVcnData(const char *file, const char*/*nextFile*/) {
	if (file)
		strcpy(_lastBlockDataFile, file);
	
	_screen->loadBitmap(Common::String::format("%s.VCN", _lastBlockDataFile).c_str(), 3, 3, 0);
	const uint8 *v = _screen->getCPagePtr(2);
	uint32 tlen = READ_LE_UINT16(v) << 5;
	v += 2;
	memcpy(_vcnExpTable, v, 32);
	v += 32;
	delete[] _vcnBlocks;
	_vcnBlocks = new uint8[tlen];
	memcpy(_vcnBlocks, v, tlen);
}

void EobCoreEngine::loadBlockProperties(const char *mazFile) {
	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));
	Common::SeekableReadStream *s = _res->createReadStream(mazFile);
	_screen->loadFileDataToPage(s, 2, 4096);
	delete s;

	if (_hasTempDataFlags & (1 << (_currentLevel - 1))) {
		restoreBlockTempData(_currentLevel);
		return;
	}

	const uint8 *p = _screen->getCPagePtr(2) + 6;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = *p++;
	}
}

void EobCoreEngine::loadDecorations(const char *cpsFile, const char *decFile) {
	_screen->loadEobBitmap(cpsFile, 3, 3);
	Common::SeekableReadStream *s = _res->createReadStream(decFile);

	_levelDecorationDataSize = s->readUint16LE();
	delete[] _levelDecorationData;
	_levelDecorationData = new LevelDecorationProperty[_levelDecorationDataSize];

	for (int i = 0; i < _levelDecorationDataSize; i++) {
		LevelDecorationProperty *l = &_levelDecorationData[i];
		for (int ii = 0; ii < 10; ii++) {
			l->shapeIndex[ii] = s->readByte();
			if (l->shapeIndex[ii] == 0xff)
				l->shapeIndex[ii] = 0xffff;
		}
		l->next = s->readByte();
		l->flags = s->readByte();
		for (int ii = 0; ii < 10; ii++)
			l->shapeX[ii] = s->readSint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->shapeY[ii] = s->readSint16LE();
	}

	int len = s->readUint16LE();
	delete[] _levelDecorationRects;
	_levelDecorationRects = new EobRect8[len];
	for (int i = 0; i < len; i++) {
		EobRect8 *l = &_levelDecorationRects[i];
		l->x = s->readUint16LE();
		l->y = s->readUint16LE();
		l->w = s->readUint16LE();
		l->h = s->readUint16LE();
	}

	delete s;
}

void EobCoreEngine::assignWallsAndDecorations(int wallIndex, int vmpIndex, int decIndex, int specialType, int flags) {
	_wllVmpMap[wallIndex] = vmpIndex;
	_wllShapeMap[wallIndex] = _mappedDecorationsCount + 1;
	_specialWallTypes[wallIndex] = specialType;
	_wllWallFlags[wallIndex] = flags ^ 4;

	if (decIndex == -1) {
		_wllShapeMap[wallIndex] = 0;
		return;
	}

	do {
		memcpy(&_levelDecorationProperties[_mappedDecorationsCount], &_levelDecorationData[decIndex], sizeof(LevelDecorationProperty));

		for (int i = 0; i < 10; i++) {
			uint16 t = _levelDecorationProperties[_mappedDecorationsCount].shapeIndex[i];
			if (t == 0xffff)
				continue;

			if (_levelDecorationShapes[t])
				continue;

			EobRect8 *r = &_levelDecorationRects[t];
			if (r->w == 0 || r->h == 0)
				error("Error trying to make decoration %d	x: %d y:%d w:%d h:%d", decIndex, r->x, r->y, r->w, r->h);

			_levelDecorationShapes[t] = _screen->encodeShape(r->x, r->y, r->w, r->h);
		}

		decIndex = _levelDecorationProperties[_mappedDecorationsCount++].next;

		if (decIndex)
			_levelDecorationProperties[_mappedDecorationsCount - 1].next = _mappedDecorationsCount + 1;
		else
			decIndex = -1;

	} while (decIndex != -1);
}

void EobCoreEngine::releaseDecorations() {
	if (_levelDecorationShapes) {
		for (int i = 0; i < 400; i++) {
			delete[] _levelDecorationShapes[i];
			_levelDecorationShapes[i] = 0;
		}
	}
	_mappedDecorationsCount = 0;
}

void EobCoreEngine::releaseDoorShapes() {
	for (int i = 0; i < 6; i++) {
		delete[] _doorShapes[i];
		_doorShapes[i] = 0;
		delete[] _doorSwitches[i].shp;
		_doorSwitches[i].shp = 0;
	}
}

void EobCoreEngine::toggleWallState(int wall, int toggle) {
	wall = wall * 10 + 3;

	for (int i = 0; i < 9 ; i++) {
		if (i == 4)
			continue;

		if (toggle)
			_wllWallFlags[wall + i] |= 2;
		else
			_wllWallFlags[wall + i] &= 0xfd;
	}
}

void EobCoreEngine::drawScene(int refresh) {
	generateBlockDrawingBuffer();
	drawVcnBlocks();
	drawSceneShapes();

	if (_sceneDrawPage2) {
		if (refresh)
			_screen->fillRect(0, 0, 176, 120, 12);

		_screen->setScreenPalette(_screen->getPalette(0));
		_sceneDrawPage2 = 0;
	}

	uint32 ct = _system->getMillis();
	if (_flashShapeTimer > ct) {
		int diff = _flashShapeTimer - ct;
		while ((diff > 0) && !shouldQuit()) {
			updateInput();
			uint32 step = MIN<uint32>(diff, _tickLength / 5);
			_system->delayMillis(step);
			diff -= step;
		}
	}

	if (_sceneDefaultUpdate) {
		resetSkipFlag();
		delayUntil(_drawSceneTimer);
	}

	if (refresh && !_partyResting)
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);

	updateEnvironmentalSfx(0);

	if (!_dialogueField && refresh && !_updateFlags)
		gui_drawCompass(false);

	if (refresh && !_partyResting)
		_screen->updateScreen();

	if (_sceneDefaultUpdate) {
		_sceneDefaultUpdate = false;
		_drawSceneTimer = _system->getMillis() /*+ 4 * _tickLength*/;
	}

	_sceneUpdateRequired = false;
}

void EobCoreEngine::drawSceneShapes(int start) {
	for (int i = start; i < 18; i++) {
		uint8 t = _dscTileIndex[i];
		uint8 s = _visibleBlocks[t]->walls[_sceneDrawVarDown];

		_shpDmX1 = 0;
		_shpDmX2 = 0;

		setLevelShapesDim(t, _shpDmX1, _shpDmX2, _sceneShpDim);

		if (_shpDmX2 <= _shpDmX1)
			continue;

		drawDecorations(t);

		if (_visibleBlocks[t]->drawObjects)
			drawBlockItems(t);

		if (t < 15) {
			uint16 w = _wllWallFlags[s];

			if (w & 8)
				drawDoor(t);

			if (_visibleBlocks[t]->flags & 7) {
				const ScreenDim *dm = _screen->getScreenDim(5);
				_screen->modifyScreenDim(5, dm->sx, _lvlShapeTop[t], dm->w, _lvlShapeBottom[t] - _lvlShapeTop[t]);
				drawMonsters(t);
				drawLevelModifyScreenDim(5, _lvlShapeLeftRight[(t << 1)], 0, _lvlShapeLeftRight[(t << 1) + 1], 15);
			}

			if (_flags.gameID == GI_EOB2 && s == 74)
				drawWallOfForce(t);
		}

		drawFlyingObjects(t);

		if (s == _teleporterWallId)
			drawTeleporter(t);
	}
}

void EobCoreEngine::drawDecorations(int index) {
	static const int16 *dscWalls[] =  { 0, 0, &_sceneDrawVarDown, &_sceneDrawVarRight, &_sceneDrawVarDown,
		&_sceneDrawVarRight, &_sceneDrawVarDown, 0, &_sceneDrawVarDown, &_sceneDrawVarLeft, &_sceneDrawVarDown,
		&_sceneDrawVarLeft,	0, 0, &_sceneDrawVarDown, &_sceneDrawVarRight, &_sceneDrawVarDown, &_sceneDrawVarRight,
		&_sceneDrawVarDown,	0, &_sceneDrawVarDown, &_sceneDrawVarLeft, &_sceneDrawVarDown, &_sceneDrawVarLeft,
		&_sceneDrawVarDown,	&_sceneDrawVarRight, &_sceneDrawVarDown, 0, &_sceneDrawVarDown, &_sceneDrawVarLeft,
		0, &_sceneDrawVarRight,	&_sceneDrawVarDown, 0, 0, &_sceneDrawVarLeft
	};

	for (int i = 1; i >= 0; i--) {
		int s = index * 2 + i;
		if (dscWalls[s]) {
			int d = *dscWalls[s];
			int8 l = _wllShapeMap[_visibleBlocks[index]->walls[d]];

			uint8 *shapeData = 0;

			int x = 0;

			while (l > 0) {
				l--;
				int8 ix = _dscShapeIndex[s];
				uint8 shpIx = ABS(ix) - 1;
				uint8 flg = _levelDecorationProperties[l].flags;

				if ((i == 0) && (flg & 1 || ((flg & 2) && _wllProcessFlag)))
					ix = -ix;

				if (_levelDecorationProperties[l].shapeIndex[shpIx] == 0xffff) {
					l = _levelDecorationProperties[l].next;
					continue;
				}

				shapeData = _levelDecorationShapes[_levelDecorationProperties[l].shapeIndex[shpIx]];
				if (shapeData) {
					x = 0;
					if (i == 0) {
						if (flg & 4)
							x += _dscShapeCoords[(index * 5 + 4) << 1];
						else
							x += _dscShapeX[index];
					}

					if (ix < 0) {
						x += (176 - _levelDecorationProperties[l].shapeX[shpIx] - (shapeData[2] << 3));
						drawBlockObject(1, 2, shapeData, x, _levelDecorationProperties[l].shapeY[shpIx], _sceneShpDim);
					} else {
						x += _levelDecorationProperties[l].shapeX[shpIx];
						drawBlockObject(0, 2, shapeData, x, _levelDecorationProperties[l].shapeY[shpIx], _sceneShpDim);

					}
				}
				l = _levelDecorationProperties[l].next;
				continue;
			}
		}
	}
}

int EobCoreEngine::calcNewBlockPositionAndTestPassability(uint16 curBlock, uint16 direction) {
	uint16 b = calcNewBlockPosition(curBlock, direction);
	int w = _levelBlockProperties[b].walls[direction ^ 2];
	int f = _wllWallFlags[w];

	if (w == 74 && _currentBlock == curBlock) {
		for (int i = 0; i < 5; i++) {

		}
	}

	if (!(f & 1) || _levelBlockProperties[b].flags & 7)
		return -1;

	return b;
}

void EobCoreEngine::notifyBlockNotPassable() {
	_txt->printMessage(_warningStrings[0]);
	snd_playSoundEffect(29);
	removeInputTop();
}

void EobCoreEngine::moveParty(uint16 block) {
	//processMonstersUnk1();
	uint16 old = _currentBlock;
	_currentBlock = block;

	runLevelScript(old, 2);

	if (++_moveCounter > 3) {
		_txt->printMessage("\r");
		_moveCounter = 0;
	}

	runLevelScript(block, 1);

	if (_levelBlockProperties[block].walls[0] == 26)
		memset(_levelBlockProperties[block].walls, 0, 4);

	//processMonstersUnk1();
	_stepCounter++;
	//_keybControlUnk = -1;
	_sceneUpdateRequired = true;

	checkFlyingObjects();
}

int EobCoreEngine::clickedDoorSwitch(uint16 block, uint16 direction) {
	uint8 v = _visibleBlocks[13]->walls[_sceneDrawVarDown];
	SpriteDecoration *d = &_doorSwitches[((v > 12 && v < 23) || v == 31) ? 3 : 0];
	int x1 = d->x + _dscShapeCoords[138] - 4;
	int y1 = d->y - 4;
	if (!posWithinRect(_mouseX, _mouseY, x1, y1, x1 + (d->shp[2] << 3) + 8, y1 + d->shp[1] + 8) && (_clickedSpecialFlag == 0x40))
		return clickedDoorNoPry(block, direction);

	processDoorSwitch(block, 0);
	snd_playSoundEffect(6);

	return 1;
}

int EobCoreEngine::clickedNiche(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	if (_itemInHand) {
		if (_dscItemShapeMap[_items[_itemInHand].icon] <= 14) {
			_txt->printMessage(_pryDoorStrings[5]);
		} else {
			setItemPosition((Item*)&_levelBlockProperties[block & 0x3ff].drawObjects, block, _itemInHand, 8);
			runLevelScript(block, 4);
			setHandItem(0);
			_sceneUpdateRequired = true;
		}
	} else {
		int d = getQueuedItem((Item*)&_levelBlockProperties[block].drawObjects, 8, -1);
		if (!d)
			return 1;
		runLevelScript(block, 8);
		setHandItem(d);
		_sceneUpdateRequired = true;
	}

	return 1;
}

int EobCoreEngine::clickedDoorPry(uint16 block, uint16 direction) {
	if (!posWithinRect(_mouseX, _mouseY, 40, 16, 136, 88) && (_clickedSpecialFlag == 0x40))
		return 0;

	int d = -1;
	for (int i = 0; i < 6; i++)  {
		if (!testCharacter(i, 0x0d))
			continue;
		if (d >= 0) {
			int s1 = _characters[i].strengthCur + _characters[i].strengthExtCur;
			int s2 = _characters[d].strengthCur + _characters[d].strengthExtCur;
			if (s1 >= s2)
				d = i;
		} else {
			d = i;
		}
	}

	if (d == -1) {
		_txt->printMessage(_pryDoorStrings[_flags.gameID == GI_EOB2 ? 1 : 0]);
		return 1;
	}

	static const uint8 forceDoorChanceTable[] = { 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10, 11, 12, 13 };
	int s = _characters[d].strengthCur > 18 ? 18 : _characters[d].strengthCur;

	if (rollDice(1, 20) < forceDoorChanceTable[s]) {
		_txt->printMessage(_pryDoorStrings[_flags.gameID == GI_EOB2 ? 2 : 1]);
		_levelBlockProperties[block].walls[direction] = _levelBlockProperties[block].walls[direction ^ 2] =
			(_levelBlockProperties[block].walls[direction] == (_flags.gameID == GI_EOB2 ? 51 : 30)) ? 8 : 18;
		openDoor(block);
	} else {
		_txt->printMessage(_pryDoorStrings[3]);
	}

	return 1;
}

int EobCoreEngine::clickedDoorNoPry(uint16 block, uint16 direction) {
	if (!posWithinRect(_mouseX, _mouseY, 40, 16, 136, 88) && (_clickedSpecialFlag == 0x40))
		return 0;

	if (!(_wllWallFlags[_levelBlockProperties[block].walls[direction]] & 0x20))
		return 0;
	_txt->printMessage(_pryDoorStrings[6]);
	return 1;
}

int EobCoreEngine::specialWallAction(int block, int direction) {
	direction ^= 2;
	uint8 type = _specialWallTypes[_levelBlockProperties[block].walls[direction]];
	if (!type || !(_clickedSpecialFlag & (((_levelBlockProperties[block].flags & 0xf8) >> 3) | 0xe0)))
		return 0;

	int res = 0;
	switch (type) {
	case 1:
		res = clickedDoorSwitch(block, direction);
		break;

	case 2:
	case 8:
		res = clickedWallShape(block, direction);
		break;

	case 3:
		res = clickedLeverOn(block, direction);
		break;

	case 4:
		res = clickedLeverOff(block, direction);
		break;

	case 5:
		res =  clickedDoorPry(block, direction);
		break;

	case 6:
		res = clickedDoorNoPry(block, direction);
		break;

	case 7:
	case 9:
		res = clickedWallOnlyScript(block);
		break;

	case 10:
		res = clickedNiche(block, direction);
		break;

	default:
		break;
	}

	_clickedSpecialFlag = 0;
	_sceneUpdateRequired = true;

	return res;
}

void EobCoreEngine::openDoor(int block) {
	openCloseDoor(block, 1);
}

void EobCoreEngine::closeDoor(int block) {
	if (block == _currentBlock || _levelBlockProperties[block].flags & 7)
		return;
	openCloseDoor(block, -1);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
