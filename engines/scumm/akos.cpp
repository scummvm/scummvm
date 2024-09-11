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

#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/akos.h"
#include "scumm/bomp.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/sound_he.h"
#include "scumm/resource.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

#include "common/pack-start.h"	// START STRUCT PACKING

struct AkosHeader {
	uint16 versionNumber;
	uint16 costumeFlags;
	uint16 choreCount;
	uint16 celsCount;
	uint16 celCompressionCodec;
	uint16 layerCount;
} PACKED_STRUCT;

struct AkosOffset {
	uint32 akcd;	// offset into the akcd data
	uint16 akci;	// offset into the akci data
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


static bool akosCompare(int a, int b, uint cmd) {
	switch (cmd) {
	case AKC_IfVarEQJump:
	case AKC_IfVarEQDo:
		return a == b;
	case AKC_IfVarNEJump:
	case AKC_IfVarNEDo:
		return a != b;
	case AKC_IfVarLTJump:
	case AKC_IfVarLTDo:
		return a < b;
	case AKC_IfVarLEJump:
	case AKC_IfVarLEDo:
		return a <= b;
	case AKC_IfVarGTJump:
	case AKC_IfVarGTDo:
		return a > b;
	case AKC_IfVarGEJump:
	case AKC_IfVarGEDo:
		return a >= b;
	default:
		return false;
	}
}

void AkosCostumeLoader::loadCostume(int id) {
	_akos = _vm->getResourceAddress(rtCostume, id);
	assert(_akos);
}

bool AkosCostumeLoader::hasManyDirections() {
	const AkosHeader *akhd;

	akhd = (const AkosHeader *)_vm->findResourceData(MKTAG('A','K','H','D'), _akos);
	return (READ_LE_UINT16(&akhd->costumeFlags) & 2) != 0;
}

void AkosCostumeLoader::costumeDecodeData(Actor *a, int frame, uint useMask) {
	uint anim;
	const byte *r;
	const AkosHeader *akhd;
	uint offs;
	int i;
	byte code;
	uint16 start, len;
	uint16 mask;

	if (a->_costume == 0)
		return;

	loadCostume(a->_costume);

	if (_vm->_game.version >= 7 && hasManyDirections())
		anim = toSimpleDir(1, a->getFacing()) + frame * 8;
	else
		anim = newDirToOldDir(a->getFacing()) + frame * 4;

	akhd = (const AkosHeader *)_vm->findResourceData(MKTAG('A','K','H','D'), _akos);

	if (anim >= READ_LE_UINT16(&akhd->choreCount))
		return;

	r = _vm->findResourceData(MKTAG('A','K','C','H'), _akos);
	assert(r);

	offs = READ_LE_UINT16(r + anim * sizeof(uint16));
	if (offs == 0)
		return;
	r += offs;

	const uint8 *akstPtr = _vm->findResourceData(MKTAG('A','K','S','T'), _akos);
	const uint8 *aksfPtr = _vm->findResourceData(MKTAG('A','K','S','F'), _akos);

	i = 0;
	mask = READ_LE_UINT16(r); r += 2;
	do {
		if (mask & AKC_ExtendWordBit) {
			const uint8 *akst = akstPtr;
			const uint8 *aksf = aksfPtr;

			code = *r++;
			if (useMask & AKC_ExtendWordBit) {
				switch (code) {
				case 1:
					a->_cost.animType[i] = AKAT_Empty;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = 0;
					a->_cost.start[i] = 0;
					a->_cost.curpos[i] = 0;
					a->_cost.heCondMaskTable[i] = 0;

					if (akst) {
						int size = _vm->getResourceDataSize(akst) / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT32(akst) == 0) {
									a->_cost.heCondMaskTable[i] = READ_LE_UINT32(akst + 4);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}
					break;
				case 4:
					a->_cost.stopped |= 1 << i;
					break;
				case 5:
					a->_cost.stopped &= ~(1 << i);
					break;
				default:
					start = READ_LE_UINT16(r); r += 2;
					len = READ_LE_UINT16(r); r += 2;

					a->_cost.heJumpOffsetTable[i] = 0;
					a->_cost.heJumpCountTable[i] = 0;
					if (aksf) {
						int size = _vm->getResourceDataSize(aksf) / 6;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT16(aksf) == start) {
									a->_cost.heJumpOffsetTable[i] = READ_LE_UINT16(aksf + 2);
									a->_cost.heJumpCountTable[i] = READ_LE_UINT16(aksf + 4);
									found = true;
									break;
								}
								aksf += 6;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}

					a->_cost.animType[i] = code;
					a->_cost.frame[i] = frame;
					a->_cost.end[i] = start + len;
					a->_cost.start[i] = start;
					a->_cost.curpos[i] = start;
					a->_cost.heCondMaskTable[i] = 0;
					if (akst) {
						int size = _vm->getResourceDataSize(akst) / 8;
						if (size > 0) {
							bool found = false;
							while (size--) {
								if (READ_LE_UINT32(akst) == start) {
									a->_cost.heCondMaskTable[i] = READ_LE_UINT32(akst + 4);
									found = true;
									break;
								}
								akst += 8;
							}
							if (!found) {
								error("Sequence not found in actor %p costume %d", (void *)a, a->_costume);
							}
						}
					}
					break;
				}
			} else {
				if (code != 1 && code != 4 && code != 5)
					r += sizeof(uint16) * 2;
			}
		}
		i++;
		mask <<= 1;
		useMask <<= 1;
	} while ((uint16)mask);
}

void AkosRenderer::setPalette(uint16 *new_palette) {
	uint size, i;

	size = _vm->getResourceDataSize(_akpl);
	if (size == 0)
		return;

	if (size > 256)
		error("akos_setPalette: %d is too many colors", size);

	if (_vm->_game.features & GF_16BIT_COLOR) {
		if (_paletteNum) {
			for (i = 0; i < size; i++)
				_palette[i] = READ_LE_UINT16(_vm->_hePalettes + _paletteNum * _vm->_hePaletteSlot + 768 + _akpl[i] * 2);
		} else if (_rgbs) {
			for (i = 0; i < size; i++) {
				if (new_palette[i] == 0xFF) {
					uint8 col = _akpl[i];
					_palette[i] = _vm->get16BitColor(_rgbs[col * 3 + 0], _rgbs[col * 3 + 1], _rgbs[col * 3 + 2]);
				} else {
					_palette[i] = new_palette[i];
				}
			}
		}
	} else if (_vm->_game.heversion >= 99 && _paletteNum) {
		for (i = 0; i < size; i++)
			_palette[i] = (byte)_vm->_hePalettes[_paletteNum * _vm->_hePaletteSlot + 768 + _akpl[i]];
	} else {
		for (i = 0; i < size; i++) {
			_palette[i] = new_palette[i] != 0xFF ? new_palette[i] : _akpl[i];
		}
	}

	if (_vm->_game.heversion == 70) {
		for (i = 0; i < size; i++)
			_palette[i] = _vm->_HEV7ActorPalette[_palette[i]];
	}

	if (size == 256) {
		byte color = new_palette[0];
		if (color == 255) {
			_palette[0] = color;
		} else {
			_useBompPalette = true;
		}
	}
}

void AkosRenderer::setCostume(int costume, int shadow) {
	const byte *akos = _vm->getResourceAddress(rtCostume, costume);
	assert(akos);

	_akhd = (const AkosHeader *)_vm->findResourceData(MKTAG('A','K','H','D'), akos);
	_akof = (const AkosOffset *)_vm->findResourceData(MKTAG('A','K','O','F'), akos);
	_akci = _vm->findResourceData(MKTAG('A','K','C','I'), akos);
	_aksq = _vm->findResourceData(MKTAG('A','K','S','Q'), akos);
	_akcd = _vm->findResourceData(MKTAG('A','K','C','D'), akos);
	_akpl = _vm->findResourceData(MKTAG('A','K','P','L'), akos);
	_codec = READ_LE_UINT16(&_akhd->celCompressionCodec);
	_akct = _vm->findResourceData(MKTAG('A','K','C','T'), akos);
	_rgbs = _vm->findResourceData(MKTAG('R','G','B','S'), akos);

	_xmap = nullptr;
	if (shadow) {
		const uint8 *xmapPtr = _vm->getResourceAddress(rtImage, shadow);
		assert(xmapPtr);
		_xmap = _vm->findResourceData(MKTAG('X','M','A','P'), xmapPtr);
		assert(_xmap);
	}
}

void AkosRenderer::setFacing(const Actor *a) {
	_mirror = (newDirToOldDir(a->getFacing()) != 0 || READ_LE_UINT16(&_akhd->costumeFlags) & 1);
	if (a->_flip)
		_mirror = !_mirror;
}

byte AkosRenderer::drawLimb(const Actor *a, int limb) {
	uint code;
	const byte *p;
	const AkosOffset *off;
	const CostumeData &cost = a->_cost;
	const CostumeInfo *costumeInfo;
	uint i, extra;
	byte result = 0;
	int xMoveCur, yMoveCur;
	uint32 sequenceLayerIndirection[32];
	bool useConditionalTable = false;
	int lastDx, lastDy;

	lastDx = lastDy = 0;
	for (i = 0; i < 32; ++i) {
		sequenceLayerIndirection[i] = i;
	}

	if (_skipLimbs)
		return 0;

	if (_vm->_game.heversion >= 70 && cost.animType[limb] == AKAT_DeltaAnim)
		return 0;

	if (cost.animType[limb] == AKAT_Empty || cost.stopped & (1 << limb))
		return 0;

	p = _aksq + cost.curpos[limb];

	code = p[0];
	if (code & AKC_ExtendBit)
		code = READ_BE_UINT16(p);

	if (_vm->_game.heversion >= 90)
		_shadowMode = 0;

	if (code == AKC_CondDrawMany || code == AKC_CondRelativeOffsetDrawMany) {
		uint16 s = cost.curpos[limb] + 4;
		uint j = 0;
		extra = p[3];
		uint8 n = extra;
		assert(n <= ARRAYSIZE(sequenceLayerIndirection));
		while (n--) {
			sequenceLayerIndirection[j++] = _aksq[s++];
		}
		useConditionalTable = true;
		p += extra + 2;
		code = (code == AKC_CondDrawMany) ? AKC_DrawMany : AKC_RelativeOffsetDrawMany;
	}

	if (code == AKC_EmptyCel || code == AKC_EndSeq)
		return 0;

	if (code != AKC_DrawMany && code != AKC_RelativeOffsetDrawMany) {
		off = _akof + (code & AKC_CelMask);

		assert((code & AKC_CelMask) * 6 < READ_BE_UINT32((const byte *)_akof - 4) - 8);
		assert((code & 0x7000) == 0);

		_srcPtr = _akcd + READ_LE_UINT32(&off->akcd);
		costumeInfo = (const CostumeInfo *) (_akci + READ_LE_UINT16(&off->akci));

		_width = READ_LE_UINT16(&costumeInfo->width);
		_height = READ_LE_UINT16(&costumeInfo->height);
		xMoveCur = _xMove + (int16)READ_LE_UINT16(&costumeInfo->relX);
		yMoveCur = _yMove + (int16)READ_LE_UINT16(&costumeInfo->relY);
		_xMove += (int16)READ_LE_UINT16(&costumeInfo->moveX);
		_yMove -= (int16)READ_LE_UINT16(&costumeInfo->moveY);

		switch (_codec) {
		case AKOS_BYLE_RLE_CODEC:
			result |= paintCelByleRLE(xMoveCur, yMoveCur);
			break;
		case AKOS_CDAT_RLE_CODEC:
			result |= paintCelCDATRLE(xMoveCur, yMoveCur);
			break;
		case AKOS_RUN_MAJMIN_CODEC:
			result |= paintCelMajMin(xMoveCur, yMoveCur);
			break;
		default:
			error("akos_drawLimb: invalid _codec %d", _codec);
		}
	} else {
		if (code == AKC_RelativeOffsetDrawMany)  {
			lastDx = (int16)READ_LE_UINT16(p + 2);
			lastDy = (int16)READ_LE_UINT16(p + 4);
			p += 4;
		}

		extra = p[2];
		p += 3;
		uint32 decFlag = sequenceLayerIndirection[0];

		for (i = 0; i != extra; i++) {
			code = p[4];
			if (code & AKC_ExtendBit)
				code = READ_BE_UINT16(p + 4);
			off = _akof + (code & 0xFFF);

			_srcPtr = _akcd + READ_LE_UINT32(&off->akcd);
			costumeInfo = (const CostumeInfo *) (_akci + READ_LE_UINT16(&off->akci));

			_width = READ_LE_UINT16(&costumeInfo->width);
			_height = READ_LE_UINT16(&costumeInfo->height);

			xMoveCur = _xMove + (int16)READ_LE_UINT16(p + 0);
			yMoveCur = _yMove + (int16)READ_LE_UINT16(p + 2);

			// WORKAROUND bug #13532: There is a frame of Freddi's eye (US release of Freddi 3) accidentally being drawn
			// big with a horizontal line at the bottom, causing this line to appear at the bottom of the screen.
			// We draw the whole frame one pixel down so it does not appear on screen.
			if (_vm->_game.id == GID_FREDDI3 && _vm->_language == Common::EN_USA && a->_costume == 258 && (code & AKC_CelMask) == 35 && _vm->enhancementEnabled(kEnhVisualChanges))
				_clipOverride.bottom -= 2;

			if (i == extra - 1) {
				_xMove += lastDx;
				_yMove -= lastDy;
			}

			uint16 shadowMask = 0;

			if (!useConditionalTable || !_akct) {
				decFlag = 1;
			} else {
				uint32 cond = READ_LE_UINT32(_akct + cost.heCondMaskTable[limb] + sequenceLayerIndirection[i] * 4);
				if (cond == 0) {
					decFlag = 1;
				} else {
					uint32 type = cond & ~0x3FFFFFFF;
					cond &= 0x3FFFFFFF;
					if (_vm->_game.heversion >= 90) {
						shadowMask = cond & 0xE000;
						cond &= ~0xE000;
					}
					if (_vm->_game.heversion >= 90 && cond == 0) {
						decFlag = 1;
					} else if (type == 0x40000000) { // restored_bit
						decFlag = (a->_heCondMask & cond) == cond ? 1 : 0;
					} else if (type == 0x80000000) { // dirty_bit
						decFlag = (a->_heCondMask & cond) ? 0 : 1;
					} else {
						decFlag = (a->_heCondMask & cond) ? 1 : 0;
					}
				}
			}

			p += (p[4] & AKC_ExtendBit) ? 6 : 5;

			if (decFlag == 0)
				continue;

			if (_vm->_game.heversion >= 90) {
				if (_vm->_game.heversion >= 99)
					_shadowMode = 0;
				if (_xmap && (shadowMask & AKC_ExtendWordBit))
					_shadowMode = 3;
			}

			switch (_codec) {
			case AKOS_BYLE_RLE_CODEC:
				result |= paintCelByleRLE(xMoveCur, yMoveCur);
				break;
			case AKOS_CDAT_RLE_CODEC:
				result |= paintCelCDATRLE(xMoveCur, yMoveCur);
				break;
			case AKOS_RUN_MAJMIN_CODEC:
				result |= paintCelMajMin(xMoveCur, yMoveCur);
				break;
			case AKOS_TRLE_CODEC:
				result |= paintCelTRLE(a->_number, a->_drawToBackBuf, xMoveCur, yMoveCur, _width, _height, _akpl[0], _xmap, 0);
				break;
			default:
				error("akos_drawLimb: invalid _codec %d", _codec);
			}
		}
	}

	return result;
}

void AkosRenderer::byleRLEDecode(ByleRLEData &dataBlock) {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	int lastColumnX, y;
	uint16 color, height, pcolor;
	const byte *scaleytab;
	bool masked;

	lastColumnX = -1;
	y = dataBlock.y;
	src = _srcPtr;
	dst = dataBlock.destPtr;
	len = dataBlock.repLen;
	color = dataBlock.repColor;
	height = _height;

	scaleytab = &dataBlock.scaleTable[MAX<int>(0, dataBlock.scaleYIndex)]; // Avoid invalid mem reads in Basketball...
	maskbit = revBitMask(dataBlock.x & 7);
	mask = _vm->getMaskBuffer(dataBlock.x - (_vm->_virtscr[kMainVirtScreen].xstart & 7), dataBlock.y, _zbuf);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> dataBlock.shr;
		len &= dataBlock.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || *scaleytab++ < _scaleY) {
				if (_actorHitMode) {
					if (color && y == _actorHitY && dataBlock.x == _actorHitX) {
						_actorHitResult = true;
						return;
					}
				} else {
					masked = (y < dataBlock.boundsRect.top || y >= dataBlock.boundsRect.bottom) || (dataBlock.x < 0 || dataBlock.x >= dataBlock.boundsRect.right) || (*mask & maskbit);
					bool skipColumn = false;

					if (color && !masked) {
						pcolor = _palette[color];
						if (_shadowMode == 1) {
							if (pcolor == 13) {
								// In shadow mode 1 skipColumn works more or less the same way as in shadow
								// mode 3. It is only ever checked and applied if pcolor is 13.
								skipColumn = (lastColumnX == dataBlock.x);
								pcolor = _shadowTable[*dst];
							}
						} else if (_shadowMode == 2) {
							error("AkosRenderer::byleRLEDecode(): shadowMode 2 not implemented."); // TODO
						} else if (_shadowMode == 3) {
							if (_vm->_game.features & GF_16BIT_COLOR) {
								// I add the column skip here, too, although I don't know whether it always
								// applies. But this is the only way to prevent recursive shading of pixels.
								// This might need more fine tuning...
								skipColumn = (lastColumnX == dataBlock.x);
								uint16 srcColor = (pcolor >> 1) & 0x7DEF;
								uint16 dstColor = (READ_UINT16(dst) >> 1) & 0x7DEF;
								pcolor = srcColor + dstColor;
							} else if (_vm->_game.heversion >= 90) {
								// I add the column skip here, too, although I don't know whether it always
								// applies. But this is the only way to prevent recursive shading of pixels.
								// This might need more fine tuning...
								skipColumn = (lastColumnX == dataBlock.x);
								pcolor = (pcolor << 8) + *dst;
								pcolor = _xmap[pcolor];
							} else if (pcolor < 8) {
								// This mode is used in COMI. The column skip only takes place when the shading
								// is actually applied (for pcolor < 8). The skip avoids shading of pixels that
								// already have been shaded.
								skipColumn = (lastColumnX == dataBlock.x);
								pcolor = (pcolor << 8) + *dst;
								pcolor = _shadowTable[pcolor];
							}
						}
						if (!skipColumn) {
							if (_vm->_bytesPerPixel == 2) {
								WRITE_UINT16(dst, pcolor);
							} else {
								*dst = pcolor;
							}
						}
					}
				}
				dst += _out.pitch;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--dataBlock.skipWidth)
					return;
				height = _height;
				y = dataBlock.y;

				scaleytab = &dataBlock.scaleTable[MAX<int>(0, dataBlock.scaleYIndex)]; // Avoid invalid mem reads in Basketball...
				lastColumnX = dataBlock.x;

				if (_scaleX == 255 || dataBlock.scaleTable[dataBlock.scaleXIndex] < _scaleX) {
					dataBlock.x += dataBlock.scaleXStep;
					if (dataBlock.x < 0 || dataBlock.x >= dataBlock.boundsRect.right)
						return;
					maskbit = revBitMask(dataBlock.x & 7);
					dataBlock.destPtr += dataBlock.scaleXStep * _vm->_bytesPerPixel;
				}

				dataBlock.scaleXIndex += dataBlock.scaleXStep;
				dst = dataBlock.destPtr;
				mask = _vm->getMaskBuffer(dataBlock.x - (_vm->_virtscr[kMainVirtScreen].xstart & 7), dataBlock.y, _zbuf);
			}
		StartPos:;
		} while (--len);
	} while (true);
}

const byte bigCostumeScaleTable[768] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFE,

	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

byte AkosRenderer::paintCelByleRLE(int xMoveCur, int yMoveCur) {
	int num_colors;
	bool actorIsScaled;
	int i, j;
	int linesToSkip = 0, startScaleIndexX, startScaleIndexY;
	Common::Rect rect;
	int step;
	byte drawFlag = 1;
	ByleRLEData compData;

	const int scaletableSize = (_vm->_game.heversion >= 61) ? 128 : 384;

	/* implement custom scale table */

	compData.scaleTable = (_vm->_game.heversion >= 61) ? smallCostumeScaleTable : bigCostumeScaleTable;
	if (_vm->VAR_CUSTOMSCALETABLE != 0xFF && _vm->_res->isResourceLoaded(rtString, _vm->VAR(_vm->VAR_CUSTOMSCALETABLE))) {
		compData.scaleTable = _vm->getStringAddressVar(_vm->VAR_CUSTOMSCALETABLE);
	}

	// Setup color decoding variables
	num_colors = _vm->getResourceDataSize(_akpl);
	if (num_colors == 32) {
		compData.mask = 7;
		compData.shr = 3;
	} else if (num_colors == 64) {
		compData.mask = 3;
		compData.shr = 2;
	} else {
		compData.mask = 15;
		compData.shr = 4;
	}

	actorIsScaled = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	compData.x = _actorX;
	compData.y = _actorY;

	compData.boundsRect.left = 0;
	compData.boundsRect.top = 0;
	compData.boundsRect.right = _out.w;
	compData.boundsRect.bottom = _out.h;

	if (actorIsScaled) {

		/* Scale direction */
		compData.scaleXStep = -1;
		if (xMoveCur < 0) {
			xMoveCur = -xMoveCur;
			compData.scaleXStep = 1;
		}

		if (_mirror) {
			/* Adjust X position */
			startScaleIndexX = j = scaletableSize - xMoveCur;
			for (i = 0; i < xMoveCur; i++) {
				if (compData.scaleTable[j++] < _scaleX)
					compData.x -= compData.scaleXStep;
			}

			rect.left = rect.right = compData.x;

			j = startScaleIndexX;
			for (i = 0, linesToSkip = 0; i < _width; i++) {
				if (rect.right < 0) {
					linesToSkip++;
					startScaleIndexX = j;
				}
				if (compData.scaleTable[j++] < _scaleX)
					rect.right++;
			}
		} else {
			/* No mirror */
			/* Adjust X position */
			startScaleIndexX = j = scaletableSize + xMoveCur;
			for (i = 0; i < xMoveCur; i++) {
				if (compData.scaleTable[j--] < _scaleX)
					compData.x += compData.scaleXStep;
			}

			rect.left = rect.right = compData.x;

			j = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (rect.left >= compData.boundsRect.right) {
					startScaleIndexX = j;
					linesToSkip++;
				}
				if (compData.scaleTable[j--] < _scaleX)
					rect.left--;
			}
		}

		if (linesToSkip)
			linesToSkip--;

		step = -1;
		if (yMoveCur < 0) {
			yMoveCur = -yMoveCur;
			step = -step;
		}

		startScaleIndexY = scaletableSize - yMoveCur;
		for (i = 0; i < yMoveCur; i++) {
			// WORKAROUND: Backyard Basketball sends out yMoveCur values higher than 128!
			// This triggers ASAN, because it tries to reach a negative index of compData.scaleTable[].
			if (startScaleIndexY < 0) {
				debug(8, "AkosRenderer::paintCelByleRLE(): Negative startScaleIndexY: %d; actor (%d), scaletableSize (%d), yMoveCur (%d), working around it...",
					_actorID, startScaleIndexY, scaletableSize, yMoveCur);
				if (compData.scaleTable[0] < _scaleY)
					compData.y -= step;

				startScaleIndexY++;
				continue;
			}

			if (compData.scaleTable[startScaleIndexY++] < _scaleY)
				compData.y -= step;
		}

		rect.top = rect.bottom = compData.y;
		startScaleIndexY = scaletableSize - yMoveCur;
		for (i = 0; i < _height; i++) {
			// WORKAROUND: See above...
			if (startScaleIndexY < 0) {
				if (compData.scaleTable[0] < _scaleY)
					rect.bottom++;

				startScaleIndexY++;
				continue;
			}

			if (compData.scaleTable[startScaleIndexY++] < _scaleY)
				rect.bottom++;
		}

		startScaleIndexY = scaletableSize - yMoveCur;
	} else {
		if (!_mirror)
			xMoveCur = -xMoveCur;

		compData.x += xMoveCur;
		compData.y += yMoveCur;

		if (_mirror) {
			rect.left = compData.x;
			rect.right = compData.x + _width;
		} else {
			rect.left = compData.x - _width;
			rect.right = compData.x;
		}

		rect.top = compData.y;
		rect.bottom = rect.top + _height;

		startScaleIndexX = scaletableSize;
		startScaleIndexY = scaletableSize;
	}

	compData.scaleXIndex = startScaleIndexX;
	compData.scaleYIndex = startScaleIndexY;
	compData.skipWidth = _width;
	compData.scaleXStep = _mirror ? 1 : -1;

	if (_vm->_game.heversion >= 71) {
		if (_clipOverride.right > _clipOverride.left && _clipOverride.bottom > _clipOverride.top) {
			compData.boundsRect = _clipOverride;
			compData.boundsRect.right += 1;
			compData.boundsRect.bottom += 1;
		}
	}

	if (_actorHitMode) {
		if (_actorHitX < rect.left || _actorHitX >= rect.right || _actorHitY < rect.top || _actorHitY >= rect.bottom)
			return 0;
	} else {
		markRectAsDirty(rect);

		if (_vm->_game.heversion >= 71) {
			ActorHE *a = (ActorHE *)_vm->derefActor(_actorID, "paintCelByleRLE");
			a->setActorUpdateArea(rect.left, rect.top, rect.right, rect.bottom + 1);
		}
	}


	if (rect.top >= compData.boundsRect.bottom || rect.bottom <= compData.boundsRect.top)
		return 0;

	if (rect.left >= compData.boundsRect.right || rect.right <= compData.boundsRect.left)
		return 0;

	compData.repLen = 0;

	if (_mirror) {
		if (!actorIsScaled)
			linesToSkip = compData.boundsRect.left - compData.x;

		if (linesToSkip > 0) {
			compData.skipWidth -= linesToSkip;
			skipCelLines(compData, linesToSkip);
			compData.x = compData.boundsRect.left;
		} else {
			linesToSkip = rect.right - compData.boundsRect.right;
			if (linesToSkip <= 0) {
				drawFlag = 2;
			} else {
				compData.skipWidth -= linesToSkip;
			}
		}
	} else {
		if (!actorIsScaled)
			linesToSkip = rect.right - compData.boundsRect.right + 1;
		if (linesToSkip > 0) {
			compData.skipWidth -= linesToSkip;
			skipCelLines(compData, linesToSkip)	;
			compData.x = compData.boundsRect.right - 1;
		} else {
			linesToSkip = (compData.boundsRect.left -1) - rect.left;

			if (linesToSkip <= 0)
				drawFlag = 2;
			else
				compData.skipWidth -= linesToSkip;
		}
	}

	if (compData.skipWidth <= 0 || _height <= 0)
		return 0;

	if (rect.left < compData.boundsRect.left)
		rect.left = compData.boundsRect.left;

	if (rect.top < compData.boundsRect.top)
		rect.top = compData.boundsRect.top;

	if (rect.top > compData.boundsRect.bottom)
		rect.top = compData.boundsRect.bottom;

	if (rect.bottom > compData.boundsRect.bottom)
		rect.bottom = compData.boundsRect.bottom;

	if (_drawTop > rect.top)
		_drawTop = rect.top;
	if (_drawBottom < rect.bottom)
		_drawBottom = rect.bottom;

	compData.width = _out.w;
	compData.height = _out.h;
	compData.destPtr = (byte *)_out.getBasePtr(compData.x, compData.y);

	byleRLEDecode(compData);

	return drawFlag;
}

void AkosRenderer::markRectAsDirty(Common::Rect rect) {
	rect.left -= _vm->_virtscr[kMainVirtScreen].xstart & 7;
	rect.right -= _vm->_virtscr[kMainVirtScreen].xstart & 7;
	_vm->markRectAsDirty(kMainVirtScreen, rect, _actorID);
}

byte AkosRenderer::paintCelCDATRLE(int xmoveCur, int ymoveCur) {
	Common::Rect clip;
	int32 maxw, maxh;

	if (_actorHitMode) {
		error("paintCelCDATRLE: _actorHitMode not yet implemented");
		return 0;
	}

	if (!_mirror) {
		clip.left = (_actorX - xmoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xmoveCur - 1;
	}

	clip.top = _actorY + ymoveCur;
	clip.right = clip.left + _width;
	clip.bottom = clip.top + _height;
	maxw = _out.w;
	maxh = _out.h;

	markRectAsDirty(clip);

	clip.clip(maxw, maxh);

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_drawTop > clip.top)
		_drawTop = clip.top;
	if (_drawBottom < clip.bottom)
		_drawBottom = clip.bottom;

	BompDrawData bdd;

	bdd.dst = _out;
	if (!_mirror) {
		bdd.x = (_actorX - xmoveCur - _width) + 1;
	} else {
		bdd.x = _actorX + xmoveCur;
	}
	bdd.y = _actorY + ymoveCur;

	bdd.src = _srcPtr;
	bdd.srcwidth = _width;
	bdd.srcheight = _height;

	bdd.scale_x = 255;
	bdd.scale_y = 255;

	bdd.maskPtr = _vm->getMaskBuffer(0, 0, _zbuf);
	bdd.numStrips = _numStrips;

	bdd.shadowMode = _shadowMode;
	bdd.shadowPalette = _vm->_shadowPalette;

	bdd.actorPalette = _useBompPalette ? _palette : nullptr;

	bdd.mirror = !_mirror;

	drawBomp(bdd);

	_useBompPalette = false;

	return 0;
}

void AkosRenderer::majMinCodecDecompress(byte *dest, int32 pitch, const byte *src, int32 width, int32 height, int32 dir,
		int32 numSkipBefore, int32 numSkipAfter, byte transparency, int maskLeft, int maskTop, int zBuf) {

	MajMinCodec majMin;
	byte *tmpBuf = majMin._majMinData.buffer;
	int maskPitch;
	byte *maskPtr;
	const byte maskBit = revBitMask(maskLeft & 7);

	if (dir < 0) {
		dest -= (width - 1);
		tmpBuf += (width - 1);
	}

	majMin.setupBitReader(*src, src + 1);

	if (numSkipBefore != 0) {
		majMin.skipData(numSkipBefore);
	}

	maskPitch = _numStrips;

	maskPtr = _vm->getMaskBuffer(maskLeft, maskTop, zBuf);

	assert(height > 0);
	assert(width > 0);
	while (height--) {
		majMin.decodeLine(tmpBuf, width, dir);
		bompApplyMask(majMin._majMinData.buffer, maskPtr, maskBit, width, transparency);
		bool HE7Check = (_vm->_game.heversion == 70);
		bompApplyShadow(_shadowMode, _shadowTable, majMin._majMinData.buffer, dest, width, transparency, HE7Check);

		if (numSkipAfter != 0)	{
			majMin.skipData(numSkipAfter);
		}
		dest += pitch;
		maskPtr += maskPitch;
	}
}

byte AkosRenderer::paintCelMajMin(int xMoveCur, int yMoveCur) {
	assert(_vm->_bytesPerPixel == 1);

	Common::Rect clip;
	int32 minX, minY, maxW, maxH;
	int32 skipX, skipY, curX, curY;
	byte transparency = (_vm->_game.heversion >= 61) ? _palette[0] : 255;

	if (_actorHitMode) {
		error("paintCelMajMin: _actorHitMode not yet implemented");
		return 0;
	}

	if (!_mirror) {
		clip.left = (_actorX - xMoveCur - _width) + 1;
	} else {
		clip.left = _actorX + xMoveCur;
	}

	clip.top = _actorY + yMoveCur;
	clip.right = clip.left + _width;
	clip.bottom = clip.top + _height;

	minX = minY = 0;
	maxW = _out.w;
	maxH = _out.h;

	if (_vm->_game.heversion >= 71) {
		if (_clipOverride.right > _clipOverride.left && _clipOverride.bottom > _clipOverride.top) {
			minX = _clipOverride.left;
			minY = _clipOverride.top;
			maxW = _clipOverride.right;
			maxH = _clipOverride.bottom;
		}
	}

	markRectAsDirty(clip);

	if (_vm->_game.heversion >= 71) {
		ActorHE *a = (ActorHE *)_vm->derefActor(_actorID, "paintCelMajMin");
		a->setActorUpdateArea(clip.left, clip.top, clip.right, clip.bottom);
	}

	skipX = 0;
	skipY = 0;
	curX = _width - 1;
	curY = _height - 1;

	if (clip.left < minX) {
		skipX = -clip.left;
		clip.left = 0;
	}

	if (clip.right > maxW) {
		curX -= clip.right - maxW;
		clip.right = maxW;
	}

	if (clip.top < minY) {
		skipY -= clip.top;
		clip.top = 0;
	}

	if (clip.bottom > maxH) {
		curY -= clip.bottom - maxH;
		clip.bottom = maxH;
	}

	if ((clip.left >= clip.right) || (clip.top >= clip.bottom))
		return 0;

	if (_drawTop > clip.top)
		_drawTop = clip.top;
	if (_drawBottom < clip.bottom)
		_drawBottom = clip.bottom;

	int32 widthUnk, heightUnk;

	heightUnk = clip.top;
	int32 dir;

	if (!_mirror) {
		dir = -1;

		int tmpSkipX = skipX;
		skipX = _width - 1 - curX;
		curX = _width - 1 - tmpSkipX;
		widthUnk = clip.right - 1;
	} else {
		dir = 1;
		widthUnk = clip.left;
	}

	int32 outHeight;

	outHeight = curY - skipY;
	if (outHeight < 0) {
		outHeight = -outHeight;
	}
	outHeight++;

	curX -= skipX;
	if (curX < 0) {
		curX = -curX;
	}
	curX++;

	int32 numSkipBefore = skipX + (skipY * _width);
	int32 numSkipAfter = _width - curX;

	byte *dst = (byte *)_out.getBasePtr(widthUnk, heightUnk);

	majMinCodecDecompress(dst, _out.pitch, _srcPtr, curX, outHeight, dir, numSkipBefore, numSkipAfter, transparency, clip.left, clip.top, _zbuf);
	return 0;
}

#ifdef ENABLE_HE
byte AkosRenderer::hePaintCel(int actor, int drawToBack, int celX, int celY, int celWidth, int celHeight, byte tcolor, bool allowFlip, const byte *shadowTablePtr,
							  void (*drawPtr)(ScummEngine *vm, Wiz *wiz, WizRawPixel *, int, int, Common::Rect *, const byte *, int, int, Common::Rect *, byte, const byte *shadowTablePtr, const WizRawPixel *conversionTable, int32 specialRenderFlags),
							  const WizRawPixel *conversionTable, int32 specialRenderFlags) {

	int plotX, plotY;
	bool xFlipFlag;
	Common::Rect destRect;
	Common::Rect sourceRect;
	Common::Rect clipRect;
	int destBufferWidth, destBufferHeight;
	WizRawPixel *destBuffer;
	Wiz *wiz = ((ScummEngine_v71he *)_vm)->_wiz;
	Actor *a = _vm->derefActor(actor, "hePaintCel");

	if (allowFlip) {
		xFlipFlag = _mirror;
	} else {
		xFlipFlag = false;
	}

	// Find cel's "plot" position with flipping etc...
	plotY = (int32)_actorY + (int32)celY;
	plotX = (xFlipFlag) ? (_actorX - celX - celWidth + 1) : (_actorX + celX);

	// Find which buffer to plot into. back or forground (STAMP ACTOR)...
	VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
	destBufferWidth = pvs->w;
	destBufferHeight = pvs->h;

	if (drawToBack) {
		destBuffer = (WizRawPixel *)pvs->getBackPixels(0, 0);
	} else {
		destBuffer = (WizRawPixel *)pvs->getPixels(0, 0);
	}

	// Setup clipping rectangle(s)...
	wiz->makeSizedRect(&sourceRect, celWidth, celHeight);
	wiz->makeSizedRect(&clipRect, destBufferWidth, destBufferHeight);
	wiz->makeSizedRectAt(&destRect, plotX, plotY, celWidth, celHeight);

	// Check to see if the actor has a clipping rect...
	if ((((ActorHE *)a)->_clipOverride.left < ((ActorHE *)a)->_clipOverride.right) &&
		(((ActorHE *)a)->_clipOverride.top < ((ActorHE *)a)->_clipOverride.bottom)) {
		// The original evaluates this as '>', but for some reason we need actor clipping
		// rectangles to max out at 640x480, instead of 639x479; this *should* not be an issue...
		if (destBufferHeight >= ((ActorHE *)a)->_clipOverride.bottom) {
			clipRect.left = ((ActorHE *)a)->_clipOverride.left;
			clipRect.right = ((ActorHE *)a)->_clipOverride.right;
			clipRect.top = ((ActorHE *)a)->_clipOverride.top;
			clipRect.bottom = ((ActorHE *)a)->_clipOverride.bottom;
		} else {
			warning(
				"AkosRenderer::hePaintCel(): Actor %d invalid clipping rect (%-3d,%3d,%-3d,%3d)", a->_number,
				((ActorHE *)a)->_clipOverride.left, ((ActorHE *)a)->_clipOverride.top,
				((ActorHE *)a)->_clipOverride.right, ((ActorHE *)a)->_clipOverride.bottom);
		}
	}

	// Clip the coords...
	wiz->clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (wiz->isRectValid(destRect)) {
		// Mark the dest area as the changed section...
		_vm->markRectAsDirty(kMainVirtScreen, destRect, actor);

		if (destRect.top < a->_top)
			a->_top = destRect.top;
		if (destRect.bottom > a->_bottom)
			a->_bottom = destRect.bottom + 1;

		_drawTop = a->_top;
		_drawBottom = a->_bottom;

		if (_vm->_game.heversion >= 71) {
			((ActorHE *)a)->setActorUpdateArea(destRect.left, destRect.top, destRect.right, destRect.bottom);
		}

		// Get final plot point and flip source coords if necessary...
		if (xFlipFlag) {
			wiz->swapRectX(&sourceRect);
			sourceRect.left = (celWidth - 1) - sourceRect.left;
			sourceRect.right = (celWidth - 1) - sourceRect.right;
		}

		// Call the decompression routine...
		if (drawPtr) {
			(*drawPtr)(_vm, wiz,
					   destBuffer, destBufferWidth, destBufferHeight, &destRect,
					   _srcPtr, celWidth, celHeight, &sourceRect, tcolor,
					   shadowTablePtr, conversionTable, specialRenderFlags);
		}

		return 2;
	}

	return 0;
}

static void heTRLEPaintPrim(ScummEngine *vm, Wiz *wiz, WizRawPixel *dstDataPtr, int dstWid, int dstHei, Common::Rect *dstRect, const byte *srcDataPtr, int srcWid, int srcHei, Common::Rect *srcRect, byte tColor, const byte *shadowTablePtr, const WizRawPixel *conversionTable, int32 specialRenderFlags) {
	if (vm->_game.heversion > 99) {
		int plotX, plotY;

		// Convert incoming src rect to flags and adjust dest position for sub rect
		int32 additionalRenderFlags = 0;

		if (srcRect->left < srcRect->right) {
			plotX = dstRect->left - srcRect->left;
		} else {
			plotX = dstRect->left - srcRect->right;
			additionalRenderFlags |= kWRFHFlip;
		}

		if (srcRect->top < srcRect->bottom) {
			plotY = dstRect->top - srcRect->top;
		} else {
			plotY = dstRect->top - srcRect->bottom;
			additionalRenderFlags |= kWRFVFlip;
		}

		// Finally call the drawing primitive
		wiz->trleFLIPDecompressImage(
			dstDataPtr, srcDataPtr, dstWid, dstHei,
			plotX, plotY, srcWid, srcHei, dstRect,
			(specialRenderFlags & kWRFSpecialRenderBitMask) | additionalRenderFlags,
			nullptr, conversionTable, nullptr);

	} else {
		wiz->auxDecompTRLEPrim(
			dstDataPtr, dstWid, dstRect, srcDataPtr, srcRect, conversionTable);
	}
}

static void heTRLEPaintPrimShadow(ScummEngine *vm, Wiz *wiz, WizRawPixel *dstDataPtr, int dstWid, int dstHei, Common::Rect *dstRect, const byte *srcDataPtr, int srcWid, int srcHei, Common::Rect *srcRect, byte tColor, const byte *shadowTablePtr, const WizRawPixel *conversionTable, int32 specialRenderFlags) {
	wiz->auxDecompMixColorsTRLEPrim(
		dstDataPtr, dstWid, dstRect, srcDataPtr, srcRect, shadowTablePtr,
		conversionTable);
}
#endif

byte AkosRenderer::paintCelTRLE(int actor, int drawToBack, int celX, int celY, int celWidth, int celHeight, byte tcolor, const byte *shadowTablePtr, int32 specialRenderFlags) {
#ifdef ENABLE_HE
	const uint8 *palPtr = nullptr;
	if (_vm->_game.features & GF_16BIT_COLOR) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot + 768;
		if (_paletteNum) {
			palPtr = _vm->_hePalettes + _paletteNum * _vm->_hePaletteSlot + 768;
		} else if (_rgbs) {
			for (uint i = 0; i < 256; i++)
				WRITE_LE_UINT16(_palette + i, _vm->get16BitColor(_rgbs[i * 3 + 0], _rgbs[i * 3 + 1], _rgbs[i * 3 + 2]));
			palPtr = (uint8 *)_palette;
		}
	} else if (_vm->_game.heversion >= 99) {
		palPtr = _vm->_hePalettes + _vm->_hePaletteSlot + 768;
	}

	if (!shadowTablePtr) {
		return hePaintCel(
			actor, drawToBack, celX, celY, celWidth, celHeight, tcolor, false, nullptr,
			heTRLEPaintPrim,
			(const WizRawPixel *)palPtr,
			specialRenderFlags);
	} else {
		return hePaintCel(
			actor, drawToBack, celX, celY, celWidth, celHeight, tcolor, false, shadowTablePtr,
			heTRLEPaintPrimShadow,
			(const WizRawPixel *)palPtr,
			specialRenderFlags);
	}
#endif
	return 0;
}

bool AkosCostumeLoader::increaseAnims(Actor *a) {
	return ((ScummEngine_v6 *)_vm)->akos_increaseAnims(_akos, a);
}

bool ScummEngine_v6::akos_increaseAnims(const byte *akos, Actor *a) {
	const byte *aksq, *akfo;
	uint size;
	bool result;

	aksq = findResourceData(MKTAG('A','K','S','Q'), akos);
	akfo = findResourceData(MKTAG('A','K','F','O'), akos);

	size = getResourceDataSize(akfo) / 2;

	result = false;
	for (int i = 0; i < 16; i++) {
		if (a->_cost.animType[i] != AKAT_Empty)
			result |= akos_increaseAnim(a, i, aksq, (const uint16 *)akfo, size);
	}
	return result;
}

bool ScummEngine_v6::akos_increaseAnim(Actor *a, int limb, const byte *aksq, const uint16 *akfo, int numakfo) {
	byte animType;
	uint startState, curState, endState;
	uint code;
	bool skipNextState, needRedraw;
	int counter, tmp2;

	animType = a->_cost.animType[limb];
	endState = a->_cost.end[limb];
	startState = curState = a->_cost.curpos[limb];
	skipNextState = false;
	needRedraw = false;

	do {
		code = aksq[curState];
		if (code & AKC_ExtendBit)
			code = READ_BE_UINT16(aksq + curState);

		switch (animType) {
		case AKAT_AlwaysRun:
		case AKAT_DeltaAnim:
			switch (code) {
			case AKC_IfVarGoTo:
			case AKC_AddVar:
			case AKC_SetVar:
			case AKC_IfVarGEDo:
			case AKC_IfVarGTDo:
			case AKC_IfVarLEDo:
			case AKC_IfVarLTDo:

			case AKC_IfVarNEDo:
			case AKC_IfVarEQDo:
			case AKC_IfSoundInVarRunningGoTo:
			case AKC_IfNotSoundInVarRunningGoTo:
			case AKC_IfSoundRunningGoTo:
			case AKC_IfNotSoundRunningGoTo:
				curState += 5;
				break;
			case AKC_JumpToOffsetInVar:
			case AKC_SetActorZClipping:
			case AKC_StartScriptVar:
			case AKC_StartSoundVar:
			case AKC_StartScript:
			case AKC_StartAnim:
			case AKC_StartVarAnim:
			case AKC_StartSound:
			case AKC_SoftSound:
			case AKC_SoftVarSound:
			case AKC_StartTalkieInVar:
				curState += 3;
				break;
			case AKC_SoundStuff:
				if (_game.heversion >= 61)
					curState += 6;
				else
					curState += 8;
				break;
			case AKC_StartActionOn:
			case AKC_SetActorVar:
			case AKC_SetDrawOffs:
				curState += 6;
				break;
			case AKC_EndOfIfDo:
			case AKC_HideActor:
			case AKC_IncVar:
			case AKC_StartSound_SpecialCase:
			case AKC_EmptyCel:
			case AKC_EndSeq:
				curState += 2;
				break;
			case AKC_IfVarGEJump:
			case AKC_IfVarGTJump:
			case AKC_IfVarLEJump:
			case AKC_IfVarLTJump:
			case AKC_IfVarNEJump:
			case AKC_IfVarEQJump:
			case AKC_SetVarRandom:
				curState += 7;
				break;
			case AKC_Flip:
			case AKC_GoToState:
			case AKC_StartActorAnim:
			case AKC_StartActorTalkie:
			case AKC_IfTalkingGoTo:
			case AKC_IfNotTalkingGoTo:
				curState += 4;
				break;
			case AKC_RelativeOffsetDrawMany:
				curState += 4;
				// Fall through
			case AKC_DrawMany:
				curState += 3;
				counter = aksq[curState - 1];
				while (--counter >= 0) {
					curState += 4;
					curState += (aksq[curState] & AKC_ExtendBit) ? 2 : 1;
				}
				break;
			case AKC_CondDrawMany:
			case AKC_CondRelativeOffsetDrawMany:
			case AKC_SetUserCondition:
			case AKC_SetVarToUserCondition:
			case AKC_SetTalkCondition:
			case AKC_SetVarToTalkCondition:
				needRedraw = true;
				curState += aksq[curState + 2];
				break;
			case AKC_DisplayAuxFrame:
				akos_queCommand(AKQC_DisplayAuxFrame, a, GW(2), 0);
				curState += 4;
				break;
			default:
				curState += (code & AKC_ExtendWordBit) ? 2 : 1;
				break;
			}
			break;
		case AKAT_LoopLayer:
			curState += (code & AKC_ExtendWordBit) ? 2 : 1;
			if (curState > endState)
				curState = a->_cost.start[limb];
			break;
		case AKAT_RunLayer:
			if (curState != endState)
				curState += (code & AKC_ExtendWordBit) ? 2 : 1;
			break;
		case AKAT_UserConstant:
			// Script controlled animation: do nothing
			break;
		default:
			break;
		}

		code = aksq[curState];
		if (code & AKC_ExtendBit)
			code = READ_BE_UINT16(aksq + curState);

		if (skipNextState && code != AKC_EndOfIfDo)
			continue;

		switch (code) {
		case AKC_StartActorAnim:
			akos_queCommand(4, derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:29"), a->getAnimVar(GB(3)), 0);
			continue;

		case AKC_SetVarRandom:
			a->setAnimVar(GB(6), _rnd.getRandomNumberRng(GW(2), GW(4)));
			continue;
		case AKC_IfVarGEJump:
		case AKC_IfVarGTJump:
		case AKC_IfVarLEJump:
		case AKC_IfVarLTJump:
		case AKC_IfVarNEJump:
		case AKC_IfVarEQJump:
			if (akosCompare(a->getAnimVar(GB(4)), GW(5), code)) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IncVar:
			a->setAnimVar(0, a->getAnimVar(0) + 1);
			continue;
		case AKC_SetVar:
			a->setAnimVar(GB(4), GW(2));
			continue;
		case AKC_AddVar:
			a->setAnimVar(GB(4), a->getAnimVar(GB(4)) + GW(2));
			continue;
		case AKC_Flip:
			a->_flip = GW(2) != 0;
			continue;
		case AKC_StartSound:
			if (_game.heversion >= 61)
				counter = GB(2);
			else
				counter = GB(2) - 1;
			if ((uint) counter < 24)
				akos_queCommand(3, a, a->_sound[counter], 0);
			continue;
		case AKC_StartSound_SpecialCase:
			akos_queCommand(3, a, a->_sound[0], 0);
			continue;
		case AKC_StartAnim:
			akos_queCommand(4, a, GB(2), 0);
			continue;
		case AKC_StartVarAnim:
			akos_queCommand(4, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_SetActorVar:
			derefActor(a->getAnimVar(GB(2)), "akos_increaseAnim:9")->setAnimVar(GB(3), GW(4));
			continue;
		case AKC_HideActor:
			akos_queCommand(1, a, 0, 0);
			continue;
		case AKC_SetActorZClipping:
			akos_queCommand(5, a, GB(2), 0);
			continue;
		case AKC_SoundStuff:
			if (_game.heversion >= 61)
				continue;
			counter = GB(2) - 1;
			if (counter >= 8)
				continue;
			tmp2 = GB(4);
			if (tmp2 < 1 || tmp2 > 3)
				error("akos_increaseAnim:8 invalid code %d", tmp2);
			akos_queCommand(tmp2 + 6, a, a->_sound[counter], GB(6));
			continue;
		case AKC_SetDrawOffs:
			akos_queCommand(6, a, GW(2), GW(4));
			continue;
		case AKC_JumpToOffsetInVar:
			if (akfo == nullptr)
				error("akos_increaseAnim: no AKFO table");
			counter = a->getAnimVar(GB(2)) - 1;
			if (_game.heversion >= 80) {
				if (counter < 0 || counter > a->_cost.heJumpCountTable[limb] - 1)
					error("akos_increaseAnim: invalid jump value %d", counter);
				curState = READ_LE_UINT16(akfo + a->_cost.heJumpOffsetTable[limb] + counter * 2);
			} else {
				if (counter < 0 || counter > numakfo - 1)
					error("akos_increaseAnim: invalid jump value %d", counter);
				curState = READ_LE_UINT16(&akfo[counter]);
			}
			break;
		case AKC_IfVarGoTo:
			if (!a->getAnimVar(GB(4)))
				continue;
			a->setAnimVar(GB(4), 0);
			curState = GUW(2);
			break;

		case AKC_EndOfIfDo:
			skipNextState = false;
			continue;

		case AKC_GoToState:
			curState = GUW(2);

			// WORKAROUND bug #3813: In the German version of SPY Fox 3: Operation Ozone
			// the wig maker room 21 contains a costume animation 352 of an LED ticker
			// with a jump to an erroneous position 846.
			// To prevent an undefined 'uSweat token' the animation is reset to its start.
			if (_game.id == GID_HEGAME && _language == Common::DE_DEU && \
			    _currentRoom == 21 && a->_costume == 352 && curState == 846) {
				curState = a->_cost.start[limb];
			}
			break;

		case AKC_EmptyCel:
		case AKC_EndSeq:
		case AKC_DrawMany:
		case AKC_DisplayAuxFrame:
		case AKC_RelativeOffsetDrawMany:
			break;

		case AKC_CondDrawMany:
		case AKC_CondRelativeOffsetDrawMany:
			needRedraw = true;
			break;

		case AKC_StartActionOn:
		case AKC_StartScript:
		case AKC_StartScriptVar:
			continue;

		case AKC_StartSoundVar:
			if (_game.heversion >= 71)
				akos_queCommand(3, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;

		case AKC_IfVarEQDo:
		case AKC_IfVarNEDo:
		case AKC_IfVarLTDo:
		case AKC_IfVarLEDo:
		case AKC_IfVarGTDo:
		case AKC_IfVarGEDo:
			if (!akosCompare(a->getAnimVar(GB(4)), GW(2), code))
				skipNextState = true;
			continue;
		case AKC_IfSoundInVarRunningGoTo:
			if (_sound->isSoundRunning( a->_sound[a->getAnimVar(GB(4))]))  {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IfNotSoundInVarRunningGoTo:
			if (!_sound->isSoundRunning(a->_sound[a->getAnimVar(GB(4))])) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IfSoundRunningGoTo:
			if (_sound->isSoundRunning(a->_sound[GB(4)])) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IfNotSoundRunningGoTo:
			if (!_sound->isSoundRunning(a->_sound[GB(4)])) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_SoftSound:
			akos_queCommand(9, a, a->_sound[GB(2)], 0);
			continue;
		case AKC_SoftVarSound:
			akos_queCommand(9, a, a->_sound[a->getAnimVar(GB(2))], 0);
			continue;
		case AKC_SetUserCondition:
			((ActorHE *)a)->setUserCondition(GB(3), a->getAnimVar(GB(4)));
			continue;
		case AKC_SetVarToUserCondition:
			a->setAnimVar(GB(4), ((ActorHE *)a)->isUserConditionSet(GB(3)));
			continue;
		case AKC_SetTalkCondition:
			((ActorHE *)a)->setTalkCondition(GB(3));
			continue;
		case AKC_SetVarToTalkCondition:
			a->setAnimVar(GB(4), ((ActorHE *)a)->isTalkConditionSet(GB(3)));
			continue;
		case AKC_StartActorTalkie:
			akos_queCommand(8, a, GB(2), 0);
			continue;
		case AKC_IfTalkingGoTo:
			if (((ActorHE *)a)->_heTalking != 0) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IfNotTalkingGoTo:
			if (((ActorHE *)a)->_heTalking == 0) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_StartTalkieInVar:
			akos_queCommand(8, a, a->getAnimVar(GB(2)), 0);
			continue;
		case AKC_IfAnyTalkingGoTo:
			if (VAR(VAR_TALK_ACTOR) != 0) {
				curState = GUW(2);
				break;
			}
			continue;
		case AKC_IfNotAnyTalkingGoTo:
			if (VAR(VAR_TALK_ACTOR) == 0) {
				curState = GUW(2);
				break;
			}
			continue;
		default:
			if ((code & AKC_CommandMask) == AKC_CommandMask)
				error("Undefined uSweat token %X", code);
		}
		break;
	} while (true);

	int code2 = aksq[curState];
	if (code2 & AKC_ExtendBit)
		code2 = READ_BE_UINT16(aksq + curState);

	if ((code2 & AKC_CommandMask) == AKC_CommandMask && code2 != AKC_DrawMany && code2 != AKC_EmptyCel && code2 != AKC_EndSeq && code2 != AKC_DisplayAuxFrame && code2 != AKC_RelativeOffsetDrawMany && code2 != AKC_CondDrawMany && code2 != AKC_CondRelativeOffsetDrawMany)
		error("Ending with undefined uSweat token %X", code2);

	a->_cost.curpos[limb] = curState;

	if (needRedraw)
		return true;
	else
		return curState != startState;
}

void ScummEngine_v6::akos_queCommand(byte cmd, Actor *a, int param1, int param2) {
	_akosQueuePos++;
	assertRange(0, _akosQueuePos, 31, "akos_queCommand: _akosQueuePos");

	_akosQueue[_akosQueuePos].cmd = cmd;
	_akosQueue[_akosQueuePos].actor = a->_number;
	_akosQueue[_akosQueuePos].param1 = param1;
	_akosQueue[_akosQueuePos].param2 = param2;
}

void ScummEngine_v6::akos_processQueue() {
	byte cmd;
	int actor, param1, param2;

	while (_akosQueuePos) {
		cmd = _akosQueue[_akosQueuePos].cmd;
		actor = _akosQueue[_akosQueuePos].actor;
		param1 = _akosQueue[_akosQueuePos].param1;
		param2 = _akosQueue[_akosQueuePos].param2;
		_akosQueuePos--;

		Actor *a = derefActor(actor, "akos_processQueue");

		switch (cmd) {
		case AKQC_PutActorInTheVoid:
			a->putActor(0, 0, 0);
			break;
		case AKQC_StartSound:
			if (_game.heversion < 95) {
				_sound->addSoundToQueue(param1, 0, -1, 0,
					HSND_BASE_FREQ_FACTOR, HSND_SOUND_PAN_CENTER, HSND_MAX_VOLUME);
			} else {
				// Later games also set VAR_LAST_SOUND variable
				_sound->startSound(param1, 0, -1, 0,
					HSND_BASE_FREQ_FACTOR, HSND_SOUND_PAN_CENTER, HSND_MAX_VOLUME);
			}

			break;
		case AKQC_StartAnimation:
			a->startAnimActor(param1);
			break;
		case AKQC_SetZClipping:
			a->_forceClip = param1;
			break;
		case AKQC_SetXYOffset:
			a->_heOffsX = param1;
			a->_heOffsY = param2;
			break;
		case AKQC_DisplayAuxFrame:
#ifdef ENABLE_HE
			assert(_game.heversion >= 71);
			((ScummEngine_v71he *)this)->heQueueAnimAuxFrame(a->_number, param1);
#endif
			break;
		case AKQC_StartTalkie:
			_actorToPrintStrFor = a->_number;

			a->_talkPosX = ((ActorHE *)a)->_heTalkQueue[param1].posX;
			a->_talkPosY = ((ActorHE *)a)->_heTalkQueue[param1].posY;
			a->_talkColor = ((ActorHE *)a)->_heTalkQueue[param1].color;

			_string[0].loadDefault();
			_string[0].color = a->_talkColor;
			actorTalk(((ActorHE *)a)->_heTalkQueue[param1].sentence);

			break;
		case AKQC_SoftStartSound:
			if (_game.heversion < 95) {
				_sound->addSoundToQueue(param1, 0, -1, ScummEngine_v70he::HESndFlags::HE_SND_SOFT_SOUND,
					HSND_BASE_FREQ_FACTOR, HSND_SOUND_PAN_CENTER, HSND_MAX_VOLUME);
			} else {
				// Later games also set VAR_LAST_SOUND variable
				_sound->startSound(param1, 0, -1, ScummEngine_v70he::HESndFlags::HE_SND_SOFT_SOUND,
					HSND_BASE_FREQ_FACTOR, HSND_SOUND_PAN_CENTER, HSND_MAX_VOLUME);
			}

			break;
		default:
			error("akos_queCommand(%d,%d,%d,%d)", cmd, a->_number, param1, param2);
		}
	}
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::akos_processQueue() {
	byte cmd;
	int actor, param1, param2;

	while (_akosQueuePos) {
		cmd = _akosQueue[_akosQueuePos].cmd;
		actor = _akosQueue[_akosQueuePos].actor;
		param1 = _akosQueue[_akosQueuePos].param1;
		param2 = _akosQueue[_akosQueuePos].param2;
		_akosQueuePos--;

		Actor *a = derefActor(actor, "akos_processQueue");

		switch (cmd) {
		case AKQC_PutActorInTheVoid:
			a->putActor(0, 0, 0);
			break;
		case AKQC_StartSound:
			if (param1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->startSfx(param1, 63);
				}
			}
			break;
		case AKQC_StartAnimation:
			a->startAnimActor(param1);
			break;
		case AKQC_SetZClipping:
			a->_forceClip = param1;
			break;
		case AKQC_SetXYOffset:
			a->_heOffsX = param1;
			a->_heOffsY = param2;
			break;
		case AKQC_SetSoundVolume:
			if (param1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setVolume(param1, param2);
				}
			}
			break;
		case AKQC_SetSoundPan:
			if (param1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setPan(param1, param2);
				}
			}
			break;
		case AKQC_SetSoundPriority:
			if (param1 != 0) {
				if (_imuseDigital) {
					_imuseDigital->setPriority(param1, param2);
				}
			}
			break;
		default:
			error("akos_queCommand(%d,%d,%d,%d)", cmd, a->_number, param1, param2);
		}
	}
}
#endif

} // End of namespace Scumm
