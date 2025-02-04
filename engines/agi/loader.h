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
 
#ifndef AGI_LOADER_H
#define AGI_LOADER_H

namespace Agi {
	
class AgiLoader {
public:
	AgiLoader(AgiEngine *vm) : _vm(vm) {}
	virtual ~AgiLoader() {}

	/**
	 * Performs one-time initializations, such as locating files
	 * with dynamic names.
	 */
	virtual void init() {}

	/**
	 * Loads all AGI directory entries from disk and and populates
	 * the AgiDir arrays in AgiGame with them.
	 */
	virtual int loadDirs() = 0;

	/**
	 * Loads a volume resource from disk.
	 */
	virtual uint8 *loadVolumeResource(AgiDir *agid) = 0;

	/**
	 * Loads AgiEngine::_objects from disk.
	 */
	virtual int loadObjects() = 0;

	/**
	 * Loads AgiBase::_words from disk.
	 */
	virtual int loadWords() = 0;

protected:
	AgiEngine *_vm;

	typedef Common::HashMap<Common::Path, Common::FSNode, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> FileMap;
	static void getPotentialDiskImages(
		const char * const *imageExtensions,
		size_t imageExtensionCount,
		Common::Array<Common::Path> &imageFiles,
		FileMap &fileMap);
};

struct AgiDiskVolume {
	uint32 disk;
	uint32 offset;

	AgiDiskVolume() : disk(_EMPTY), offset(0) {}
	AgiDiskVolume(uint32 d, uint32 o) : disk(d), offset(o) {}
};

/**
 * Apple II version of the format for LOGDIR, VIEWDIR, etc.
 * See AgiLoader_A2::loadDir for more details.
 */
enum A2DirVersion {
	A2DirVersionOld,  // 4 bits for volume, 8 for track
	A2DirVersionNew,  // 5 bits for volume, 7 for track
};

class AgiLoader_A2 : public AgiLoader {
public:
	AgiLoader_A2(AgiEngine *vm) : AgiLoader(vm) {}
	~AgiLoader_A2() override;

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::Array<Common::SeekableReadStream *> _disks;
	Common::Array<AgiDiskVolume> _volumes;
	AgiDir _logDir;
	AgiDir _picDir;
	AgiDir _viewDir;
	AgiDir _soundDir;
	AgiDir _objects;
	AgiDir _words;

	int readDiskOne(Common::SeekableReadStream &stream, Common::Array<uint32> &volumeMap);
	static bool readInitDir(Common::SeekableReadStream &stream, byte index, AgiDir &agid);
	static bool readDir(Common::SeekableReadStream &stream, int position, AgiDir &agid);
	static bool readVolumeMap(Common::SeekableReadStream &stream, uint32 position, uint32 bufferLength, Common::Array<uint32> &volumeMap);

	A2DirVersion detectDirVersion(Common::SeekableReadStream &stream) const;
	static bool loadDir(AgiDir *dir, Common::SeekableReadStream &disk, uint32 dirOffset, uint32 dirLength, A2DirVersion dirVersion);
};

class AgiLoader_v1 : public AgiLoader {
public:
	AgiLoader_v1(AgiEngine *vm) : AgiLoader(vm) {}

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::Array<Common::String> _imageFiles;
	Common::Array<AgiDiskVolume> _volumes;
	AgiDir _logDir;
	AgiDir _picDir;
	AgiDir _viewDir;
	AgiDir _soundDir;
	AgiDir _objects;
	AgiDir _words;

	bool readDiskOneV1(Common::SeekableReadStream &stream);
	bool readDiskOneV2001(Common::SeekableReadStream &stream, int &vol0Offset);
	static bool readInitDirV1(Common::SeekableReadStream &stream, byte index, AgiDir &agid);
	static bool readInitDirV2001(Common::SeekableReadStream &stream, byte index, AgiDir &agid);

	bool loadDir(AgiDir *dir, Common::File &disk, uint32 dirOffset, uint32 dirLength);
};

class AgiLoader_v2 : public AgiLoader {
private:
	bool _hasV3VolumeFormat;

	int loadDir(AgiDir *agid, const char *fname);
	bool detectV3VolumeFormat();

public:
	AgiLoader_v2(AgiEngine *vm) : _hasV3VolumeFormat(false), AgiLoader(vm) {}

	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;
};

class AgiLoader_v3 : public AgiLoader {
private:
	Common::String _name; /**< prefix in directory and/or volume file names (`GR' for goldrush) */

	int loadDir(AgiDir *agid, Common::File *fp, uint32 offs, uint32 len);

public:
	AgiLoader_v3(AgiEngine *vm) : AgiLoader(vm) {}

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;
};

class GalLoader : public AgiLoader {
public:
	GalLoader(AgiEngine *vm) : _dirOffset(0), AgiLoader(vm) {}

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::String _imageFile;
	int _dirOffset;

	static bool isDirectory(Common::SeekableReadStream &stream, uint32 dirOffset);
	static uint32 readDirectoryEntry(Common::SeekableReadStream &stream, uint32 *sectorCount);
};

class GalLoader_A2 : public AgiLoader {
public:
	GalLoader_A2(AgiEngine *vm) : AgiLoader(vm) {}
	~GalLoader_A2();

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::Array<Common::SeekableReadStream *> _disks;

	static bool readDiskOne(Common::SeekableReadStream &disk, AgiDir *logicDir);
	static bool readDirectoryEntry(Common::SeekableReadStream &stream, AgiDir &dirEntry);
	static bool validateDisk(Common::SeekableReadStream &disk, byte diskIndex, AgiDir *logicDir);

	static bool loadDir(AgiDir *dir, Common::SeekableReadStream &disk, uint32 dirOffset, uint32 dirCount);
};

} // End of namespace Agi

#endif /* AGI_LOADER_H */
