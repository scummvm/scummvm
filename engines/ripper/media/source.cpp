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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/stream.h"

#include "ripper/detection.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

MediaFormat detectMediaFormat(Common::SeekableReadStream &stream) {
	const int64 originalPosition = stream.pos();
	byte magic[8] = { 0 };
	const uint32 bytesRead = stream.read(magic, sizeof(magic));
	if (!stream.seek(originalPosition))
		return kMediaFormatUnknown;
	if (bytesRead >= 8 && memcmp(magic, "IAVF2.00", 8) == 0)
		return kMediaFormatIavf;
	if (bytesRead >= 4 &&
			(memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0))
		return kMediaFormatSmacker;
	return kMediaFormatUnknown;
}

const char *mediaFormatName(MediaFormat format) {
	switch (format) {
	case kMediaFormatSmacker:
		return "smacker";
	case kMediaFormatIavf:
		return "iavf";
	default:
		return "unknown";
	}
}

Common::SeekableReadStream *MediaPlayer::openSource(const Common::String &path,
		SourcePolicy policy, Common::String &source) const {
	source.clear();
	ResourceManager *resources = _engine->getResources();

	if (policy == kSourceDirectFile || policy == kSourceSoundEffect ||
			policy == kSourceBlockingAudio) {
		Common::ScopedPtr<Common::File> file(new Common::File());
		if (file->open(Common::Path(path))) {
			source = "filesystem";
			return file.release();
		}
	}

	if (policy == kSourceConfiguredPath || policy == kSourceBlockingAudio) {
		Common::SeekableReadStream *stream =
			resources ? resources->createReadStreamForPath(path) : nullptr;
		if (stream) {
			source = "search-path";
			return stream;
		}
	}

	if (policy == kSourceInterfaceLibrary) {
		Common::SeekableReadStream *stream =
			resources ? resources->interface().createReadStreamForMember(path) : nullptr;
		if (stream) {
			source = "interface-library";
			return stream;
		}
	}

	if (policy == kSourcePuzzleLibrary) {
		Common::SeekableReadStream *stream =
			resources && resources->puzzle().hasMember(path) ?
				resources->puzzle().createReadStreamForMember(path) : nullptr;
		if (stream) {
			source = "puzzle-library";
			return stream;
		}
		stream = resources ? resources->createReadStreamForPath(path) : nullptr;
		if (stream) {
			source = "search-path";
			return stream;
		}
	}

	if (policy == kSourceSoundEffect || policy == kSourceBlockingAudio) {
		Common::SeekableReadStream *stream =
			resources && resources->sound().hasMember(path) ?
				resources->sound().createReadStreamForMember(path) : nullptr;
		if (stream) {
			source = "sound-library";
			return stream;
		}
	}

	return nullptr;
}

SmackerPlaybackResult MediaPlayer::playValidatedSmacker(Common::SeekableReadStream *stream,
		const Common::String &name, const char *description,
		const SmackerPlaybackPlan &plan) {
	Common::ScopedPtr<Common::SeekableReadStream> ownedStream(stream);
	if (!ownedStream) {
		warning("Ripper: could not open %s Smacker '%s'", description, name.c_str());
		return SmackerPlaybackResult();
	}

	const MediaFormat format = detectMediaFormat(*ownedStream);
	if (format != kMediaFormatSmacker) {
		warning("Ripper: unsupported %s media '%s' format=%s",
			description, name.c_str(), mediaFormatName(format));
		return SmackerPlaybackResult();
	}

	debugC(3, kDebugVideo,
		"Ripper: validated %s Smacker media='%s'",
		description, name.c_str());
	return playSmacker(ownedStream.release(), name, plan);
}

} // End of namespace Ripper
