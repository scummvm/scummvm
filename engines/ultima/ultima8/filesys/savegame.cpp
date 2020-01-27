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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/savegame.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/shared/engine/ultima.h"
#include "common/system.h"
#include "common/savefile.h"
#include "graphics/thumbnail.h"

namespace Ultima {
namespace Ultima8 {

#define SAVEGAME_IDENT MKTAG('V', 'M', 'U', '8')
#define SAVEGAME_VERSION 5

SavegameReader::SavegameReader(IDataSource *ds, bool metadataOnly) : _file(ds), _version(0) {
	if (!MetaEngine::readSavegameHeader(ds->GetRawStream(), &_header, false))
		return;

	// Set total play time
	g_engine->setTotalPlayTime(_header.playtime * 1000);

	// Validate the identifier for a valid savegame
	uint32 ident = ds->read4();
	if (ident != SAVEGAME_IDENT)
		return;

	_version = ds->read4();
	if (metadataOnly)
		return;

	// Load the index
	uint count = ds->read2();

	for (uint idx = 0; idx < count; ++idx) {
		char name[12];
		ds->read(name, 12);
		name[11] = '\0';

		FileEntry fe;
		fe._size = ds->read4();
		fe._offset = ds->getPos();

		_index[Common::String(name)] = fe;
		ds->skip(fe._size);
	}
}

SavegameReader::~SavegameReader() {
	delete _file;
}

SavegameReader::State SavegameReader::isValid() const {
	if (_version == 0)
		return SAVE_CORRUPT;
	else if (_version < SAVEGAME_VERSION)
		return SAVE_OUT_OF_DATE;
	else if (_version > SAVEGAME_VERSION)
		return SAVE_TOO_RECENT;
	else
		return SAVE_VALID;
}

IDataSource *SavegameReader::getDataSource(const std::string &name) {
	assert(_index.contains(name));

	const FileEntry &fe = _index[name];
	uint8 *data = (uint8 *)malloc(fe._size);
	_file->seek(fe._offset);
	_file->read(data, fe._size);

	return new IBufferDataSource(data, fe._size, false, true);
}


SavegameWriter::SavegameWriter(ODataSource *ds) : _file(ds) {
	assert(_file);
}

SavegameWriter::~SavegameWriter() {
}

bool SavegameWriter::finish() {
	 // Write ident and savegame version
	_file->write4(SAVEGAME_IDENT);
	_file->write4(SAVEGAME_VERSION);

	// Iterate through writing out the files
	_file->write2(_index.size());
	for (uint idx = 0; idx < _index.size(); ++idx) {
		// Set up a 12 byte space containing the resource name
		FileEntry &fe = _index[idx];
		char name[12];
		Common::fill(&name[0], &name[12], '\0');
		strncpy(name, fe._name.c_str(), 11);

		// Write out name, size, and data
		_file->write(name, 12);
		_file->write4(fe.size());
		_file->write(&fe[0], fe.size());
	}
	
	// Handle adding savegame header
	Common::OutSaveFile *dest = dynamic_cast<Common::OutSaveFile *>(_file->GetRawStream());
	MetaEngine::appendExtendedSave(dest, Shared::g_ultima->getTotalPlayTime() / 1000, _description);
	dest->finalize();

	return true;
}

bool SavegameWriter::writeFile(const std::string &name, const uint8 *data, uint32 size) {
	assert(name.size() <= 11);
	_index.push_back(FileEntry());

	FileEntry &fe = _index.back();
	fe._name = name;
	fe.resize(size);
	Common::copy(data, data + size, &fe[0]);

	return true;
}

bool SavegameWriter::writeFile(const std::string &name, OAutoBufferDataSource *ods) {
	return writeFile(name, ods->getBuf(), ods->getSize());
}

} // End of namespace Ultima8
} // End of namespace Ultima
