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

#ifndef PINK_ACTION_CEL_H
#define PINK_ACTION_CEL_H

#include "pink/objects/actions/action.h"

namespace Pink {

class CelDecoder;

class ActionCEL : public Action {
public:
	ActionCEL();
	~ActionCEL() override;

	void deserialize(Archive &archive) override;

	bool initPalette(Director *director) override;

	void start() override;
	void end() override;

	virtual void update();

	void pause(bool paused) override;

	uint32 getZ();
	CelDecoder *getDecoder();

protected:
	virtual void onStart() = 0;

	CelDecoder *_decoder;
	Common::String _fileName;
	uint32 _z;
};

} // End of namespace Pink

#endif
