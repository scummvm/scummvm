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

#include "awe/serializer.h"
#include "awe/util.h"

namespace Awe {

Serializer::Serializer(Common::WriteStream *stream, uint8 *ptrBlock, uint16 saveVer)
	: _stream(stream), _mode(SM_SAVE), _ptrBlock(ptrBlock), _saveVer(saveVer) {
}

Serializer::Serializer(Common::SeekableReadStream *stream, uint8 *ptrBlock, uint16 saveVer)
	: _stream(stream), _mode(SM_LOAD), _ptrBlock(ptrBlock), _saveVer(saveVer) {
}

void Serializer::saveOrLoadEntries(Entry *entry) {
	switch (_mode) {
	case SM_SAVE:
		saveEntries(entry);
		break;
	case SM_LOAD:
		loadEntries(entry);
		break;	
	}
}

void Serializer::saveEntries(Entry *entry) {
	Common::WriteStream *stream =
		dynamic_cast<Common::WriteStream *>(_stream);

	for (; entry->type != SET_END; ++entry) {
		if (entry->maxVer == CUR_VER) {
			switch (entry->type) {
			case SET_INT:
				saveInt(entry->size, entry->data);
				break;
			case SET_ARRAY:
				if (entry->size == Serializer::SES_INT8) {
					stream->write(entry->data, entry->n);
				} else {
					uint8 *p = (uint8 *)entry->data;
					for (int i = 0; i < entry->n; ++i) {
						saveInt(entry->size, p);
						p += entry->size;
					}
				}
				break;
			case SET_PTR:
				stream->writeUint32BE(*(uint8 **)(entry->data) - _ptrBlock);
				break;
			case SET_END:
				break;
			}
		}
	}
}

void Serializer::loadEntries(Entry *entry) {
	Common::SeekableReadStream *stream =
		dynamic_cast<Common::SeekableReadStream *>(_stream);

	for (; entry->type != SET_END; ++entry) {
		if (_saveVer >= entry->minVer && _saveVer <= entry->maxVer) {
			switch (entry->type) {
			case SET_INT:
				loadInt(entry->size, entry->data);
				break;
			case SET_ARRAY:
				if (entry->size == Serializer::SES_INT8) {
					stream->read(entry->data, entry->n);
				} else {
					uint8 *p = (uint8 *)entry->data;
					for (int i = 0; i < entry->n; ++i) {
						loadInt(entry->size, p);
						p += entry->size;
					}
				}
				break;
			case SET_PTR:
				*(uint8 **)(entry->data) = _ptrBlock + stream->readUint32BE();
				break;
			case SET_END:
				break;				
			}
		}
	}
}

void Serializer::saveInt(uint8 es, void *p) {
	Common::WriteStream *stream =
		dynamic_cast<Common::WriteStream *>(_stream);

	switch (es) {
	case SES_INT8:
		stream->writeByte(*(uint8 *)p);
		break;
	case SES_INT16:
		stream->writeUint16BE(*(uint16 *)p);
		break;
	case SES_INT32:
		stream->writeUint32BE(*(uint32 *)p);
		break;
	}
}

void Serializer::loadInt(uint8 es, void *p) {
	Common::SeekableReadStream *stream =
		dynamic_cast<Common::SeekableReadStream *>(_stream);

	switch (es) {
	case SES_INT8:
		*(uint8 *)p = stream->readByte();
		break;
	case SES_INT16:
		*(uint16 *)p = stream->readUint16BE();
		break;
	case SES_INT32:
		*(uint32 *)p = stream->readUint32BE();
		break;
	}
}

} // namespace Awe
