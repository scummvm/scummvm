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

#ifndef SHERLOCK_SCALPEL_H
#define SHERLOCK_SCALPEL_H

#include "sherlock/sherlock.h"
#include "sherlock/scalpel/chess.h"
#include "sherlock/scalpel/darts.h"

namespace Sherlock {

namespace Scalpel {

class ScalpelEngine : public SherlockEngine {
private:
	Chess *_chess;
	Darts *_darts;
	int _tempFadeStyle;
	int _chessResult;

	bool showCityCutscene();
	bool showAlleyCutscene();
	bool showStreetCutscene();
	bool showOfficeCutscene();
protected:
	virtual void initialize();

	virtual void showOpening();

	virtual void startScene();
public:
	ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	virtual ~ScalpelEngine();

	void eraseMirror12();
	void doMirror12();
	void flushMirror12();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
