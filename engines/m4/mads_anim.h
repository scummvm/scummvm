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

#ifndef M4_MADS_ANIM_H
#define M4_MADS_ANIM_H

#include "m4/viewmgr.h"
#include "m4/compression.h"
#include "m4/animation.h"

#include "common/str-array.h"

namespace M4 {

typedef void (*TextviewCallback)(MadsM4Engine *vm);

class TextviewView : public View {
private:
	bool _animating;

	char _resourceName[80];
	Common::SeekableReadStream *_script;
	uint16 _spareScreens[10];
	M4Surface *_spareScreen;
	RGBList *_bgCurrent, *_bgSpare;
	int _translationX;
	int _panX, _panY, _panSpeed;
	int _panCountdown;
	char _currentLine[80];
	uint32 _scrollTimeout;
	int _scrollCount;
	int _lineY;
	M4Surface _bgSurface;
	M4Surface _textSurface;
	TextviewCallback _callback;
	bool _soundDriverLoaded;
	bool _processEvents;

	void reset();
	void processLines();
	void processCommand();
	void processText();
	int getParameter(char **paramP);
public:
	TextviewView(MadsM4Engine *vm);
	~TextviewView();

	void setScript(const char *resourceName, TextviewCallback callback);
	bool isAnimating() { return _animating; }
	void scriptDone();

	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
	void updateState();
};

typedef void (*AnimviewCallback)(MadsM4Engine *vm);

class AnimviewView : public View, MadsView {
private:
	char _resourceName[80];
	Common::SeekableReadStream *_script;
	bool _scriptDone;
	uint32 _previousUpdate;
	char _currentLine[80];
	M4Surface _backgroundSurface;
	M4Surface _codeSurface;
	AnimviewCallback _callback;
	bool _soundDriverLoaded;
	RGBList *_palData;
	int _transition;
	MadsAnimation *_activeAnimation;
	bool _bgLoadFlag;
	int _startFrame;

	void reset();
	void readNextCommand();
	void processCommand();
public:
	AnimviewView(MadsM4Engine *vm);
	~AnimviewView();

	void setScript(const char *resourceName, AnimviewCallback callback);
	void scriptDone();

	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
	void updateState();
};

}

#endif
