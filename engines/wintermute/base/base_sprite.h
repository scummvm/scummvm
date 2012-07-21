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

#ifndef WINTERMUTE_BSPRITE_H
#define WINTERMUTE_BSPRITE_H


#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/base/base_script_holder.h"

namespace WinterMute {
class CBFrame;
class CBSurface;
class CBObject;
class CBSprite: public CBScriptHolder {
public:
	bool killAllSounds();
	CBSurface *getSurface();
	char *_editorBgFile;
	int _editorBgOffsetX;
	int _editorBgOffsetY;
	int _editorBgAlpha;
	bool _streamed;
	bool _streamedKeepLoaded;
	void cleanup();
	void setDefaults();
	bool _precise;
	DECLARE_PERSISTENT(CBSprite, CBScriptHolder)

	bool _editorAllFrames;
	bool getBoundingRect(Rect32 *rect, int x, int y, float scaleX = 100, float scaleY = 100);
	int _moveY;
	int _moveX;
	bool display(int x, int y, CBObject *registerOwner = NULL, float zoomX = 100, float zoomY = 100, uint32 alpha = 0xFFFFFFFF, float rotate = 0.0f, TSpriteBlendMode blendMode = BLEND_NORMAL);
	bool GetCurrentFrame(float zoomX = 100, float zoomY = 100);
	bool _canBreak;
	bool _editorMuted;
	bool _continuous;
	void reset();
	CBObject *_owner;
	bool _changed;
	bool _paused;
	bool _finished;
	bool loadBuffer(byte *buffer, bool compete = true, int lifeTime = -1, TSpriteCacheType cacheType = CACHE_ALL);
	bool loadFile(const char *filename, int lifeTime = -1, TSpriteCacheType cacheType = CACHE_ALL);
	uint32 _lastFrameTime;
	bool draw(int x, int y, CBObject *Register = NULL, float zoomX = 100, float zoomY = 100, uint32 alpha = 0xFFFFFFFF);
	bool _looping;
	int _currentFrame;
	bool addFrame(const char *filename, uint32 delay = 0, int hotspotX = 0, int hotspotY = 0, Rect32 *rect = NULL);
	CBSprite(CBGame *inGame, CBObject *owner = NULL);
	virtual ~CBSprite();
	CBArray<CBFrame *, CBFrame *> _frames;
	bool saveAsText(CBDynBuffer *buffer, int indent);

	// scripting interface
	virtual CScValue *scGetProperty(const char *name);
	virtual bool scSetProperty(const char *name, CScValue *value);
	virtual bool scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name);
	virtual const char *scToString();
};

} // end of namespace WinterMute

#endif
