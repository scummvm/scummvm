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

#ifndef MSN_DEF_H
#define MSN_DEF_H

namespace Supernova {

const int kScreenWidth  = 320;
const int kScreenHeight = 200;
const int kFontWidth = 5;
const int kFontHeight = 8;
const int kTextSpeed[] = {19, 14, 10, 7, 4};
const int kMsecPerTick = 55;

const int kMaxSection = 40;
const int kMaxDialog = 2;
const int kMaxObject = 25;
const int kMaxCarry = 30;

//const int kRoomsChap0 =  1;
//const int kRoomsChap1 = 16;
//const int kRoomsChap2 =  9;
//const int kRoomsChap3 = 21;
//const int kRoomsNum = kRoomsChap0 + kRoomsChap1 + kRoomsChap2 + kRoomsChap3;

const byte kShownFalse = 0;
const byte kShownTrue = 1;

enum MessagePosition {
	kMessageNormal,
	kMessageLeft,
	kMessageRight,
	kMessageCenter,
	kMessageTop
};

enum AudioIndex {
	kAudioGreat,                // 44|0
	kAudioCrash,                // 45|0
	kAudioVoiceHalt,            // 46|0
	kAudioGunShot,              // 46|2510
	kAudioUndef2,               // 46|4020
	kAudioVoiceSupernova,       // 47|0
	kAudioVoiceYeah,            // 47|24010
	kAudioUndef3,               // 48|0
	kAudioUndef4,               // 48|2510
	kAudioShock,                // 48|10520
	kAudioTurntable,            // 48|13530
	kAudioSiren,                // 50|0
	kAudioUndef6,               // 50|12786
	kAudioRocks,                // 51|0
	kAudioDeath,                // 53|0
	kAudioAlarm,                // 54|0
	kAudioUndef7,               // 54|8010
	kAudioDoorSound,            // 54|24020
	kAudioDoorOpen,             // 54|30030
	kAudioDoorClose,            // 54|31040
	kAudioNumSamples
};

enum MusicIndex {
	kMusicIntro = 52,
	kMusicOutro = 49
};

struct AudioInfo {
	int _filenumber;
	int _offsetStart;
	int _offsetEnd;
};

const int kColorBlack       =  0;
const int kColorWhite25     =  1;
const int kColorWhite35     =  2;
const int kColorWhite44     =  3;
const int kColorWhite99     =  4;
const int kColorDarkGreen   =  5;
const int kColorGreen       =  6;
const int kColorDarkRed     =  7;
const int kColorRed         =  8;
const int kColorDarkBlue    =  9;
const int kColorBlue        = 10;
const int kColorWhite63     = 11;
const int kColorLightBlue   = 12;
const int kColorLightGreen  = 13;
const int kColorLightYellow = 14;
const int kColorLightRed    = 15;
const int kColorCursorTransparent = kColorWhite25;

const byte mouseNormal[64] = {
	0xff,0x3f,0xff,0x1f,0xff,0x0f,0xff,0x07,
	0xff,0x03,0xff,0x01,0xff,0x00,0x7f,0x00,
	0x3f,0x00,0x1f,0x00,0x0f,0x00,0x0f,0x00,
	0xff,0x00,0x7f,0x18,0x7f,0x38,0x7f,0xfc,

	0x00,0x00,0x00,0x40,0x00,0x60,0x00,0x70,
	0x00,0x78,0x00,0x7c,0x00,0x7e,0x00,0x7f,
	0x80,0x7f,0xc0,0x7f,0xe0,0x7f,0x00,0x7e,
	0x00,0x66,0x00,0x43,0x00,0x03,0x00,0x00
};

const byte mouseWait[64] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,
	0x01,0x80,0x01,0x80,0x11,0x88,0x31,0x8c,
	0x31,0x8c,0x11,0x88,0x01,0x80,0x01,0x80,
	0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0xfe,0x7f,0xf4,0x2f,0xf4,0x2f,
	0x14,0x28,0x24,0x24,0x44,0x22,0x84,0x21,
	0x84,0x21,0xc4,0x23,0xe4,0x27,0x74,0x2e,
	0x34,0x2c,0x14,0x28,0xfe,0x7f,0x00,0x00
};

const byte font[][5] = {
	{0x00,0x00,0x00,0xff,0x00},
	{0x5f,0xff,0x00,0x00,0x00},
	{0x03,0x00,0x03,0xff,0x00},
	{0x14,0x7f,0x14,0x7f,0x14},
	{0x24,0x2a,0x7f,0x2a,0x12},
	{0x61,0x10,0x08,0x04,0x43},
	{0x38,0x4e,0x59,0x26,0x50},
	{0x03,0xff,0x00,0x00,0x00},
	{0x3e,0x41,0xff,0x00,0x00},
	{0x41,0x3e,0xff,0x00,0x00},
	{0x10,0x54,0x38,0x54,0x10},
	{0x10,0x10,0x7c,0x10,0x10},
	{0x80,0x40,0xff,0x00,0x00},
	{0x10,0x10,0x10,0x10,0x10},
	{0x40,0xff,0x00,0x00,0x00},
	{0x60,0x10,0x08,0x04,0x03},

	{0x3e,0x41,0x41,0x41,0x3e}, /* digits */
	{0x04,0x02,0x7f,0xff,0x00},
	{0x42,0x61,0x51,0x49,0x46},
	{0x22,0x41,0x49,0x49,0x36},
	{0x18,0x14,0x12,0x7f,0x10},
	{0x27,0x45,0x45,0x45,0x39},
	{0x3e,0x49,0x49,0x49,0x32},
	{0x01,0x61,0x19,0x07,0x01},
	{0x36,0x49,0x49,0x49,0x36},
	{0x26,0x49,0x49,0x49,0x3e},

	{0x44,0xff,0x00,0x00,0x00},
	{0x80,0x44,0xff,0x00,0x00},
	{0x10,0x28,0x44,0xff,0x00},
	{0x28,0x28,0x28,0x28,0x28},
	{0x44,0x28,0x10,0xff,0x00},
	{0x02,0x01,0x51,0x09,0x06},
	{0x3e,0x41,0x5d,0x5d,0x1e},

	{0x7c,0x12,0x11,0x12,0x7c}, /* uppercase letters*/
	{0x7f,0x49,0x49,0x49,0x36},
	{0x3e,0x41,0x41,0x41,0x22},
	{0x7f,0x41,0x41,0x22,0x1c},
	{0x7f,0x49,0x49,0x49,0xff},
	{0x7f,0x09,0x09,0x09,0xff},
	{0x3e,0x41,0x41,0x49,0x3a},
	{0x7f,0x08,0x08,0x08,0x7f},
	{0x41,0x7f,0x41,0xff,0x00},
	{0x20,0x40,0x40,0x3f,0xff},
	{0x7f,0x08,0x14,0x22,0x41},
	{0x7f,0x40,0x40,0x40,0xff},
	{0x7f,0x02,0x04,0x02,0x7f},
	{0x7f,0x02,0x0c,0x10,0x7f},
	{0x3e,0x41,0x41,0x41,0x3e},
	{0x7f,0x09,0x09,0x09,0x06},
	{0x3e,0x41,0x51,0x21,0x5e},
	{0x7f,0x09,0x19,0x29,0x46},
	{0x26,0x49,0x49,0x49,0x32},
	{0x01,0x01,0x7f,0x01,0x01},
	{0x3f,0x40,0x40,0x40,0x3f},
	{0x07,0x18,0x60,0x18,0x07},
	{0x1f,0x60,0x18,0x60,0x1f},
	{0x63,0x14,0x08,0x14,0x63},
	{0x03,0x04,0x78,0x04,0x03},
	{0x61,0x51,0x49,0x45,0x43},

	{0x7f,0x41,0x41,0xff,0x00},
	{0x03,0x04,0x08,0x10,0x60},
	{0x41,0x41,0x7f,0xff,0x00},
	{0x02,0x01,0x02,0xff,0x00},
	{0x80,0x80,0x80,0x80,0x80},
	{0x01,0x02,0xff,0x00,0x00},

	{0x38,0x44,0x44,0x44,0x7c}, /* lowercase letters */
	{0x7f,0x44,0x44,0x44,0x38},
	{0x38,0x44,0x44,0x44,0x44},
	{0x38,0x44,0x44,0x44,0x7f},
	{0x38,0x54,0x54,0x54,0x58},
	{0x04,0x7e,0x05,0x01,0xff},
	{0x98,0xa4,0xa4,0xa4,0x7c},
	{0x7f,0x04,0x04,0x04,0x78},
	{0x7d,0xff,0x00,0x00,0x00},
	{0x80,0x80,0x7d,0xff,0x00},
	{0x7f,0x10,0x28,0x44,0xff},
	{0x7f,0xff,0x00,0x00,0x00},
	{0x7c,0x04,0x7c,0x04,0x78},
	{0x7c,0x04,0x04,0x04,0x78},
	{0x38,0x44,0x44,0x44,0x38},
	{0xfc,0x24,0x24,0x24,0x18},
	{0x18,0x24,0x24,0x24,0xfc},
	{0x7c,0x08,0x04,0x04,0xff},
	{0x48,0x54,0x54,0x54,0x24},
	{0x04,0x3e,0x44,0x40,0xff},
	{0x7c,0x40,0x40,0x40,0x3c},
	{0x0c,0x30,0x40,0x30,0x0c},
	{0x3c,0x40,0x3c,0x40,0x3c},
	{0x44,0x28,0x10,0x28,0x44},
	{0x9c,0xa0,0xa0,0xa0,0x7c},
	{0x44,0x64,0x54,0x4c,0x44},

	{0x08,0x36,0x41,0xff,0x00},
	{0x77,0xff,0x00,0x00,0x00},
	{0x41,0x36,0x08,0xff,0x00},
	{0x02,0x01,0x02,0x01,0xff},
	{0xff,0x00,0x00,0x00,0x00},

	{0xfe,0x49,0x49,0x4e,0x30}, /* sharp S */

	{0x7c,0x41,0x40,0x41,0x3c}, /* umlauts */

	{0x04,0x06,0x7f,0x06,0x04}, /* arrows */
	{0x20,0x60,0xfe,0x60,0x20},

	{0x38,0x45,0x44,0x45,0x7c}, /* umlauts */
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0x79,0x14,0x12,0x14,0x79},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0x38,0x45,0x44,0x45,0x38},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00,0x00},
	{0x3d,0x42,0x42,0x42,0x3d},
	{0x3d,0x40,0x40,0x40,0x3d},
};

// Default palette
const byte initVGAPalette[768] = {
	0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x58, 0x58, 0x58, 0x70, 0x70, 0x70, 0xfc, 0xfc, 0xfc, 0x00, 0xd0, 0x00,
	0x00, 0xfc, 0x00, 0xd8, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0xb0, 0xa0, 0xa0, 0xa0,
	0x50, 0xc8, 0xfc, 0x28, 0xfc, 0x28, 0xf0, 0xf0, 0x00, 0xfc, 0x28, 0x28, 0x00, 0x00, 0x00, 0x14, 0x14, 0x14,
	0x20, 0x20, 0x20, 0x2c, 0x2c, 0x2c, 0x38, 0x38, 0x38, 0x44, 0x44, 0x44, 0x50, 0x50, 0x50, 0x60, 0x60, 0x60,
	0x70, 0x70, 0x70, 0x80, 0x80, 0x80, 0x90, 0x90, 0x90, 0xa0, 0xa0, 0xa0, 0xb4, 0xb4, 0xb4, 0xc8, 0xc8, 0xc8,
	0xe0, 0xe0, 0xe0, 0xfc, 0xfc, 0xfc, 0x00, 0x00, 0xfc, 0x40, 0x00, 0xfc, 0x7c, 0x00, 0xfc, 0xbc, 0x00, 0xfc,
	0xfc, 0x00, 0xfc, 0xfc, 0x00, 0xbc, 0xfc, 0x00, 0x7c, 0xfc, 0x00, 0x40, 0xfc, 0x00, 0x00, 0xfc, 0x40, 0x00,
	0xfc, 0x7c, 0x00, 0xfc, 0xbc, 0x00, 0xfc, 0xfc, 0x00, 0xbc, 0xfc, 0x00, 0x7c, 0xfc, 0x00, 0x40, 0xfc, 0x00,
	0x00, 0xfc, 0x00, 0x00, 0xfc, 0x40, 0x00, 0xfc, 0x7c, 0x00, 0xfc, 0xbc, 0x00, 0xfc, 0xfc, 0x00, 0xbc, 0xfc,
	0x00, 0x7c, 0xfc, 0x00, 0x40, 0xfc, 0x7c, 0x7c, 0xfc, 0x9c, 0x7c, 0xfc, 0xbc, 0x7c, 0xfc, 0xdc, 0x7c, 0xfc,
	0xfc, 0x7c, 0xfc, 0xfc, 0x7c, 0xdc, 0xfc, 0x7c, 0xbc, 0xfc, 0x7c, 0x9c, 0xfc, 0x7c, 0x7c, 0xfc, 0x9c, 0x7c,
	0xfc, 0xbc, 0x7c, 0xfc, 0xdc, 0x7c, 0xfc, 0xfc, 0x7c, 0xdc, 0xfc, 0x7c, 0xbc, 0xfc, 0x7c, 0x9c, 0xfc, 0x7c,
	0x7c, 0xfc, 0x7c, 0x7c, 0xfc, 0x9c, 0x7c, 0xfc, 0xbc, 0x7c, 0xfc, 0xdc, 0x7c, 0xfc, 0xfc, 0x7c, 0xdc, 0xfc,
	0x7c, 0xbc, 0xfc, 0x7c, 0x9c, 0xfc, 0xb4, 0xb4, 0xfc, 0xc4, 0xb4, 0xfc, 0xd8, 0xb4, 0xfc, 0xe8, 0xb4, 0xfc,
	0xfc, 0xb4, 0xfc, 0xfc, 0xb4, 0xe8, 0xfc, 0xb4, 0xd8, 0xfc, 0xb4, 0xc4, 0xfc, 0xb4, 0xb4, 0xfc, 0xc4, 0xb4,
	0xfc, 0xd8, 0xb4, 0xfc, 0xe8, 0xb4, 0xfc, 0xfc, 0xb4, 0xe8, 0xfc, 0xb4, 0xd8, 0xfc, 0xb4, 0xc4, 0xfc, 0xb4,
	0xb4, 0xfc, 0xb4, 0xb4, 0xfc, 0xc4, 0xb4, 0xfc, 0xd8, 0xb4, 0xfc, 0xe8, 0xb4, 0xfc, 0xfc, 0xb4, 0xe8, 0xfc,
	0xb4, 0xd8, 0xfc, 0xb4, 0xc4, 0xfc, 0x00, 0x00, 0x70, 0x1c, 0x00, 0x70, 0x38, 0x00, 0x70, 0x54, 0x00, 0x70,
	0x70, 0x00, 0x70, 0x70, 0x00, 0x54, 0x70, 0x00, 0x38, 0x70, 0x00, 0x1c, 0x70, 0x00, 0x00, 0x70, 0x1c, 0x00,
	0x70, 0x38, 0x00, 0x70, 0x54, 0x00, 0x70, 0x70, 0x00, 0x54, 0x70, 0x00, 0x38, 0x70, 0x00, 0x1c, 0x70, 0x00,
	0x00, 0x70, 0x00, 0x00, 0x70, 0x1c, 0x00, 0x70, 0x38, 0x00, 0x70, 0x54, 0x00, 0x70, 0x70, 0x00, 0x54, 0x70,
	0x00, 0x38, 0x70, 0x00, 0x1c, 0x70, 0x38, 0x38, 0x70, 0x44, 0x38, 0x70, 0x54, 0x38, 0x70, 0x60, 0x38, 0x70,
	0x70, 0x38, 0x70, 0x70, 0x38, 0x60, 0x70, 0x38, 0x54, 0x70, 0x38, 0x44, 0x70, 0x38, 0x38, 0x70, 0x44, 0x38,
	0x70, 0x54, 0x38, 0x70, 0x60, 0x38, 0x70, 0x70, 0x38, 0x60, 0x70, 0x38, 0x54, 0x70, 0x38, 0x44, 0x70, 0x38,
	0x38, 0x70, 0x38, 0x38, 0x70, 0x44, 0x38, 0x70, 0x54, 0x38, 0x70, 0x60, 0x38, 0x70, 0x70, 0x38, 0x60, 0x70,
	0x38, 0x54, 0x70, 0x38, 0x44, 0x70, 0x50, 0x50, 0x70, 0x58, 0x50, 0x70, 0x60, 0x50, 0x70, 0x68, 0x50, 0x70,
	0x70, 0x50, 0x70, 0x70, 0x50, 0x68, 0x70, 0x50, 0x60, 0x70, 0x50, 0x58, 0x70, 0x50, 0x50, 0x70, 0x58, 0x50,
	0x70, 0x60, 0x50, 0x70, 0x68, 0x50, 0x70, 0x70, 0x50, 0x68, 0x70, 0x50, 0x60, 0x70, 0x50, 0x58, 0x70, 0x50,
	0x50, 0x70, 0x50, 0x50, 0x70, 0x58, 0x50, 0x70, 0x60, 0x50, 0x70, 0x68, 0x50, 0x70, 0x70, 0x50, 0x68, 0x70,
	0x50, 0x60, 0x70, 0x50, 0x58, 0x70, 0x00, 0x00, 0x40, 0x10, 0x00, 0x40, 0x20, 0x00, 0x40, 0x30, 0x00, 0x40,
	0x40, 0x00, 0x40, 0x40, 0x00, 0x30, 0x40, 0x00, 0x20, 0x40, 0x00, 0x10, 0x40, 0x00, 0x00, 0x40, 0x10, 0x00,
	0x40, 0x20, 0x00, 0x40, 0x30, 0x00, 0x40, 0x40, 0x00, 0x30, 0x40, 0x00, 0x20, 0x40, 0x00, 0x10, 0x40, 0x00,
	0x00, 0x40, 0x00, 0x00, 0x40, 0x10, 0x00, 0x40, 0x20, 0x00, 0x40, 0x30, 0x00, 0x40, 0x40, 0x00, 0x30, 0x40,
	0x00, 0x20, 0x40, 0x00, 0x10, 0x40, 0x20, 0x20, 0x40, 0x28, 0x20, 0x40, 0x30, 0x20, 0x40, 0x38, 0x20, 0x40,
	0x40, 0x20, 0x40, 0x40, 0x20, 0x38, 0x40, 0x20, 0x30, 0x40, 0x20, 0x28, 0x40, 0x20, 0x20, 0x40, 0x28, 0x20,
	0x40, 0x30, 0x20, 0x40, 0x38, 0x20, 0x40, 0x40, 0x20, 0x38, 0x40, 0x20, 0x30, 0x40, 0x20, 0x28, 0x40, 0x20,
	0x20, 0x40, 0x20, 0x20, 0x40, 0x28, 0x20, 0x40, 0x30, 0x20, 0x40, 0x38, 0x20, 0x40, 0x40, 0x20, 0x38, 0x40,
	0x20, 0x30, 0x40, 0x20, 0x28, 0x40, 0x2c, 0x2c, 0x40, 0x30, 0x2c, 0x40, 0x34, 0x2c, 0x40, 0x3c, 0x2c, 0x40,
	0x40, 0x2c, 0x40, 0x40, 0x2c, 0x3c, 0x40, 0x2c, 0x34, 0x40, 0x2c, 0x30, 0x40, 0x2c, 0x2c, 0x40, 0x30, 0x2c,
	0x40, 0x34, 0x2c, 0x40, 0x3c, 0x2c, 0x40, 0x40, 0x2c, 0x3c, 0x40, 0x2c, 0x34, 0x40, 0x2c, 0x30, 0x40, 0x2c,
	0x2c, 0x40, 0x2c, 0x2c, 0x40, 0x30, 0x2c, 0x40, 0x34, 0x2c, 0x40, 0x3c, 0x2c, 0x40, 0x40, 0x2c, 0x3c, 0x40,
	0x2c, 0x34, 0x40, 0x2c, 0x30, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

enum ObjectType {
	NULLTYPE    =      0,
	TAKE        =      1,
	OPENABLE    =      2,
	OPENED      =      4,
	CLOSED      =      8,
	EXIT        =     16,
	PRESS       =     32,
	COMBINABLE  =     64,
	CARRIED     =    128,
	UNNECESSARY =    256,
	WORN        =    512,
	TALK        =   1024,
	OCCUPIED    =   2048,
	CAUGHT      =   4096
};

enum Action {
	ACTION_WALK,
	ACTION_LOOK,
	ACTION_TAKE,
	ACTION_OPEN,
	ACTION_CLOSE,
	ACTION_PRESS,
	ACTION_PULL,
	ACTION_USE,
	ACTION_TALK,
	ACTION_GIVE
};

enum RoomID {
	INTRO,CORRIDOR,HALL,SLEEP,COCKPIT,AIRLOCK,
	HOLD,LANDINGMODULE,GENERATOR,OUTSIDE,
	CABIN_R1,CABIN_R2,CABIN_R3,CABIN_L1,CABIN_L2,CABIN_L3,BATHROOM,

	ROCKS,CAVE,MEETUP,ENTRANCE,REST,ROGER,GLIDER,MEETUP2,MEETUP3,

	CELL,CORRIDOR1,CORRIDOR2,CORRIDOR3,CORRIDOR4,CORRIDOR5,CORRIDOR6,CORRIDOR7,CORRIDOR8,CORRIDOR9,
	BCORRIDOR,GUARD,GUARD3,OFFICE_L1,OFFICE_L2,OFFICE_R1,OFFICE_R2,OFFICE_L,
	ELEVATOR,STATION,SIGN,OUTRO,NUMROOMS,NULLROOM
};

enum ObjectID {
	INVALIDOBJECT = -1,
	NULLOBJECT = 0,
	KEYCARD,KNIFE,WATCH,
	SOCKET,
	BUTTON,HATCH1,
	BUTTON1,BUTTON2,MANOMETER,SUIT,HELMET,LIFESUPPORT,
	SCRAP_LK,OUTERHATCH_TOP,GENERATOR_TOP,TERMINALSTRIP,LANDINGMOD_OUTERHATCH,
	HOLD_WIRE,
	LANDINGMOD_BUTTON,LANDINGMOD_SOCKET,LANDINGMOD_WIRE,LANDINGMOD_HATCH,LANDINGMOD_MONITOR,
	KEYBOARD,
	KEYCARD2,OUTERHATCH,GENERATOR_WIRE,TRAP,SHORT_WIRE,CLIP,
	VOLTMETER,LADDER,GENERATOR_ROPE,
	KITCHEN_HATCH,SLEEP_SLOT,
	MONITOR,INSTRUMENTS,
	COMPUTER,CABINS,CABIN,
	SLOT_K1,SLOT_K2,SLOT_K3,SLOT_K4,
	SHELF1,SHELF2,SHELF3,SHELF4,
	ROPE,BOOK,DISCMAN,CHESS,
	SLOT_KL1,SLOT_KL2,SLOT_KL3,SLOT_KL4,
	SHELF_L1,SHELF_L2,SHELF_L3,SHELF_L4,
	PISTOL,BOOK2,SPOOL,
	RECORD,TURNTABLE,TURNTABLE_BUTTON,WIRE,WIRE2,PLUG,
	PEN,
	BATHROOM_DOOR,BATHROOM_EXIT,SHOWER,TOILET,

	STONE,
	SPACESHIPS,SPACESHIP,STAR,DOOR,MEETUP_SIGN,
	PORTER,BATHROOM_BUTTON,BATHROOM_SIGN,KITCHEN_SIGN,CAR_SLOT,
	ARSANO_BATHROOM,COINS,SCHNUCK,EGG,PILL,PILL_HULL,STAIRCASE,
	MEETUP_EXIT,
	ROGER_W,WALLET,KEYCARD_R,CUP,
	GLIDER_BUTTON1,GLIDER_BUTTON2,GLIDER_BUTTON3,GLIDER_BUTTON4,GLIDER_SLOT,GLIDER_BUTTONS,
	GLIDER_DISPLAY,GLIDER_INSTRUMENTS,GLIDER_KEYCARD,
	UFO,

	CELL_BUTTON,CELL_TABLE,CELL_WIRE,TRAY,CELL_DOOR,MAGNET,
	NEWSPAPER,TABLE,
	PILLAR1,PILLAR2,DOOR1,DOOR2,DOOR3,DOOR4,
	GUARDIAN,LAMP,
	MASTERKEYCARD,PAINTING,MONEY,LOCKER,LETTER,
	JUNGLE,STATION_SLOT,STATION_SIGN,

	TICKETS
};

enum StringID {
	kNoString = -1,
	// 0
	kStringCommandGo = 0, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
	kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive,
	kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen, kStringStatusCommandClose,
	kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk, kStringStatusCommandGive,
	kStringTitleVersion, kStringTitle1, kStringTitle2, kStringTitle3, kStringIntro1,
	kStringIntro2, kStringIntro3, kStringIntro4, kStringIntro5, kStringIntro6,
	kStringIntro7, kStringIntro8, kStringIntro9, kStringIntro10, kStringIntro11,
	kStringIntro12, kStringIntro13, kStringBroken, kStringDefaultDescription, kStringTakeMessage,
	kStringKeycard, kStringKeycardDescription, kStringKnife, kStringKnifeDescription, kStringWatch,
	kStringDiscman, kStringDiscmanDescription, kStringHatch, kStringButton, kStringHatchButtonDescription,
	// 50
	kStringLadder, kStringExit, kStringCockpitHatchDescription, kStringKitchenHatchDescription, kStringStasisHatchDescription,
	kStringStasisHatchDescription2, kStringSlot, kStringSlotDescription, kStringCorridor, kStringComputer,
	kStringComputerPassword, kStringInstruments, kStringInstrumentsDescription1, kStringMonitor, kStringMonitorDescription,
	kStringImage, kStringGenericDescription1, kStringGenericDescription2, kStringGenericDescription3, kStringGenericDescription4,
	kStringMagnete, kStringMagneteDescription, kStringPen, kStringPenDescription, kStringShelf,
	kStringCompartment, kStringSocket, kStringToilet, kStringPistol, kStringPistolDescription,
	kStringBooks, kStringBooksDescription, kStringSpool, kStringSpoolDescription, kStringBook,
	kStringUnderwear, kStringUnderwearDescription, kStringClothes, kStringJunk, kStringJunkDescription,
	kStringFolders, kStringFoldersDescription, kStringPoster, kStringPosterDescription1, kStringPosterDescription2,
	kStringSpeaker, kStringRecord, kStringRecordDescription, kStringRecordStand, kStringRecordStandDescription,
	// 100
	kStringTurntable, kStringTurntableDescription, kStringWire, kStringPlug, kStringImageDescription1,
	kStringDrawingInstruments, kStringDrawingInstrumentsDescription, kStringChessGame, kStringChessGameDescription1, kStringTennisRacket,
	kStringTennisRacketDescription, kStringTennisBall, kStringChessGameDescription2, kStringBed, kStringBedDescription,
	kStringCompartmentDescription, kStringAlbums, kStringAlbumsDescription, kStringRope, kStringRopeDescription,
	kStringShelfDescription, kStringClothesDescription, kStringSocks, kStringBookHitchhiker, kStringBathroom,
	kStringBathroomDescription, kStringShower, kStringHatchDescription1, kStringHatchDescription2, kStringHelmet,
	kStringHelmetDescription, kStringSuit, kStringSuitDescription, kStringLifeSupport, kStringLifeSupportDescription,
	kStringScrap, kStringScrapDescription1, kStringTerminalStrip, kStringScrapDescription2, kStringReactor,
	kStringReactorDescription, kStringNozzle, kStringPumpkin, kStringPumpkinDescription, kStringLandingModule,
	kStringLandingModuleDescription, kStringHatchDescription3, kStringGenerator, kStringGeneratorDescription, kStringScrapDescription3,
	// 150
	kSafetyButtonDescription, kStringKeyboard, kStringGeneratorWire, kStringEmptySpool, kStringKeycard2,
	kStringKeycard2Description, kStringTrap, kStringVoltmeter, kStringClip, kStringWireDescription,
	kStringStone, kStringCaveOpening, kStringCaveOpeningDescription, kStringExitDescription, kStringCave,
	kStringSign, kStringSignDescription, kStringEntrance, kStringStar, kStringSpaceshift,
	kStringPorter, kStringPorterDescription, kStringDoor, kStringChewingGum, kStringGummyBears,
	kStringChocolateBall, kStringEgg, kStringLiquorice, kStringPill, kStringPillDescription,
	kStringVendingMachine, kStringVendingMachineDescription, kStringToiletDescription, kStringStaircase, kStringCoins,
	kStringCoinsDescription, kStringTabletPackage, kStringTabletPackageDescription, kStringChair, kStringShoes,
	kStringShoesDescription, kStringFrogFace, kStringScrible, kStringScribleDescription, kStringWallet,
	kStringMenu, kStringMenuDescription, kStringCup, kStringCupDescription, kStringBill,
	// 200
	kStringBillDescription, kStringKeycard3, kStringAnnouncement, kStringAnnouncementDescription, kStringRoger,
	kStringUfo, kStringUfoDescription, kStringTray, kStringTrayDescription, kStringLamp,
	kStringLampDescription, kStringEyes, kStringEyesDescription, kStringSocketDescription, kStringMetalBlock,
	kStringMetalBlockDescription, kStringRobot, kStringRobotDescription, kStringTable, kStringTableDescription,
	kStringCellDoor, kStringCellDoorDescription, kStringLaptop, kStringWristwatch, kStringPillar,
	kStringDoorDescription1, kStringDoorDescription2, kStringDoorDescription3, kStringDoorDescription4, kStringDontEnter,
	kStringAxacussan, kStringAxacussanDescription, kStringImageDescription2, kStringMastercard, kStringMastercardDescription,
	kStringLamp2, kStringGenericDescription5, kStringMoney, kStringMoneyDescription1, kStringLocker,
	kStringLockerDescription, kStringLetter, kStringCube, kStringGenericDescription6, kStringGenericDescription7,
	kStringStrangeThing, kStringGenericDescription8, kStringImageDescription3, kStringPlant, kStringStatue,
	// 250
	kStringStatueDescription, kStringPlantDescription, kStringComputerDescription, kStringGraffiti, kStringGraffitiDescription,
	kStringMoneyDescription2, kStringJungle, kStringJungleDescription, kStringOutro1, kStringOutro2,
	kStringOutro3, kStringOutro4, kStringOutro5, kStringOutro6, kStringOutro7,
	kStringOutro8, kStringOutro9, kStringOutro10, kStringOutro11, kStringOutro12,
	kStringOutro13, kStringOutro14, kStringWireAndPlug, kStringWireAndClip, kStringWireAndPlug2,
	// 275
	kStringSignDescription2, kStringCoin, kStringDoorDescription5, kStringDoorDescription6, kStringKeycard2Description2,
	kSringSpoolAndClip, kStringIntroCutscene1, kStringIntroCutscene2, kStringIntroCutscene3, kStringIntroCutscene4,
	kStringIntroCutscene5, kStringIntroCutscene6, kStringIntroCutscene7, kStringIntroCutscene8, kStringIntroCutscene9,
	kStringIntroCutscene10, kStringIntroCutscene11, kStringIntroCutscene12, kStringIntroCutscene13, kStringIntroCutscene14,
	kStringIntroCutscene15, kStringIntroCutscene16, kStringIntroCutscene17, kStringIntroCutscene18, kStringIntroCutscene19,
	// 300
	kStringIntroCutscene20, kStringIntroCutscene21, kStringIntroCutscene22, kStringIntroCutscene23, kStringIntroCutscene24,
	kStringIntroCutscene25, kStringIntroCutscene26, kStringIntroCutscene27, kStringIntroCutscene28, kStringIntroCutscene29,
	kStringIntroCutscene30, kStringIntroCutscene31, kStringIntroCutscene32, kStringIntroCutscene33, kStringIntroCutscene34,
	kStringIntroCutscene35, kStringIntroCutscene36, kStringIntroCutscene37, kStringIntroCutscene38, kStringIntroCutscene39,
	kStringIntroCutscene40, kStringIntroCutscene41, kStringIntroCutscene42, kStringShipHall1,       kStringShipSleepCabin1,
	//325
	kStringShipSleepCabin2,  kStringShipSleepCabin3,  kStringShipSleepCabin4,  kStringShipSleepCabin5,  kStringShipSleepCabin6,
	kStringShipSleepCabin7,  kStringShipSleepCabin8,  kStringShipSleepCabin9,  kStringShipSleepCabin10, kStringShipSleepCabin11,
	kStringShipSleepCabin12, kStringShipSleepCabin13, kStringShipSleepCabin14, kStringShipSleepCabin15, kStringShipSleepCabin16,
	kStringShipCockpit1, kStringShipCockpit2, kStringShipCockpit3, kStringShipCockpit4, kStringShipCockpit5,
	kStringShipCockpit6, kStringShipCockpit7, kStringShipCockpit8, kStringShipCockpit9, kStringShipCockpit10,
	// 350
	kStringShipCockpit11, kStringShipCockpit12, kStringShipCockpit13, kStringShipCabinL3_1, kStringShipCabinL3_2,
	kStringShipCabinL3_3, kStringShipCabinL3_4, kStringShipCabinL3_5, kStringShipAirlock1, kStringShipAirlock2,
	kStringShipAirlock3, kStringShipAirlock4, kStringShipHold1, kStringCable1, kStringCable2,
	kStringCable3, kStringCable4, kStringShipHold2, kStringShipHold3, kStringShipHold4,
	kStringShipHold5, kStringShipHold6, kStringShipHold7, kStringShipHold8, kStringShipHold9,
	// 375
	kStringShipHold10, kStringShipHold11, kStringShipHold12, kStringShipHold13, kStringShipHold14,
	kStringShipHold15, kStringShipHold16, kStringArsanoMeetup1, kStringArsanoMeetup2, kStringArsanoMeetup3,
	kStringArsanoEntrance1, kStringArsanoEntrance2, kStringArsanoEntrance3, kStringArsanoEntrance4, kStringArsanoEntrance5,
	kStringArsanoEntrance6, kStringArsanoEntrance7, kStringArsanoEntrance8, kStringArsanoEntrance9, kStringArsanoEntrance10,
	kStringArsanoEntrance11, kStringArsanoEntrance12, kStringArsanoEntrance13, kStringArsanoEntrance14, kStringArsanoEntrance15,
	// 400
	kStringArsanoEntrance16, kStringArsanoEntrance17, kStringArsanoEntrance18, kStringArsanoEntrance19, kStringArsanoEntrance20,
	kStringArsanoEntrance21, kStringArsanoEntrance22, kStringArsanoEntrance23, kStringArsanoEntrance24, kStringArsanoEntrance25,
	kStringArsanoEntrance26, kStringArsanoEntrance27, kStringArsanoDialog1, kStringArsanoDialog2, kStringArsanoDialog3,
	kStringArsanoDialog4, kStringArsanoDialog5, kStringArsanoDialog6, kStringArsanoDialog7, kStringArsanoDialog8,
	kStringArsanoDialog9, kStringDialogSeparator, kStringDialogArsanoRoger1, kStringDialogArsanoRoger2, kStringDialogArsanoRoger3,
	// 425
	kStringDialogArsanoMeetup3_1, kStringDialogArsanoMeetup3_2, kStringDialogArsanoMeetup3_3, kStringDialogArsanoMeetup3_4, kStringDialogArsanoMeetup3_5,
	kStringArsanoRoger1, kStringArsanoRoger2, kStringArsanoRoger3, kStringArsanoRoger4, kStringArsanoRoger5,
	kStringArsanoRoger6, kStringArsanoRoger7, kStringArsanoRoger8, kStringArsanoRoger9, kStringArsanoRoger10,
	kStringArsanoRoger11, kStringArsanoRoger12, kStringArsanoRoger13, kStringArsanoRoger14, kStringArsanoRoger15,
	kStringArsanoRoger16, kStringArsanoRoger17, kStringArsanoRoger18, kStringArsanoRoger19, kStringArsanoRoger20,
	// 450
	kStringArsanoRoger21, kStringArsanoRoger22, kStringArsanoRoger23, kStringArsanoRoger24, kStringArsanoRoger25,
	kStringArsanoRoger26, kStringArsanoRoger27, kStringArsanoRoger28, kStringArsanoRoger29, kStringArsanoRoger30,
	kStringArsanoRoger31, kStringArsanoRoger32, kStringArsanoRoger33, kStringArsanoRoger34, kStringArsanoRoger35,
	kStringArsanoRoger36, kStringArsanoRoger37, kStringArsanoRoger38, kStringArsanoRoger39, kStringArsanoRoger40,
	kStringArsanoGlider1, kStringArsanoMeetup2_1, kStringArsanoMeetup2_2, kStringArsanoMeetup2_3, kStringArsanoMeetup2_4,
	// 475
	kStringArsanoMeetup2_5, kStringArsanoMeetup2_6, kStringArsanoMeetup2_7, kStringArsanoMeetup2_8, kStringArsanoMeetup2_9,
	kStringArsanoMeetup2_10, kStringArsanoMeetup2_11, kStringArsanoMeetup2_12, kStringArsanoMeetup2_13, kStringArsanoMeetup3_1,
	kStringArsanoMeetup3_2, kStringArsanoMeetup3_3, kStringArsanoMeetup3_4, kStringArsanoMeetup3_5, kStringArsanoMeetup3_6,
	kStringArsanoMeetup3_7, kStringArsanoMeetup3_8, kStringArsanoMeetup3_9, kStringArsanoMeetup3_10, kStringArsanoMeetup3_11,
	kStringArsanoMeetup3_12, kStringArsanoMeetup3_13, kStringArsanoMeetup3_14, kStringArsanoMeetup3_15, kStringArsanoMeetup3_16,
	// 500
	kStringArsanoMeetup3_17, kStringArsanoMeetup3_18, kStringArsanoMeetup3_19, kStringArsanoMeetup3_20, kStringArsanoMeetup3_21,
	kStringArsanoMeetup3_22, kStringArsanoMeetup3_23, kStringArsanoMeetup3_24, kStringArsanoMeetup3_25, kStringArsanoMeetup3_26,
	kStringArsanoMeetup3_27, kStringArsanoMeetup3_28, kStringAxacussCell_1, kStringAxacussCell_2, kStringAxacussCell_3,
	kStringAxacussCell_4, kStringAxacussCell_5, kStringOk, kStringDialogArsanoMeetup2_1, kStringDialogArsanoMeetup2_2,
	kStringDialogArsanoMeetup2_3, kStringDialogArsanoMeetup2_4, kStringDialogArsanoMeetup2_5, kStringDialogArsanoMeetup2_6, kStringDialogArsanoMeetup2_7,
	// 525
	kStringDialogArsanoMeetup2_8, kStringDialogArsanoMeetup2_9, kStringDialogArsanoMeetup2_10, kStringDialogArsanoMeetup2_11, kStringDialogAxacussCorridor5_1,
	kStringDialogAxacussCorridor5_2, kStringDialogAxacussCorridor5_3, kStringDialogAxacussCorridor5_4, kStringDialogAxacussCorridor5_5, kStringDialogAxacussCorridor5_6,
	kStringDialogAxacussCorridor5_7, kStringDialogX1, kStringDialogX2, kStringDialogX3, kStringAxacussCorridor5_1,
	kStringAxacussCorridor5_2, kStringAxacussCorridor5_3, kStringAxacussCorridor5_4, kStringAxacussCorridor5_5, kStringAxacussCorridor5_6,
	kStringAxacussCorridor5_7, kStringAxacussBcorridor_1, kStringAxacussOffice1_1, kStringAxacussOffice1_2, kStringAxacussOffice1_3,
	// 550
	kStringAxacussOffice1_4, kStringAxacussOffice1_5, kStringAxacussOffice1_6, kStringAxacussOffice1_7, kStringAxacussOffice1_8,
	kStringAxacussOffice1_9, kStringAxacussOffice1_10, kStringAxacussOffice1_11, kStringAxacussOffice1_12, kStringAxacussOffice1_13,
	kStringAxacussOffice1_14, kStringAxacussOffice1_15, kStringAxacussOffice1_16, kStringAxacussOffice3_1, kStringAxacussElevator_1,
	kStringAxacussElevator_2, kStringAxacussElevator_3, kStringShock, kStringShot, kStringCloseLocker_1,
	kStringIsHelmetOff_1, kStringGenericInteract_1, kStringGenericInteract_2, kStringGenericInteract_3, kStringGenericInteract_4,
	// 575
	kStringGenericInteract_5, kStringGenericInteract_6, kStringGenericInteract_7, kStringGenericInteract_8, kStringGenericInteract_9,
	kStringGenericInteract_10, kStringGenericInteract_11, kStringGenericInteract_12, kPhrasalVerbParticleGiveTo, kPhrasalVerbParticleUseWith,
	kStringGenericInteract_13, kStringGenericInteract_14, kStringGenericInteract_15, kStringGenericInteract_16, kStringGenericInteract_17,
	kStringGenericInteract_18, kStringGenericInteract_19, kStringGenericInteract_20, kStringGenericInteract_21, kStringGenericInteract_22,
	kStringGenericInteract_23, kStringGenericInteract_24, kStringGenericInteract_25, kStringGenericInteract_26, kStringGenericInteract_27,
	// 600
	kStringGenericInteract_28, kStringGenericInteract_29, kStringGenericInteract_30, kStringGenericInteract_31, kStringGenericInteract_32,
	kStringGenericInteract_33, kStringGenericInteract_34, kStringGenericInteract_35, kStringGenericInteract_36, kStringGenericInteract_37,
	kStringGenericInteract_38, kStringGenericInteract_39, kStringGenericInteract_40, kStringGenericInteract_41, kStringGenericInteract_42,
	kStringGenericInteract_43, kStringConversationEnd, kStringSupernova1, kStringSupernova2, kStringSupernova3,
	kStringSupernova4, kStringSupernova5, kStringSupernova6, kStringSupernova7, kStringSupernova8
	// 625
};

static StringID guiCommands[] = {
	kStringCommandGo, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
	kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive
};

static StringID guiStatusCommands[] = {
	kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen, kStringStatusCommandClose,
	kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk, kStringStatusCommandGive
};

ObjectType operator|(ObjectType a, ObjectType b);
ObjectType operator&(ObjectType a, ObjectType b);
ObjectType operator^(ObjectType a, ObjectType b);
ObjectType &operator|=(ObjectType &a, ObjectType b);
ObjectType &operator&=(ObjectType &a, ObjectType b);
ObjectType &operator^=(ObjectType &a, ObjectType b);

struct Object {
	static const Object nullObject;

	Object()
		: _name(kNoString)
		, _description(kStringDefaultDescription)
		, _id(INVALIDOBJECT)
		, _roomId(NULLROOM)
		, _type(NULLTYPE)
		, _click(0)
		, _click2(0)
		, _section(0)
		, _exitRoom(NULLROOM)
		, _direction(0)
	{}
	Object(byte roomId, StringID name, StringID description, ObjectID id, ObjectType type,
	       byte click, byte click2, byte section = 0, RoomID exitRoom = NULLROOM, byte direction = 0)
		: _name(name)
		, _description(description)
		, _id(id)
		, _roomId(roomId)
		, _type(type)
		, _click(click)
		, _click2(click2)
		, _section(section)
		, _exitRoom(exitRoom)
		, _direction(direction)
	{}

	static void setObjectNull(Object *&obj) {
		obj = const_cast<Object *>(&nullObject);
	}
	static bool isNullObject(Object *obj) {
		return obj == &nullObject;
	}
	void resetProperty(ObjectType type = NULLTYPE) {
		_type = type;
	}

	void setProperty(ObjectType type) {
		_type |= type;
	}

	void disableProperty(ObjectType type) {
		_type ^= type;
	}

	bool hasProperty(ObjectType type) const {
		return _type & type;
	}

	static bool combine(Object &obj1, Object &obj2, ObjectID id1, ObjectID id2) {
		if (obj1.hasProperty(COMBINABLE))
			return (((obj1._id == id1) && (obj2._id == id2)) ||
			        ((obj1._id == id2) && (obj2._id == id1)));
		else
			return false;
	}

	byte _roomId;
	StringID _name;
	StringID _description;
	ObjectID _id;
	ObjectType _type;
	byte _click;
	byte _click2;
	byte _section;
	RoomID _exitRoom;
	byte _direction;
};

#define ticksToMsec(x) (x * kMsecPerTick)

}

#endif // MSN_DEF_H
