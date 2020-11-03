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

#if defined(__ANDROID__)

#include <sys/types.h>
#include <unistd.h>

#include "common/str.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "backends/platform/android/jni-android.h"
#include "backends/platform/android/asset-archive.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AssetInputStream : public Common::SeekableReadStream {
public:
	AssetInputStream(AAssetManager *as, const Common::String &path);
	virtual ~AssetInputStream();

	virtual bool eos() const { return _eos; }

	virtual void clearErr() {_eos = false; }

	virtual uint32 read(void *dataPtr, uint32 dataSize);

	virtual int32 pos() const { return _pos; }

	virtual int32 size() const { return _len; }

	virtual bool seek(int32 offset, int whence = SEEK_SET);

private:
	void close();
	AAsset *_asset;

	uint32 _pos;
	uint32 _len;
	bool _eos;
};

AssetInputStream::AssetInputStream(AAssetManager *as, const Common::String &path) :
	_eos(false), _pos(0) {
	_asset = AAssetManager_open(as, path.c_str(), AASSET_MODE_RANDOM);
	_len = AAsset_getLength(_asset);
}

AssetInputStream::~AssetInputStream() {
	if (_asset != NULL) {
		AAsset_close(_asset);
	}
}

void AssetInputStream::close() {
	AAsset_close(_asset);
}

uint32 AssetInputStream::read(void *dataPtr, uint32 dataSize) {
	uint32 readlen = AAsset_read(_asset, dataPtr, dataSize);
	_pos += readlen;
	if (readlen != dataSize) {
		_eos = true;
	}
	return readlen;
}

bool AssetInputStream::seek(int32 offset, int whence) {
	int res = AAsset_seek(_asset, offset, whence);
	if (res == -1) {
		return false;
	}
	if (whence == SEEK_CUR) {
		_pos += offset;
	} else if (whence == SEEK_SET) {
		_pos = offset;
	} else if (whence == SEEK_END) {
		_pos = _len + offset;
	}
	assert(_pos <= _len);
	_eos = false;
	return true;
}

AndroidAssetArchive::AndroidAssetArchive(jobject am) : _hasCached(false) {
	JNIEnv *env = JNI::getEnv();

	_am = AAssetManager_fromJava(env, am);
}

AndroidAssetArchive::~AndroidAssetArchive() {
}

bool AndroidAssetArchive::hasFile(const Common::String &name) const {
	AAsset *asset = AAssetManager_open(_am, name.c_str(), AASSET_MODE_RANDOM);
	bool exists = false;
	if (asset != NULL) {
		exists = true;
		AAsset_close(asset);
	}
	return exists;
}

int AndroidAssetArchive::listMembers(Common::ArchiveMemberList &member_list) const {
	if (_hasCached) {
		member_list.insert(member_list.end(), _cachedMembers.begin(), _cachedMembers.end());
		return _cachedMembers.size();
	}

	int count = 0;
	Common::List<Common::String> dirs;
	dirs.push_back("");
#ifdef BACKEND_ANDROID3D
	// ResidualVM specific: multiple directories
	dirs.push_back("shaders");
#endif
	for (const auto& currentDir : dirs) {
		AAssetDir *dir = AAssetManager_openDir(_am, "");
		const char *file = AAssetDir_getNextFileName(dir);

		while (file) {
			member_list.push_back(getMember(currentDir + Common::String(file)));
			++count;
			file = AAssetDir_getNextFileName(dir);
		}
		AAssetDir_close(dir);
	}

	_cachedMembers = Common::ArchiveMemberList(member_list);
	_hasCached = true;

	return count;
}

const Common::ArchiveMemberPtr AndroidAssetArchive::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *AndroidAssetArchive::createReadStreamForMember(const Common::String &path) const {
	if (!hasFile(path)) {
		return nullptr;
	}
	return new AssetInputStream(_am, path);
}

#endif
