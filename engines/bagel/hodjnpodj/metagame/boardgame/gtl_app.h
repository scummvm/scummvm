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

#ifndef HODJNPODJ_METAGAME_GTL_APP_H
#define HODJNPODJ_METAGAME_GTL_APP_H

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

class CGtlApp {
	class ConfManProxy {
	public:
		int GetDebugInt(const char *name);
	};
public:
	int  m_iNumOpens = 0;		// number of document opens (including "New")
	const char *m_szFilename = "meta/meta.gtl";	// input file name

	bool m_bTitle = false;				// flag -- use window with title bar
	int  m_iX = 0, m_iY = 0;			// position of screen
	int  m_iHeight = 0, m_iWidth = 0;	// height/width of screen
	bool m_bControlDialog = false;		// display control dialog box
	bool m_bMenuDialog = false;			// display menu dialog box
	bool m_bInfoDialog = false;			// display info dialog box
	bool m_bNodeDialog = false;			// display node dialog box
	bool m_bShowNodes = false;			// show nodes on map
	bool m_bPaintBackground = false;	// paint background on map paint
	bool m_bStartMetaGame = false;		// start meta game automatically
	bool m_bDumpGamePlay = false;		// dump game play
	ConfManProxy m_cBdbgMgr;

	void initInstance();
	void setDialogBkColor() {}

public:
	CGtlApp() {
		initInstance();
	}

	void DoWaitCursor(int cNode) {
	}
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
