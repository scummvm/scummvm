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

#ifndef GOB_PREGOB_PREGOB_H
#define GOB_PREGOB_PREGOB_H

#include "common/str.h"
#include "common/array.h"

#include "gob/util.h"

#include "gob/sound/sounddesc.h"

#include "gob/pregob/txtfile.h"

namespace Gob {

class GobEngine;
class Surface;

class ANIObject;

class PreGob {
public:
	PreGob(GobEngine *vm);
	virtual ~PreGob();

	virtual void run() = 0;

protected:
	void initScreen(); ///< Initialize the game screen.

	void fadeOut(); ///< Fade to black.
	void fadeIn();  ///< Fade to the current palette.

	void clearScreen();

	/** Change the palette.
	 *
	 *  @param palette The palette to change to.
	 *  @param size Size of the palette in colors.
	 */
	void setPalette(const byte *palette, uint16 size); ///< Change the palette

	void addCursor();
	void removeCursor();

	void setCursor(Surface &sprite, int16 hotspotX, int16 hotspotY);
	void setCursor(Surface &sprite, int16 left, int16 top, int16 right, int16 bottom,
	               int16 hotspotX, int16 hotspotY);

	void showCursor();
	void hideCursor();

	bool isCursorVisible() const;

	void loadSounds(const char * const *sounds, uint soundCount);
	void freeSounds();

	void playSound(uint sound, int16 frequency = 0, int16 repCount = 0);
	void stopSound();

	void endFrame(bool doInput);

	int16 checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);
	int16 waitInput (int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);
	int16 waitInput();
	bool  hasInput();

	void clearAnim(ANIObject &ani);
	void drawAnim(ANIObject &ani);
	void redrawAnim(ANIObject &ani);

	Common::String getLocFile(const Common::String &file) const;
	TXTFile *loadTXT(const Common::String &txtFile, TXTFile::Format format) const;

	virtual void fixTXTStrings(TXTFile &txt) const;


	GobEngine *_vm;

private:
	bool _fadedOut; ///< Did we fade out?

	Common::Array<SoundDesc> _sounds;
};

} // End of namespace Gob

#endif // GOB_PREGOB_PREGOB_H
