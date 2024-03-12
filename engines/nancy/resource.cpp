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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"
#include "common/config-manager.h"

#include "image/bmp.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/decompress.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"
#include "engines/nancy/iff.h"

static char treePrefix[] = "_tree_";

namespace Nancy {

bool ResourceManager::loadImage(const Common::Path &name, Graphics::ManagedSurface &surf, const Common::String &treeName, Common::Rect *outSrc, Common::Rect *outDest) {
	if (name.empty()) {
		return false;
	}

	// Detect and load autotext surfaces
	Common::String baseName(name.baseName());
	if (baseName.hasPrefixIgnoreCase("USE_")) {
		int surfID = -1;

		if (baseName.hasPrefixIgnoreCase("USE_AUTOTEXT")) {
			surfID = baseName[12] - '1';
		} else if (baseName.hasPrefixIgnoreCase("USE_AUTOJOURNAL")) { // nancy6/7
			surfID = baseName.substr(15).asUint64() + 2;
		} else if (baseName.hasPrefixIgnoreCase("USE_AUTOLIST")) { // nancy8
			surfID = baseName.substr(12).asUint64() + 2;
		}

		if (surfID >= 0) {
			surf.copyFrom(g_nancy->_graphics->getAutotextSurface(surfID));
			if (outSrc) {
				// Slightly hacky, but we pass the size of the drawn text using the outSrc parameter;
				// value is only guaranteed to be valid for an active surface.
				// This is used for PeepholePuzzle scrolling.
				*outSrc = g_nancy->_graphics->getAutotextSurfaceBounds(surfID);
			}

			return true;
		}
	}

	CifInfo info;
	Common::SeekableReadStream *stream = nullptr;

	// First, check for external .bmp (TVD only; can also be enabled via a hidden ConfMan option)
	if (g_nancy->getGameType() == kGameTypeVampire || ConfMan.getBool("external_bmp", ConfMan.getActiveDomainName())) {
		stream = SearchMan.createReadStreamForMember(name.append(".bmp"));
		if (stream) {
			// Found external image
			Image::BitmapDecoder bmpDec;
			bmpDec.loadStream(*stream);
			surf.copyFrom(*bmpDec.getSurface());
			surf.setPalette(bmpDec.getPalette(), bmpDec.getPaletteStartIndex(), MIN<uint>(256, bmpDec.getPaletteColorCount())); // LOGO.BMP reports 257 colors
		}
	}

	if (g_nancy->getGameType() == kGameTypeVampire) {
		// .cifs/ciftrees were introduced with nancy1. We also don't need to flip endianness, since the BMP decoder should handle that by itself
		return false;
	}

	// Check for loose .cif images. This bypasses tree search even with a provided treeName
	if (!stream) {
		stream = SearchMan.createReadStreamForMember(name.append(".cif"));
		if (stream) {
			// .cifs are compressed, so we need to extract
			CifFile cifFile(stream, name); // cifFile takes ownership of the current stream
			stream = cifFile.createReadStream();
			info = cifFile._info;
		}
	}

	// Search inside the ciftrees
	if (!stream) {
		if (!treeName.empty()) {
			// Tree name was provided, bypass SearchMan
			Common::String upper = treeName;
			upper.toUppercase();
			const CifTree *tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);

			stream = tree->createReadStreamForMember(Common::Path(name));
			info = tree->getCifInfo(name);
		}

		if (!stream) {
			// Tree name was not provided, or lookup failed. Use SearchMan
			stream = SearchMan.createReadStreamForMember(Common::Path(name));

			if (!stream) {
				warning("Couldn't open image file %s", name.toString().c_str());
				return false;
			}

			// Search for the info struct in all ciftrees
			const CifTree *tree = nullptr;
			for (uint i = 0; i < _cifTreeNames.size(); ++i) {
				// No provided tree name, check inside every loaded tree
				Common::String upper = _cifTreeNames[i];
				upper.toUppercase();
				if (SearchMan.getArchive(treePrefix + upper)->hasFile(Common::Path(name))) {
					tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
					break;
				}
			}

			if (tree) {
				info = tree->getCifInfo(name);
			} else {
				// Image was found inside ciftree, but its CifInfo wasn't. This _should_ be unreachable
				error("Couldn't find CifInfo struct inside loaded CifTrees");
			}
		}
	}

	// Sanity checks
	if (info.type != CifInfo::kResTypeImage) {
		warning("Resource '%s' is not an image", name.toString().c_str());
		delete stream;
		return false;
	}

	if (info.depth != 16) {
		warning("Image '%s' has unsupported depth %i", name.toString().c_str(), info.depth);
		delete stream;
		return false;
	}

	// Load the src/dest rects when requested
	if (outSrc) {
		*outSrc = info.src;
	}

	if (outDest) {
		*outDest = info.dest;
	}

	// Finally, copy the data into the surface
	uint32 bufSize = info.pitch * info.height * (info.depth / 16);
	byte *buf = new byte[bufSize];
	stream->read(buf, bufSize);

	#ifdef SCUMM_BIG_ENDIAN
	// Flip endianness on BE machines
	for (uint i = 0; i < bufSize / 2; ++i) {
		((uint16 *)buf)[i] = SWAP_BYTES_16(((uint16 *)buf)[i]);
	}
	#endif

	GraphicsManager::copyToManaged(buf, surf, info.width, info.height, g_nancy->_graphics->getInputPixelFormat());
	delete[] buf;
	delete stream;
	return true;
}

IFF *ResourceManager::loadIFF(const Common::Path &name) {
	// First, try to load external .cif
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(name.append(".cif"));
	if (stream) {
		// .cifs are compressed, so we need to extract
		CifFile cifFile(stream, name); // cifFile takes ownership of the current stream
		stream = cifFile.createReadStream();
	}

	if (!stream) {
		// Then, look for external .iff. These are uncompressed
		stream = SearchMan.createReadStreamForMember(name.append(".iff"));

		// Finally, look inside ciftrees
		if (!stream) {
			stream = SearchMan.createReadStreamForMember(Common::Path(name));
		}
	}

	if (stream) {
		return new IFF(stream);
	}

	return nullptr;
}

bool ResourceManager::readCifTree(const Common::String &name, const Common::String &ext, int priority) {
	CifTree *tree = CifTree::makeCifTreeArchive(name, ext);
	if (!tree) {
		return false;
	}

	// Add a prefix to avoid clashes with the ciftree folder present in some games.
	// Also, set the name itself to uppercase since SearchMan is case-sensitive.
	// Final name to look up is _tree_TREENAME
	Common::String upper = name;
	upper.toUppercase();
	SearchMan.add(treePrefix + upper, tree, priority, true);
	_cifTreeNames.push_back(name);
	return true;
}

PatchTree *ResourceManager::readPatchTree(Common::SeekableReadStream *stream, const Common::String &name, int priority) {
	if (!stream) {
		return nullptr;
	}

	PatchTree *tree = new PatchTree(stream, Common::Path(name));
	Common::Serializer ser(stream, nullptr);

	if (!tree->sync(ser)) {
		delete tree;
		return nullptr;
	}

	Common::String upper = name;
	upper.toUppercase();
	SearchMan.add(treePrefix + upper, tree, priority, true);
	_cifTreeNames.push_back(name);
	return tree;
}

Common::String ResourceManager::getCifDescription(const Common::String &treeName, const Common::Path &name) const {
	const CifTree *tree = nullptr;
	if (treeName.size()) {
		Common::String upper = treeName;
		upper.toUppercase();
		tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
	} else {
		for (uint i = 0; i < _cifTreeNames.size(); ++i) {
			// No provided tree name, check inside every loaded tree
			Common::String upper = _cifTreeNames[i];
			upper.toUppercase();
			if (SearchMan.getArchive(treePrefix + upper)->hasFile(name)) {
				tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
				break;
			}
		}
	}

	if (!tree) {
		error("Couldn't find CifInfo struct inside loaded CifTrees");
	}

	const CifInfo &info = tree->getCifInfo(name);

	Common::String desc;
	desc = Common::String::format("Name: %s\n", info.name.toString().c_str());
	desc += Common::String::format("Type: %i\n", info.type);
	desc += Common::String::format("Compression: %i\n", info.comp);
	desc += Common::String::format("Size: %i\n", info.size);
	desc += Common::String::format("Compressed size: %i\n", info.compressedSize);
	desc += Common::String::format("Width: %i\n", info.width);
	desc += Common::String::format("Pitch: %i\n", info.pitch);
	desc += Common::String::format("Height: %i\n", info.height);
	desc += Common::String::format("Bit depth: %i\n", info.depth);

	return desc;
}

void ResourceManager::list(const Common::String &treeName, Common::Array<Common::Path> &outList, CifInfo::ResType type) const {
	if (treeName.size()) {
		Common::String upper = treeName;
		upper.toUppercase();
		const CifTree *tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
		if (!tree) {
			return;
		}
		for (auto &i : tree->_fileMap) {
			if (type == CifInfo::kResTypeAny || i._value.type == type) {
				outList.push_back(i._key);
			}
		}
	} else {
		for (uint i = 0; i < _cifTreeNames.size(); ++i) {
			// No provided tree name, check inside every loaded tree
			Common::String upper = _cifTreeNames[i];
			upper.toUppercase();
			const CifTree *tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
			for (auto &it : tree->_fileMap) {
				if (type == CifInfo::kResTypeAny || it._value.type == type) {
					outList.push_back(it._key);
				}
			}
		}
	}
}

bool ResourceManager::exportCif(const Common::String &treeName, const Common::Path &name) {
	if (!SearchMan.hasFile(name)) {
		return false;
	}

	// First, look for a loose .cif file
	CifInfo info;
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(name.append(".cif"));
	if (stream) {
		// .cifs are compressed, so we need to extract
		CifFile cifFile(stream, name); // cifFile takes ownership of the current stream
		stream = cifFile.createReadStreamRaw();
		info = cifFile._info;
	}

	if (!stream) {
		// Then, look for an external .iff. These are uncompressed
		stream = SearchMan.createReadStreamForMember(name.append(".iff"));
		if (stream) {
			info.comp = CifInfo::kResCompressionNone;
			info.type = CifInfo::kResTypeScript;
			info.name = name;
			info.compressedSize = info.size = stream->size();
		} else {
			// Look inside ciftrees
			const CifTree *tree = nullptr;
			for (uint j = 0; j < _cifTreeNames.size(); ++j) {
				Common::String upper = _cifTreeNames[j];
				upper.toUppercase();
				if (SearchMan.getArchive(treePrefix + upper)->hasFile(name)) {
					tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
					break;
				}
			}

			if (tree) {
				stream = tree->createReadStreamRaw(name);
				info = tree->getCifInfo(name);
			} else {
				// Finally, use SearchMan to get a loose file. This is useful if we want to add files that
				// would regularly not be in a ciftree (e.g. sounds)
				stream = SearchMan.createReadStreamForMember(name);
				if (!stream) {
					warning("Couldn't open resource %s", name.toString().c_str());
					return false;
				}

				info.comp = CifInfo::kResCompressionNone;
				info.type = CifInfo::kResTypeScript;
				info.name = name;
				info.compressedSize = info.size = stream->size();
			}
		}
	}

	CifFile file;
	file._info = info;

	Common::DumpFile dump;
	dump.open(name.append(".cif"));

	Common::Serializer ser(nullptr, &dump);
	file.sync(ser);

	dump.writeStream(stream);
	dump.close();

	delete stream;
	return true;
}

bool ResourceManager::exportCifTree(const Common::String &treeName, const Common::Array<Common::Path> &names) {
	Common::Array<Common::SeekableReadStream *> resStreams;
	CifTree file;

	uint32 headerSize = 1024 * 2;
	uint32 infoSize = 0;
	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		headerSize += 30;
		infoSize = 38;
	} else {
		headerSize += 32;
		if (g_nancy->getGameType() <= kGameTypeNancy2) {
			// Format 1, short filenames
			infoSize = 70;
		} else {
			// Format 1 or 2*, with long filenames
			infoSize = 94;
		}
	}

	for (uint i = 0; i < names.size(); ++i) {
		const Common::Path &path = names[i];
		// First, look for loose .cif files
		CifInfo info;
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(path.append(".cif"));
		if (stream) {
			// .cifs are compressed, so we need to extract
			CifFile cifFile(stream, path); // cifFile takes ownership of the current stream
			stream = cifFile.createReadStreamRaw();
			info = cifFile._info;
		}

		if (!stream) {
			// Then, look for external .iff. These are uncompressed
			stream = SearchMan.createReadStreamForMember(path.append(".iff"));
			if (stream) {
				info.comp = CifInfo::kResCompressionNone;
				info.type = CifInfo::kResTypeScript;
				info.name = path;
				info.compressedSize = info.size = stream->size();
			} else {
				// Look inside ciftrees
				const CifTree *tree = nullptr;
				for (uint j = 0; j < _cifTreeNames.size(); ++j) {
					Common::String upper = _cifTreeNames[j];
					upper.toUppercase();
					if (SearchMan.getArchive(treePrefix + upper)->hasFile(path)) {
						tree = (const CifTree *)SearchMan.getArchive(treePrefix + upper);
						break;
					}
				}

				if (tree) {
					stream = tree->createReadStreamRaw(path);
					info = tree->getCifInfo(path);
				} else {
					// Finally, use SearchMan to get a loose file. This is useful if we want to add files that
					// would regularly not be in a ciftree (e.g. sounds)
					stream = SearchMan.createReadStreamForMember(path);
					if (!stream) {
						warning("Couldn't open resource %s", path.toString().c_str());
						continue;
					}

					info.comp = CifInfo::kResCompressionNone;
					info.type = CifInfo::kResTypeScript;
					info.name = path;
					info.compressedSize = info.size = stream->size();
				}
			}
		}

		resStreams.push_back(stream);
		file._writeFileMap.push_back(info);
	}

	uint16 dataOffset = headerSize + file._writeFileMap.size() * infoSize; // Initial offset after header/infos
	for (uint i = 0; i < file._writeFileMap.size(); ++i) {
		file._writeFileMap[i].dataOffset = dataOffset;
		for (uint j = 0; j < i; ++j) {
			file._writeFileMap[i].dataOffset += resStreams[j]->size(); // Final offset, following raw data of previous files
		}
	}

	Common::DumpFile dump;
	dump.open(Common::Path(treeName + ".dat"));

	Common::Serializer ser(nullptr, &dump);
	file.sync(ser);

	for (uint i = 0; i < resStreams.size(); ++i) {
		dump.writeStream(resStreams[i]);
		delete resStreams[i];
	}

	dump.close();
	return true;
}

} // End of namespace Nancy
