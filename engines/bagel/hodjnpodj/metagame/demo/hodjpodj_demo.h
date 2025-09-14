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

#ifndef BAGEL_METAGAME_DEMO_HODJPODJ_H
#define BAGEL_METAGAME_DEMO_HODJPODJ_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/metagame/frame/movytmpl.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Demo {

class CHodjPodjDemoWindow : public CFrameWnd {
private:
	Frame::CMovieWindow pMovie;
	CPalette *pGamePalette = nullptr;
	CSound *pBackgroundMidi = nullptr;
	int _currentCommand = -1;
	int _minigame = -1;

	void drawBitmap(const char *filename);
	void startBackgroundMidi();
	void stopBackgroundMidi();

public:
	CHodjPodjDemoWindow();

	void blackScreen();

	void playMovie(const int, const char *, bool);
	void skipSplash();
	void loadNewDLL(LPARAM lParam);

protected:
	bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	void OnTimer(uintptr nEventID);
	void OnLButtonDown(uint nFlags, CPoint point);
	void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	void OnClose();
	void OnParentNotify(unsigned int msg, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

} // namespace Demo
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
