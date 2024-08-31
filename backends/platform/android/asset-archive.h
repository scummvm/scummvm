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

#ifndef _ANDROID_ASSET_H_
#define _ANDROID_ASSET_H_

#include <jni.h>

#include "common/str.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/archive.h"

#include <android/asset_manager.h>

class AndroidAssetArchive : public Common::Archive {
public:
	AndroidAssetArchive(jobject am);
	virtual ~AndroidAssetArchive();

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	AAssetManager *_am;
	mutable Common::ArchiveMemberList _cachedMembers;
	mutable bool _hasCached;
};

#endif
