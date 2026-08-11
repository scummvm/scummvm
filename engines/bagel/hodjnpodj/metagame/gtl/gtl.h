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

#ifndef BAGEL_METAGAME_GTL_GTL_H
#define BAGEL_METAGAME_GTL_GTL_H

#define GTLMDI 0        /* 0=single document ifc; 1=multiple */

#define GTLDLL 1    /* 0 = EXE, 1 = DLL */

#include "bagel/hodjnpodj/metagame/gtl/resource.h"       // main symbols
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldat.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

/////////////////////////////////////////////////////////////////////////////
// CGtlApp:
// See gtl.cpp for the implementation of this class
//

class CGtlApp : public CWinApp {
public:
	CBdbgMgr m_cBdbgMgr;				// Boffo games debug manager object
	bool m_bTitle = false;				// flag -- use window with title bar
	int  m_iNumOpens = 0;				// number of document opens (including "New")
	char m_szFilename[60] = {};			// input file name
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

	virtual bool InitInstance() override;

	CGtlApp();  // constructor
	~CGtlApp(); // destructor

	//{{AFX_MSG(CGtlApp)
	afx_msg void OnAppAbout();
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	bool InitApplication() override;
	void CreateInstance();
	bool CallOnFileNew() ;
	bool CallOnFileOpen() ;
	bool CallOnFileSave() ;
	//- DoMessageBox -- override of CWinApp function to
	//              display message box messages
	virtual int DoMessageBox(const char *lpszPrompt,
	                         unsigned int nType, unsigned int nIDPrompt) ;
	bool OnIdle(long lCount) override;

private:
	static void focusChange(CWnd *oldFocus, CWnd *newFocus);
};

#if GTLMDI

class CGtlMDIChildWnd : public CMDIChildWnd {
	DECLARE_DYNAMIC(CGtlMDIChildWnd)
protected:
	bool PreCreateWindow(CREATESTRUCT& cs);
};
#endif

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
