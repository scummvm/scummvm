
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

#ifndef BAGEL_DIALOGS_OPT_WINDOW_H
#define BAGEL_DIALOGS_OPT_WINDOW_H

#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gui/scroll_bar.h"
#include "bagel/boflib/gui/text_box.h"

namespace Bagel {

struct SYSTEM_DATA {
	int _nMusicVolume;
	int _nSoundVolume;

	bool _bFlythroughs;
	bool _bPanimations;
	int _nCorrection;
	int _nPanSpeed;
} ;

#define NUM_SYS_BUTTONS 8

class CBagOptWindow : public CBofDialog {
public:
	CBagOptWindow();

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	/**
	 * Get the system settings from CBofApp.
	 */
	void loadIniSettings();

	/**
	 * Save Out New Settings to the ini file of the App.
	 */
	void saveOutNewSettings();

	/**
	 * Change Settings to the defaults
	 */
	void returnToDefaults();

	/**
	 * Changes the states of buttons & controls to reflect the current variable values.
	 */
	void putDialogData();

	/**
	 * Changes the current variable values to reflect the states of buttons & controls.
	 */
	void getDialogData();

protected:
	void onPaint(CBofRect *pRect) override;
	void onInitDialog() override;

	void onBofScrollBar(CBofObject *pObject, int nPos) override;
	void onBofButton(CBofObject *pObject, int nState) override;
	void onKeyHit(uint32 lKey, uint32 lRepCount) override;

	// Data members
	//
	CBofBmpButton *_pButtons[NUM_SYS_BUTTONS];
	SYSTEM_DATA _cSystemData;
	ST_COLORSCHEME _cColorScheme;

	CBofCheckButton *_pFlythroughs;
	CBofCheckButton *_pPanimations;

	CBofScrollBar *_pMidiVolumeScroll;
	CBofScrollBar *_pWaveVolumeScroll;
	CBofScrollBar *_pCorrectionScroll;
	CBofScrollBar *_pPanSpeedScroll;
	CBofPalette *_pSavePalette;

	bool _bDirty;
};

const char *buildSysDir(const char *pszFile);

} // namespace Bagel

#endif
