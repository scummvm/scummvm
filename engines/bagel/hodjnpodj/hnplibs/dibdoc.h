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

#ifndef HODJNPODJ_HNPLIBS_DIBDOC_H
#define HODJNPODJ_HNPLIBS_DIBDOC_H

#include "bagel/hodjnpodj/hnplibs/dibapi.h"

namespace Bagel {
namespace HodjNPodj {

class CDibDoc : public CDocument {
	DECLARE_DYNCREATE(CDibDoc)
	friend class CSprite;

public:
	CDibDoc();

	// Attributes
	HDIB GetHDIB() const {
		return m_hDIB;
	}
	CPalette *GetDocPalette() const {
		return m_palDIB;
	}
	CPalette *DetachPalette();
	CSize GetDocSize() const {
		return m_sizeDoc;
	}

	// Operations
	void ReplaceHDIB(HDIB hDIB);
	void InitDIBData();

	// Implementation
	virtual ~CDibDoc();
	virtual bool SaveDocument(const char* pszPathName);
	virtual bool OpenDocument(const char* pszPathName);
	virtual bool OpenResourceDocument(const int nResID);
	virtual bool OpenResourceDocument(const char* pszPathName);

private:
	HDIB m_hDIB = nullptr;
	CPalette *m_palDIB = nullptr;
	CSize m_sizeDoc;

protected:
	//{{AFX_MSG(CDibDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
