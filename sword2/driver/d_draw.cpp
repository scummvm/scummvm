/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/menu.h"
#include "sword2/driver/render.h"

namespace Sword2 {

Graphics::Graphics(Sword2Engine *vm, int16 width, int16 height) 
	: _vm(vm), _iconCount(0), _needFullRedraw(false),
	  _fadeStatus(RDFADE_NONE), _mouseSprite(NULL), _mouseAnim(NULL),
	  _luggageAnim(NULL), _layer(0), _renderAverageTime(60),
	  _lightMask(NULL), _screenWide(width), _screenDeep(height) {

	int i, j;

	_buffer = _dirtyGrid = NULL;

	_buffer = (byte *) malloc(width * height);
	if (!_buffer)
		error("Could not initialise display");

	_vm->_system->init_size(width, height);

	_gridWide = width / CELLWIDE;
	_gridDeep = height / CELLDEEP;

	if ((width % CELLWIDE) || (height % CELLDEEP))
		error("Bad cell size");

	_dirtyGrid = (byte *) calloc(_gridWide, _gridDeep);
	if (!_buffer)
		error("Could not initialise dirty grid");

	for (i = 0; i < ARRAYSIZE(_blockSurfaces); i++)
		_blockSurfaces[i] = NULL;

	for (i = 0; i < 2; i++) {
		for (j = 0; j < RDMENU_MAXPOCKETS; j++) {
			_icons[i][j] = NULL;
			_pocketStatus[i][j] = 0;
		}

		_menuStatus[i] = RDMENU_HIDDEN;
	}
}

Graphics::~Graphics() {
	free(_buffer);
	free(_dirtyGrid);
	closeBackgroundLayer();
	free(_lightMask);
	free(_mouseAnim);
	free(_luggageAnim);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < RDMENU_MAXPOCKETS; j++)
			free(_icons[i][j]);
}

/**
 * @return the graphics detail setting
 */

int8 Graphics::getRenderLevel(void) {
	return _renderLevel;
}

void Graphics::setRenderLevel(int8 level) {
	_renderLevel = level;

	switch (_renderLevel) {
	case 0:
		// Lowest setting: no fancy stuff
		_renderCaps = 0;
		break;
	case 1:
		// Medium-low setting: transparency-blending
		_renderCaps = RDBLTFX_SPRITEBLEND;
		break;
	case 2:
		// Medium-high setting: transparency-blending + shading
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND;
		break;
	case 3:
		// Highest setting: transparency-blending + shading +
		// edge-blending + improved stretching
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND | RDBLTFX_EDGEBLEND;
		break;
	}
}

/**
 * Fill the screen buffer with palette colour zero. Note that it does not
 * touch the menu areas of the screen.
 */

void Graphics::clearScene(void) {
	memset(_buffer + MENUDEEP * _screenWide, 0, _screenWide * RENDERDEEP);
}

} // End of namespace Sword2
