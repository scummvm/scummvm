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

#ifndef XEEN_META_ENGINE_H
#define XEEN_META_ENGINE_H

#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

namespace MM {
namespace Xeen {

class XeenMetaEngine {
public:
	static SaveStateDescriptor querySaveMetaInfos(const MetaEngine *metaEngine,
		const char *target, int slot);
	static SaveStateList listSaves(const MetaEngine *metaEngine, const char *target);
};

} // End of namespace Xeen
} // End of namespace MM

#endif
