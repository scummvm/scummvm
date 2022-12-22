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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_UIDIALOG_H
#define SAGA2_UIDIALOG_H

namespace Saga2 {

struct SaveFileHeader;

// constants
const uint32    kDialogGroupID   = MKTAG('D', 'I', 'A', 'L');

// this should eventually point to the script that contains the credits
const uint16 kCreditsScriptNum          = 0;    // this has a scripts now
// >>> need to make resource for defining
// script numbers

// enumerations
enum placardTypes {
	WOOD_TYPE,
	STONE_TYPE,
	BRASS_TYPE
};



/* ===================================================================== *
   protoypes
 * ===================================================================== */

// dialog funcs
int16 OptionsDialog(bool disableSaveResume = false);

// helper funcs
void initFileFields(char **fieldStrings);
char **destroyFileFields();
bool getSaveName(int8 saveNo, SaveFileHeader &header);
int16 userDialog(const char *title, const char *msg, const char *btnMsg1,
                 const char *btnMsg2,
                 const char *btnMsg3);

void placardWindow(int8 type, char *text);

void updateAutoAggressionButton(bool setting);
void updateAutoWeaponButton(bool setting);


/* ===================================================================== *
   PlacardWindow class
 * ===================================================================== */


class CPlacardWindow : public ModalWindow {
private:

	enum {
		kMaxLines    = 16,
		kMaxText     = 512
	};

	int16   _titleCount;
	Point16 _titlePos[kMaxLines];
	char    *_titleStrings[kMaxLines];
	char    _titleBuf[kMaxText];

	textPallete _textPal;
	gFont       *_textFont;

	void positionText(
	    char *windowText,
	    const Rect16 &textArea);

	int16 SplitString(
	    char *text,
	    char *textStart[],
	    int16 maxStrings,
	    char delimiter);

protected:
	bool pointerHit(gPanelMessage &msg);

public:
	CPlacardWindow(const Rect16 &r,
	               uint16 ident,
	               AppFunc *cmd,
	               char *windowText,
	               textPallete &pal,
	               gFont *font);

	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &r);
};

class CPlacardPanel : public CPlaqText {
	enum {
		kMaxLines    = 16,
		kMaxText     = 512
	};

	int16   _titleCount;
	Point16 _titlePos[kMaxLines];
	char    *_titleStrings[kMaxLines];
	char    _titleBuf[kMaxText];

	void positionText(const char *windowText, const Rect16 &textArea);

	int16 SplitString(
	    char *text,
	    char *textStart[],
	    int16 maxStrings,
	    char delimiter);
public:
	CPlacardPanel(gPanelList &, const Rect16 &, const char *, gFont *,
	              int16, textPallete &, int16, AppFunc *cmd = NULL);

	void drawClipped(gPort &,
	                 const Point16 &,
	                 const Rect16 &);

};

} // end of namespace Saga2

#endif
