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

#include "common/memstream.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/sprite.h"

#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/text.h"
#include "director/castmember/palette.h"
#include "director/castmember/filmloop.h"


namespace Director {

bool RIFXArchive::writeToFile(Common::String filename, Movie *movie) {
	// If the filename is empty, we save the movie with the name of the current movie
	if (filename.empty()) {
		filename = movie->getMacName();
	}

	Common::String saveFileName = g_director->getTargetName() + "-" + filename;
	// Don't open the save file as compressed which doesn't support seeking
	Common::OutSaveFile *saveFile = g_engine->getSaveFileManager()->openForSaving(saveFileName, false);

	if (!saveFile) {
		warning("RIFXArchive::writeToFile: Failed to open file %s for saving", saveFileName.c_str());
		return false;
	}

	// Update the resources, their sizes and offsets
	Common::Array<Resource *> builtResources = rebuildResources(movie);
	// ignoring the startOffset
	// For RIFX stream, moreoffset = 0, we won't be writing macbinary
	// Don't need to allocate this much size in case 'junk' and 'free' resources are ignored
	// Or might need to allocate even more size if extra chunks are written
	_size = getArchiveSize(builtResources);
	saveFile->writeUint32LE(_metaTag); // The _metaTag is "RIFX" or "XFIR"

	saveFile->writeUint32LE(getResourceSize(_metaTag, 0) - 8); // The size of the RIFX archive, except header and size
	saveFile->writeUint32LE(_rifxType);	// e.g. "MV93", "MV95"

	switch (_rifxType) {
	case MKTAG('M', 'V', '9', '3'):
	case MKTAG('M', 'C', '9', '5'):
	case MKTAG('A', 'P', 'P', 'L'):
		writeMemoryMap(saveFile, builtResources);
		break;

	case MKTAG('F', 'G', 'D', 'M'):
	case MKTAG('F', 'G', 'D', 'C'):
		writeAfterBurnerMap(saveFile);
		break;
	default:
		break;
	}

	Cast *cast = movie->getCast();
	ResourceMap castResMap = _types[MKTAG('C', 'A', 'S', 't')];

	for (auto &it : builtResources) {
		debugC(5, kDebugSaving, "RIFXArchive::writeToFile: writing resource '%s': index: %d, size: %d, offset = %d, index: %d",
			tag2str(it->tag), it->index, it->size, it->offset, it->index);

		switch (it->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			// meta resource
			break;

		case MKTAG('i', 'm', 'a', 'p'):
		case MKTAG('m', 'm', 'a', 'p'):
			// Already written
			break;

		case MKTAG('K', 'E', 'Y', '*'):
			writeKeyTable(saveFile, it->offset);
			break;

		case MKTAG('C', 'A', 'S', '*'):
			writeCast(saveFile, it->offset, it->libResourceId);
			break;

		case MKTAG('C', 'A', 'S', 't'):
			cast = movie->getCastByLibResourceID(it->libResourceId);
			cast->saveCastData(saveFile, it);
			break;

		case MKTAG('V', 'W', 'C', 'F'):
			// There is only 'VWCF' resource, that is for the internal cast
			// The external casts don't have a config
			// movie->getCast() returns the internal cast
			cast = movie->getCast();
			cast->saveConfig(saveFile, it->offset);
			break;

		case MKTAG('B', 'I', 'T', 'D'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				cast = movie->getCastByLibResourceID(parent.libResourceId);
				BitmapCastMember *target = (BitmapCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				target->writeBITDResource(saveFile, it->offset);
			}
			break;

		case MKTAG('S', 'T', 'X', 'T'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				cast = movie->getCastByLibResourceID(parent.libResourceId);
				TextCastMember *target = (TextCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				target->writeSTXTResource(saveFile, it->offset);
			}
			break;

		case MKTAG('C', 'L', 'U', 'T'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				cast = movie->getCastByLibResourceID(parent.libResourceId);
				PaletteCastMember *target = (PaletteCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				target->writePaletteData(saveFile, it->offset);
			}
			break;

		case MKTAG('S', 'C', 'V', 'W'):
			{

				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				cast = movie->getCastByLibResourceID(parent.libResourceId);
				FilmLoopCastMember *target = (FilmLoopCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				target->writeSCVWResource(saveFile, it->offset);
			}
			break;

		case MKTAG('V', 'W', 'S', 'C'):
			movie->getScore()->writeVWSCResource(saveFile, it->offset);
			break;

		default:
			debugC(7, kDebugSaving, "Saving resource %s as it is, without modification", tag2str(it->tag));
			saveFile->seek(it->offset, SEEK_SET);
			saveFile->writeUint32LE(it->tag);
			saveFile->writeUint32LE(it->size);
			saveFile->writeStream(getResource(it->tag, it->index));
			break;
		}
	}

	Common::DumpFile out;

	// Write the movie out, stored in dumpData
	if (saveFile) {
		saveFile->flush();
		debugC(3, kDebugSaving, "RIFXArchive::writeStream: Saved the movie as file %s", saveFileName.c_str());
	} else {
		warning("RIFXArchive::writeStream: Error saving the file %s", saveFileName.c_str());
	}

	delete saveFile;
	for (auto it : builtResources) {
		delete it;
	}
	return true;
}

bool RIFXArchive::writeMemoryMap(Common::SeekableWriteStream *writeStream, Common::Array<Resource *> resources) {
	Resource mmap;

	for (auto it : resources) {
		if (it->tag == MKTAG('m', 'm', 'a', 'p')) {
			mmap = *it;
		}
	}

	writeStream->writeUint32LE(MKTAG('i', 'm', 'a', 'p')); // The "imap" resource
	writeStream->writeUint32LE(_imapLength);		// length of "imap" resource
	writeStream->writeUint32LE(_mapversion);		// "imap" version
	writeStream->writeUint32LE(mmap.offset);		// offset of the "mmap" resource
	writeStream->writeUint32LE(_version);

	writeStream->seek(mmap.offset);
	writeStream->writeUint32LE(MKTAG('m', 'm', 'a', 'p'));
	writeStream->writeUint32LE(mmap.size);

	writeStream->writeUint16LE(_mmapHeaderSize);
	writeStream->writeUint16LE(_mmapEntrySize);

	uint32 newResCount = resources.size();
	writeStream->writeUint32LE(newResCount + _totalCount - _resCount); // _totalCount - _resCount is the number of empty entries
	writeStream->writeUint32LE(newResCount);
	writeStream->seek(8, SEEK_CUR);		// In the original file, these 8 bytes are all 0xFF, so this will produce a diff

	// ID of the first 'free' resource, we don't make use of it
	writeStream->writeUint32LE(0);

	for (auto &it : resources) {
		debugC(3, kDebugSaving, "RIFXArchive::writeMemoryMap: Memory map entry: '%s', size: %d, offset: %08x, flags: %x, unk1: %x, nextFreeResourceID: %d",
			tag2str(it->tag), it->size, it->offset, it->flags, it->unk1, it->nextFreeResourceID);

		// Write down the tag, the size and offset of the current resource
		writeStream->writeUint32LE(it->tag);
		writeStream->writeUint32LE(it->size);
		writeStream->writeUint32LE(it->offset);

		// Currently ignoring flags, unk1 and nextFreeResourceID
		writeStream->writeUint16LE(it->flags);
		writeStream->writeUint16LE(it->unk1);
		writeStream->writeUint32LE(it->nextFreeResourceID);
	}

	return true;
}

bool RIFXArchive::writeAfterBurnerMap(Common::SeekableWriteStream *writeStream) {
	warning("RIFXArchive::writeAfterBurnerMap: STUB: Incomplete function, needs further changes, AfterBurnerMap not written");
	return false;

#if 0
	writeStream->writeUint32LE(MKTAG('F', 'v', 'e', 'r'));

	writeStream->writeUint32LE(_fverLength);
	uint32 start = writeStream->pos();

	writeStream->writeUint32LE(_afterBurnerVersion);
	uint32 end = writeStream->pos();

	if (end - start != _fverLength) {
		warning("RIFXArchive::writeAfterburnerMap(): Expected Fver of length %d but read %d bytes", _fverLength, end - start);
		writeStream->seek(start + _fverLength);
	}

	writeStream->writeUint32LE(MKTAG('F', 'c', 'd', 'r'));
	writeStream->writeUint32LE(_fcdrLength);
	writeStream->seek(_fcdrLength, SEEK_CUR);

	writeStream->writeUint32LE(MKTAG('A', 'B', 'M', 'P'));

	return true;
#endif
}

bool RIFXArchive::writeKeyTable(Common::SeekableWriteStream *writeStream, uint32 offset) {
	writeStream->seek(offset);

	writeStream->writeUint32LE(MKTAG('K', 'E', 'Y', '*'));
	writeStream->writeUint32LE(getResourceSize(MKTAG('K', 'E', 'Y', '*'), getResourceIDList(MKTAG('K', 'E', 'Y', '*'))[0]));

	writeStream->writeUint16LE(_keyTableEntrySize);
	writeStream->writeUint16LE(_keyTableEntrySize2);
	writeStream->writeUint32LE(_keyTableEntryCount);
	writeStream->writeUint32LE(_keyTableUsedCount);

	debugC(3, kDebugSaving, "RIFXArchive::writeKeyTable: writing key table:");

	for (auto &childTag : _keyData) {
		KeyMap keyMap = childTag._value;

		for (auto &parentIndex : keyMap) {
			KeyArray keyArray = parentIndex._value;

			for (auto childIndex : keyArray) {
				debugC(3, kDebugSaving, "_keyData contains tag: %s, parentIndex: %d, childIndex: %d", tag2str(childTag._key), parentIndex._key, childIndex);
				writeStream->writeUint32LE(childIndex);
				writeStream->writeUint32LE(parentIndex._key);
				writeStream->writeUint32LE(childTag._key);
			}
		}
	}

	return true;
}

bool RIFXArchive::writeCast(Common::SeekableWriteStream *writeStream, uint32 offset, uint32 libResourceId) {
	writeStream->seek(offset);
	uint32 casSize = getCASResourceSize(libResourceId);

	uint castTag = MKTAG('C', 'A', 'S', 't');
	writeStream->writeUint32LE(MKTAG('C', 'A', 'S', '*'));
	writeStream->writeUint32LE(casSize);

	Common::HashMap<uint16, uint16> castIndexes;

	// We can't just write all the 'CASt' indices randomly, we have to sort them by castId
	// Since the order they appear matters, they are given castIds accordingly
	uint32 maxCastId = 0;
	for (auto &it : _types[castTag]) {
		if (it._value.libResourceId == libResourceId) {
			castIndexes[it._value.castId] = it._value.index;
			maxCastId = MAX(maxCastId, it._value.castId);
		}
	}

	debugC(5, kDebugSaving, "RIFXArchive::writeCast: Writing CAS* resource: size: %d, maxCastID: %d, libResourceID: %d", casSize, maxCastId, libResourceId);
	debugCN(5, kDebugSaving, "'CASt' indexes: [");
	for (uint32 i = 0; i <= maxCastId; i++) {
		uint32 castIndex = castIndexes.getValOrDefault(i, 0);
		if (castIndex) {
			debugCN(5, kDebugSaving, (i == 0 ? "%d" : ", %d"), castIndex);
			writeStream->writeUint32BE(castIndex);
		}
	}
	debugC(5, kDebugSaving, "]");

	return true;
}

Common::Array<Resource *> RIFXArchive::rebuildResources(Movie *movie) {
	// Currently handled Resource types:
	// imap		// BITD
	// mmap		// CLUT
	// RIFX		// STXT
	// KEY*		// SCVW (filmloop)
	// CAS*		// VWCF
	// CASt
	// SCVW
	// 'RTE0', 'RTE1', 'RTE2'

	// First we'll have to update the _types table to include all the newly added
	// cast members, and their 'CASt' resources
	Cast *cast = nullptr;
	ResourceMap &castResMap = _types[MKTAG('C', 'A', 'S', 't')];

	// Iterate over all the casts
	for (auto it : *(movie->getCasts())) {
		cast = it._value;

		// Iterate over all the loaded members of the cast to check for new cast members
		for (auto jt : *(cast->_loadedCast)) {
			// If the index is -1, that means the cast member is new or duplicated
			if (jt._value->_index == -1) {
				Resource *res = nullptr;
				uint16 targetCastId = jt._value->getID() - cast->_castArrayStart;

				// Checking if the castId already exists in the CASt resources
				for (auto castRes : castResMap) {
					if (castRes._value.castId == targetCastId) {
						res = &castRes._value;
					}
				}

				if (!res) {
					// If the castId is new, create a new resource
					// Assigning the next available index to the resource
					res = &castResMap[_resources.size()];
					res->tag = MKTAG('C', 'A', 'S', 't');
					res->accessed = true;

					res->libResourceId = cast->_libResourceId;
					res->children = jt._value->_children;
					res->index = _resources.size();
					res->castId = jt._value->getID() - cast->_castArrayStart;

					for (auto child : jt._value->_children) {
						_keyData[child.tag][res->index].push_back(child.index);
						_keyTableUsedCount += 1;
						_keyTableEntryCount += 1;
					}
					_resources.push_back(res);

					debugC(5, kDebugSaving, "RIFXArchive::rebuildResources(): new 'CASt' resource added");
				} else {
					// The castId is not new, overwrite the key data of the previous cast
					for (auto child : res->children) {
						// Remove the data of the previous (removed) 'CASt'
						int8 count = _keyData[child.tag][res->index].size();
						_keyData[child.tag][res->index].clear();
						_keyTableUsedCount -= count;
						_keyTableEntryCount -= count;
					}

					res->children = jt._value->_children;

					for (auto child : res->children) {
						_keyData[child.tag][res->index].push_back(child.index);
						_keyTableUsedCount += 1;
						_keyTableEntryCount += 1;
					}
				}
			}
		}
	}

	// TODO: Then we'll need to see if there are any other newly added resources
	// Now when you use `duplicate`, say BitmapCastMember, the cast member is duplicated
	// but its children resources are not, meaning the duplicated BitmapCastMember is also loaded from the same 'BITD' resource
	// So it is not necessary to duplicate the 'BITD' resource
	// However, in case an entirely new cast member is added, say a filmloop is recorded, then that requires a new 'SCVW' resource
	// Same goes for if a new cast is added to the movie
	// Ignoring that for now

	// Next step is to recalculate the sizes and the offsets of all the resources

	// Since the first 3 resources are determined (RIFX, imap and mmap)
	// (the mmap doesn't need to be the third resource, but for simplicity, it's better there)
	// We'll start writing after that RIFX header, mmap and imap resources
	// The first 12 bytes are metaTag ('RIFX'), size of file, and RIFX type ('MV93', 'MV95', etc.)
	// The +8 bytes are to account for the header and size
	uint32 currentSize = 12 + (getImapSize() + 8) + (getMmapSize() + 8);

	// need to make a new resources array, because we need the old offsets as well as new ones
	Common::Array<Resource *> builtResources;

	for (auto it: _resources) {
		builtResources.push_back(new Resource(it));
	}

	uint32 resSize = 0;
	for (auto &it : builtResources) {
		switch (it->tag) {
		case MKTAG('R', 'I', 'F', 'X'):
		case MKTAG('X', 'F', 'I', 'R'):
			// only one resource only
			// Size will be determined after all other sizes have been calculated
			it->offset = 0;
			break;

		case MKTAG('i', 'm', 'a', 'p'):
			// one resource only
			it->size = getImapSize();
			it->offset = 12;		// First 12 bytes are reserved for metaTag ('RIFX'), size of file, and RIFX type ('MV93', 'MV95', etc.)
			break;

		case MKTAG('m', 'm', 'a', 'p'):
			// one resource only
			it->size = getMmapSize();
			it->offset = 12 + (getImapSize() + 8);	// The +8 is to account for header and size
			break;

		case MKTAG('C', 'A', 'S', 't'):
			{
				cast = movie->getCastByLibResourceID(it->libResourceId);
				// The castIds of cast members start from _castArrayStart
				CastMember *target = cast->getCastMember(it->castId + cast->_castArrayStart);

				if (target) {
					resSize = target->getCastResourceSize();
					it->size = resSize;		// getCastResourceSize returns size without header and size
				} else {
					resSize = it->size;
				}
				it->offset = currentSize;
				currentSize += resSize + 8;
			}
			break;

		case MKTAG('C', 'A', 'S', '*'):
			// Currently handling only movies with one 'CAS*' resource, i.e. only one cast
			resSize = getCASResourceSize(it->libResourceId);	// getCASResourceSize() returns size without header and size
			it->size = resSize;
			it->offset = currentSize;
			currentSize += resSize + 8;
			break;

		case MKTAG('K', 'E', 'Y', '*'):
			resSize = getKeyTableResourceSize();
			it->size = resSize;
			it->offset = currentSize;
			currentSize += resSize + 8;
			break;

		case MKTAG('V', 'W', 'C', 'F'):
			{
				// Only one cast config per movie
				// No need to update the key mapping
				cast = movie->getCast();
				resSize = cast->getConfigSize();

				it->offset = currentSize;

				currentSize += resSize + 8;			// getConfigSize() doesn't include header and size
				it->size = resSize;
			}
			break;

		case MKTAG('S', 'T', 'X', 'T'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				TextCastMember *target = (TextCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				resSize = target->getSTXTResourceSize();

				it->offset = currentSize;
				it->size = resSize;

				currentSize += resSize + 8;
			}
			break;

		case MKTAG('C', 'L', 'U', 'T'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				// Get the appropriate cast in case of multiple casts
				cast = movie->getCastByLibResourceID(parent.libResourceId);
				PaletteCastMember *target = (PaletteCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				resSize = target->getPaletteDataSize();

				it->offset = currentSize;
				it->size = resSize;

				currentSize += resSize + 8;
			}
			break;

		case MKTAG('B', 'I', 'T', 'D'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				// Get the appropriate cast in case of multiple casts
				cast = movie->getCastByLibResourceID(parent.libResourceId);
				BitmapCastMember *target = (BitmapCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				resSize = target->getBITDResourceSize();

				it->offset = currentSize;
				it->size = resSize;

				currentSize += resSize + 8;
			}
			break;

		case MKTAG('S', 'C', 'V', 'W'):
			{
				uint32 parentIndex = findParentIndex(it->tag, it->index);
				Resource parent = castResMap[parentIndex];

				FilmLoopCastMember *target = (FilmLoopCastMember *)cast->getCastMember(parent.castId + cast->_castArrayStart);
				resSize = target->getSCVWResourceSize();

				it->offset = currentSize;
				it->size = resSize;

				currentSize += resSize + 8;
			}
			break;

		case MKTAG('V', 'W', 'S', 'C'):
			resSize = movie->getScore()->getVWSCResourceSize();
			it->size = resSize;
			it->offset = currentSize;
			currentSize += resSize + 8;		// The size doesn't include the header and the size entry
			break;

		case MKTAG('f', 'r', 'e', 'e'):
		case MKTAG('j', 'u', 'n', 'k'):
			// These resources do not hold any data
			it->size = 0;

			// We could just ignore these and not write them at all
			it->offset = currentSize;
			currentSize += 8;
			break;

		default:
			it->offset = currentSize;
			currentSize += it->size + 8;		// This size doesn't include the header and size entry
			break;

		}
		debugC(3, kDebugSaving, "Rebuild RIFX resource index %d: '%s', %d bytes @ 0x%08x (%d), flags: %x unk1: %x nextFreeResourceId: %d",
			it->index, tag2str(it->tag), it->size, it->offset, it->offset, it->flags, it->unk1, it->nextFreeResourceID);
	}

	// Now that all sizes have been updated, we can safely calculate the overall archive size
	for (auto &it : builtResources) {
		if (it->tag == MKTAG('R', 'I', 'F', 'X') || it->tag == MKTAG('X', 'F', 'I', 'R')) {
			it->size = getArchiveSize(builtResources) + 8;
		}
	}

	return builtResources;
}

uint32 RIFXArchive::getImapSize() {
	// The length of imap doesn't change
	// This is the length without header and size
	return _imapLength;
}

uint32 RIFXArchive::getMmapSize() {
	// The headers: 24 bytes and and 20 bytes per resources
	return 24 + 20 * _resources.size();
}

uint32 RIFXArchive::getArchiveSize(Common::Array<Resource *> resources) {
	// This will be called after updating the size of all the resources
	uint32 size = 0;

	for (auto it : resources) {
		if (it->tag != MKTAG('R', 'I', 'F', 'X') && it->tag != MKTAG('X', 'F', 'I', 'R')) {
			size += it->size + 8;		// The 8 is to account for the header and size
		}
	}
	return size;
}

uint32 RIFXArchive::getCASResourceSize(uint32 libResourceID) {
	uint castTag = MKTAG('C', 'A', 'S', 't');
	uint32 maxCastId = 0;

	// maxCastId is the basically the number of cast members present in the cast
	// This is the number of entries present in the 'CAS*' resource
	for (auto &it : _types[castTag]) {
		if (it._value.libResourceId == libResourceID) {
			maxCastId = MAX(maxCastId, it._value.castId);
		}
	}

	return (maxCastId + 1) * 4;
}

uint32 RIFXArchive::getKeyTableResourceSize() {
	// 12 bytes of header + 12 * number of entries
	uint32 size = 0;
	for (auto &childTag : _keyData) {
		KeyMap keyMap = childTag._value;

		for (auto &parentIndex : keyMap) {
			KeyArray keyArray = parentIndex._value;

			size += keyArray.size() * 12u;
		}
	}
	return size + 12u;
}

void dumpFile(Common::String fileName, uint32 id, uint32 tag, byte *dumpData, uint32 dumpSize) {
	debug("dumpFile():: dumping file %s, resource: %s (id: %d)", fileName.c_str(), tag2str(tag), id);
	Common::DumpFile out;
	Common::String fname = Common::String::format("./dumps/%d-%s-%s", id, tag2str(tag), fileName.c_str());

	// Write the movie out, stored in dumpData
	if (out.open(Common::Path(fname), true)) {
		out.write(dumpData, dumpSize);
		out.flush();
		out.close();
	} else {
		warning("RIFXArchive::writeStream: Error saving the file %s", fname.c_str());
	}
}

uint32 RIFXArchive::findParentIndex(uint32 tag, uint16 index) {
	// We have to find the parent
	// Look into the keyData, for all parents of resource tag
	// If the parent contains this resource's index, that's our parent
	for (auto &it : _keyData[tag]) {
		for (auto &kt : it._value) {
			if (kt == index) {
				return it._key;
			}
		}
	}

	warning("RIFXArchive::findParentIndex: The parent for resource: %s, index: %d, was not found", tag2str(tag), index);
	return 0;
}

SavedArchive::SavedArchive(Common::String target) {
	Common::StringArray saveFileList = g_engine->getSaveFileManager()->listSavefiles(target + "-*");

	debugC(3, kDebugLoading, "DirectorEngine:: loadSaveFiles: Loading save files");
	for (auto saveFileName : saveFileList) {
		// Derive the original file name from the save file name
		// Save files are named target_name-save_filename
		Common::String origFileName = saveFileName.substr(target.size() + 1);
		debugC(3, kDebugLoading, "Found save file: %s -> %s", saveFileName.c_str(), origFileName.c_str());

		_files[origFileName] = saveFileName;
	}
}

bool SavedArchive::hasFile(const Common::Path &path) const {
	return (_files.find(path.toString()) != _files.end());
}

int SavedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, *this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr SavedArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *SavedArchive::createReadStreamForMember(const Common::Path &path) const {
	FileMap::const_iterator fDesc = _files.find(path.toString());
	if (fDesc == _files.end())
		return nullptr;

	return g_engine->getSaveFileManager()->openForLoading(fDesc->_value);
}

} // End of namespace Director
