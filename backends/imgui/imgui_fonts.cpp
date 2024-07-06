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

#include "common/file.h"
#include "common/config-manager.h"
#include "common/compression/unzip.h"
#include "backends/imgui/imgui_fonts.h"

namespace {

class FontReader;

class FontArchive {
	friend class FontReader;

public:
	void openArchive(const char *archiveName) {
		if (_archive)
			delete _archive;

		Common::SeekableReadStream *archiveStream = nullptr;
		if (ConfMan.hasKey("extrapath")) {
			Common::FSDirectory extrapath(ConfMan.getPath("extrapath"));
			archiveStream = extrapath.createReadStreamForMember(archiveName);
		}

		if (!archiveStream) {
			archiveStream = SearchMan.createReadStreamForMember(archiveName);
		}

		_archive = Common::makeZipArchive(archiveStream);
	}

	~FontArchive() {
		if (_archive)
			delete _archive;
	}

private:
	Common::Archive *_archive = nullptr;
};

class FontReader {
public:
	FontReader(FontArchive &archive) : _archive(archive) {}
	bool openFile(Common::File &file, const char *fileName) {
		if (!_archive._archive)
			return false;
		return file.open(Common::Path(fileName, Common::Path::kNoSeparator), *_archive._archive);
	}

private:
	FontArchive &_archive;
};

} // namespace

namespace ImGui {

ImFont *addTTFFontFromArchive(const char *filename, float size_pixels, const ImFontConfig *font_cfg_template, const ImWchar *glyph_ranges) {
	Common::File f;
	FontArchive archive;
	FontReader reader(archive);

	archive.openArchive("fonts.dat");
	if (!reader.openFile(f, filename)) {
		archive.openArchive("fonts-cjk.dat");
		if (!reader.openFile(f, filename)) {
			return nullptr;
		}
	}

	uint size = f.size();

	uint8 *ttfFile = (uint8 *)ImGui::MemAlloc(size);
	f.read(ttfFile, size);
	ImGuiIO &io = ImGui::GetIO();
	return io.Fonts->AddFontFromMemoryTTF(ttfFile, size, size_pixels, font_cfg_template, glyph_ranges);
}

} // namespace ImGui
