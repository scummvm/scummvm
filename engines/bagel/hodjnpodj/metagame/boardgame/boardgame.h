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
 */

#ifndef HODJNPODJ_METAGAME_BOARDGAME_H
#define HODJNPODJ_METAGAME_BOARDGAME_H

#include "bagel/hodjnpodj/views/view.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

struct BoardgameVars {
	char m_szIniFilename[100];		// .INI file name
	char m_szIniSectionname[15];	// .INI file section name
	bool m_bDebug;				// flag -- debugging mode
	bool m_bDebugMessages;		// debugging messages
	bool m_bTimeMessage;		// time stamp message
	bool m_bTrack, m_bTrace;	// debugging -- track/trace flags
	bool m_bTraceError;			// debugging -- trace error exits
	int  m_iConstructorMsgLevel; // level of constructor messages
	bool m_bVerifyDc;			// verify device context is ok
	int  m_iDebugValues[100];	// misc debugging values
	int  m_iTraceObjectCount;	// max # objects to trace
	void *m_lpTraceObjects;		// trace object array
	int  m_iTraceObjectCurrent;	// # of array elements in use
	int  m_iErrorCount;			// number of errors encountered

	BoardgameVars() {
		clearVars();
	}
	void clearVars();
};

class Boardgame : public View, public BoardgameVars {
private:
	bool bJustReturned = false;

public:
	Boardgame();
	~Boardgame() override {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	void draw() override;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
