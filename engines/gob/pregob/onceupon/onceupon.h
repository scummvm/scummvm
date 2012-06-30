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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GOB_PREGOB_ONCEUPON_ONCEUPON_H
#define GOB_PREGOB_ONCEUPON_ONCEUPON_H

#include "common/system.h"

#include "gob/pregob/pregob.h"

namespace Gob {

class Surface;
class Font;

class ANIObject;

namespace OnceUpon {

class OnceUpon : public PreGob {
public:
	OnceUpon(GobEngine *vm);
	~OnceUpon();

protected:
	void init();
	void deinit();

	void setGamePalette(uint palette);

	bool doCopyProtection(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4]);

	void showWait();  ///< Show the wait / loading screen.
	void showIntro(); ///< Show the whole intro.

	void showChapter(int chapter); ///< Show a chapter intro text.


	// Fonts
	Font *_jeudak;
	Font *_lettre;
	Font *_plettre;
	Font *_glettre;

private:
	void setCopyProtectionPalette();

	void setAnimState(ANIObject &ani, uint16 state, bool once, bool pause) const;

	// Copy protection helpers
	int8 cpSetup(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4], const Surface sprites[2]);
	int8 cpFindShape(int16 x, int16 y) const;
	void cpWrong();

	// Intro parts
	void showQuote();
	void showTitle();

	// Title music
	void playTitleMusic();
	void playTitleMusicDOS();
	void playTitleMusicAmiga();
	void playTitleMusicAtariST();
	void stopTitleMusic();


	bool _openedArchives;
};

} // End of namespace OnceUpon

} // End of namespace Gob

#endif // GOB_PREGOB_ONCEUPON_ONCEUPON_H
