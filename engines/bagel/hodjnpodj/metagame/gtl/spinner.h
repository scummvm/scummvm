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

#ifndef BAGEL_METAGAME_GTL_SPINNER_H
#define BAGEL_METAGAME_GTL_SPINNER_H

#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define SPINNER_SPEC                ".\\art\\spinner.bmp"
#define HODJ_SPINNER_NUMBERS_SPEC   ".\\art\\spinblue.bmp"
#define PODJ_SPINNER_NUMBERS_SPEC   ".\\art\\spinred.bmp"
#define SPINNER_SOUND               ".\\sound\\spinner.wav"

#define SPINNER_COUNT       64
#define SPINNER_CYCLE       15

#define SPINNER_WAIT        2

#define SPINNER_SLOT_DDX    30
#define SPINNER_SLOT_DDY    47
#define SPINNER_SLOTA_DX    23
#define SPINNER_SLOTA_DY    14
#define SPINNER_SLOTB_DX    66
#define SPINNER_SLOTB_DY    14


class CSpinner : public CObject {
	DECLARE_DYNCREATE(CSpinner)

public:
	CSpinner();
	CSpinner(CWnd *pWnd, CDC *pDC, int nX = 0, int nY = 0, bool bHodj = true);
	~CSpinner();

	int Animate() {
		return (Animate(m_nX, m_nY));
	}
	int Animate(int nX, int nY);

private:
	bool Initialize(CWnd *pWnd, CDC *pDC, int nX = 0, int nY = 0, bool bHodj = true);
	int Spin();
	bool Hide();
	bool Show() {
		return (Show(m_nX, m_nY));
	}
	bool Show(int nX, int nY);

	void SetupSpinner();

	static bool HandleMessages();

private:
	CWnd    *m_pWnd = nullptr;		// window for messages
	CDC     *m_pDC = nullptr;		// context for display
	CSprite *m_pSprite = nullptr;	// sprite for spinner
	int     m_nValue = 0;			// current spinner value
	int     m_nX = 0;				// x location of sprite
	int     m_nY = 0;				// y location of sprite
	bool    m_bVisible = false;		// whether spinner is visible
	bool    m_bHodj = false;		// which character is active
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
