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

#ifndef GNAP_GROUPCS_H
#define GNAP_GROUPCS_H

#include "gnap/debugger.h"

namespace Gnap {

class GnapEngine;
class CutScene;

class Scene16: public CutScene {
public:
	Scene16(GnapEngine *vm);
	virtual ~Scene16() {}

	virtual int init();
};

class Scene471: public CutScene {
public:
	Scene471(GnapEngine *vm);
	virtual ~Scene471() {}

	virtual int init();
};

class Scene472: public CutScene {
public:
	Scene472(GnapEngine *vm);
	virtual ~Scene472() {}

	virtual int init();
};

class Scene473: public CutScene {
public:
	Scene473(GnapEngine *vm);
	virtual ~Scene473() {}

	virtual int init();
};

class Scene474: public CutScene {
public:
	Scene474(GnapEngine *vm);
	virtual ~Scene474() {}

	virtual int init();
};

class Scene475: public CutScene {
public:
	Scene475(GnapEngine *vm);
	virtual ~Scene475() {}

	virtual int init();
};

class Scene476: public CutScene {
public:
	Scene476(GnapEngine *vm);
	virtual ~Scene476() {}

	virtual int init();
};

class Scene477: public CutScene {
public:
	Scene477(GnapEngine *vm);
	virtual ~Scene477() {}

	virtual int init();
};

class Scene48: public CutScene {
public:
	Scene48(GnapEngine *vm);
	virtual ~Scene48() {}

	virtual int init();
};

class Scene541: public CutScene {
public:
	Scene541(GnapEngine *vm);
	virtual ~Scene541() {}

	virtual int init();
};

class Scene542: public CutScene {
public:
	Scene542(GnapEngine *vm);
	virtual ~Scene542() {}

	virtual int init();
};
} // End of namespace Gnap

#endif // GNAP_GROUPCS_H
