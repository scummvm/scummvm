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

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/memstream.h"
#include "common/str-enc.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/cinema.h"
#include "harold/bcb/translation.h"

namespace BCB {

Common::SeekableReadStream *CinemaViewer::openPackage(BCBEngine &engine) {
	Common::SeekableReadStream *stream = engine.createProgramReadStream("bcb-cinema.dat");
	return stream ? stream : SearchMan.createReadStreamForMember("bcb-cinema.dat");
}

bool CinemaViewer::loadLabels() {
	TextStringTable labels;
	if (!labels.load(_archive->createReadStreamForMember("cinema.yml")))
		return false;
	for (uint i = 0; i < ARRAYSIZE(kCinemaPlaylists); ++i) {
		const Common::String key = Common::String::format("cinema.playlist.%u", i + 1);
		const char *name = labels.getString(key);
		if (!name || !*name)
			return false;
		_labels.push_back(translate(key, Common::U32String(name, Common::kUtf8)));
	}
	return true;
}

bool CinemaViewer::loadPicture(uint index, Graphics::ManagedSurface &surface) {
	if (index >= 3) {
		const Common::String path = Common::String::format("bmp/still/16m/pcin%u.bmp", index - 2);
		Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> bitmap(
			_engine.loadDiscBitmapFromAnyDisc(path.c_str()));
		if (!bitmap) {
			warning("BCB Cinema: Missing picture %s", path.c_str());
			return false;
		}
		surface.copyFrom(*bitmap);
		return true;
	}
	const char *path = kCinemaPicturePaths[index];
	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> picture(
		_engine.loadImageFromStream(_archive->createReadStreamForMember(Common::Path(path)), path));
	if (!picture) {
		warning("BCB Cinema: Missing picture %s", path);
		return false;
	}
	if (index == 0) {
		if (!applyPatchImage(*picture, _archive->createReadStreamForMember("patches/pcinm_patch.png"), "BCB Cinema")) {
			warning("BCB Cinema: Missing or invalid menu patch");
			return false;
		}
		applyTranslatedCinemaMenu(*picture, _engine._textTranslation);
	}
	surface.copyFrom(*picture);
	return true;
}

bool CinemaViewer::load() {
	Common::SeekableReadStream *stream = openPackage(_engine);
	if (!stream)
		return false;
	_archive.reset(Common::makeZipArchive(stream));
	if (!_archive)
		return false;
	if (!loadLabels()) {
		warning("BCB Cinema: Missing or invalid cinema.yml");
		return false;
	}
	_canvas.create(640, 480, g_system->getScreenFormat());
	for (uint i = 0; i < 3; ++i)
		if (!loadPicture(i, _sheets[i]))
			return false;
	for (uint i = 0; i < ARRAYSIZE(kCinemaButtons); ++i) {
		const Graphics::ManagedSurface &sheet = _sheets[i < 12 ? 0 : i < 20 ? 1 : 2];
		if (!Common::Rect(sheet.w, sheet.h).contains(kCinemaButtons[i].source))
			return false;
	}
#ifdef USE_FREETYPE2
	_listFont.reset(Graphics::loadTTFFontFromArchive("NotoSansJP-Regular.otf", 12, Graphics::kTTFSizeModeCharacter));
	_captionFont.reset(Graphics::loadTTFFontFromArchive("NotoSansJP-Regular.otf", 14, Graphics::kTTFSizeModeCharacter));
	if (!_listFont)
		_listFont.reset(Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", 12, Graphics::kTTFSizeModeCharacter));
	if (!_captionFont)
		_captionFont.reset(Graphics::loadTTFFontFromArchive("VL-Gothic-Regular.ttf", 14, Graphics::kTTFSizeModeCharacter));
#endif
	return true;
}

bool CinemaViewer::loadPlaylist() {
	_movies.clear();
	if (_selectedPlaylist < 1 || _selectedPlaylist > static_cast<int>(ARRAYSIZE(kCinemaPlaylists)))
		return false;
	const CinemaPlaylist &playlist = kCinemaPlaylists[_selectedPlaylist - 1];
	_movies = Common::Span<const CinemaMovie>(kCinemaMovies + playlist.firstMovie, playlist.movieCount);
	return _movies.size() != 0;
}

void CinemaViewer::loadCaptions(uint movie) {
	_captions.clear();
	_nextCaption = 0;
	uint32 size = 0;
	const byte *data = _engine.findAllBinRecordData(Common::String::format("MES%04u.TXT", movie), size);
	if (!data || !size)
		return;
	Common::MemoryReadStream text(data, size);
	while (!text.eos()) {
		Common::String time = text.readLine();
		if (time.empty())
			break;
		Caption caption;
		caption.frame = atoi(time.c_str());
		caption.lines[0] = text.readLine().decode(_engine._executableTextEncoding);
		caption.lines[1] = text.readLine().decode(_engine._executableTextEncoding);
		_captions.push_back(caption);
	}
}

Common::U32String CinemaViewer::translate(const Common::String &key, const Common::U32String &original) const {
	const char *text = _engine._textTranslation.strings.getString(key);
	return text ? Common::U32String(text, Common::kUtf8) : original;
}

} // namespace BCB
