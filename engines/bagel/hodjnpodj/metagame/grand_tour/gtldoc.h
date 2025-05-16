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

#ifndef BAGEL_METAGAME_GTL_GTLDOC_H
#define BAGEL_METAGAME_GTL_GTLDOC_H

#include "gtldat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

class CGtlDoc : public CDocument
{
protected: // create from serialization only
	CGtlDoc();
	DECLARE_DYNCREATE(CGtlDoc)

// Attributes
public:
    char m_cStartData ;
    class CGtlData * m_xpGtlData ;	// data pointer
    class CInfDlg * m_xpcInfDlg ;	// modeless information dialog
    class CCtlDlg * m_xpcCtlDlg ;	// modeless control dialog
    class CNodeDlg * m_xpcNodeDlg ;	// modeless node dialog
    class CMenuDlg * m_xpcMenuDlg ;	// modeless menu dialog
    class CGtlView * m_xpcLastFocusView ;	// last view with focus
    class CGtlView * m_xpcLastMouseView ;	// last view touched by mouse

    class CGtlFrame * m_xpGtlFrame ;	// ptr to frame window
    char m_cEndData ;


// Operations


// Implementation
public:
	virtual ~CGtlDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();

// Generated message map functions
protected:
	//{{AFX_MSG(CGtlDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


// gtldoc.cpp -- implementation of the CGtlDoc class

//- CGtlDoc::DeleteContents() --
public: void CGtlDoc::DeleteContents() ;
//- CGtlDoc::InitDocument -- initialize document to specified file
protected: void CGtlDoc::InitDocument(const char * xpszPathName) ;
//- CGtlDoc::OnOpenDocument --
virtual BOOL CGtlDoc::OnOpenDocument(const char* xpszPathName) ;
//- CGtlDoc::OnSaveDocument --
virtual BOOL CGtlDoc::OnSaveDocument(const char* xpszPathName) ;
//- CGtlDoc::DoOnFileSaveAs -- 
public: virtual BOOL CGtlDoc::DoOnFileSaveAs(void) ;
//- CGtlDoc::OnChangedViewList -- called by MFC when a view is
//		added or deleted
private: virtual VOID CGtlDoc::OnChangedViewList(void) ;
//- CGtlDoc::FixChecks -- fix dialog box check marks
public: BOOL CGtlDoc::FixChecks(void);

};

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
