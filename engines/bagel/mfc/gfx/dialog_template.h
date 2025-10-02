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

#include "common/array.h"
#include "common/stream.h"
#include "graphics/managed_surface.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/afxstr.h"

namespace Bagel {
namespace MFC {

class CDialog;

namespace Gfx {

/*
 * Dialog Styles
 */
#define DS_ABSALIGN         0x01L
#define DS_SYSMODAL         0x02L
#define DS_LOCALEDIT        0x20L   /* 16-bit: Edit items get Local storage. 32-bit and up: meaningless. */
#define DS_SETFONT          0x40L   /* User specified font for Dlg controls */
#define DS_MODALFRAME       0x80L   /* Can be combined with WS_CAPTION  */
#define DS_NOIDLEMSG        0x100L  /* WM_ENTERIDLE message will not be sent */
#define DS_SETFOREGROUND    0x200L  /* not in win3.1 */

// Template data pointers. Note that they're
// void *because in ScummVM we do endian-safe
// loading of the data being pointed to
typedef const void *LPCDLGTEMPLATE;
typedef void *LPDLGTEMPLATE;

class CDialogTemplate {
	struct FontInfo {
		byte _pointSize;
		Common::String _fontName;
	};
	struct Header {
		uint16 _style = 0;
		byte _itemCount = 0;
		int16 _x = 0, _y = 0;
		int16 _w = 0, _h = 0;
		Common::String _menuName;
		Common::String _className;
		Common::String _caption;
		FontInfo _fontInfo;

		void load(Common::SeekableReadStream &src);
	};
	struct Item {
		uint16 _style = 0;
		int16 _x = 0;
		int16 _y = 0;
		int16 _w = 0;
		int16 _h = 0;
		uint16 _id = 0;

		Common::String _className;
		uint16 _titleId = 0;
		Common::String _title;
		Common::Array<byte> _data;

		void load(Common::SeekableReadStream &src);
	};
private:
	Header _header;
	Common::Array<Item> _items;

private:
	static byte *GetFontSizeField(LPCDLGTEMPLATE pTemplate);
	static unsigned int GetTemplateSize(LPCDLGTEMPLATE *pTemplate);
	bool setTemplate(LPCDLGTEMPLATE pTemplate);

	/**
	 * Gets the base pixel x/y amounts, used for
	 * translating DLU units to pixel units
	 */
	void getBaseUnits(int &x, int &y);

	LOGFONT ParseFontFromTemplate();

public:
	HGLOBAL m_hTemplate = nullptr;
	uint32 m_dwTemplateSize = 0;
	bool m_bSystemFont = false;

public:
	CDialogTemplate(LPCDLGTEMPLATE pTemplate = NULL);
	CDialogTemplate(HGLOBAL hGlobal);

	/**
	 * Sets up window with parsed template data
	 */
	void loadTemplate(CDialog *dialog);

	bool HasFont() const;
	bool SetFont(const char *lpFaceName, uint16 nFontSize);
	bool SetSystemFont(uint16 nFontSize = 0);
	bool GetFont(CString &strFaceName, uint16 &nFontSize) const;
	void GetSizeInDialogUnits(SIZE *pSize) const;
	void GetSizeInPixels(SIZE *pSize) const;

	static bool GetFont(LPCDLGTEMPLATE pTemplate,
		CString &strFaceName, uint16 &nFontSize);
};

} // namespace Gfx

using Gfx::LPDLGTEMPLATE;
using Gfx::LPCDLGTEMPLATE;
using Gfx::CDialogTemplate;

} // namespace MFC
} // namespace Bagel

#endif
