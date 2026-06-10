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

#ifndef SCUMM_BYLE_RLE_DECODE_M68K_H
#define SCUMM_BYLE_RLE_DECODE_M68K_H

#include "scumm/base-costume.h"

extern "C" void ByleRLEDecode_Mode0(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Mode1(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Mode3(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Mode3_HE90(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Classic(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX, /* unused */
	const byte _scaleY, /* unused */
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette /* unused */);

extern "C" void ByleRLEDecode_Scaled_Mode0(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Scaled_Mode0_SMask(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable, /* unused */
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Scaled_Mode1(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Scaled_Mode1_SMask(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

extern "C" void ByleRLEDecode_Scaled_Mode3(
	Scumm::BaseCostumeRenderer::ByleRLEData *pcompData,
	const byte _scaleX,
	const byte _scaleY,
	const int _height,
	const int pitch,
	const int _numStrips,
	const byte *_srcPtr,
	const byte *_shadowTable,
	const uint16 *_palette);

#endif
