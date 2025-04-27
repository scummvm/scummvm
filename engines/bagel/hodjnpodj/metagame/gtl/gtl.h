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

#ifndef HODJNPODJ_METAGAME_GTL_GTL_H
#define HODJNPODJ_METAGAME_GTL_GTL_H

#include "bagel/hodjnpodj/metagame/gtl/resource.h"

#define GTLMDI 0        /* 0=single document ifc; 1=multiple */

#define GTLDLL 1    /* 0 = EXE, 1 = DLL */

#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldat.h"
#include "bagel/hodjnpodj/metagame/gtl/gamedll.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

/////////////////////////////////////////////////////////////////////////////
// CGtlApp:
// See gtl.cpp for the implementation of this class
//

class CGtlApp : public CWinApp
{
public:
    char m_cStartData ;
    CBdbgMgr m_cBdbgMgr ;       // Boffo games debug manager object
    BOOL m_bTitle ;             // flag -- use window with title bar
    int  m_iNumOpens ;          // number of document opens (including "New")
    char m_szFilename[60] ;     // input file name
    int  m_iX, m_iY ;           // position of screen
    int  m_iHeight, m_iWidth ;  // height/width of screen
    BOOL m_bControlDialog ;     // display control dialog box
    BOOL m_bMenuDialog ;        // display menu dialog box
    BOOL m_bInfoDialog ;        // display info dialog box
    BOOL m_bNodeDialog ;        // display node dialog box
    BOOL m_bShowNodes ;         // show nodes on map
    BOOL m_bPaintBackground ;   // paint background on map paint
    BOOL m_bStartMetaGame ;     // start meta game automatically
    BOOL m_bDumpGamePlay ;      // dump game play
    char m_cEndData ;

// Overrides
        virtual BOOL InitInstance();
#if GTLDLL
        virtual int ExitInstance();
#endif

// Implementation
    CGtlApp();  // constructor
    ~CGtlApp(); // destructor

        //{{AFX_MSG(CGtlApp)
        afx_msg void OnAppAbout();
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()


// gtl.cpp : Defines the class behaviors for the application.

public:
		void CreateInstance( void );

//- CGtlApp::CallOnFileNew -- 
public: BOOL CGtlApp::CallOnFileNew(void) ;
//- CGtlApp::CallOnFileOpen -- 
public: BOOL CGtlApp::CallOnFileOpen(void) ;
//- CGtlApp::CallOnFileSave -- 
public: BOOL CGtlApp::CallOnFileSave(void) ;
//- CGtlApp::DoMessageBox -- override of CWinApp function to
//              display message box messages
public: virtual int CGtlApp::DoMessageBox(LPCSTR lpszPrompt,
        UINT nType, UINT nIDPrompt) ;


};

#if GTLMDI

class CGtlMDIChildWnd : public CMDIChildWnd
{
        DECLARE_DYNAMIC(CGtlMDIChildWnd)
protected:
        BOOL PreCreateWindow(CREATESTRUCT& cs);
};
#endif

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
