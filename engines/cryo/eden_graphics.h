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

#ifndef CRYO_EDEN_GRAPHICS_H
#define CRYO_EDEN_GRAPHICS_H

#include "cryo/defs.h" // Room

namespace Cryo {

class EdenGame;
class HnmPlayer;

class EdenGraphics {
public:
	EdenGraphics(EdenGame *game, HnmPlayer *video);

	// Original name: noclipax
	void drawSprite(int16 index, int16 x, int16 y, bool withBlack = false, bool onSubtitle = false);

	// Original name: af_subtitle
	void displaySubtitles();

	// Original name: bars_in
	void showBars();

	void sundcurs(int16 x, int16 y);

	void rundcurs();

	void unglow();

	void glow(int16 index);

	void setGlowX(int16 value);

	void setGlowY(int16 value);	

	// Original name : blackbars
	void drawBlackBars();

	// Original name: bars_out
	void hideBars();

	// Original name: afsalle
	void displayRoom();

	// Original name: af_image
	void displayImage();

	void effetpix();

	// Original name: effet1
	void displayEffect1();

	// Original name: effet2
	void displayEffect2();

	void setShowBlackBars(bool value);

	bool getShowBlackBars();

	void paneltobuf();

	void cursbuftopanel();

	void langbuftopanel();

	View *getSubtitlesView();

	View *getMainView();

	byte *getHnmViewBuf();

	void setCurCharRect(Common::Rect *charRect);

	void setPaletteColor(byte *buffer);

	// Original name: sauvefondbouche
	void saveMouthBackground();

	// Original name: restaurefondbouche
	void restoreMouthBackground();

	void openWindow();

	bool _savedUnderSubtitles;

	void setSavedUnderSubtitles(bool value);

	byte *getSubtitlesViewBuf();

	View *getUnderBarsView();

	void SendPalette2Screen(int16 value);

	void setFade(bool value);

	bool getFade();

	// Original name: effet3
	void displayEffect3();

	void setDestRect(int16 sx, int16 sy, int16 ex, int16 ey);

	void setSrcRect(int16 sx, int16 sy, int16 ex, int16 ey);

	void fadeToBlack(int delay);

	// Original name: fadetoblack128
	void fadeToBlackLowPalette(int delay);

	// Original name: fadefromblack128
	void fadeFromBlackLowPalette(int delay);

	void clearScreen();

	void playHNM(int16 num);

	void setCursKeepPos(int16 x, int16 y);

	void restoreUnderSubtitles();

	void initRects();

	void initGlobals();

	void saveTopFrieze(int16 x);

	void saveBottomFrieze();

	void restoreTopFrieze();

	void restoreBottomFrieze();

private:
	EdenGame *_game;
	HnmPlayer *_video;

	int16 _glowX;
	int16 _glowY;
	int16 _glowW;
	int16 _glowH;

	bool  _showVideoSubtitle;

	Common::Point _cursKeepPos;

	View  *_mainView;
	View  *_underSubtitlesView;
	View  *_subtitlesView;
	View  *_underBarsView;

	Common::Rect _underSubtitlesScreenRect;
	Common::Rect _underSubtitlesBackupRect;	

	Common::Rect _underTopBarScreenRect;
	Common::Rect _underBottomBarBackupRect;
	Common::Rect _underBottomBarScreenRect;
	Common::Rect _underTopBarBackupRect;

	byte *_underSubtitlesViewBuf; // CHECKME: Useless?	

	byte *_mainViewBuf;	

	View  *_hnmView;
	byte *_hnmViewBuf;
	byte *_view2Buf;

	Common::Rect *_curCharacterRect;

	Common::Rect _rect_dst, _rect_src;

	View  *_view2;	

	int _hnmFrameNum;

	bool  _videoCanceledFlag;  //TODO: hnm_canceled

	color_t _globalPalette[256];    //TODO palette_t

	byte *_subtitlesViewBuf;

	bool _needToFade;

	color3_t _newColor;
	color_t  _oldPalette[256];    // TODO palette_t ?
	color_t  _newPalette[256];

	bool _showBlackBars;	

	void saveUnderSubtitles(int16 y);



	void displayHNMSubtitle();

	void readPalette(byte *ptr);

	void getglow(int16 x, int16 y, int16 w, int16 h);

	void loadMouthRectFromCurChar();

	// Original name afsalle1
	void displaySingleRoom(Room *room);

	// Original name: effet4
	void displayEffect4();

	void colimacon(int16 pattern[]);

	// Original name: rectanglenoir32
	void blackRect32();

	////// film.c
	// Original name: showfilm
	void showMovie(char arg1);

	// Original name bullehnm
	void handleHNMSubtitles();
};

}   // namespace Cryo

#endif // CRYO_EDEN_GRAPHICS_H