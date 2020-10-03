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

#include "glk/quetzal.h"
#include "glk/glk_api.h"
#include "glk/events.h"
#include "common/system.h"
#include "common/language.h"
#include "common/memstream.h"
#include "common/translation.h"

namespace Glk {

/*--------------------------------------------------------------------------*/

uint32 QuetzalBase::getInterpreterTag(InterpreterType interpType) {
	switch (interpType) {
	case INTERPRETER_ADRIFT:
		return MKTAG('A', 'D', 'R', 'I');
	case INTERPRETER_ADVSYS:
		return MKTAG('A', 'S', 'Y', 'S');
	case INTERPRETER_AGT:
		return MKTAG('A', 'G', 'T', ' ');
	case INTERPRETER_AGILITY:
		return MKTAG('A', 'G', 'I', 'L');
	case INTERPRETER_ALAN2:
		return MKTAG('A', 'L', 'N', '2');
	case INTERPRETER_ALAN3:
		return MKTAG('A', 'L', 'N', '3');
	case INTERPRETER_ARCHETYPE:
		return MKTAG('A', 'R', 'C', 'H');
	case INTERPRETER_COMPREHEND:
		return MKTAG('C', 'O', 'M', 'P');
	case INTERPRETER_GEAS:
		return MKTAG('G', 'E', 'A', 'S');
	case INTERPRETER_GLULX:
		return MKTAG('G', 'L', 'U', 'L');
	case INTERPRETER_HUGO:
		return MKTAG('H', 'U', 'G', 'O');
	case INTERPRETER_JACL:
		return MKTAG('J', 'A', 'C', 'L');
	case INTERPRETER_LEVEL9:
		return MKTAG('L', 'V', 'L', '9');
	case INTERPRETER_MAGNETIC:
		return MKTAG('M', 'A', 'G', 'N');
	case INTERPRETER_QUEST:
		return MKTAG('Q', 'U', 'E', 'S');
	case INTERPRETER_SCARE:
		return MKTAG('S', 'C', 'A', 'R');
	case INTERPRETER_SCOTT:
		return MKTAG('S', 'C', 'O', 'T');
	case INTERPRETER_TADS2:
		return MKTAG('T', 'A', 'D', '2');
	case INTERPRETER_TADS3:
		return MKTAG('T', 'A', 'D', '3');
	case INTERPRETER_ZCODE:
		return MKTAG('Z', 'C', 'O', 'D');
	default:
		error("Invalid interpreter type");
	}
}

/*--------------------------------------------------------------------------*/

void QuetzalReader::clear() {
	_chunks.clear();
	_stream = nullptr;
}

bool QuetzalReader::open(Common::SeekableReadStream *stream, uint32 formType) {
	clear();
	stream->seek(0);
	_stream = stream;

	if (stream->readUint32BE() != ID_FORM)
		return false;

	uint32 size = stream->readUint32BE();
	uint32 fileFormType = stream->readUint32BE();

	if ((formType != 0 && fileFormType != formType) ||
		(formType == 0 && fileFormType != ID_IFZS && fileFormType != ID_IFSF))
		return false;

	if ((int)size > stream->size() || (size & 1) || (size < 4))
		return false;
	size -= 4;

	// Iterate through reading chunk headers
	while (size > 0) {
		if (size < 8)
			// Couldn't contain a chunk
			return false;

		// Get in the chunk header
		Chunk c;
		c._id = stream->readUint32BE();
		c._size = stream->readUint32BE();
		c._offset = stream->pos();
		_chunks.push_back(c);

		int chunkRemainder = c._size + (c._size & 1);
		if ((stream->pos() + chunkRemainder) > stream->size())
			// Chunk goes beyond the file size
			return false;

		size -= 8 + chunkRemainder;
		stream->skip(chunkRemainder);
	}

	return true;
}

bool QuetzalReader::getSavegameDescription(Common::SeekableReadStream *rs, Common::String &saveName) {
	QuetzalReader r;
	if (!r.open(rs, 0))
		return false;

	for (Iterator it = r.begin(); it != r.end(); ++it) {
		if ((*it)._id == ID_ANNO) {
			Common::SeekableReadStream *s = it.getStream();
			saveName = readString(s);
			delete s;

			return true;
		}
	}

	saveName = _("Untitled Savegame");
	return true;
}

bool QuetzalReader::getSavegameMetaInfo(Common::SeekableReadStream *rs, SaveStateDescriptor &ssd) {
	QuetzalReader r;
	if (!r.open(rs, 0))
		return false;

	ssd.setDescription(_("Untitled Savegame"));

	for (Iterator it = r.begin(); it != r.end(); ++it) {
		if ((*it)._id == ID_ANNO) {
			Common::SeekableReadStream *s = it.getStream();
			ssd.setDescription(readString(s));
			delete s;

		} else if ((*it)._id == ID_SCVM) {
			Common::SeekableReadStream *s = it.getStream();
			int year = s->readUint16BE();
			int month = s->readUint16BE();
			int day = s->readUint16BE();
			int hour = s->readUint16BE();
			int minute = s->readUint16BE();
			uint32 playTime = s->readUint32BE();
			delete s;

			ssd.setSaveDate(year, month, day);
			ssd.setSaveTime(hour, minute);
			ssd.setPlayTime(playTime);
		}
	}

	return true;
}

Common::String QuetzalReader::readString(Common::ReadStream *src) {
	char c;
	Common::String result;
	while ((c = src->readByte()) != 0)
		result += c;

	return result;
}

/*--------------------------------------------------------------------------*/

Common::WriteStream &QuetzalWriter::add(uint32 chunkId) {
	// Sanity check to prevent adding the same chunk multiple times
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		if (_chunks[idx]._id == chunkId)
			error("Duplicate chunk added");
	}

	_chunks.push_back(Chunk(chunkId));
	return _chunks.back()._stream;
}

void QuetzalWriter::save(Common::WriteStream *out, const Common::String &saveName, uint32 formType) {
	// Add chunks common to all Glk savegames
	addCommonChunks(saveName);
	
	// Calculate the size of the chunks
	uint size = 4;
	for (uint idx = 0; idx < _chunks.size(); ++idx)
		size += 8 + _chunks[idx]._stream.size() + (_chunks[idx]._stream.size() & 1);

	// Write out the header
	out->writeUint32BE(ID_FORM);
	out->writeUint32BE(size);
	out->writeUint32BE(formType);

	// Loop through writing the chunks
	for (uint idx = 0; idx < _chunks.size(); ++idx) {
		Common::MemoryWriteStreamDynamic &s = _chunks[idx]._stream;

		out->writeUint32BE(_chunks[idx]._id);
		out->writeUint32BE(s.size());
		out->write(s.getData(), s.size());
		if (s.size() & 1)
			out->writeByte(0);
	}
}

void QuetzalWriter::addCommonChunks(const Common::String &saveName) {
	// Write 'ANNO' chunk
	{
		Common::WriteStream &ws = add(ID_ANNO);
		ws.write(saveName.c_str(), saveName.size());
		ws.writeByte(0);
	}

	// Write 'SCVM' chunk with game version & gameplay statistics
	{
		Common::WriteStream &ws = add(ID_SCVM);

		// Write out the save date/time
		TimeDate td;
		g_system->getTimeAndDate(td);
		ws.writeSint16BE(td.tm_year + 1900);
		ws.writeSint16BE(td.tm_mon + 1);
		ws.writeSint16BE(td.tm_mday);
		ws.writeSint16BE(td.tm_hour);
		ws.writeSint16BE(td.tm_min);
		ws.writeUint32BE(g_vm->_events->getTotalPlayTicks());

		// Write out intrepreter type, language, and game Id
		ws.writeUint32BE(getInterpreterTag(g_vm->getInterpreterType()));
		const char *langCode = getLanguageCode(g_vm->getLanguage());
		if (langCode)
			ws.write(langCode, strlen(langCode) + 1);
		else
			ws.writeByte('\0');

		Common::String md5 = g_vm->getGameMD5();
		ws.write(md5.c_str(), md5.size());
		ws.writeByte('\0');
	}
}

} // End of namespace Glk
