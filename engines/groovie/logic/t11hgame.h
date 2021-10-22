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

#ifndef GROOVIE_LOGIC_T11HGAME_H
#define GROOVIE_LOGIC_T11HGAME_H

#include "common/textconsole.h"
#include "common/random.h"
#include "groovie/logic/beehive.h"
#include "groovie/logic/gallery.h"

namespace Groovie {

class GroovieEngine;

#ifdef ENABLE_GROOVIE2
class T11hCake;
#endif

class T11hGame {
public:
#ifdef ENABLE_GROOVIE2
	T11hGame(byte *scriptVariables);
	~T11hGame();

	void handleOp(uint8 op);

private:
	Common::RandomSource _random;

	void opMouseTrap();
	void opConnectFour();
	void opPente();
	void opGallery();
	void opTriangle();

	byte opGalleryAI(byte *pieceStatus, int depth);

	void clearAIs();

	T11hCake *_cake;
	byte *_scriptVariables;

	BeehiveGame _beehive;
	GalleryGame _gallery;
#endif
};

} // End of Groovie namespace

#endif // GROOVIE_LOGIC_T11HGAME_H
