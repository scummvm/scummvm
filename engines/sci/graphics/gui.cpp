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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/gui.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/controls.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/portrait.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"
#include "sci/graphics/view.h"
#include "sci/sound/audio.h"

namespace Sci {

SciGui::SciGui(EngineState *state, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, GfxCursor *cursor, GfxPorts *ports, AudioPlayer *audio)
	: _s(state), _screen(screen), _palette(palette), _cache(cache), _cursor(cursor), _ports(ports), _audio(audio) {

	// FIXME/TODO: If SciGui inits all the stuff below, then it should *own* it,
	// not SciEngine. Conversely, if we want SciEngine to own this stuff,
	// then it should init it!
	_coordAdjuster = new GfxCoordAdjuster16(_ports);
	g_sci->_gfxCoordAdjuster = _coordAdjuster;
	_cursor->init(_coordAdjuster, _s->_event);
	_compare = new GfxCompare(_s->_segMan, g_sci->getKernel(), _cache, _screen, _coordAdjuster);
	g_sci->_gfxCompare = _compare;
	_transitions = new GfxTransitions(this, _screen, _palette, g_sci->getResMan()->isVGA());
	_paint16 = new GfxPaint16(g_sci->getResMan(), _s->_segMan, g_sci->getKernel(), this, _cache, _ports, _coordAdjuster, _screen, _palette, _transitions);
	g_sci->_gfxPaint = _paint16;
	g_sci->_gfxPaint16 = _paint16;
	_animate = new GfxAnimate(_s, _cache, _ports, _paint16, _screen, _palette, _cursor, _transitions);
	g_sci->_gfxAnimate = _animate;
	_text16 = new GfxText16(g_sci->getResMan(), _cache, _ports, _paint16, _screen);
	_controls = new GfxControls(_s->_segMan, _ports, _paint16, _text16, _screen);
	g_sci->_gfxControls = _controls;
	_menu = new GfxMenu(_s->_event, _s->_segMan, this, _ports, _paint16, _text16, _screen, _cursor);
	g_sci->_gfxMenu = _menu;
}

SciGui::~SciGui() {
	delete _menu;
	delete _controls;
	delete _text16;
	delete _animate;
	delete _paint16;
	delete _transitions;
	delete _compare;
	delete _coordAdjuster;
}

void SciGui::resetEngineState(EngineState *s) {
	_s = s;
	_animate->resetEngineState(s);
}

void SciGui::init(bool usesOldGfxFunctions) {
	_ports->init(usesOldGfxFunctions, this, _paint16, _text16, _s->_gameId);
	_paint16->init(_animate, _text16);
}

void SciGui::wait(int16 ticks) {
	_s->wait(ticks);
}

void SciGui::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, *textWidth, *textHeight);
	_text16->Size(rect, text, font, maxWidth);
	*textWidth = rect.width();
	*textHeight = rect.height();
}

// Used SCI1+ for text codes
void SciGui::textFonts(int argc, reg_t *argv) {
	_text16->CodeSetFonts(argc, argv);
}

// Used SCI1+ for text codes
void SciGui::textColors(int argc, reg_t *argv) {
	_text16->CodeSetColors(argc, argv);
}

reg_t SciGui::portraitLoad(Common::String resourceName) {
	//Portrait *myPortrait = new Portrait(g_sci->getResMan(), _screen, _palette, resourceName);
	return NULL_REG;
}

void SciGui::portraitShow(Common::String resourceName, Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq) {
	Portrait *myPortrait = new Portrait(g_sci->getResMan(), _s->_event, this, _screen, _palette, _audio, resourceName);
	// TODO: cache portraits
	// adjust given coordinates to curPort (but dont adjust coordinates on upscaledHires_Save_Box and give us hires coordinates
	//  on kDrawCel, yeah this whole stuff makes sense)
	position.x += _ports->getPort()->left; position.y += _ports->getPort()->top;
	_screen->adjustToUpscaledCoordinates(position.y, position.x);
	myPortrait->doit(position, resourceId, noun, verb, cond, seq);
	delete myPortrait;
}

void SciGui::portraitUnload(uint16 portraitId) {
}

bool SciGui::debugEGAdrawingVisualize(bool state) {
	_paint16->setEGAdrawingVisualize(state);
	return false;
}

} // End of namespace Sci
