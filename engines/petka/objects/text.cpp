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

#include "graphics/fontman.h"
#include "graphics/font.h"

#include "petka/q_manager.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/objects/text.h"

namespace Petka {

QText::QText(const Common::U32String &text, uint32 rgb) {
	_resourceId = -2;
	_z = 3000;
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	_rect = font->getBoundingBox("TEST FONT", 0, 0, 0);

	Graphics::Surface *s = g_vm->resMgr()->findOrCreateSurface(-2, _rect.width(), _rect.height());
	font->drawString(s, "TEST FONT", 0, 0, _rect.width(), rgb);
}

void QText::draw() {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	const Graphics::Surface *s = g_vm->resMgr()->loadBitmap(_resourceId);
	g_vm->videoSystem()->screen().transBlitFrom(*s, Common::Point((640 - _rect.width()) / 2, 480 - _rect.height()));
}

} // End of namespace Petka
