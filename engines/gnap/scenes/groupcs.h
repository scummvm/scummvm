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
	~Scene16() override {}

	int init() override;
};

class Scene471: public CutScene {
public:
	Scene471(GnapEngine *vm);
	~Scene471() override {}

	int init() override;
};

class Scene472: public CutScene {
public:
	Scene472(GnapEngine *vm);
	~Scene472() override {}

	int init() override;
};

class Scene473: public CutScene {
public:
	Scene473(GnapEngine *vm);
	~Scene473() override {}

	int init() override;
};

class Scene474: public CutScene {
public:
	Scene474(GnapEngine *vm);
	~Scene474() override {}

	int init() override;
};

class Scene475: public CutScene {
public:
	Scene475(GnapEngine *vm);
	~Scene475() override {}

	int init() override;
};

class Scene476: public CutScene {
public:
	Scene476(GnapEngine *vm);
	~Scene476() override {}

	int init() override;
};

class Scene477: public CutScene {
public:
	Scene477(GnapEngine *vm);
	~Scene477() override {}

	int init() override;
};

class Scene48: public CutScene {
public:
	Scene48(GnapEngine *vm);
	~Scene48() override {}

	int init() override;
};

class Scene541: public CutScene {
public:
	Scene541(GnapEngine *vm);
	~Scene541() override {}

	int init() override;
};

class Scene542: public CutScene {
public:
	Scene542(GnapEngine *vm);
	~Scene542() override {}

	int init() override;
};
} // End of namespace Gnap

#endif // GNAP_GROUPCS_H
