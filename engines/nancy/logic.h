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

#ifndef NANCY_LOGIC_H
#define NANCY_LOGIC_H

#include "engines/nancy/action/actionrecord.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/array.h"
#include "common/func.h"

namespace Nancy {

class NancyEngine;
class SceneManager;

class Logic {
    friend class SceneManager;

public:
    Logic(NancyEngine* engine): _engine(engine) {}
    virtual ~Logic() {}

    bool addNewActionRecord(Common::SeekableReadStream &inputData);

protected:
    virtual ActionRecord *createActionRecord(uint16 type);
    NancyEngine *_engine;
    Common::Array<ActionRecord *> _records;
};

} // End of namespace Nancy

#endif // NANCY_LOGIC_H