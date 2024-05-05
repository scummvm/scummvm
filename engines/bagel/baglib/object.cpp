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

#include "bagel/baglib/object.h"
#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/master_win.h"

namespace Bagel {

extern bool g_noMenuFl;

CBofString getStringTypeOfObject(BagObjectType n) {
	switch (n) {
	case BASE_OBJ:
		return "UKN";
	case BMP_OBJ:
		return "BMP";
	case SPRITE_OBJ:
		return "SPR";
	case BUTTON_OBJ:
		return "BUT";
	case SOUND_OBJ:
		return "SND";
	case LINK_OBJ:
		return "LNK";
	case RESPRNT_OBJ:
		return "RPO";
	case DOSSIER_OBJ:
		return "DOS";
	case CHAR_OBJ:
		return "CHR";
	case AREA_OBJ:
		return "ARE";
	case MOVIE_OBJ:
		return "MOVIE";
	case VAR_OBJ:
		return "VAR";
	case COMMAND_OBJ:
		return "COMMAND";
	case THING_OBJ:
		return "TNG";
	default:
		return "UKN";
	};
}

CBagObject::CBagObject() {
	_xObjType = BASE_OBJ;
	_nX = -1;
	_nY = -1;

	// All objects now default to timeless
	_nProperties = TIMELESS;

	_nId = 0;
	_nOverCursor = 0;

	// Init state
	_nState = 0;

	_pMenu = nullptr;
	_pEvalExpr = nullptr;
	_bInteractive = true;
	_bNoMenu = false;

	// Allocate this on an as-needed basis...
	_psName = nullptr;

	// Object is dirty by default, doesn't break anything else
	// Some objects such as the pda message light are always updated
	setDirty(true);
	setAlwaysUpdate(false);

	// All messages start as not message waiting.
	setMsgWaiting(false);
}

CBagObject::~CBagObject() {
	if (_pMenu != nullptr) {
		delete _pMenu;
		_pMenu = nullptr;
	}

	if (_psName && (_psName != &_sFileName)) {
		delete _psName;
	}

	_psName = nullptr;
}

ErrorCode CBagObject::attach() {
	setVisible();
	return CBagParseObject::attach();
}

ErrorCode CBagObject::detach() {
	setVisible(false);
	return CBagParseObject::detach();
}

ErrorCode CBagObject::update(CBofBitmap * /*pBmp*/, CPoint /*pt*/, CRect * /*pSrcRect*/, int /*nMaskColor*/) {
	return ERR_NONE;
}

int CBagObject::getProperty(const CBofString &sProp) {
	if (!sProp.find("STATE"))
		return getState();

	if (!sProp.find("MODAL"))
		return isModal();

	return 0;
}

void CBagObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.find("STATE"))
		setState(nVal);
	else if (!sProp.find("TIMELESS")) {
		if (nVal)
			setTimeless();
		else
			setTimeless(false);
	} else if (!sProp.find("MODAL")) {
		if (nVal)
			setModal();
		else
			setModal(false);
	}
}

void CBagObject::setProperty(BAG_OBJECT_PROPERTIES xProp, bool bVal) {
	if (bVal)
		_nProperties |= xProp;
	else
		_nProperties &= ~xProp;
}

bool CBagObject::runObject() {
	if (isTimeless())
		return true;

	VAR_MANAGER->IncrementTimers();

	return true;
}

ParseCodes CBagObject::setInfo(CBagIfstream &istr) {
	ParseCodes rc = UNKNOWN_TOKEN;

	while (!istr.eof()) {
		istr.eatWhite();
		char ch = (char)istr.getCh();
		switch (ch) {
		//
		//  =filename.ext
		//
		case '=': {
			rc = UPDATED_OBJECT;
			char szLocalBuff[256];
			szLocalBuff[0] = 0;
			CBofString s(szLocalBuff, 256);
			getAlphaNumFromStream(istr, s);
			MACROREPLACE(s);
			setFileName(s);
			break;
		}
		//
		//  { menu objects .... }  - Add menu items
		//
		case '{': {
			rc = UPDATED_OBJECT;
			if (!_pMenu) {
				if ((_pMenu = new CBagMenu) != nullptr) {

					// Try to cut down the number of Storage Devices by
					// removing these unused ones from the list.
					//
					SDEV_MANAGER->UnRegisterStorageDev(_pMenu);
				}
			}

			istr.putBack();

			char szBuff[256];
			Common::sprintf_s(szBuff, "Menu:%d", CBagMasterWin::_menuCount++);
			CBofString s(szBuff, 256);

			_pMenu->loadFileFromStream(istr, s, false);
		}
		break;
		//
		//  ^id;  - Set id
		//
		case '^': {
			rc = UPDATED_OBJECT;
			char c = (char)istr.peek();
			if (Common::isDigit(c)) {
				int nId;
				getIntFromStream(istr, nId);
				setRefId(nId);
			} else {
				char szLocalBuff[256];
				szLocalBuff[0] = 0;
				CBofString s(szLocalBuff, 256);
				getAlphaNumFromStream(istr, s);
				setRefName(s);
			}
			break;
		}
		//
		//  *state;  - Set state
		//
		case '*': {
			rc = UPDATED_OBJECT;
			int nState;
			getIntFromStream(istr, nState);
			setState(nState);
			break;
		}
		//
		//  %cusror;  - Set cursor
		//
		case '%': {
			rc = UPDATED_OBJECT;
			int nCursor;
			getIntFromStream(istr, nCursor);
			setOverCursor(nCursor);
			break;
		}
		//
		//  [left,top,right,bottom]  - Set position
		//
		case '[': {
			rc = UPDATED_OBJECT;
			CBofRect r;
			istr.putBack();
			getRectFromStream(istr, r);
			setPosition(r.topLeft());
			if (r.width() && r.height())
				setSize(CSize(r.width(), r.height()));
			break;
		}
		//
		//  IS [NOT] [MOVABLE|MODAL|VISIBLE|STRETCHABLE|HIGHLIGHT|ACTIVE|TRANSPARENT|HIDE_ON_CLICK|IMMEDIATE_RUN|LOCAL|CONSTANT_UPDATE|PRELOAD|FOREGROUND]
		//
		case 'I': {
			if (istr.peek() != 'S') {
				istr.putBack();
				return rc;
				break;
			}
			char szLocalBuff[256];
			szLocalBuff[0] = 0;
			CBofString s(szLocalBuff, 256);
			bool b = true;
			istr.getCh();
			istr.eatWhite();
			getAlphaNumFromStream(istr, s);
			if (!s.find("NOT")) {
				getAlphaNumFromStream(istr, s);
				istr.eatWhite();
				b = false;
			}
			if (!s.find("MOVABLE")) {
				setMovable(b);
			} else if (!s.find("MODAL")) {
				setModal(b);
			} else if (!s.find("VISIBLE")) {
				setVisible(b);
			} else if (!s.find("STRETCHABLE")) {
				setStretchable(b);
			} else if (!s.find("HIGHLIGHT")) {
				setHighlight(b);
			} else if (!s.find("ACTIVE")) {
				setActive(b);
			} else if (!s.find("TRANSPARENT")) {
				setTransparent(b);
			} else if (!s.find("HIDE_ON_CLICK")) {
				setHideOnClick(b);
			} else if (!s.find("IMMEDIATE_RUN")) {
				setImmediateRun(b);
			} else if (!s.find("TIMELESS")) {
				setTimeless(b);
			} else if (!s.find("LOCAL")) {
				setLocal(b);
			} else if (!s.find("CONSTANT_UPDATE")) {
				setConstantUpdate(b);
			} else if (!s.find("PRELOAD")) {
				setPreload(b);
			} else if (!s.find("NOMENU")) {
				_bNoMenu = true;

			} else if (!s.find("FOREGROUND")) {
				setForeGround(b);
			} else {
				putbackStringOnStream(istr, s);
				if (!b)
					putbackStringOnStream(istr, " NOT ");
				putbackStringOnStream(istr, "IS ");
				return rc;
				break;
			}
			rc = UPDATED_OBJECT;
			break;
		}
		//
		//  ; is the end of the line, exit this object
		//
		case ';':
			return PARSING_DONE;
			break;

		//
		//  no match return from function
		//
		default:
			istr.putBack();
			return rc;
		}
	}

	return rc;
}

void CBagObject::onLButtonUp(uint32 nFlags, CBofPoint * /*xPoint*/, void *) {
	if (getMenuPtr()) {

		CBofPoint pt = getMousePos();
		CBofWindow *pWnd = CBofApp::getApp()->getMainWindow();
		pWnd->screenToClient(&pt);

		// Just send the mouse pos
		CRect r = getRect();
		getMenuPtr()->trackPopupMenu(nFlags, pt.x, pt.y, pWnd, nullptr, &r);

	} else {
		g_noMenuFl = true;
	}

	runObject();
}

bool CBagObject::onMouseMove(uint32 /*nFlags*/, CPoint /*xPoint*/, void *) {
	return false;
}

const CBofString &CBagObject::getRefName() {
	if (_psName) {
		return *_psName;
	}

	return _emptyString;
}

// Since we have an inordinate number of bag objects that replicate the
// filename as the objects name, if this is the case, then use a pointer
// to the filename.
void CBagObject::setRefName(const CBofString &s) {
	// If we got an old one hanging around then trash it.
	if (_psName && _psName != &_sFileName) {
		delete _psName;
	}
	_psName = nullptr;

	// If it's the same as the filename, then point to the filename
	if (!_sFileName.isEmpty() && _sFileName.compare(s) == 0) {
		_psName = &_sFileName;
	} else if (!s.isEmpty()) {
		_psName = new CBofString(s);
	}
}

} // namespace Bagel
