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

#ifndef HOPKINS_GLOBALS_H
#define HOPKINS_GLOBALS_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

/**
 * Engine Globals
 */
class Globals {
public:
	int FADESPD;
	int FR;
	int SVGA;
	int MANU_SCROLL;
	int SPEED_SCROLL;
	int internet;
	int PUBEXIT;
	bool XFULLSCREEN;
	int XSETMODE;
	int XZOOM;
	bool XFORCE16;
	bool XFORCE8;
	bool CARD_SB;
	int vitesse;
	int INSTALL_TYPE;
	Common::String HOPIMAGE;
	Common::String HOPANIM;
	Common::String HOPLINK;
	Common::String HOPSAVE;
	Common::String HOPSOUND;
	Common::String HOPMUSIC;
	Common::String HOPVOICE;
	Common::String HOPANM;
	Common::String HOPSEQ;
	Common::String FICH_ZONE;
	Common::String FICH_TEXTE;
	int SOUNDVOL;
	int MUSICVOL;
	int VOICEVOL;
	bool SOUNDOFF;
	bool MUSICOFF;
	bool VOICEOFF;

	Globals();
	void setConfig();
};

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
