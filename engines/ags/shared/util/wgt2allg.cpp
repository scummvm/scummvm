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

#include "ags/shared/core/assetmanager.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/gfx/allegrobitmap.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/lib/allegro.h"

namespace AGS3 {

using namespace AGS::Shared;

void wsetrgb(int coll, int r, int g, int b, color *pall) {
	pall[coll].r = r;
	pall[coll].g = g;
	pall[coll].b = b;
}

void wcolrotate(unsigned char start, unsigned char finish, int dir, color *pall) {
	int jj;
	if (dir == 0) {
		// rotate left
		color tempp = pall[start];

		for (jj = start; jj < finish; jj++)
			pall[jj] = pall[jj + 1];

		pall[finish] = tempp;
	} else {
		// rotate right
		color tempp = pall[finish];

		for (jj = finish - 1; jj >= start; jj--)
			pall[jj + 1] = pall[jj];

		pall[start] = tempp;
	}
}

Bitmap *wnewblock(Shared::Bitmap *src, int x1, int y1, int x2, int y2) {
	Bitmap *tempbitm;
	int twid = (x2 - x1) + 1, thit = (y2 - y1) + 1;

	if (twid < 1)
		twid = 1;

	if (thit < 1)
		thit = 1;

	tempbitm = BitmapHelper::CreateBitmap(twid, thit);

	if (tempbitm == nullptr)
		return nullptr;

	tempbitm->Blit(src, x1, y1, 0, 0, tempbitm->GetWidth(), tempbitm->GetHeight());
	return tempbitm;
}

int wloadsprites(color *pall, char *filnam, Bitmap **sarray, int strt, int eend) {
	int vers;
	char buff[20];
	int numspri = 0, vv, hh, wdd, htt;

	Stream *in = Shared::AssetManager::OpenAsset(filnam);
	if (in == nullptr)
		return -1;

	vers = in->ReadInt16();
	in->ReadArray(&buff[0], 13, 1);
	for (vv = 0; vv < 256; vv++)        // there's a filler byte
		in->ReadArray(&pall[vv], 3, 1);

	if (vers > 4)
		return -1;

	if (vers == 4)
		numspri = in->ReadInt16();
	else {
		numspri = in->ReadInt16();
		if ((numspri < 2) || (numspri > 200))
			numspri = 200;
	}

	for (vv = strt; vv <= eend; vv++)
		sarray[vv] = nullptr;

	for (vv = 0; vv <= numspri; vv++) {
		int coldep = in->ReadInt16();

		if (coldep == 0) {
			sarray[vv] = nullptr;
			if (in->EOS())
				break;

			continue;
		}

		if (in->EOS())
			break;

		if (vv > eend)
			break;

		wdd = in->ReadInt16();
		htt = in->ReadInt16();
		if (vv < strt) {
			in->Seek(wdd * htt);
			continue;
		}
		sarray[vv] = BitmapHelper::CreateBitmap(wdd, htt, coldep * 8);

		if (sarray[vv] == nullptr) {
			delete in;
			return -1;
		}

		for (hh = 0; hh < htt; hh++)
			in->ReadArray(&sarray[vv]->GetScanLineForWriting(hh)[0], wdd * coldep, 1);
	}
	delete in;
	return 0;
}

void wputblock(Shared::Bitmap *ds, int xx, int yy, Bitmap *bll, int xray) {
	if (xray)
		ds->Blit(bll, xx, yy, Shared::kBitmap_Transparency);
	else
		ds->Blit(bll, 0, 0, xx, yy, bll->GetWidth(), bll->GetHeight());
}

Bitmap wputblock_wrapper; // [IKM] argh! :[
void wputblock_raw(Shared::Bitmap *ds, int xx, int yy, BITMAP *bll, int xray) {
	wputblock_wrapper.WrapAllegroBitmap(bll, true);
	if (xray)
		ds->Blit(&wputblock_wrapper, xx, yy, Shared::kBitmap_Transparency);
	else
		ds->Blit(&wputblock_wrapper, 0, 0, xx, yy, wputblock_wrapper.GetWidth(), wputblock_wrapper.GetHeight());
}

const int col_lookups[32] = {
	0x000000, 0x0000A0, 0x00A000, 0x00A0A0, 0xA00000,   // 4
	0xA000A0, 0xA05000, 0xA0A0A0, 0x505050, 0x5050FF, 0x50FF50, 0x50FFFF,       // 11
	0xFF5050, 0xFF50FF, 0xFFFF50, 0xFFFFFF, 0x000000, 0x101010, 0x202020,       // 18
	0x303030, 0x404040, 0x505050, 0x606060, 0x707070, 0x808080, 0x909090,       // 25
	0xA0A0A0, 0xB0B0B0, 0xC0C0C0, 0xD0D0D0, 0xE0E0E0, 0xF0F0F0
};

int __wremap_keep_transparent = 1;

void wremap(color *pal1, Bitmap *picc, color *pal2) {
	int jj;
	unsigned char color_mapped_table[256];

	for (jj = 0; jj < 256; jj++) {
		if ((pal1[jj].r == 0) && (pal1[jj].g == 0) && (pal1[jj].b == 0)) {
			color_mapped_table[jj] = 0;
		} else {
			color_mapped_table[jj] = bestfit_color(pal2, pal1[jj].r, pal1[jj].g, pal1[jj].b);
		}
	}

	if (__wremap_keep_transparent > 0) {
		// keep transparency
		color_mapped_table[0] = 0;
		// any other pixels which are being mapped to 0, map to 16 instead
		for (jj = 1; jj < 256; jj++) {
			if (color_mapped_table[jj] == 0)
				color_mapped_table[jj] = 16;
		}
	}

	int pic_size = picc->GetWidth() * picc->GetHeight();
	for (jj = 0; jj < pic_size; jj++) {
		int xxl = jj % (picc->GetWidth()), yyl = jj / (picc->GetWidth());
		int rr = picc->GetPixel(xxl, yyl);
		picc->PutPixel(xxl, yyl, color_mapped_table[rr]);
	}
}

void wremapall(color *pal1, Bitmap *picc, color *pal2) {
	__wremap_keep_transparent--;
	wremap(pal1, picc, pal2);
	__wremap_keep_transparent++;
}

} // namespace AGS3
