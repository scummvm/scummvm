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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "graphics/transform_tools.h"
#include "graphics/transform_struct.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseSubFrame, false)

//////////////////////////////////////////////////////////////////////////
BaseSubFrame::BaseSubFrame(BaseGame *inGame) : BaseScriptable(inGame, true) {
	_surface = nullptr;
	_hotspotX = Graphics::kDefaultHotspotX;
	_hotspotY = Graphics::kDefaultHotspotY;
	_alpha = Graphics::kDefaultRgbaMod;
	_transparent = 0xFFFF00FF;

	_wantsDefaultRect = false;
	_rect.setEmpty();

	_editorSelected = false;

	_surfaceFilename = nullptr;
	_cKDefault = true;
	_cKRed = _cKBlue = _cKGreen = 0;
	_lifeTime = -1;
	_keepLoaded = false;

	_2DOnly = _3DOnly = false;
	_decoration = false;

	_mirrorX = _mirrorY = false;
}


//////////////////////////////////////////////////////////////////////////
BaseSubFrame::~BaseSubFrame() {
	if (_surface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
	}
	delete[] _surfaceFilename;
	_surfaceFilename = nullptr;
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
bool BaseSubFrame::loadBuffer(char *buffer, int lifeTime, bool keepLoaded) {
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
	BaseParser parser;
	Rect32 rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	bool customTrans = false;
	rect.setEmpty();
	char *surfaceFile = nullptr;

	delete _surface;
	_surface = nullptr;

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_IMAGE:
			surfaceFile = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			customTrans = true;
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
			parseEditorProperty(params, false);
			break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		BaseEngine::LOG(0, "Syntax error in SUBFRAME definition");
		return STATUS_FAILED;
	}

	if (surfaceFile != nullptr) {
		if (customTrans) {
			setSurface(surfaceFile, false, r, g, b, lifeTime, keepLoaded);
		} else {
			setSurface(surfaceFile, true, 0, 0, 0, lifeTime, keepLoaded);
		}
	}

	_alpha = BYTETORGBA(ar, ag, ab, alpha);
	if (customTrans) {
		_transparent = BYTETORGBA(r, g, b, 0xFF);
	}

	/*
	if (_surface == nullptr)
	{
	    BaseEngine::LOG(0, "Error parsing sub-frame. Image not set.");
	    return STATUS_FAILED;
	}
	*/
	if (rect.isRectEmpty()) {
		setDefaultRect();
	} else {
		setRect(rect);
	}

	return STATUS_OK;
}

Rect32 BaseSubFrame::getRect() {
	if (_wantsDefaultRect && _surface) {
		_rect.setRect(0, 0, _surface->getWidth(), _surface->getHeight());
		_wantsDefaultRect = false;
	}
	return _rect;
}

void BaseSubFrame::setRect(Rect32 rect) {
	_wantsDefaultRect = false;
	_rect = rect;
}

const char* BaseSubFrame::getSurfaceFilename() {
	return _surfaceFilename;
}

//////////////////////////////////////////////////////////////////////
bool BaseSubFrame::draw(int x, int y, BaseObject *registerOwner, float zoomX, float zoomY, bool precise, uint32 alpha, float rotate, Graphics::TSpriteBlendMode blendMode) {

	rotate = fmod(rotate, 360.0f);
	if (rotate < 0) {
		rotate += 360.0f;
	}

	if (!_surface) {
		return STATUS_OK;
	}

	if (registerOwner != nullptr && !_decoration) {
		if (zoomX == Graphics::kDefaultZoomX && zoomY == Graphics::kDefaultZoomY) {
			BaseEngine::getRenderer()->addRectToList(new BaseActiveRect(_gameRef,  registerOwner, this, x - _hotspotX + getRect().left, y  - _hotspotY + getRect().top, getRect().right - getRect().left, getRect().bottom - getRect().top, zoomX, zoomY, precise));
		} else {
			BaseEngine::getRenderer()->addRectToList(new BaseActiveRect(_gameRef,  registerOwner, this, (int)(x - (_hotspotX + getRect().left) * (zoomX / 100)), (int)(y - (_hotspotY + getRect().top) * (zoomY / 100)), (int)((getRect().right - getRect().left) * (zoomX / 100)), (int)((getRect().bottom - getRect().top) * (zoomY / 100)), zoomX, zoomY, precise));
		}
	}
	if (_gameRef->getSuspendedRendering()) {
		return STATUS_OK;
	}

	bool res;

	//if (Alpha==0xFFFFFFFF) Alpha = _alpha; // TODO: better (combine owner's and self alpha)
	if (_alpha != Graphics::kDefaultRgbaMod) {
		alpha = _alpha;
	}

	if (rotate != Graphics::kDefaultAngle) {
		Point32 boxOffset, rotatedHotspot, hotspotOffset;
		Common::Point origin(x, y);
		Common::Point newOrigin;
		Rect32 oldRect1 = getRect();
		Common::Rect oldRect(oldRect1.left, oldRect1.top, oldRect1.right, oldRect1.bottom);
		Common::Point newHotspot;
		Graphics::TransformStruct transform = Graphics::TransformStruct(zoomX, zoomY, (uint32)rotate, _hotspotX, _hotspotY, blendMode, alpha, _mirrorX, _mirrorY, 0, 0);
		Rect32 newRect = Graphics::TransformTools::newRect(oldRect, transform, &newHotspot);
		newOrigin = origin - newHotspot;
		res = _surface->displayTransform(newOrigin.x, newOrigin.y, oldRect, newRect, transform);
	} else {
		if (zoomX == Graphics::kDefaultZoomX && zoomY == Graphics::kDefaultZoomY) {
			res = _surface->displayTrans(x - _hotspotX, y - _hotspotY, getRect(), alpha, blendMode, _mirrorX, _mirrorY);
		} else {
			res = _surface->displayTransZoom((int)(x - _hotspotX * (zoomX / Graphics::kDefaultZoomX)), (int)(y - _hotspotY * (zoomY / Graphics::kDefaultZoomY)), getRect(), zoomX, zoomY, alpha, blendMode, _mirrorX, _mirrorY);
		}
	}

	return res;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::getBoundingRect(Rect32 *rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) {
		return false;
	}

	float ratioX = scaleX / 100.0f;
	float ratioY = scaleY / 100.0f;

	rect->setRect((int)(x - _hotspotX * ratioX),
				  (int)(y - _hotspotY * ratioY),
				  (int)(x - _hotspotX * ratioX + (getRect().right - getRect().left) * ratioX),
				  (int)(y - _hotspotY * ratioY + (getRect().bottom - getRect().top) * ratioY));
	return true;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::saveAsText(BaseDynamicBuffer *buffer, int indent, bool complete) {
	if (complete) {
		buffer->putTextIndent(indent, "SUBFRAME {\n");
	}

	if (_surface && _surface->getFileNameStr() != "") {
		buffer->putTextIndent(indent + 2, "IMAGE = \"%s\"\n", _surface->getFileName());
	}

	if (_transparent != 0xFFFF00FF) {
		buffer->putTextIndent(indent + 2, "TRANSPARENT { %d,%d,%d }\n", RGBCOLGetR(_transparent), RGBCOLGetG(_transparent), RGBCOLGetB(_transparent));
	}

	Rect32 rect;
	rect.setEmpty();
	if (_surface) {
		rect.setRect(0, 0, _surface->getWidth(), _surface->getHeight());
	}
	if (!(rect == getRect())) {
		buffer->putTextIndent(indent + 2, "RECT { %d,%d,%d,%d }\n", getRect().left, getRect().top, getRect().right, getRect().bottom);
	}

	if (_hotspotX != 0 || _hotspotY != 0) {
		buffer->putTextIndent(indent + 2, "HOTSPOT {%d, %d}\n", _hotspotX, _hotspotY);
	}

	if (_alpha != 0xFFFFFFFF) {
		buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", RGBCOLGetR(_alpha), RGBCOLGetG(_alpha), RGBCOLGetB(_alpha));
		buffer->putTextIndent(indent + 2, "ALPHA = %d\n", RGBCOLGetA(_alpha));
	}

	if (_mirrorX) {
		buffer->putTextIndent(indent + 2, "MIRROR_X=%s\n", _mirrorX ? "TRUE" : "FALSE");
	}

	if (_mirrorY) {
		buffer->putTextIndent(indent + 2, "MIRROR_Y=%s\n", _mirrorY ? "TRUE" : "FALSE");
	}

	if (_2DOnly) {
		buffer->putTextIndent(indent + 2, "2D_ONLY=%s\n", _2DOnly ? "TRUE" : "FALSE");
	}

	if (_3DOnly) {
		buffer->putTextIndent(indent + 2, "3D_ONLY=%s\n", _3DOnly ? "TRUE" : "FALSE");
	}

	if (_decoration) {
		buffer->putTextIndent(indent + 2, "DECORATION=%s\n", _decoration ? "TRUE" : "FALSE");
	}

	if (_editorSelected) {
		buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	}

	BaseClass::saveAsText(buffer, indent + 2);


	if (complete) {
		buffer->putTextIndent(indent, "}\n\n");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseSubFrame::setDefaultRect() {
	if (_surface) {
		_wantsDefaultRect = true;
	} else {
		_wantsDefaultRect = false;
		_rect.setEmpty();
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::persist(BasePersistenceManager *persistMgr) {

	BaseScriptable::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_2DOnly));
	persistMgr->transferBool(TMEMBER(_3DOnly));
	persistMgr->transferUint32(TMEMBER(_alpha));
	persistMgr->transferBool(TMEMBER(_decoration));
	persistMgr->transferBool(TMEMBER(_editorSelected));
	persistMgr->transferSint32(TMEMBER(_hotspotX));
	persistMgr->transferSint32(TMEMBER(_hotspotY));
	persistMgr->transferRect32(TMEMBER(_rect));
	persistMgr->transferBool(TMEMBER(_wantsDefaultRect));

	persistMgr->transferCharPtr(TMEMBER(_surfaceFilename));
	persistMgr->transferBool(TMEMBER(_cKDefault));
	persistMgr->transferByte(TMEMBER(_cKRed));
	persistMgr->transferByte(TMEMBER(_cKGreen));
	persistMgr->transferByte(TMEMBER(_cKBlue));
	persistMgr->transferSint32(TMEMBER(_lifeTime));

	persistMgr->transferBool(TMEMBER(_keepLoaded));
	persistMgr->transferBool(TMEMBER(_mirrorX));
	persistMgr->transferBool(TMEMBER(_mirrorY));
	persistMgr->transferUint32(TMEMBER(_transparent));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetImage") == 0) {
		stack->correctParams(0);

		if (!_surfaceFilename) {
			stack->pushNULL();
		} else {
			stack->pushString(_surfaceFilename);
		}
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetHeight
	// Used to find sprite center at methods.script in fix_offset()
	// Return value is integer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHeight") == 0) {
		stack->correctParams(0);
		if (_surface) {
			stack->pushInt(_surface->getHeight());
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetWidth
	// Used to find sprite center at methods.script in fix_offset()
	// Return value is integer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWidth") == 0) {
		stack->correctParams(0);
		if (_surface) {
			stack->pushInt(_surface->getWidth());
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] GetPixelAt
	// Used for dynamic light at mixing.script in make_RGB() and make_HSV()
	// Return value is passed to Game.GetRValue(), Game.GetGValue(), etc...
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPixelAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		byte r, g, b, a;
		if (_surface && _surface->getPixel(x, y, &r, &g, &b, &a)) {
			uint32 pixel = BYTETORGBA(r, g, b, a);
			stack->pushInt(pixel);
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetImage") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (val->isNULL()) {
			if (_surface) {
				_gameRef->_surfaceStorage->removeSurface(_surface);
			}
			delete[] _surfaceFilename;
			_surfaceFilename = nullptr;
			stack->pushBool(true);
		} else {
			const char *filename = val->getString();
			if (DID_SUCCEED(setSurface(filename))) {
				setDefaultRect();
				stack->pushBool(true);
			} else {
				stack->pushBool(false);
			}
		}

		return STATUS_OK;
	} else {
		return BaseScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseSubFrame::scGetProperty(const Common::String &name) {
	if (!_scValue) {
		_scValue = new ScValue(_gameRef);
	}
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("subframe");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AlphaColor") {

		_scValue->setInt((int)_alpha);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TransparentColor (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TransparentColor") {
		_scValue->setInt((int)_transparent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is2DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Is2DOnly") {
		_scValue->setBool(_2DOnly);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is3DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Is3DOnly") {
		_scValue->setBool(_3DOnly);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MirrorX") {
		_scValue->setBool(_mirrorX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MirrorY") {
		_scValue->setBool(_mirrorY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Decoration") {
		_scValue->setBool(_decoration);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "HotspotX") {
		_scValue->setInt(_hotspotX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "HotspotY") {
		_scValue->setInt(_hotspotY);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::scSetProperty(const char *name, ScValue *value) {
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
	} else {
		return BaseScriptable::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseSubFrame::scToString() {
	return "[subframe]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::setSurface(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	if (_surface) {
		_gameRef->_surfaceStorage->removeSurface(_surface);
		_surface = nullptr;
	}

	delete[] _surfaceFilename;
	_surfaceFilename = nullptr;

	_surface = _gameRef->_surfaceStorage->addSurface(filename, defaultCK, ckRed, ckGreen, ckBlue, lifeTime, keepLoaded);
	if (_surface) {
		_surfaceFilename = new char[filename.size() + 1];
		strcpy(_surfaceFilename, filename.c_str());

		_cKDefault = defaultCK;
		_cKRed = ckRed;
		_cKGreen = ckGreen;
		_cKBlue = ckBlue;
		_lifeTime = lifeTime;
		_keepLoaded = keepLoaded;

		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseSubFrame::setSurfaceSimple() {
	if (!_surfaceFilename) {
		_surface = nullptr;
		return STATUS_OK;
	}
	_surface = _gameRef->_surfaceStorage->addSurface(_surfaceFilename, _cKDefault, _cKRed, _cKGreen, _cKBlue, _lifeTime, _keepLoaded);
	if (_surface) {
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}

Common::String BaseSubFrame::debuggerToString() const {
	return Common::String::format("%p: BaseSubFrame \"%s\" - Mirror:(%d, %d), Hotspot:(%d, %d), ", (const void *)this, getName(), _mirrorX, _mirrorY, _hotspotX, _hotspotY);
}

} // End of namespace Wintermute
