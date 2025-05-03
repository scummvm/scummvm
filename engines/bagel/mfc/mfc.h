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

#ifndef BAGEL_MFC_MFC_H
#define BAGEL_MFC_MFC_H

#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/winnt.h"
#include "bagel/mfc/atltime.h"
#include "bagel/mfc/atltypes.h"

namespace Bagel {
namespace MFC {

class CObject {
protected:
	virtual ~CObject() = 0;
public:
	CObject();
	CObject(const CObject &objectSrc);
};

class CGdiObject : public CObject {
public:
	~CGdiObject() override {}
};

class CPen : public CGdiObject {
public:
	~CPen() override {}
};

class CBrush : public CGdiObject {
public:
	~CBrush() override {}
};

class CFont : public CGdiObject {
public:
	~CFont() override {}
};

class CBitmap : public CGdiObject {
public:
	~CBitmap() override {}
};

class CPalette : public CGdiObject {
public:
	~CPalette() override {}
};

class CRgn : public CGdiObject {
public:
	~CRgn() override {}
};

class CDC : public CObject {
public:
	~CDC() override {}
};

class CCmdTarget : public CObject {
public:
	~CCmdTarget() override {}
};

class CWnd : public CCmdTarget {
public:
	~CWnd() override {}
};

class CDialog : public CWnd {
public:
	~CDialog() override {}
};

class CStatic : public CWnd {
public:
	~CStatic() override {}
};

class CButton : public CWnd {
public:
	~CButton() override {}
};

class CListBox : public CWnd {
public:
	~CListBox() override {}
};

class CCheckListBox : public CListBox {
public:
	~CCheckListBox() override {}
};

class CView : public CWnd {
public:
	~CView() override {}
};

class CScrollView : public CView {
public:
	~CScrollView() override {}
};

class CWinThread : public CCmdTarget {
public:
	~CWinThread() override {}
};

class CWinApp : public CWinThread {
public:
	~CWinApp() override {}
};

} // namespace MFC
} // namespace Bagel

#endif
