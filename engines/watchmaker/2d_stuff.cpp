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

#include "watchmaker/2d_stuff.h"
#include "watchmaker/fonts.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

void TwoDeeStuff::writeBitmapListTo(SDDBitmap* target) {
	memcpy(target, DDBitmapsList, sizeof(struct SDDBitmap)*MAX_DD_BITMAPS);
}

void TwoDeeStuff::garbageCollectPreRenderedText() {
	// Destroys pre-rendered writings that are no longer needed
	SDDText *r, *t;
	int32 c, a;
	for (c = 0, r = &RendText[0]; c < MAX_REND_TEXTS; c++, r++) {
		if (!r->text[0]) continue;

		for (a = 0, t = &DDTextsList[0]; a < MAX_DD_TEXTS; a++, t++) {
			if (!t->text[0]) continue;

			if (!strcmp(t->text, r->text) && (t->color == r->color) && (t->font == r->font))
				break;
		}
		// If it should no longer be displayed
		if (a >= MAX_DD_TEXTS) {
			rReleaseBitmap(r->tnum);
			memset(r, 0, sizeof(struct SDDText));
		}
	}
}

void TwoDeeStuff::clearBitmapList() {
	int32 a;
	SDDBitmap *b;
	for (a = 0, b = &DDBitmapsList[0]; a < MAX_DD_BITMAPS; a++, b++) {
		b->tnum = b->px = b->py = b->ox = b->oy = b->dx = b->dy = 0;
	}
}

void TwoDeeStuff::clearTextList() {
	int32 a;
	SDDText *t;
	for (a = 0, t = &DDTextsList[0]; a < MAX_DD_TEXTS; a++, t++) {
		memset(t->text, 0, sizeof(t->text));
		t->tnum = 0;
	}
}

int32 TwoDeeStuff::findFreeBitmap() {
	int32 a = 0;
	for (a = 0; a < MAX_DD_BITMAPS; a++)
		if (!DDBitmapsList[a].tnum)
			return a;

	warning("Too many DD Bitmaps!");
	return -1;
}

void TwoDeeStuff::displayDDBitmap(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	int32 a = findFreeBitmap();
	if (a == -1) {
		warning("Skipping draw");
		return;
	}

	DDBitmapsList[a].tnum = tnum;
	DDBitmapsList[a].px   = _renderer->rFitX(px);
	DDBitmapsList[a].py   = _renderer->rFitY(py);
	DDBitmapsList[a].ox   = _renderer->rFitX(px + ox) - _renderer->rFitX(px);
	DDBitmapsList[a].oy   = _renderer->rFitY(py + oy) - _renderer->rFitY(py);
	DDBitmapsList[a].dx   = _renderer->rFitX(px + dx) - _renderer->rFitX(px);
	DDBitmapsList[a].dy   = _renderer->rFitY(py + dy) - _renderer->rFitY(py);
	if (dx <= 0) DDBitmapsList[a].dx += _renderer->getBitmapDimX(tnum) - DDBitmapsList[a].ox;
	if (dy <= 0) DDBitmapsList[a].dy += _renderer->getBitmapDimY(tnum) - DDBitmapsList[a].oy;
}

void TwoDeeStuff::displayDDBitmap_NoFit(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	int32 a = findFreeBitmap();
	if (a == -1) {
		warning("Skipping draw");
		return;
	}

	DDBitmapsList[a].tnum = tnum;
	DDBitmapsList[a].px   = (px);
	DDBitmapsList[a].py   = (py);
	DDBitmapsList[a].ox   = (px + ox) - (px);
	DDBitmapsList[a].oy   = (py + oy) - (py);
	DDBitmapsList[a].dx   = (px + dx) - (px);
	DDBitmapsList[a].dy   = (py + dy) - (py);
	if (dx <= 0) DDBitmapsList[a].dx += _renderer->getBitmapDimX(tnum) - DDBitmapsList[a].ox;
	if (dy <= 0) DDBitmapsList[a].dy += _renderer->getBitmapDimY(tnum) - DDBitmapsList[a].oy;
}

int32 TwoDeeStuff::rendDDText(char *text, FontKind font, FontColor color) {
	struct SDDText *r;
	int32 c, tdx, tdy;
	char info[100];

	if ((!text) || (text[0] == '\0')) return -1;

	for (c = 0, r = &RendText[0]; c < MAX_REND_TEXTS; c++, r++) {
		if (r->text[0]) continue;
		// Get the size of the text to render
		_renderer->_fonts->getTextDim(text, font, &tdx, &tdy);
		// Create a surface to contain it
		r->tnum = rCreateSurface(tdx, tdy, rBITMAPSURFACE);
		_renderer->clearBitmap(r->tnum, 0, 0, tdx, tdy, 0, 0, 0);
		// Render the lettering on the surface
		//DebugLogWindow("Creo testo %s | %d %d",text,tdx,tdy );
		_renderer->printText(text, r->tnum,  font, color, 0, 0);
		Common::strlcpy(info, "text: ", sizeof(info));
		strncat(info, text, 15);
		//DebugLogWindow("Creato %s",info);
		rSetBitmapName(r->tnum, info);
		Common::strlcpy(r->text, text, sizeof(r->text));
		r->color = color;
		r->font = font;
		return r->tnum;
	}

	return -1;
}

void TwoDeeStuff::displayDDText(char *text, FontKind font, FontColor color, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy) {
	struct SDDText *t, *r;
	int32 a, c;

	if ((!text) || (text[0] == '\0')) return;

	for (a = 0; a < MAX_DD_TEXTS; a++)
		if (!DDTextsList[a].text[0])
			break;

	if (a >= MAX_DD_TEXTS) {
		warning("Too many DD Texts!");
		return ;
	}

	DDTextsList[a] = SDDText(text, font, color, -1);

	t = &DDTextsList[a];
	// Try searching the pre-rendered scripts
	for (c = 0, r = &RendText[0]; c < MAX_REND_TEXTS; c++, r++) {
		if (!r->text[0]) continue;

		if (!strcmp(t->text, r->text) && (t->color == r->color) && (t->font == r->font)) {
			this->displayDDBitmap(r->tnum, px, py, ox, oy, dx, dy);
			break;
		}
	}
	// if I didn't prerender the script, I render it now
	if (c >= MAX_REND_TEXTS) {
		/*      if( ( r->tnum = RendDDText( t->text, t->font, t->color ) ) > 0 )
					// Add the bitmap with pre-rendered lettering to display
					DisplayDDBitmap( r->tnum, px, py, ox, oy, dx, dy );*/

		int32 tn;
		if ((tn = rendDDText(t->text, t->font, t->color)) > 0)
			// Add the bitmap with pre-rendered lettering to display
			this->displayDDBitmap(tn, px, py, ox, oy, dx, dy);
	}
}


} // End of namespace Watchmaker
