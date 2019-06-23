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

#ifndef HDB_WINDOW_H
#define HDB_WINDOW_H

namespace HDB {

enum {
	kDialogTextLeft = 64,
	kDialogTextRight = (kDialogTextLeft + kTileWidth * 9)
};

struct DialogInfo {
	char		title[64];				// TITLE string
	int			tileIndex;					// this is for a character picture
	char		string[160];			// actual text in the dialog

	bool		active;					// is it drawing or not?
	int			x, y;					// where to draw dialog
	int			width, height;			// size of the dialog itself
	int			titleWidth;
	Picture		*gfx;					// character tile (picture)
	int			more;					// whether we want to draw the MORE icon or not
	int			el, er, et, eb;			// saves the text edges
	char		luaMore[64];			// the name of the function to call after clicking the MORE button

	DialogInfo() : title(""), tileIndex(0), string(""), active(false), x(0), y(0), width(0), height(0), titleWidth(0), gfx(NULL), more(0), el(0), er(0), et(0), eb(0), luaMore("") {}
};

class Window {
public:

	bool init();
	void restartSystem();

	// Dialog Functions

	void openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore);
	void setDialogDelay(int delay);
	uint32 getDialogDelay() {
		return _dialogDelay;
	}

private:

	DialogInfo _dialogInfo;
	uint32 _dialogDelay;	// Used for Cinematics

	// Windows GFX
	Picture *_gfxTL, *_gfxTM, *_gfxTR;
	Picture *_gfxL, *_gfxM, *_gfxR;
	Picture *_gfxBL, *_gfxBM, *_gfxBR;
	Picture *_gfxTitleL, *_gfxTitleM, *_gfxTitleR;
	Picture *_gGfxTL, *_gGfxTM, *_gGfxTR;
	Picture *_gGfxL, *_gGfxM, *_gGfxR;
	Picture *_gGfxBL, *_gGfxBM, *_gGfxBR;
	Picture *_gGfxTitleL, *_gGfxTitleM, *_gGfxTitleR;
	Picture *_gfxResources, *_gfxDeliveries;
	Picture *_gfxIndent, *_gfxArrowTo, *_gfxHandright;
	Picture *_gfxTry, *_gfxAgain, *_gfxInvSelect;
	Tile *_gfxMonkeystone;
	Picture *_gfxLevelRestart, *_gfxPuasePlaque;

};

} // End of Namespace

#endif // !HDB_WINDOW_H
