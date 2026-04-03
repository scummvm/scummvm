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

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

extern ZBasic *g_zbasic;
extern Toolbox *g_toolbox;

void FoolGame::sub_141_004() {
	// 141:0004
	g_zbasic->get(0x6c, 0x127, 0x84, 0x127, this->arr_bmp_c38c);
	this->sub_128_55c(g_zbasic->str(334));
	g_toolbox->InvertRect(this->arr_rect_1910c);
	g_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_b3ec);
	g_zbasic->put(0x6c, 0x127, this->arr_bmp_c38c, kPutCopy);
	this->sub_141_934();
	this->fillRect(0x14f, 0, 0x156, 0x7, 0x2);
	this->fillRect(0x14f, 0x1f9, 0x156, 0x200, 2);
	this->arr_rect_1eb8.top = 0x1e;
	this->arr_rect_1eb8.left = 0x190;
	this->arr_rect_1eb8.bottom = 0x2e;
	this->arr_rect_1eb8.right = 0x1a8;
	// 141:0122
	g_zbasic->get(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_rect_1eb8.right, this->arr_rect_1eb8.bottom, this->arr_bmp_bbbc);
	g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kPutCopy);
	this->var_ev_46.where.y = 0x14;
	// JMP 0x3c2
	while (true) {
		// 141:01c8
		// JMP 0x232
		while (this->var_ev_46.where.y >= 0x14) {
			// 141:01cc
			this->sub_128_c6a(-1);
			this->sub_141_3ca();
			if (g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1eb8)) {
				this->sub_141_8b8();
				this->sub_141_9f6();
			}
			// 141:0200
			if (this->var_i16_7c6 == 2) {
				this->var_i16_7c6 = 0;
			}
			if (this->var_i16_7c6 == 4) {
				this->sub_128_3536();
			}
			if ((this->var_i16_7c6 & 1)) {
				// 141:09f6
				this->sub_141_882();
				return;
			}
			if (this->var_ev_46.what == kNullEvent)
				g_toolbox->Delay(0);
		}
		// 141:023a
		g_zbasic->get(0x6c, 0x127, 0x84, 0x137, this->arr_bmp_c38c);
		this->sub_128_55c(g_zbasic->str(335)); // "~"
		this->var_i16_233a = 1;
		// JMP 0x2be
		while (this->var_ev_46.where.y < 0x14) {
			// 141:0282
			this->sub_141_3ca();
			this->sub_128_c6a(-1);
			if (this->var_i16_7c6 == 2) {
				this->var_i16_7c6 = 0;
			}
			if (this->var_i16_7c6 == 4) {
				this->sub_128_3536();
			}

		}
		// 141:02c6
		this->sub_128_c6a(0);
		if (!g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1910c)) {
			this->sub_141_35a();
		}
		this->var_i32_692 = g_toolbox->TickCount();
		// 141:02fa
		do {
			this->sub_128_c6a(0);
			if (((this->var_ev_46.modifiers & kModMouseButtonUp) == 0) &&
				g_toolbox->PtInRect(this->var_ev_46.where, this->arr_rect_1910c)) {
				// JMP 0x9a2
				// 141:09a2
				this->sub_141_882();
				g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_bbbc, kPutCopy);
				this->sub_141_934();
				if (this->var_i16_c04 < 0x64) {
					this->var_i16_c04 = 0x64;
				}
				this->sub_141_882();
				return;
			}
			// 141:0342
		} while (g_toolbox->TickCount() <= (this->var_i32_692 + 0x2d));
		// 141:035a
		g_toolbox->PenNormal();
		g_toolbox->PenMode(kNotPatXor);
		g_toolbox->PenPat(this->arr_pat_58f4[1]);
		g_toolbox->PenSize(0x17, 0x10);
		for (int i = 0; i <= 5; i++) {
			g_toolbox->MoveTo(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top);
			g_toolbox->LineTo(0x6c, 0x127);
		}
		// 141:03bc
		this->sub_141_882();
		g_toolbox->PenNormal();
	}
	// 141:03c2
	this->sub_141_3ca();
}

void FoolGame::sub_141_3ca() {
	g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.left, this->arr_bmp_b3ec, kPutCopy);
	this->var_i16_68a = this->var_ev_46.where.x - 0xc - this->arr_rect_1eb8.left;
	this->var_i16_68c = this->var_ev_46.where.y - 8 - this->arr_rect_1eb8.top;
	// 141:0452
	if (this->var_i16_68c < -4) {
		this->var_i16_68c = -4;
	}
	if (this->var_i16_68c > 4) {
		this->var_i16_68c = 4;
	}
	if (this->var_i16_68a < -6) {
		this->var_i16_68a = -6;
	}
	if (this->var_i16_68a > 6) {
		this->var_i16_68a = 6;
	}
	g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kPutCopy);
	// 141:04ce
	this->arr_rect_1ec0.top = this->arr_rect_1eb8.top + this->var_i16_68c;
	this->arr_rect_1ec0.left = this->arr_rect_1eb8.left + this->var_i16_68a;
	this->arr_rect_1ec0.bottom = this->arr_rect_1eb8.bottom + this->var_i16_68c;
	this->arr_rect_1ec0.right = this->arr_rect_1eb8.right + this->var_i16_68a;

	g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kPutCopy);
	// 141:05a2
	if (this->arr_rect_1ec0.top < 0x14) {
		this->arr_rect_1ec0.top = 0x14;
		this->arr_rect_1ec0.bottom = 0x24;
	}
	// 141:05dc
	if (this->arr_rect_1ec0.left < 0) {
		this->arr_rect_1ec0.left = 0;
		this->arr_rect_1ec0.right = 0x18;
	}
	// 141:060c
	if (this->arr_rect_1ec0.bottom > SCREEN_HEIGHT) {
		this->arr_rect_1ec0.top = 0x146;
		this->arr_rect_1ec0.bottom = 0x156;
	}
	if (this->arr_rect_1ec0.right > SCREEN_WIDTH) {
		this->arr_rect_1ec0.left = 0x1e8;
		this->arr_rect_1ec0.right = 0x200;
	}
	g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_b3ec, kPutCopy);
	// 141:06bc
	if (!((this->arr_rect_1eb8.top == this->arr_rect_1ec0.top) &&
		((this->arr_rect_1eb8.left == this->arr_rect_1ec0.left)))) {
		g_zbasic->put(this->arr_rect_1eb8.left, this->arr_rect_1eb8.top, this->arr_bmp_bbbc, kPutCopy);
		this->arr_rect_1eb8 = this->arr_rect_1ec0;

	}
	// 141:0880
	warning("STUB: %s", __func__);
}

void FoolGame::sub_141_882() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_141_8b8() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_141_934() {
	warning("STUB: %s", __func__);
}

void FoolGame::sub_141_9f6() {
	warning("STUB: %s", __func__);
}

}
