/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdafx.h>
#include "brenderer.h"

#include "common/engine.h" // for debug, warning, error

#include <assert.h>

void BaseRenderer::clean() {
	if(_data) {
		delete[] _data;
		_data = 0;
		_width = _height = 0;
	}
}

BaseRenderer::BaseRenderer() :
	_data(0),
	_frame(0),
	_nbframes(0),
	_width(0), 
	_height(0) {
}

BaseRenderer::~BaseRenderer() {
	clean();
}

bool BaseRenderer::initFrame(const Point &p) {
	clean();
	_width = p.getX();
	_height = p.getY();
	assert(_width && _height);
	_data = new char[_width * _height];
	if(!_data) error("base_renderer unable to allocate frame buffer");
	return true;
}

char *BaseRenderer::lockFrame(int32 frame) {
	_frame = frame; 
	if(!_data) error("no allocated image buffer in lock_frame");
	return _data;
}

bool BaseRenderer::unlockFrame() {
	return true;
}

bool BaseRenderer::flipFrame() {
	save();
	return true;
}

bool BaseRenderer::setPalette(const Palette & pal) {
	_pal = pal;
	return true;
}
