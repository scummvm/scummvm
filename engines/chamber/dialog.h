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

extern unsigned int cur_str_index;
extern unsigned int cur_dlg_index;

enum DirtyRectKind {
	DirtyRectFree = 0,
	DirtyRectBubble = 1,    /*bubble with spike*/
	DirtyRectSprite = 2,    /*portrait*/
	DirtyRectText = 3       /*text bubble w/o spike*/
};

typedef struct dirty_rect_t {
	unsigned char kind;
	unsigned int offs;
	unsigned char height;
	unsigned char width;
	unsigned char y;        /*for DirtyRectBubble this is spike offs*/
	unsigned char x;
} dirty_rect_t;

#define MAX_DIRTY_RECT 10
extern dirty_rect_t dirty_rects[];

#define SPIKE_MASK     0xE0
#define SPIKE_UPLEFT   0
#define SPIKE_UPRIGHT  0x20
#define SPIKE_DNRIGHT  0x80
#define SPIKE_DNLEFT   0xA0
#define SPIKE_BUBBLES  0x40
#define SPIKE_BUBRIGHT 0xC0
#define SPIKE_BUBLEFT  0xE0

void AddDirtyRect(unsigned char kind, unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned int offs);
void GetDirtyRectAndFree(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs);
void GetDirtyRectAndSetSprite(int index, unsigned char *kind, unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h, unsigned int *offs);

void PopDirtyRects(unsigned char kind);
void DrawPersonBubble(unsigned char x, unsigned char y, unsigned char flags, unsigned char *msg);
void DesciTextBox(unsigned int x, unsigned int y, unsigned int width, unsigned char *msg);

void PromptWait(void);

unsigned char *SeekToString(unsigned char *bank, unsigned int num);
unsigned char *SeekToStringScr(unsigned char *bank, unsigned int num, unsigned char **ptr);

} // End of namespace Chamber

#endif
