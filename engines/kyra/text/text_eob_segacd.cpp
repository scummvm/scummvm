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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/engine/eob.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/text/text_eob_segacd.h"

namespace Kyra {

TextDisplayer_SegaCD::TextDisplayer_SegaCD(EoBEngine *engine, Screen_EoB *scr) : TextDisplayer_rpg(engine, scr), _renderer(scr->sega_getRenderer()), _curDim(0) {
	assert(_renderer);
}

TextDisplayer_SegaCD::~TextDisplayer_SegaCD() {

}

int TextDisplayer_SegaCD::clearDim(int dim) {
	int res = _curDim;
	_curDim = dim;
	const ScreenDim *s = &_dimTable[dim];
	uint32 size = (s->w * s->h) >> 1;
	uint8 *buf = new uint8[size];
	memset(buf, s->unkA, size);
	_renderer->loadToVRAM(buf, size, (s->unkC & 0x7FF) << 5);
	delete[] buf;
	return res;
}

const ScreenDim TextDisplayer_SegaCD::_dimTable[3] = {
	{ 0x0001, 0x0017, 0x0118, 0x0018, 0xff, 0x44, 0x2597, 0x0000 },
	{ 0x0012, 0x0009, 0x00a0, 0x0080, 0xff, 0x00, 0x0153, 0x0028 },
	{ 0x0001, 0x0014, 0x0130, 0x0030, 0xff, 0xee, 0xe51c, 0x0000 }
};

} // End of namespace Kyra

#endif // (ENABLE_EOB || ENABLE_LOL)
