/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "stdafx.h"

#include "serializer.h"
#include "file.h"
#include "util.h"

namespace Awe {

Serializer::Serializer(File *stream, Mode mode, uint8 *ptrBlock, uint16 saveVer)
	: _stream(stream), _mode(mode), _ptrBlock(ptrBlock), _saveVer(saveVer) {
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
	for (; entry->type != SET_END; ++entry) {
		if (entry->maxVer == CUR_VER) {
			switch (entry->type) {
			case SET_INT:
				saveInt(entry->size, entry->data);
				break;
			case SET_ARRAY:
				if (entry->size == Serializer::SES_INT8) {
					_stream->write(entry->data, entry->n);
				} else {
					uint8 *p = (uint8 *)entry->data;
					for (int i = 0; i < entry->n; ++i) {
						saveInt(entry->size, p);
						p += entry->size;
					}
				}
				break;
			case SET_PTR:
				_stream->writeUint32BE(*(uint8 **)(entry->data) - _ptrBlock);
				break;
			case SET_END:
				break;
			}
		}
	}
}

void Serializer::loadEntries(Entry *entry) {
	for (; entry->type != SET_END; ++entry) {
		if (_saveVer >= entry->minVer && _saveVer <= entry->maxVer) {
			switch (entry->type) {
			case SET_INT:
				loadInt(entry->size, entry->data);
				break;
			case SET_ARRAY:
				if (entry->size == Serializer::SES_INT8) {
					_stream->read(entry->data, entry->n);
				} else {
					uint8 *p = (uint8 *)entry->data;
					for (int i = 0; i < entry->n; ++i) {
						loadInt(entry->size, p);
						p += entry->size;
					}
				}
				break;
			case SET_PTR:
				*(uint8 **)(entry->data) = _ptrBlock + _stream->readUint32BE();
				break;
			case SET_END:
				break;				
			}
		}
	}
}

void Serializer::saveInt(uint8 es, void *p) {
	switch (es) {
	case SES_INT8:
		_stream->writeByte(*(uint8 *)p);
		break;
	case SES_INT16:
		_stream->writeUint16BE(*(uint16 *)p);
		break;
	case SES_INT32:
		_stream->writeUint32BE(*(uint32 *)p);
		break;
	}
}

void Serializer::loadInt(uint8 es, void *p) {
	switch (es) {
	case SES_INT8:
		*(uint8 *)p = _stream->readByte();
		break;
	case SES_INT16:
		*(uint16 *)p = _stream->readUint16BE();
		break;
	case SES_INT32:
		*(uint32 *)p = _stream->readUint32BE();
		break;
	}
}

}
