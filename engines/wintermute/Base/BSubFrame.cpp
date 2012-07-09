/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BActiveRect.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSubFrame, false)

//////////////////////////////////////////////////////////////////////////
CBSubFrame::CBSubFrame(CBGame *inGame): CBScriptable(inGame, true) {
	_surface = NULL;
	_hotspotX = _hotspotY = 0;
	_alpha = 0xFFFFFFFF;
	_transparent = 0xFFFF00FF;

	CBPlatform::setRectEmpty(&_rect);

	_editorSelected = false;

	_surfaceFilename = NULL;
	_cKDefault = true;
	_cKRed = _cKBlue = _cKGreen = 0;
	_lifeTime = -1;
	_keepLoaded = false;

	_2DOnly = _3DOnly = false;
	_decoration = false;

	_mirrorX = _mirrorY = false;
}


//////////////////////////////////////////////////////////////////////////
CBSubFrame::~CBSubFrame() {
	if (_surface) Game->_surfaceStorage->removeSurface(_surface);
	delete[] _surfaceFilename;
	_surfaceFilename = NULL;
}


TOKEN_DEF_START
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(RECT)
TOKEN_DEF(HOTSPOT)
TOKEN_DEF(2D_ONLY)
TOKEN_DEF(3D_ONLY)
TOKEN_DEF(DECORATION)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(MIRROR_X)
TOKEN_DEF(MIRROR_Y)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::loadBuffer(byte *buffer, int lifeTime, bool keepLoaded) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(RECT)
	TOKEN_TABLE(HOTSPOT)
	TOKEN_TABLE(2D_ONLY)
	TOKEN_TABLE(3D_ONLY)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(MIRROR_X)
	TOKEN_TABLE(MIRROR_Y)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);
	Rect32 rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	bool custoTrans = false;
	CBPlatform::setRectEmpty(&rect);
	char *surfaceFile = NULL;

	delete _surface;
	_surface = NULL;

	while ((cmd = parser.getCommand((char **)&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_IMAGE:
			surfaceFile = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			custoTrans = true;
			break;

		case TOKEN_RECT:
			parser.scanStr(params, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			break;

		case TOKEN_HOTSPOT:
			parser.scanStr(params, "%d,%d", &_hotspotX, &_hotspotY);
			break;

		case TOKEN_2D_ONLY:
			parser.scanStr(params, "%b", &_2DOnly);
			break;

		case TOKEN_3D_ONLY:
			parser.scanStr(params, "%b", &_3DOnly);
			break;

		case TOKEN_MIRROR_X:
			parser.scanStr(params, "%b", &_mirrorX);
			break;

		case TOKEN_MIRROR_Y:
			parser.scanStr(params, "%b", &_mirrorY);
			break;

		case TOKEN_DECORATION:
			parser.scanStr(params, "%b", &_decoration);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr(params, "%b", &_editorSelected);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty((byte *)params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SUBFRAME definition");
		return STATUS_FAILED;
	}

	if (surfaceFile != NULL) {
		if (custoTrans) setSurface(surfaceFile, false, r, g, b, lifeTime, keepLoaded);
		else setSurface(surfaceFile, true, 0, 0, 0, lifeTime, keepLoaded);
	}

	_alpha = BYTETORGBA(ar, ag, ab, alpha);
	if (custoTrans) _transparent = BYTETORGBA(r, g, b, 0xFF);

	/*
	if(_surface == NULL)
	{
	    Game->LOG(0, "Error parsing sub-frame. Image not set.");
	    return STATUS_FAILED;
	}
	*/
	if (CBPlatform::isRectEmpty(&rect)) setDefaultRect();
	else _rect = rect;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::draw(int x, int y, CBObject *registerOwner, float zoomX, float zoomY, bool precise, uint32 alpha, float rotate, TSpriteBlendMode blendMode) {
	if (!_surface) return STATUS_OK;

	if (registerOwner != NULL && !_decoration) {
		if (zoomX == 100 && zoomY == 100) {
			Game->_renderer->_rectList.add(new CBActiveRect(Game, registerOwner, this, x - _hotspotX + _rect.left, y  - _hotspotY + _rect.top, _rect.right - _rect.left, _rect.bottom - _rect.top, zoomX, zoomY, precise));
		} else {
			Game->_renderer->_rectList.add(new CBActiveRect(Game, registerOwner, this, (int)(x - (_hotspotX + _rect.left) * (zoomX / 100)), (int)(y - (_hotspotY + _rect.top) * (zoomY / 100)), (int)((_rect.right - _rect.left) * (zoomX / 100)), (int)((_rect.bottom - _rect.top) * (zoomY / 100)), zoomX, zoomY, precise));
		}
	}
	if (Game->_suspendedRendering) return STATUS_OK;

	ERRORCODE res;

	//if(Alpha==0xFFFFFFFF) Alpha = _alpha; // TODO: better (combine owner's and self alpha)
	if (_alpha != 0xFFFFFFFF) alpha = _alpha;

	if (rotate != 0.0f) {
		res = _surface->displayTransform((int)(x - _hotspotX * (zoomX / 100)), (int)(y - _hotspotY * (zoomY / 100)), _hotspotX, _hotspotY, _rect, zoomX, zoomY, alpha, rotate, blendMode, _mirrorX, _mirrorY);
	} else {
		if (zoomX == 100 && zoomY == 100) res = _surface->displayTrans(x - _hotspotX, y - _hotspotY, _rect, alpha, blendMode, _mirrorX, _mirrorY);
		else res = _surface->displayTransZoom((int)(x - _hotspotX * (zoomX / 100)), (int)(y - _hotspotY * (zoomY / 100)), _rect, zoomX, zoomY, alpha, blendMode, _mirrorX, _mirrorY);
	}

	return res;
}


//////////////////////////////////////////////////////////////////////////
bool CBSubFrame::getBoundingRect(Rect32 *rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) return false;

	float ratioX = scaleX / 100.0f;
	float ratioY = scaleY / 100.0f;

	CBPlatform::setRect(rect,
	                    (int)(x - _hotspotX * ratioX),
	                    (int)(y - _hotspotY * ratioY),
	                    (int)(x - _hotspotX * ratioX + (_rect.right - _rect.left) * ratioX),
	                    (int)(y - _hotspotY * ratioY + (_rect.bottom - _rect.top) * ratioY));
	return true;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::saveAsText(CBDynBuffer *buffer, int indent, bool complete) {
	if (complete)
		buffer->putTextIndent(indent, "SUBFRAME {\n");

	if (_surface && _surface->getFileName() != "")
		buffer->putTextIndent(indent + 2, "IMAGE = \"%s\"\n", _surface->getFileName());

	if (_transparent != 0xFFFF00FF)
		buffer->putTextIndent(indent + 2, "TRANSPARENT { %d,%d,%d }\n", RGBCOLGetR(_transparent), RGBCOLGetG(_transparent), RGBCOLGetB(_transparent));

	Rect32 rect;
	CBPlatform::setRectEmpty(&rect);
	if (_surface) CBPlatform::setRect(&rect, 0, 0, _surface->getWidth(), _surface->getHeight());
	if (!CBPlatform::equalRect(&rect, &_rect))
		buffer->putTextIndent(indent + 2, "RECT { %d,%d,%d,%d }\n", _rect.left, _rect.top, _rect.right, _rect.bottom);

	if (_hotspotX != 0 || _hotspotY != 0)
		buffer->putTextIndent(indent + 2, "HOTSPOT {%d, %d}\n", _hotspotX, _hotspotY);

	if (_alpha != 0xFFFFFFFF) {
		buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", RGBCOLGetR(_alpha), RGBCOLGetG(_alpha), RGBCOLGetB(_alpha));
		buffer->putTextIndent(indent + 2, "ALPHA = %d\n", RGBCOLGetA(_alpha));
	}

	if (_mirrorX)
		buffer->putTextIndent(indent + 2, "MIRROR_X=%s\n", _mirrorX ? "TRUE" : "FALSE");

	if (_mirrorY)
		buffer->putTextIndent(indent + 2, "MIRROR_Y=%s\n", _mirrorY ? "TRUE" : "FALSE");

	if (_2DOnly)
		buffer->putTextIndent(indent + 2, "2D_ONLY=%s\n", _2DOnly ? "TRUE" : "FALSE");

	if (_3DOnly)
		buffer->putTextIndent(indent + 2, "3D_ONLY=%s\n", _3DOnly ? "TRUE" : "FALSE");

	if (_decoration)
		buffer->putTextIndent(indent + 2, "DECORATION=%s\n", _decoration ? "TRUE" : "FALSE");

	if (_editorSelected)
		buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");

	CBBase::saveAsText(buffer, indent + 2);


	if (complete)
		buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBSubFrame::setDefaultRect() {
	if (_surface) {
		CBPlatform::setRect(&_rect, 0, 0, _surface->getWidth(), _surface->getHeight());
	} else CBPlatform::setRectEmpty(&_rect);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_2DOnly));
	persistMgr->transfer(TMEMBER(_3DOnly));
	persistMgr->transfer(TMEMBER(_alpha));
	persistMgr->transfer(TMEMBER(_decoration));
	persistMgr->transfer(TMEMBER(_editorSelected));
	persistMgr->transfer(TMEMBER(_hotspotX));
	persistMgr->transfer(TMEMBER(_hotspotY));
	persistMgr->transfer(TMEMBER(_rect));

	persistMgr->transfer(TMEMBER(_surfaceFilename));
	persistMgr->transfer(TMEMBER(_cKDefault));
	persistMgr->transfer(TMEMBER(_cKRed));
	persistMgr->transfer(TMEMBER(_cKGreen));
	persistMgr->transfer(TMEMBER(_cKBlue));
	persistMgr->transfer(TMEMBER(_lifeTime));

	persistMgr->transfer(TMEMBER(_keepLoaded));
	persistMgr->transfer(TMEMBER(_mirrorX));
	persistMgr->transfer(TMEMBER(_mirrorY));
	persistMgr->transfer(TMEMBER(_transparent));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetImage") == 0) {
		stack->correctParams(0);

		if (!_surfaceFilename) stack->pushNULL();
		else stack->pushString(_surfaceFilename);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetImage") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		if (Val->isNULL()) {
			if (_surface) Game->_surfaceStorage->removeSurface(_surface);
			delete[] _surfaceFilename;
			_surfaceFilename = NULL;
			stack->pushBool(true);
		} else {
			const char *filename = Val->getString();
			if (DID_SUCCEED(setSurface(filename))) {
				setDefaultRect();
				stack->pushBool(true);
			} else stack->pushBool(false);
		}

		return STATUS_OK;
	}

	else return CBScriptable::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSubFrame::scGetProperty(const char *name) {
	if (!_scValue) _scValue = new CScValue(Game);
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("subframe");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaColor") == 0) {

		_scValue->setInt((int)_alpha);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TransparentColor (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TransparentColor") == 0) {
		_scValue->setInt((int)_transparent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is2DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Is2DOnly") == 0) {
		_scValue->setBool(_2DOnly);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is3DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Is3DOnly") == 0) {
		_scValue->setBool(_3DOnly);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MirrorX") == 0) {
		_scValue->setBool(_mirrorX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MirrorY") == 0) {
		_scValue->setBool(_mirrorY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Decoration") == 0) {
		_scValue->setBool(_decoration);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HotspotX") == 0) {
		_scValue->setInt(_hotspotX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HotspotY") == 0) {
		_scValue->setInt(_hotspotY);
		return _scValue;
	}

	else return CBScriptable::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "AlphaColor") == 0) {
		_alpha = (uint32)value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is2DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Is2DOnly") == 0) {
		_2DOnly = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is3DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Is3DOnly") == 0) {
		_3DOnly = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MirrorX") == 0) {
		_mirrorX = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MirrorY") == 0) {
		_mirrorY = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Decoration") == 0) {
		_decoration = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HotspotX") == 0) {
		_hotspotX = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HotspotY") == 0) {
		_hotspotY = value->getInt();
		return STATUS_OK;
	}

	else return CBScriptable::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBSubFrame::scToString() {
	return "[subframe]";
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::setSurface(const char *filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	if (_surface) {
		Game->_surfaceStorage->removeSurface(_surface);
		_surface = NULL;
	}

	delete[] _surfaceFilename;
	_surfaceFilename = NULL;

	_surface = Game->_surfaceStorage->addSurface(filename, defaultCK, ckRed, ckGreen, ckBlue, lifeTime, keepLoaded);
	if (_surface) {
		_surfaceFilename = new char[strlen(filename) + 1];
		strcpy(_surfaceFilename, filename);

		_cKDefault = defaultCK;
		_cKRed = ckRed;
		_cKGreen = ckGreen;
		_cKBlue = ckBlue;
		_lifeTime = lifeTime;
		_keepLoaded = keepLoaded;

		return STATUS_OK;
	} else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CBSubFrame::setSurfaceSimple() {
	if (!_surfaceFilename) {
		_surface = NULL;
		return STATUS_OK;
	}
	_surface = Game->_surfaceStorage->addSurface(_surfaceFilename, _cKDefault, _cKRed, _cKGreen, _cKBlue, _lifeTime, _keepLoaded);
	if (_surface) return STATUS_OK;
	else return STATUS_FAILED;
}

} // end of namespace WinterMute
