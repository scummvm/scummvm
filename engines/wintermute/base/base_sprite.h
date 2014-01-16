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

#ifndef WINTERMUTE_BASE_SPRITE_H
#define WINTERMUTE_BASE_SPRITE_H


#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/base/base_script_holder.h"
#include "engines/wintermute/graphics/transform_tools.h"

namespace Wintermute {
class BaseFrame;
class BaseSurface;
class BaseObject;
class BaseSprite: public BaseScriptHolder {
public:
	BaseSurface *getSurface();
	void cleanup();
	void setDefaults();
	DECLARE_PERSISTENT(BaseSprite, BaseScriptHolder)

	bool getBoundingRect(Rect32 *rect, int x, int y, float scaleX = kDefaultZoomX, float scaleY = kDefaultZoomY);
	int32 _moveY;
	int32 _moveX;
	bool display(int x, int y, BaseObject *registerOwner = nullptr, float zoomX = kDefaultZoomX, float zoomY = kDefaultZoomY, uint32 alpha = kDefaultRgbaMod, float rotate = kDefaultAngle, TSpriteBlendMode blendMode = BLEND_NORMAL);
	bool getCurrentFrame(float zoomX = kDefaultZoomX, float zoomY = kDefaultZoomY);
	void reset();
	bool isChanged();
	bool isFinished();
	bool loadBuffer(char *buffer, bool compete = true, int lifeTime = -1, TSpriteCacheType cacheType = CACHE_ALL);
	bool loadFile(const Common::String &filename, int lifeTime = -1, TSpriteCacheType cacheType = CACHE_ALL);
	bool draw(int x, int y, BaseObject *Register = nullptr, float zoomX = kDefaultZoomX, float zoomY = kDefaultZoomY, uint32 alpha = kDefaultRgbaMod);
	bool _looping;
	int32 _currentFrame;
	bool addFrame(const char *filename, uint32 delay = 0, int hotspotX = 0, int hotspotY = 0, Rect32 *rect = nullptr);
	BaseSprite(BaseGame *inGame, BaseObject *owner = nullptr);
	virtual ~BaseSprite();
	BaseArray<BaseFrame *> _frames;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent);

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name) override;
	virtual bool scSetProperty(const char *name, ScValue *value) override;
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	virtual const char *scToString() override;
private:
	BaseObject *_owner;
	bool _canBreak;
	bool _changed;
	bool _editorAllFrames;
	char *_editorBgFile;
	int32 _editorBgOffsetX;
	int32 _editorBgOffsetY;
	int32 _editorBgAlpha;
	bool _editorMuted;
	bool _finished;
	bool _continuous;
	uint32 _lastFrameTime;
	bool _precise;
	bool _paused;
	bool _streamed;
	bool _streamedKeepLoaded;
	bool killAllSounds();
};

} // End of namespace Wintermute

#endif
