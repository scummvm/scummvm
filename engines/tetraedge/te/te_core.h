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

#ifndef TETRAEDGE_TE_TE_CORE_H
#define TETRAEDGE_TE_TE_CORE_H

#include "common/array.h"
#include "common/str.h"
#include "common/path.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "tetraedge/te/te_i_loc.h"
#include "tetraedge/te/te_i_codec.h"
#include "tetraedge/te/te_timer.h"

namespace Tetraedge {

class TeCore {
public:
	TeCore();

	void addLoc(TeILoc *loc);
	void create();
	TeICodec *createVideoCodec(const TetraedgeFSNode &node);
	TeICodec *createVideoCodec(const Common::String &extn);
	const Common::String &fileFlagSystemFlag(const Common::String &name) const;
	bool fileFlagSystemFlagsContains(const Common::String &name) const;
	Common::Array<Common::String> fileFlagSystemPossibleFlags();
	bool fileFlagSystemPossibleFlagsContains(const Common::String &name) const;
	void fileFlagSystemSetFlag(const Common::String &name, const Common::String &val);

	const Common::String &language() const;
	void language(const Common::String &val);

	TeILoc *loc() { return _loc; }

	bool onActivityTrackingAlarm();

	// Note: this is not in the original, but it's not clear how the original
	// adds things like "PC-MacOSX" to the path, and there is not clear logic
	// to them, so here we are.
	TetraedgeFSNode findFile(const Common::Path &path, bool quiet = false) const;
	TetraedgeFSNode getFSNode(const Common::Path &path) const;

	bool _coreNotReady;

private:
	TeILoc *_loc;

	Common::HashMap<Common::String, Common::String, Common::CaseSensitiveString_Hash, Common::CaseSensitiveString_EqualTo> _fileSystemFlags;
	Common::FSDirectory _resourcesRoot;

	TeTimer _activityTrackingTimer;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CORE_H
