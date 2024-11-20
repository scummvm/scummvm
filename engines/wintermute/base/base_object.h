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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_OBJECT_H
#define WINTERMUTE_BASE_OBJECT_H


#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/persistent.h"
#include "common/events.h"
#include "graphics/transform_struct.h"

namespace Wintermute {

class BaseSprite;
class BaseSound;
class BaseSurface;
class BaseScriptHolder;
class ScValue;
class ScStack;
class ScScript;
class XModel;

class BaseObject : public BaseScriptHolder {
protected:
	bool _autoSoundPanning;
	uint32 _sFXStart;
	bool setSFXTime(uint32 time);
	bool setSFXVolume(int volume);
	bool resumeSFX();
	bool pauseSFX();
	bool stopSFX(bool deleteSound = true);
	bool playSFX(const char *filename, bool looping = false, bool playNow = true, const char *eventName = nullptr, uint32 loopStart = 0);
	BaseSound *_sFX;
	TSFXType _sFXType;
	float _sFXParam1;
	float _sFXParam2;
	float _sFXParam3;
	float _sFXParam4;
	float _relativeRotate;
	bool _rotateValid;
	float _rotate;
	void setSoundEvent(const char *eventName);
	bool _rotatable;
	float _scaleX;
	float _scaleY;
	float _relativeScale;
	bool _editorSelected;
	bool _editorAlwaysRegister;
	bool _ready;
	Rect32 _rect;
	bool _rectSet;
	int32 _iD;
	char *_soundEvent;
public:
	Graphics::TSpriteBlendMode _blendMode;
#ifdef ENABLE_WME3D
	virtual bool renderModel();
#endif
	virtual bool afterMove();
	float _scale;
	uint32 _alphaColor;
	virtual bool isReady();
	virtual bool getExtendedFlag(const char *flagName);
	virtual bool resetSoundPan();
	virtual bool updateSounds();
	bool updateOneSound(BaseSound *sound);
	int32 _sFXVolume;

	virtual bool handleMouseWheel(int32 delta);
	virtual bool handleMouse(TMouseEvent event, TMouseButton button);
	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual int32 getHeight();
	bool setCursor(const char *filename);
	bool setActiveCursor(const char *filename);
	bool cleanup();
	const char *getCaption(int caseVal = 1);
	void setCaption(const char *caption, int caseVal = 1);

	bool _editorOnly;
	bool _is3D;

	DECLARE_PERSISTENT(BaseObject, BaseScriptHolder)
	virtual bool showCursor();
	BaseSprite *_cursor;
	bool _sharedCursors;
	BaseSprite *_activeCursor;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;
	bool listen(BaseScriptHolder *param1, uint32 param2) override;

	bool _movable;
	bool _zoomable;
	bool _shadowable;
	int32 _posY;
	int32 _posX;
	bool _registrable;
	char *_caption[7];
	bool _saveState;

	BaseObject(BaseGame *inGame);
	~BaseObject() override;
	// base
	virtual bool update()  {
		return STATUS_FAILED;
	};
	virtual bool display() {
		return STATUS_FAILED;
	};
	virtual bool invalidateDeviceObjects()  {
		return STATUS_OK;
	};
	virtual bool restoreDeviceObjects()     {
		return STATUS_OK;
	};
	bool _nonIntMouseEvents;

#ifdef ENABLE_WME3D
	float _angle;
	XModel *_xmodel;
	XModel *_shadowModel;
	DXMatrix _worldMatrix;
	DXVector3 _posVector;
	bool getMatrix(DXMatrix *modelMatrix, DXVector3 *posVect = nullptr);
	uint32 _shadowColor;
	BaseSurface *_shadowImage;
	float _shadowSize;
	float _scale3D;
	DXVector3 _shadowLightPos;
	bool _drawBackfaces;
	TShadowType _shadowType;

	virtual uint32 getAnimTransitionTime(char *from, char *to) {
		return 0;
	};
#endif

public:
	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
};

} // End of namespace Wintermute

#endif
