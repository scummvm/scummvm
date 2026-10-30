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
#include "common/md5.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "image/bmp.h"

#include "hollywood/image_translation.h"
#include "hollywood/monsters/graphics.h"

namespace Hollywood {

ImageTranslationArchive::ImageTranslationArchive(Common::Archive *archive) : _archive(archive) {
}

bool ImageTranslationArchive::load(const Common::Path &originalTextArchive) {
	_sources.clear();
	_overlays.clear();
	Common::ScopedPtr<Common::SeekableReadStream> file(_archive->createReadStreamForMember("monsters.en.idx"));
	if (!file || file->readUint32BE() != MKTAG('H', 'M', 'T', 'I') || file->readUint16LE() != 1)
		return false;
	const uint sourceCount = file->readUint16LE();
	const Common::String sourceMD5 = file->readString(0, 32);
	Common::File original;
	if (!original.open(originalTextArchive) || Common::computeStreamMD5AsString(original) != sourceMD5) {
		warning("Text translation does not match %s", originalTextArchive.toString().c_str());
		return false;
	}

	for (uint i = 0; i < sourceCount; ++i) {
		const Common::String extension = file->readString(0, 3);
		const Common::String md5 = file->readString(0, 32);
		const uint overlayCount = file->readByte();
		if (extension.size() != 3 || md5.size() != 32)
			return false;
		const Common::String source = "RESOURCE." + extension;
		_sources[source] = md5;
		for (uint j = 0; j < overlayCount; ++j) {
			Overlay overlay;
			overlay.source = source;
			overlay.chunk = file->readByte();
			const uint bitmapChunk = file->readByte();
			const uint layout = file->readByte();
			overlay.paletteChunk = file->readByte();
			overlay.x = file->readUint16LE();
			overlay.y = file->readUint16LE();
			if (overlay.chunk >= kResourceChunkCount || bitmapChunk >= kResourceChunkCount ||
					layout > kPalette || (overlay.paletteChunk != 255 && overlay.paletteChunk >= kResourceChunkCount))
				return false;
			overlay.layout = (Layout)layout;
			overlay.baseName = Common::String::format("images/%s-%u", extension.c_str(), bitmapChunk);
			overlay.baseName.toLowercase();
			if (!_archive->hasFile(Common::Path(overlay.baseName + ".bmp")) ||
					(overlay.layout != kPalette && !_archive->hasFile(Common::Path(overlay.baseName + ".mask"))))
				return false;
			_overlays.push_back(overlay);
		}
	}
	return !file->err() && !file->eos() && file->pos() == file->size();
}

bool ImageTranslationArchive::hasFile(const Common::Path &path) const {
	return _sources.contains(path.toString());
}

int ImageTranslationArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const auto &source : _sources)
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(Common::Path(source._key), *this)));
	return _sources.size();
}

const Common::ArchiveMemberPtr ImageTranslationArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

bool ImageTranslationArchive::readOriginalChunk(Common::SeekableReadStream &file,
		const ResourceChunkTable &table, uint index, Common::Array<byte> &data) const {
	if (!table.isValidChunk(index) || table.offsets[index] > file.size() ||
			table.sizes[index] > file.size() - table.offsets[index] || !file.seek(table.offsets[index]))
		return false;
	data.resize(table.sizes[index]);
	return file.read(data.data(), data.size()) == data.size();
}

void ImageTranslationArchive::remapPalette(Common::Array<byte> &pixels,
		const Common::Array<byte> &originalPalette, const byte *translatedPalette) const {
	byte mapping[256];
	for (uint i = 0; i < ARRAYSIZE(mapping); ++i) {
		mapping[i] = i;
		const byte color[3] = { (byte)(originalPalette[i * 3] * 4),
			(byte)(originalPalette[i * 3 + 1] * 4), (byte)(originalPalette[i * 3 + 2] * 4) };
		if (!memcmp(color, translatedPalette + i * 3, 3))
			continue;
		for (uint j = 0; j < ARRAYSIZE(mapping); ++j) {
			if (!memcmp(color, translatedPalette + j * 3, 3)) {
				mapping[i] = j;
				break;
			}
		}
	}
	for (uint i = 0; i < pixels.size(); ++i)
		pixels[i] = mapping[pixels[i]];
}

bool ImageTranslationArchive::applyImage(const Overlay &overlay, Common::SeekableReadStream &file,
		const ResourceChunkTable &table, Common::Array<byte> &data) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_archive->createReadStreamForMember(Common::Path(overlay.baseName + ".bmp")));
	Image::BitmapDecoder bitmap;
	if (!stream || !bitmap.loadStream(*stream) || !bitmap.getSurface() ||
			bitmap.getSurface()->format.bytesPerPixel != 1 || bitmap.getPalette().size() != 256 ||
			bitmap.getSurface()->w <= 0 || bitmap.getSurface()->h <= 0)
		return false;

	const byte *palette = bitmap.getPalette().data();
	if (overlay.layout == kPalette) {
		if (data.size() != kPaletteSize)
			return false;
		for (uint i = 0; i < data.size(); ++i)
			data[i] = palette[i] / 4;
		return true;
	}

	const Graphics::Surface &image = *bitmap.getSurface();
	Common::ScopedPtr<Common::SeekableReadStream> maskStream(_archive->createReadStreamForMember(Common::Path(overlay.baseName + ".mask")));
	const uint pixelCount = image.w * image.h;
	if (!maskStream || maskStream->size() != pixelCount)
		return false;
	Common::Array<byte> mask;
	mask.resize(pixelCount);
	if (maskStream->read(mask.data(), mask.size()) != mask.size())
		return false;

	Common::Array<byte> pixels;
	if (overlay.layout == kBlocks) {
		IndexedSurfaceBuffer original;
		original.resize(kSceneBufferByteCount);
		original.clear();
		if (image.w > original.surface().w || image.h > original.surface().h ||
				overlay.x > original.surface().w - image.w ||
				overlay.y > original.surface().h - image.h)
			return false;
		drawResourceBlockList(data, 0, original.surface());
		pixels.resize(pixelCount);
		for (int row = 0; row < image.h; ++row)
			memcpy(pixels.data() + row * image.w, original.surface().getBasePtr(overlay.x, overlay.y + row), image.w);
	} else if (overlay.layout == kPixels) {
		if (data.size() != pixelCount)
			return false;
		pixels = data;
		if (overlay.paletteChunk != 255) {
			Common::Array<byte> originalPalette;
			if (!readOriginalChunk(file, table, overlay.paletteChunk, originalPalette) ||
					originalPalette.size() != kPaletteSize)
				return false;
			remapPalette(pixels, originalPalette, palette);
		}
	} else {
		return false;
	}

	for (int row = 0; row < image.h; ++row) {
		const byte *translated = (const byte *)image.getBasePtr(0, row);
		for (int column = 0; column < image.w; ++column) {
			const uint offset = row * image.w + column;
			if (mask[offset] > 1)
				return false;
			if (mask[offset])
				pixels[offset] = translated[column];
		}
	}

	if (overlay.layout == kPixels) {
		data.swap(pixels);
	} else {
		data.resize(2 + image.h * (6 + image.w));
		Common::MemoryWriteStream output(data.data(), data.size());
		output.writeUint16LE(image.h);
		for (int row = 0; row < image.h; ++row) {
			output.writeUint16LE(overlay.x);
			output.writeUint16LE(overlay.y + row);
			output.writeUint16LE(image.w);
			output.write(pixels.data() + row * image.w, image.w);
		}
	}
	return true;
}

Common::SharedArchiveContents ImageTranslationArchive::readContentsForPath(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::SharedArchiveContents();

	// Read below this archive to avoid resolving our own translated resource again.
	Common::ScopedPtr<Common::SeekableReadStream> original(SearchMan.createReadStreamForMemberNext(path, this));
	if (!original || Common::computeStreamMD5AsString(*original) != _sources[path.toString()]) {
		warning("Image translation does not match %s; using original graphics", path.toString().c_str());
		return Common::SharedArchiveContents();
	}
	ResourceChunkTable table;
	if (!table.load(*original))
		return Common::SharedArchiveContents();

	Common::Array<byte> chunks[kResourceChunkCount];
	Common::String name = path.toString();
	name.toUppercase();
	uint32 size = kResourceChunkCount * 8;
	for (uint i = 0; i < table.count; ++i) {
		if (!table.isValidChunk(i))
			continue;
		if (!readOriginalChunk(*original, table, i, chunks[i]))
			return Common::SharedArchiveContents();
		for (const auto &overlay : _overlays) {
			if (overlay.source == name && overlay.chunk == i && !applyImage(overlay, *original, table, chunks[i])) {
				warning("Invalid translated image %s chunk %u; using original graphics", name.c_str(), i);
				return Common::SharedArchiveContents();
			}
		}
		if (chunks[i].size() > UINT32_MAX - size)
			return Common::SharedArchiveContents();
		size += chunks[i].size();
	}

	byte *buffer = new byte[size];
	Common::MemoryWriteStream output(buffer, size);
	uint32 offset = kResourceChunkCount * 8;
	for (uint i = 0; i < kResourceChunkCount; ++i) {
		output.writeUint32LE(chunks[i].empty() ? 0 : offset);
		offset += chunks[i].size();
	}
	for (uint i = 0; i < kResourceChunkCount; ++i)
		output.writeUint32LE(chunks[i].size());
	for (uint i = 0; i < kResourceChunkCount; ++i) {
		if (!chunks[i].empty())
			output.write(chunks[i].data(), chunks[i].size());
	}
	return Common::SharedArchiveContents(buffer, size);
}

} // End of namespace Hollywood
