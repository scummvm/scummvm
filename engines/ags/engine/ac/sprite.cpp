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

#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/sprite.h"
#include "ac/system.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/agsplugin.h"
#include "plugin/plugin_engine.h"
#include "ac/spritecache.h"
#include "gfx/bitmap.h"
#include "gfx/graphicsdriver.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern SpriteCache spriteset;
extern int our_eip, eip_guinum, eip_guiobj;
extern color palette[256];
extern IGraphicsDriver *gfxDriver;
extern AGSPlatformDriver *platform;

void get_new_size_for_sprite(int ee, int ww, int hh, int &newwid, int &newhit) {
	newwid = ww;
	newhit = hh;
	const SpriteInfo &spinfo = game.SpriteInfos[ee];
	if (!game.AllowRelativeRes() || !spinfo.IsRelativeRes())
		return;
	ctx_data_to_game_size(newwid, newhit, spinfo.IsLegacyHiRes());
}

// set any alpha-transparent pixels in the image to the appropriate
// RGB mask value so that the blit calls work correctly
void set_rgb_mask_using_alpha_channel(Bitmap *image) {
	int x, y;

	for (y = 0; y < image->GetHeight(); y++) {
		unsigned int *psrc = (unsigned int *)image->GetScanLine(y);

		for (x = 0; x < image->GetWidth(); x++) {
			if ((psrc[x] & 0xff000000) == 0x00000000)
				psrc[x] = MASK_COLOR_32;
		}
	}
}

// from is a 32-bit RGBA image, to is a 15/16/24-bit destination image
Bitmap *remove_alpha_channel(Bitmap *from) {
	const int game_cd = game.GetColorDepth();
	Bitmap *to = BitmapHelper::CreateBitmap(from->GetWidth(), from->GetHeight(), game_cd);
	const int maskcol = to->GetMaskColor();
	int y, x;
	unsigned int c, b, g, r;

	if (game_cd == 24) { // 32-to-24
		for (y = 0; y < from->GetHeight(); y++) {
			unsigned int *psrc = (unsigned int *)from->GetScanLine(y);
			unsigned char *pdest = (unsigned char *)to->GetScanLine(y);

			for (x = 0; x < from->GetWidth(); x++) {
				c = psrc[x];
				// less than 50% opaque, remove the pixel
				if (((c >> 24) & 0x00ff) < 128)
					c = maskcol;

				// copy the RGB values across
				memcpy(&pdest[x * 3], &c, 3);
			}
		}
	} else if (game_cd > 8) { // 32 to 15 or 16
		for (y = 0; y < from->GetHeight(); y++) {
			unsigned int *psrc = (unsigned int *)from->GetScanLine(y);
			unsigned short *pdest = (unsigned short *)to->GetScanLine(y);

			for (x = 0; x < from->GetWidth(); x++) {
				c = psrc[x];
				// less than 50% opaque, remove the pixel
				if (((c >> 24) & 0x00ff) < 128)
					pdest[x] = maskcol;
				else {
					// otherwise, copy it across
					r = (c >> 16) & 0x00ff;
					g = (c >> 8) & 0x00ff;
					b = c & 0x00ff;
					pdest[x] = makecol_depth(game_cd, r, g, b);
				}
			}
		}
	} else { // 32 to 8-bit game
		// TODO: consider similar to above approach if this becomes a wanted feature
		to->Blit(from);
	}
	return to;
}

void pre_save_sprite(int ee) {
	// not used, we don't save
}

// these vars are global to help with debugging
Bitmap *tmpdbl, *curspr;
int newwid, newhit;
void initialize_sprite(int ee) {

	if ((ee < 0) || (ee > spriteset.GetSpriteSlotCount()))
		quit("initialize_sprite: invalid sprite number");

	if ((spriteset[ee] == nullptr) && (ee > 0)) {
		// replace empty sprites with blue cups, to avoid crashes
		spriteset.RemapSpriteToSprite0(ee);
	} else if (spriteset[ee] == nullptr) {
		game.SpriteInfos[ee].Width = 0;
		game.SpriteInfos[ee].Height = 0;
	} else {
		// stretch sprites to correct resolution
		int oldeip = our_eip;
		our_eip = 4300;

		if (game.SpriteInfos[ee].Flags & SPF_HADALPHACHANNEL) {
			// we stripped the alpha channel out last time, put
			// it back so that we can remove it properly again
			game.SpriteInfos[ee].Flags |= SPF_ALPHACHANNEL;
		}

		curspr = spriteset[ee];
		get_new_size_for_sprite(ee, curspr->GetWidth(), curspr->GetHeight(), newwid, newhit);

		eip_guinum = ee;
		eip_guiobj = newwid;

		if ((newwid != curspr->GetWidth()) || (newhit != curspr->GetHeight())) {
			tmpdbl = BitmapHelper::CreateTransparentBitmap(newwid, newhit, curspr->GetColorDepth());
			if (tmpdbl == nullptr)
				quit("Not enough memory to load sprite graphics");
			tmpdbl->Acquire();
			curspr->Acquire();
			tmpdbl->StretchBlt(curspr, RectWH(0, 0, tmpdbl->GetWidth(), tmpdbl->GetHeight()), Common::kBitmap_Transparency);
			curspr->Release();
			tmpdbl->Release();
			delete curspr;
			spriteset.SubstituteBitmap(ee, tmpdbl);
		}

		game.SpriteInfos[ee].Width = spriteset[ee]->GetWidth();
		game.SpriteInfos[ee].Height = spriteset[ee]->GetHeight();

		spriteset.SubstituteBitmap(ee, PrepareSpriteForUse(spriteset[ee], (game.SpriteInfos[ee].Flags & SPF_ALPHACHANNEL) != 0));

		if (game.GetColorDepth() < 32) {
			game.SpriteInfos[ee].Flags &= ~SPF_ALPHACHANNEL;
			// save the fact that it had one for the next time this
			// is re-loaded from disk
			game.SpriteInfos[ee].Flags |= SPF_HADALPHACHANNEL;
		}

		pl_run_plugin_hooks(AGSE_SPRITELOAD, ee);
		update_polled_stuff_if_runtime();

		our_eip = oldeip;
	}
}

} // namespace AGS3
