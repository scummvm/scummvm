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

#include "harvester/resources.h"

#include "common/debug.h"
#include "common/stream.h"
#include "harvester/detection.h"
#include "harvester/xfile_archive.h"

namespace Harvester {

namespace {

struct ArchiveSpec {
	char archiveSetId;
	const char *indexPath;
	const char *dataPath;
	int priority;
};

static const ArchiveSpec kArchiveSpecs[] = {
	{ '1', "INDEX.001", "HARVEST.DAT", 30 },
	{ '2', "INDEX.002", "SOUND.DAT", 29 },
	{ '3', "INDEX.003", "HARVEST2.DAT", 28 }
};

static bool hasArchiveSetPrefix(const Common::String &path) {
	return path.size() >= 3 &&
		path[0] >= '0' && path[0] <= '9' &&
		path[1] == ':' && path[2] == '/';
}

static const ArchiveSpec *findArchiveSpec(char archiveSetId) {
	for (const ArchiveSpec &spec : kArchiveSpecs) {
		if (spec.archiveSetId == archiveSetId)
			return &spec;
	}

	return nullptr;
}

static Common::String stripLeadingSlashes(const Common::String &path) {
	Common::String normalized = path;
	while (!normalized.empty() && normalized[0] == '/')
		normalized.erase(0, 1);
	return normalized;
}

static Common::String buildDiscArchiveName(int discNumber, char archiveSetId) {
	return Common::String::format("harvester-disc-%d-xfile-%c", discNumber, archiveSetId);
}

static Common::String buildDiscDirectoryFilePath(int discNumber, const Common::String &path) {
	return Common::String::format("CD%d/%s", discNumber, stripLeadingSlashes(path).c_str());
}

static Common::String resolveDiscArchiveStoragePath(int discNumber, const Common::String &path) {
	const Common::String normalized = stripLeadingSlashes(path);
	if (normalized.empty())
		return Common::String();

	const Common::String discPath = buildDiscDirectoryFilePath(discNumber, normalized);
	if (SearchMan.hasFile(Common::Path(discPath, '/')))
		return discPath;

	if (discNumber == ResourceManager::kFirstDiscNumber && SearchMan.hasFile(Common::Path(normalized, '/')))
		return normalized;

	return Common::String();
}

static Common::String resolveDiscLooseResourcePath(int discNumber, const Common::String &path) {
	const Common::String normalized = stripLeadingSlashes(path);
	if (normalized.empty())
		return Common::String();

	const Common::String discPath = buildDiscDirectoryFilePath(discNumber, normalized);
	if (SearchMan.hasFile(Common::Path(discPath, '/')))
		return discPath;

	if (SearchMan.hasFile(Common::Path(normalized, '/')))
		return normalized;

	return Common::String();
}

static Common::String normalizeHarvesterLookupPath(const Common::String &path) {
	Common::String normalized(path);

	for (uint i = 0; i < normalized.size(); ++i) {
		if (normalized[i] == '\\')
			normalized.setChar('/', i);
	}

	while (normalized.hasPrefix("./"))
		normalized.erase(0, 2);

	if (hasArchiveSetPrefix(normalized)) {
		const char archiveSetId = normalized[0];
		Common::String memberPath = stripLeadingSlashes(normalized.substr(3));
		return memberPath.empty()
			? Common::String::format("%c:/", archiveSetId)
			: Common::String::format("%c:/%s", archiveSetId, memberPath.c_str());
	}

	if (normalized.size() >= 3 && normalized[1] == ':' && normalized[2] == '/')
		normalized.erase(0, 3);

	return stripLeadingSlashes(normalized);
}

} // End of anonymous namespace

Common::String normalizeHarvesterResourcePath(const Common::String &path) {
	Common::String normalized(path);

	for (uint i = 0; i < normalized.size(); ++i) {
		if (normalized[i] == '\\')
			normalized.setChar('/', i);
	}

	while (normalized.hasPrefix("./"))
		normalized.erase(0, 2);

	if (normalized.size() >= 3 && normalized[1] == ':' && normalized[2] == '/')
		normalized.erase(0, 3);

	while (!normalized.empty() && normalized[0] == '/')
		normalized.erase(0, 1);

	return normalized;
}

ResourceManager::ResourceManager() {
	reset();
}

ResourceManager::~ResourceManager() {
	_search.clear();
}

void ResourceManager::reset() {
	_search.clear();
	_currentDisc = 1;
	_search.add("harvester-loose-files", &SearchMan, 0, false);
}

Common::String ResourceManager::normalizeResourcePath(const Common::String &path) const {
	return normalizeHarvesterLookupPath(path);
}

bool ResourceManager::mountStartupArchives() {
	return setCurrentDisc(_currentDisc);
}

bool ResourceManager::setCurrentDisc(int discNumber) {
	if (discNumber <= 0)
		return false;
	if (!ensureDiscMounted(discNumber))
		return false;

	unmountOtherDiscArchives(discNumber);
	_currentDisc = discNumber;
	debugC(1, kDebugResources, "Harvester: switched active disc to %d", discNumber);
	return true;
}

bool ResourceManager::ensureDiscMounted(int discNumber) {
	if (discNumber <= 0)
		return false;

	bool mountedAny = false;
	for (const ArchiveSpec &spec : kArchiveSpecs) {
		if (getMountedDiscArchive(discNumber, spec.archiveSetId)) {
			mountedAny = true;
			continue;
		}

		const Common::String indexPath = resolveDiscArchiveStoragePath(discNumber, spec.indexPath);
		const Common::String dataPath = resolveDiscArchiveStoragePath(discNumber, spec.dataPath);
		if (indexPath.empty() || dataPath.empty()) {
			debugC(1, kDebugResources,
				"Harvester: missing archive files for disc %d set %c (%s + %s)",
				discNumber, spec.archiveSetId, spec.indexPath, spec.dataPath);
			continue;
		}

		XFileArchive *archive = new XFileArchive();
		if (!archive->open(indexPath, dataPath)) {
			debugC(1, kDebugResources,
				"Harvester: failed to mount disc %d set %c from %s + %s",
				discNumber, spec.archiveSetId, indexPath.c_str(), dataPath.c_str());
			delete archive;
			continue;
		}

		mountArchive(buildDiscArchiveName(discNumber, spec.archiveSetId), archive, spec.priority, true);
		debugC(1, kDebugResources,
			"Harvester: mounted disc %d set %c from %s + %s",
			discNumber, spec.archiveSetId, indexPath.c_str(), dataPath.c_str());
		mountedAny = true;
	}

	return mountedAny;
}

void ResourceManager::unmountOtherDiscArchives(int keepDiscNumber) {
	for (int discNumber = ResourceManager::kFirstDiscNumber;
			discNumber <= ResourceManager::kLastDiscNumber; ++discNumber) {
		if (discNumber == keepDiscNumber)
			continue;

		for (const ArchiveSpec &spec : kArchiveSpecs) {
			const Common::String archiveName = buildDiscArchiveName(discNumber, spec.archiveSetId);
			if (_search.hasArchive(archiveName)) {
				debugC(1, kDebugResources,
					"Harvester: unmounted disc %d set %c",
					discNumber, spec.archiveSetId);
				_search.remove(archiveName);
			}
		}
	}
}

Common::Archive *ResourceManager::getMountedDiscArchive(int discNumber, char archiveSetId) const {
	return _search.getArchive(buildDiscArchiveName(discNumber, archiveSetId));
}

Common::Archive *ResourceManager::findArchiveForMember(char archiveSetId, const Common::Path &memberPath) const {
	Common::Archive *archive = getMountedDiscArchive(_currentDisc, archiveSetId);
	if (archive && archive->hasFile(memberPath))
		return archive;

	return nullptr;
}

bool ResourceManager::hasInMountedArchives(const Common::Path &memberPath) const {
	for (const ArchiveSpec &spec : kArchiveSpecs) {
		if (findArchiveForMember(spec.archiveSetId, memberPath))
			return true;
	}

	return false;
}

Common::SeekableReadStream *ResourceManager::openFromMountedArchives(const Common::Path &memberPath) const {
	for (const ArchiveSpec &spec : kArchiveSpecs) {
		Common::Archive *archive = findArchiveForMember(spec.archiveSetId, memberPath);
		if (!archive)
			continue;

		Common::SeekableReadStream *stream = archive->createReadStreamForMember(memberPath);
		if (stream)
			return stream;
	}

	return nullptr;
}

bool ResourceManager::hasFile(const Common::String &path) const {
	const Common::String normalized = normalizeResourcePath(path);
	if (normalized.empty())
		return false;

	if (hasArchiveSetPrefix(normalized)) {
		const ArchiveSpec *spec = findArchiveSpec(normalized[0]);
		const Common::String memberPath = stripLeadingSlashes(normalized.substr(3));
		if (!spec || memberPath.empty())
			return false;

		if (findArchiveForMember(spec->archiveSetId, Common::Path(memberPath, '/')) != nullptr)
			return true;

		return !resolveDiscArchiveStoragePath(_currentDisc, memberPath).empty();
	}

	const Common::Path memberPath(normalized, '/');
	if (hasInMountedArchives(memberPath))
		return true;

	return !resolveDiscLooseResourcePath(_currentDisc, normalized).empty();
}

Common::SeekableReadStream *ResourceManager::openFile(const Common::String &path) const {
	const Common::String normalized = normalizeResourcePath(path);
	if (normalized.empty())
		return nullptr;

	Common::SeekableReadStream *stream = nullptr;
	if (hasArchiveSetPrefix(normalized)) {
		const ArchiveSpec *spec = findArchiveSpec(normalized[0]);
		const Common::String memberPath = stripLeadingSlashes(normalized.substr(3));
		Common::Archive *archive = spec ? findArchiveForMember(spec->archiveSetId, Common::Path(memberPath, '/')) : nullptr;
		if (archive && !memberPath.empty())
			stream = archive->createReadStreamForMember(Common::Path(memberPath, '/'));
		if (!stream && !memberPath.empty()) {
			const Common::String loosePath = resolveDiscArchiveStoragePath(_currentDisc, memberPath);
			if (!loosePath.empty())
				stream = SearchMan.createReadStreamForMember(Common::Path(loosePath, '/'));
		}
	} else {
		const Common::Path memberPath(normalized, '/');
		stream = openFromMountedArchives(memberPath);
		if (!stream) {
			const Common::String loosePath = resolveDiscLooseResourcePath(_currentDisc, normalized);
			if (!loosePath.empty())
				stream = SearchMan.createReadStreamForMember(Common::Path(loosePath, '/'));
		}
	}

	debugC(3, kDebugResources, "Harvester: openFile(disc=%d, '%s' -> '%s') %s",
		_currentDisc, path.c_str(), normalized.c_str(), stream ? "hit" : "miss");
	return stream;
}

bool ResourceManager::loadFile(const Common::String &path, Common::Array<byte> &data) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(openFile(path));
	if (!stream)
		return false;

	data.resize(stream->size());
	if (!data.empty())
		stream->read(data.data(), data.size());

	return true;
}

void ResourceManager::mountArchive(const Common::String &name, Common::Archive *archive, int priority, bool autoFree) {
	_search.add(name, archive, priority, autoFree);
}

} // End of namespace Harvester
