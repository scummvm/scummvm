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
 */

#include "tinsel/actors.h"
#include "tinsel/dw.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/object.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

void Font::SetTagFontHandle(SCNHANDLE hFont) {
	_hTagFont = _hRegularTagFont = hFont;
	if (TinselV0)
		SetTalkFontHandle(hFont);	// Also re-use for talk text
}

SCNHANDLE Font::GetTalkFontHandle() {
	// FIXME: Mac Discworld 1 was rendering the talk font as black.
	// Currently hacking around it by simply using the tooltip font
	return TinselV1Mac ? _hTagFont : _hTalkFont;
}

void Font::FettleFontPal(SCNHANDLE fontPal) {
	IMAGE *pImg;

	assert(fontPal);
	assert(_hTagFont); // Tag font not declared
	assert(_hTalkFont); // Talk font not declared

	pImg = (IMAGE *)_vm->_handle->LockMem(_vm->_handle->GetFontImageHandle(_hTagFont)); // get image for char 0
	if (!TinselV2)
		pImg->hImgPal = TO_32(fontPal);
	else
		pImg->hImgPal = 0;

	pImg = (IMAGE *)_vm->_handle->LockMem(_vm->_handle->GetFontImageHandle(_hTalkFont)); // get image for char 0
	if (!TinselV2)
		pImg->hImgPal = TO_32(fontPal);
	else
		pImg->hImgPal = 0;

	if (TinselV2 && SysVar(SV_TAGCOLOR)) {
		const COLORREF c = _vm->_actor->GetActorRGB(-1);
		SetTagColorRef(c);
		UpdateDACqueue(SysVar(SV_TAGCOLOR), c);
	}
}

} // End of namespace Tinsel
