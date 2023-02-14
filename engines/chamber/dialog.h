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

#ifndef CHAMBER_DIALOG_H
#define CHAMBER_DIALOG_H

namespace Chamber {

extern uint16 cur_str_index;
extern uint16 cur_dlg_index;

enum DirtyRectKind {
	DirtyRectFree = 0,
	DirtyRectBubble = 1,    /*bubble with spike*/
	DirtyRectSprite = 2,    /*portrait*/
	DirtyRectText = 3       /*text bubble w/o spike*/
};

typedef struct dirty_rect_t {
	byte kind;
	uint16 offs;
	byte height;
	byte width;
	byte y;        /*for DirtyRectBubble this is spike offs*/
	byte x;
} dirty_rect_t;

#define MAX_DIRTY_RECT 10
extern dirty_rect_t dirty_rects[];
extern dirty_rect_t *last_dirty_rect;

#define SPIKE_MASK     0xE0
#define SPIKE_UPLEFT   0
#define SPIKE_UPRIGHT  0x20
#define SPIKE_DNRIGHT  0x80
#define SPIKE_DNLEFT   0xA0
#define SPIKE_BUBBLES  0x40
#define SPIKE_BUBRIGHT 0xC0
#define SPIKE_BUBLEFT  0xE0

void addDirtyRect(byte kind, byte x, byte y, byte w, byte h, uint16 offs);
void getDirtyRectAndFree(int16 index, byte *kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs);
void getDirtyRectAndSetSprite(int16 index, byte *kind, byte *x, byte *y, byte *w, byte *h, uint16 *offs);

void popDirtyRects(byte kind);
void drawPersonBubble(byte x, byte y, byte flags, byte *msg);
void desciTextBox(uint16 x, uint16 y, uint16 width, byte *msg);

void promptWait(void);

byte *seekToString(byte *bank, uint16 num);
byte *seekToStringScr(byte *bank, uint16 num, byte **ptr);

} // End of namespace Chamber

#endif
