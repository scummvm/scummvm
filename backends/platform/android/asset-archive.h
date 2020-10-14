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

#ifndef _ANDROID_ASSET_H_
#define _ANDROID_ASSET_H_

#if defined(__ANDROID__)

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

	virtual bool hasFile(const Common::String &name) const override;
	virtual int listMembers(Common::ArchiveMemberList &list) const override;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const override;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override;

private:
	AAssetManager *_am;
	mutable Common::ArchiveMemberList _cachedMembers;
	mutable bool _hasCached;
};

#endif
#endif
