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

#ifndef HODJNPODJ_GFX_MINIGAME_VIEW_H
#define HODJNPODJ_GFX_MINIGAME_VIEW_H

#include "common/archive.h"
#include "common/hashmap.h"
#include "common/formats/winexe_ne.h"
#include "bagel/hodjnpodj/gfx/view.h"

namespace Bagel {
namespace HodjNPodj {

/**
 * Base view class for the main view for each minigame
 */
class MinigameView : public View, public Common::Archive {
private:
	Common::String _resourceFilename;
	Common::HashMap<Common::String, int,
		Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _resourceFiles;

protected:
	void addResource(const Common::String &filename, int resourceId) {
		_resourceFiles[filename] = resourceId;
	}

public:
	MinigameView(const Common::String &name, const Common::String &resFilename) :
		View(name), _resourceFilename(resFilename) {
	}
	virtual ~MinigameView() {}

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;

	// Archive methods
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const;
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
