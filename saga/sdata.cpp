/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
// Type SDataWord_T must be unpadded
#include "saga.h"

#include "gfx.h"
#include "text_mod.h"
#include "script_mod.h"
#include "script.h"
#include "sdata.h"

namespace Saga {

SData::SData() {
	unsigned int i;
	void *alloc_ptr;

	debug(0, "Initializing script data buffers");
	for (i = 0; i < R_SCRIPT_DATABUF_NUM; i++) {
		alloc_ptr = malloc(sizeof *_vm->_script->_data_buf[0]);
		if (alloc_ptr == NULL) {
			error("Couldn't allocate memory for script data buffer %d", i);
		}

		_vm->_script->_data_buf[i] = (R_SCRIPT_DATABUF *)alloc_ptr;
		alloc_ptr = calloc(R_SCRIPT_DATABUF_LEN, sizeof(SDataWord_T));

		if (alloc_ptr == NULL) {
			error("Couldn't allocate memory for script data buffer %d", i);
		}

		_vm->_script->_data_buf[i]->len = R_SCRIPT_DATABUF_LEN;
		_vm->_script->_data_buf[i]->data = (SDataWord_T *)alloc_ptr;
	}
}

SData::~SData() {
}

int SData::getWord(int n_buf, int n_word, SDataWord_T *data) {
	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if ((n_word < 0) || (n_word >= _vm->_script->_data_buf[n_buf]->len)) {
		return R_FAILURE;
	}

	if (data == NULL) {
		return R_FAILURE;
	}

	*data = _vm->_script->_data_buf[n_buf]->data[n_word];

	return R_SUCCESS;
}

int SData::putWord(int n_buf, int n_word, SDataWord_T data) {
	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if ((n_word < 0) || (n_word >= _vm->_script->_data_buf[n_buf]->len)) {
		return R_FAILURE;
	}

	_vm->_script->_data_buf[n_buf]->data[n_word] = data;

	return R_SUCCESS;
}

int SData::setBit(int n_buf, SDataWord_T n_bit, int bitstate) {
	int n_word;
	int n_bitpos;

	SDataWord_T bit_pattern = 0x01;

	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if (n_bit >= (unsigned long)_vm->_script->_data_buf[n_buf]->len * (sizeof(SDataWord_T) * CHAR_BIT)) {
		return R_FAILURE;
	}

	n_word = n_bit / (sizeof(SDataWord_T) * CHAR_BIT);
	n_bitpos = n_bit % (sizeof(SDataWord_T) * CHAR_BIT);

	bit_pattern <<= ((sizeof(SDataWord_T) * CHAR_BIT) - (n_bitpos + 1));

	if (bitstate) {
		_vm->_script->_data_buf[n_buf]->data[n_word] |= bit_pattern;
	} else {
		_vm->_script->_data_buf[n_buf]->data[n_word] &= ~bit_pattern;
	}

	return R_SUCCESS;
}

int SData::getBit(int n_buf, SDataWord_T n_bit, int *bitstate) {
	int n_word;
	int n_bitpos;

	SDataWord_T bit_pattern = 0x01;

	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if (n_bit >= (SDataWord_T) _vm->_script->_data_buf[n_buf]->len * (sizeof(SDataWord_T) * CHAR_BIT)) {
		return R_FAILURE;
	}

	n_word = n_bit / (sizeof(SDataWord_T) * CHAR_BIT);
	n_bitpos = n_bit % (sizeof(SDataWord_T) * CHAR_BIT);

	bit_pattern <<= ((sizeof(SDataWord_T) * CHAR_BIT) - (n_bitpos + 1));

	*bitstate = (_vm->_script->_data_buf[n_buf]->data[n_word] & bit_pattern) ? 1 : 0;

	return R_SUCCESS;
}

int SData::readWordS(SDataWord_T word) {
	uint16 u_int = word;
	int s_int;

	if (u_int & 0x8000U) {
		s_int = (int)(u_int - 0x8000U) - 0x7FFF - 1;
	} else {
		s_int = u_int;
	}

	return s_int;
}

uint16 SData::readWordU(SDataWord_T word) {
	uint16 u_int = (uint16) word;

	return u_int;
}

} // End of namespace Saga
