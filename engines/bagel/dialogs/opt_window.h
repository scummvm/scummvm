
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

#ifndef BAGEL_BAGLIB_OPT_WINDOW_H
#define BAGEL_BAGLIB_OPT_WINDOW_H

#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/dialogs/save_dialog.h"

namespace Bagel {

struct SYSTEM_DATA {
	INT m_nMusicVolume;
	INT m_nSoundVolume;

	BOOL m_bFlythroughs;
	BOOL m_bPanimations;
	INT m_nCorrection;
	INT m_nPanSpeed;
} ;

#define NUM_SYS_BUTTONS 8

class CBagOptWindow : public CBofDialog {
public:
	CBagOptWindow();

#if BOF_DEBUG
	virtual ~CBagOptWindow();
#endif

	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	/**
	 * Get the system settings from CBofApp.
	 */
	VOID LoadIniSettings();

	/**
	 * Save Out New Settings to the ini file of the App.
	 */
	VOID SaveOutNewSettings();

	/**
	 * Change Settings to the defaults
	 */
	VOID ReturnToDefaults();

	/**
	 * Changes the states of buttons & controls to reflect the current variable values.
	 */
	VOID PutDialogData();

	/**
	 * Changes the current variable values to reflect the states of buttons & controls.
	 */
	VOID GetDialogData();

	VOID UpdateOptions();

protected:
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnInitDialog();

	virtual VOID OnBofScrollBar(CBofObject *pObject, INT nPos);
	virtual VOID OnBofButton(CBofObject *pObject, INT nState);
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	// Data members
	//
	CBofBmpButton *m_pButtons[NUM_SYS_BUTTONS];
	SYSTEM_DATA m_cSystemData;
	ST_COLORSCHEME m_cColorScheme;

	CBofCheckButton *m_pFlythroughs;
	CBofCheckButton *m_pPanimations;

	CBofScrollBar *m_pMidiVolumeScroll;
	CBofScrollBar *m_pWaveVolumeScroll;
	CBofScrollBar *m_pCorrectionScroll;
	CBofScrollBar *m_pPanSpeedScroll;
	CBofPalette *m_pSavePalette;

	BOOL m_bDirty;
};

const CHAR *BuildSysDir(const CHAR *pszFile);

} // namespace Bagel

#endif
