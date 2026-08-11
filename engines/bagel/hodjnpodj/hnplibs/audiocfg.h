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

#ifndef HODJNPODJ_HNPLIBS_AUDIOCFG_H
#define HODJNPODJ_HNPLIBS_AUDIOCFG_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {

#define PAGE_SIZE     2
#define LEVEL_MIN     1
#define LEVEL_DEF     ((VOLUME_INDEX_MAX * 3) >> 2)
#define LEVEL_MAX     VOLUME_INDEX_MAX

class CAudioCfgDlg : public CBmpDialog {
public:
	CAudioCfgDlg(CWnd *pParent, CPalette *pPalette, unsigned int);

protected:

	virtual void DoDataExchange(CDataExchange *) override;
	virtual bool OnCommand(WPARAM, LPARAM) override;
	virtual bool OnInitDialog() override;
	void PutDlgData();
	void GetDlgData();
	void LoadIniSettings();
	void SaveIniSettings();
	void UpdateOptions();
	void ClearDialogImage();

	afx_msg bool OnEraseBkgnd(CDC *);
	void OnHScroll(unsigned int, unsigned int, CScrollBar *);
	void OnClose();
	void OnDestroy();
	void OnPaint();
	DECLARE_MESSAGE_MAP()

	CScrollBar   *m_pScrollBar1 = nullptr;
	CCheckButton *m_pMusicButton = nullptr;
	CCheckButton *m_pSoundButton = nullptr;

	CText        *m_pTxtVolume = nullptr;

	int          m_nVolume = 0;
	bool         m_bMusic = false;
	bool         m_bSound = false;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
