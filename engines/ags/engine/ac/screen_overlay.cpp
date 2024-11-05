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

#include "common/std/utility.h"
#include "ags/engine/ac/dynamic_sprite.h"
#include "ags/engine/ac/screen_overlay.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

ScreenOverlay::ScreenOverlay(ScreenOverlay &&over) {
	*this = std::move(over);
}

ScreenOverlay::~ScreenOverlay() {
	if (_sprnum > 0 && !IsSpriteShared())
		free_dynamic_sprite(_sprnum, false);
}

// TODO: this may be avoided if we somehow make (dynamic) sprites reference counted when assigning ID too
ScreenOverlay &ScreenOverlay::operator=(ScreenOverlay &&over) {
	*this = over;
	over._sprnum = 0;
	return *this;
}

void ScreenOverlay::ResetImage() {
	if (_sprnum > 0 && !IsSpriteShared())
		free_dynamic_sprite(_sprnum, false);
	_sprnum = 0;
	_flags &= ~(kOver_SpriteShared | kOver_AlphaChannel);
	scaleWidth = scaleHeight = offsetX = offsetY = 0;
}

Bitmap *ScreenOverlay::GetImage() const {
	return _GP(spriteset)[_sprnum];
}

Size ScreenOverlay::GetGraphicSize() const {
	return Size(_GP(game).SpriteInfos[_sprnum].Width, _GP(game).SpriteInfos[_sprnum].Height);
}

void ScreenOverlay::SetImage(std::unique_ptr<Shared::Bitmap> pic, bool has_alpha, int offx, int offy) {
	ResetImage();
	if (pic) {
		_flags |= kOver_AlphaChannel * has_alpha;
		offsetX = offx;
		offsetY = offy;
		scaleWidth = pic->GetWidth();
		scaleHeight = pic->GetHeight();
		_sprnum = add_dynamic_sprite(std::move(pic), has_alpha);
	}
	MarkChanged();
}

void ScreenOverlay::SetSpriteNum(int sprnum, int offx, int offy) {
	ResetImage();

	assert(sprnum >= 0 && sprnum < (int)_GP(game).SpriteInfos.size());
	if (sprnum < 0 || sprnum >= (int)_GP(game).SpriteInfos.size())
		return;

	_flags |= kOver_SpriteShared | kOver_AlphaChannel * ((_GP(game).SpriteInfos[sprnum].Flags & SPF_ALPHACHANNEL) != 0);
	_sprnum = sprnum;
	offsetX = offx;
	offsetY = offy;
	scaleWidth = _GP(game).SpriteInfos[sprnum].Width;
	scaleHeight = _GP(game).SpriteInfos[sprnum].Height;
	MarkChanged();
}

void ScreenOverlay::ReadFromSavegame(Stream *in, bool &has_bitmap, int32_t cmp_ver) {
	ResetImage();
	in->ReadInt32(); // ddb 32-bit pointer value (nasty legacy format)
	int pic = in->ReadInt32();
	type = in->ReadInt32();
	x = in->ReadInt32();
	y = in->ReadInt32();
	timeout = in->ReadInt32();
	bgSpeechForChar = in->ReadInt32();
	associatedOverlayHandle = in->ReadInt32();
	if (cmp_ver >= kOverSvgVersion_36025) {
		_flags = in->ReadInt16();
	} else {
		if (in->ReadBool()) // has alpha
			_flags |= kOver_AlphaChannel;
		if (!(in->ReadBool())) // screen relative position
			_flags |= kOver_PositionAtRoomXY;
	}

	if (cmp_ver < 0) {
		in->ReadInt16(); // alignment padding to int32 (finalize struct)
	}
	if (cmp_ver >= kOverSvgVersion_35028) {
		offsetX = in->ReadInt32();
		offsetY = in->ReadInt32();
	}
	if (cmp_ver >= kOverSvgVersion_36008) {
		zorder = in->ReadInt32();
		transparency = in->ReadInt32();
		scaleWidth = in->ReadInt32();
		scaleHeight = in->ReadInt32();
	}

	// New saves always save overlay images as a part of the dynamicsprite set;
	// old saves could contain images saved along with overlays
	if ((cmp_ver >= kOverSvgVersion_36108) || (_flags & kOver_SpriteShared)) {
		_sprnum = pic;
		has_bitmap = false;
	} else {
		_sprnum = 0;
		has_bitmap = pic != 0;
	}
}

void ScreenOverlay::WriteToSavegame(Stream *out) const {
	out->WriteInt32(0); // ddb 32-bit pointer value (nasty legacy format)
	out->WriteInt32(_sprnum); // sprite id
	out->WriteInt32(type);
	out->WriteInt32(x);
	out->WriteInt32(y);
	out->WriteInt32(timeout);
	out->WriteInt32(bgSpeechForChar);
	out->WriteInt32(associatedOverlayHandle);
	out->WriteInt16(_flags);
	// since cmp_ver = 1
	out->WriteInt32(offsetX);
	out->WriteInt32(offsetY);
	// since cmp_ver = 2
	out->WriteInt32(zorder);
	out->WriteInt32(transparency);
	out->WriteInt32(scaleWidth);
	out->WriteInt32(scaleHeight);
}

} // namespace AGS3
