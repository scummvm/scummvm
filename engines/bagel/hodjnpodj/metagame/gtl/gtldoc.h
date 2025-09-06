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

#include "bagel/hodjnpodj/metagame/gtl/gtldat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

class CGtlDoc : public CDocument {
protected: // create from serialization only
	CGtlDoc();
	DECLARE_DYNCREATE(CGtlDoc)

// Attributes
public:
	char m_cStartData ;
	class CGtlData *m_xpGtlData ;   // data pointer
	class CGtlView *m_xpcLastFocusView ;    // last view with focus
	class CGtlView *m_xpcLastMouseView ;    // last view touched by mouse

	class CGtlFrame *m_xpGtlFrame ;     // ptr to frame window
	char m_cEndData ;


// Operations


// Implementation
public:
	virtual ~CGtlDoc();
	virtual void Serialize(CArchive &ar);   // overridden for document i/o

protected:
	virtual bool OnNewDocument() override;

// Generated message map functions
protected:
	//{{AFX_MSG(CGtlDoc)
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


// gtldoc.cpp -- implementation of the CGtlDoc class

public:
	void DeleteContents() override;
protected:
	//- InitDocument -- initialize document to specified file
	void InitDocument(const char *xpszPathName);
	virtual bool OnOpenDocument(const char* xpszPathName) override;
	virtual bool OnSaveDocument(const char* xpszPathName);

public:
	virtual bool DoOnFileSaveAs();

private:
	//- OnChangedViewList -- called by MFC when a view is
	//      added or deleted
	virtual void OnChangedViewList() override;

public:
	//- FixChecks -- fix dialog box check marks
	bool FixChecks();

};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
