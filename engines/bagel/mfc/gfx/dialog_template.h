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

#ifndef BAGEL_MFC_GFX_DIALOG_TEMPLATE_H
#define BAGEL_MFC_GFX_DIALOG_TEMPLATE_H

#include "graphics/managed_surface.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/afxstr.h"

namespace Bagel {
namespace MFC {

class CWnd;

namespace Gfx {


typedef struct {
	DWORD style;
	DWORD dwExtendedStyle;
	WORD cdit;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATE;
typedef CONST DLGTEMPLATE *LPCDLGTEMPLATE;
typedef DLGTEMPLATE *LPDLGTEMPLATE;

class CDialogTemplate {
protected:
	static BYTE *AFX_CDECL GetFontSizeField(const DLGTEMPLATE *pTemplate);
	static UINT AFX_CDECL GetTemplateSize(const DLGTEMPLATE *pTemplate);
	BOOL SetTemplate(const DLGTEMPLATE *pTemplate, UINT cb);

public:
	HGLOBAL m_hTemplate;
	DWORD m_dwTemplateSize;
	BOOL m_bSystemFont;

public:
	CDialogTemplate(const DLGTEMPLATE *pTemplate = NULL);
	CDialogTemplate(HGLOBAL hGlobal);
	~CDialogTemplate();

	BOOL HasFont() const;
	BOOL SetFont(LPCSTR lpFaceName, WORD nFontSize);
	BOOL SetSystemFont(WORD nFontSize = 0);
	BOOL GetFont(CString &strFaceName, WORD &nFontSize) const;
	void GetSizeInDialogUnits(SIZE *pSize) const;
	void GetSizeInPixels(SIZE *pSize) const;

	static BOOL GetFont(const DLGTEMPLATE *pTemplate,
		CString &strFaceName, WORD &nFontSize);

	// Operations
	BOOL Load(LPCSTR lpDialogTemplateID);
	HGLOBAL Detach();
};

} // namespace Gfx

using Gfx::LPDLGTEMPLATE;
using Gfx::LPCDLGTEMPLATE;
using Gfx::CDialogTemplate;

} // namespace MFC
} // namespace Bagel

#endif
