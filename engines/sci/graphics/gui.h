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
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_GRAPHICS_GUI_H
#define SCI_GRAPHICS_GUI_H

#include "sci/graphics/helpers.h"

namespace Sci {

class GfxScreen;
class GfxPalette;
class GfxCursor;
class GfxCache;
class GfxCompare;
class GfxCoordAdjuster16;
class GfxPorts;
class GfxPaint16;
class GfxAnimate;
class GfxControls;
class GfxMenu;
class GfxText16;
class GfxTransitions;

class SciGui {
public:
	SciGui(EngineState *s, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, GfxCursor *cursor, GfxPorts *ports, AudioPlayer *audio);
	virtual ~SciGui();

	virtual void init(bool usesOldGfxFunctions);

	virtual void wait(int16 ticks);

	virtual void textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight);
	virtual void textFonts(int argc, reg_t *argv);
	virtual void textColors(int argc, reg_t *argv);

	virtual reg_t portraitLoad(Common::String resourceName);
	virtual void portraitShow(Common::String resourceName, Common::Point position, uint16 resourceNum, uint16 noun, uint16 verb, uint16 cond, uint16 seq);
	virtual void portraitUnload(uint16 portraitId);

	virtual bool debugEGAdrawingVisualize(bool state);

	// FIXME: Don't store EngineState
	virtual void resetEngineState(EngineState *s);

protected:
	GfxCursor *_cursor;
	EngineState *_s;
	GfxScreen *_screen;
	GfxPalette *_palette;
	GfxCache *_cache;
	GfxCoordAdjuster16 *_coordAdjuster;
	GfxCompare *_compare;
	GfxPorts *_ports;
	GfxPaint16 *_paint16;

private:
	AudioPlayer *_audio;
	GfxAnimate *_animate;
	GfxControls *_controls;
	GfxMenu *_menu;
	GfxText16 *_text16;
	GfxTransitions *_transitions;

	bool _usesOldGfxFunctions;
};

} // End of namespace Sci

#endif
