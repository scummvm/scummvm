/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef KYRA_H
#define KYRA_H

//#include "common/scummsys.h"
#include "base/engine.h"
#include "base/gameDetector.h"
#include "common/util.h"

enum {
	GF_FLOPPY	= 1 << 0,
	GF_TALKIE	= 1 << 1,
	GF_KYRA1	= 1 << 2,
	GF_KYRA2	= 1 << 3
};

namespace Kyra {
	class Resourcemanager;
	class CPSImage;
	class Font;
	class Palette;
	class VMContext;

class KyraEngine : public Engine {
public:
	KyraEngine(GameDetector *detector, OSystem *syst);
	~KyraEngine();
	void errorString( const char *buf_input, char *buf_output);

	void updateScreen(void);
	void setCurrentPalette(Palette* pal, bool delNextTime = true);

	Resourcemanager* resManager(void) { return _resMgr; }
//	MidiDriver* midiDriver(void) { return _midiDriver; }

protected:
	void go();
	void shutdown();
	Resourcemanager* _resMgr;
	uint8 *_screen;

	Font* _font;
	CPSImage* _mouse;
	CPSImage* _items;

	VMContext* _currentScript; // our current script
	VMContext* _npcScript; // script from NPCs
};

} // End of namespace Kyra

#endif
