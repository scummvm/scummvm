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

#include "gfx_mod.h"
#include "text_mod.h"
#include "script_mod.h"
#include "script.h"
#include "sdata.h"

namespace Saga {

int SDATA_Init() {
	unsigned int i;
	void *alloc_ptr;

	debug(0, "Initializing script data buffers.");
	for (i = 0; i < R_SCRIPT_DATABUF_NUM; i++) {
		alloc_ptr = malloc(sizeof *ScriptModule.data_buf[0]);
		if (alloc_ptr == NULL) {
			warning("Error allocating memory for script data buffer %d", i);
			return R_MEM;
		}

		ScriptModule.data_buf[i] = (R_SCRIPT_DATABUF *)alloc_ptr;
		alloc_ptr = calloc(R_SCRIPT_DATABUF_LEN, sizeof(SDataWord_T));

		if (alloc_ptr == NULL) {
			warning("Error allocating memory for script data buffer %d", i);
			return R_MEM;
		}

		ScriptModule.data_buf[i]->len = R_SCRIPT_DATABUF_LEN;
		ScriptModule.data_buf[i]->data = (SDataWord_T *)alloc_ptr;
	}

	return R_SUCCESS;
}

int SDATA_GetWord(int n_buf, int n_word, SDataWord_T * data) {
	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if ((n_word < 0) || (n_word >= ScriptModule.data_buf[n_buf]->len)) {
		return R_FAILURE;
	}

	if (data == NULL) {
		return R_FAILURE;
	}

	*data = ScriptModule.data_buf[n_buf]->data[n_word];

	return R_SUCCESS;
}

int SDATA_PutWord(int n_buf, int n_word, SDataWord_T data) {
	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if ((n_word < 0) || (n_word >= ScriptModule.data_buf[n_buf]->len)) {
		return R_FAILURE;
	}

	ScriptModule.data_buf[n_buf]->data[n_word] = data;

	return R_SUCCESS;
}

int SDATA_SetBit(int n_buf, SDataWord_T n_bit, int bitstate) {
	int n_word;
	int n_bitpos;

	SDataWord_T bit_pattern = 0x01;

	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if (n_bit >= (unsigned long)ScriptModule.data_buf[n_buf]->len * (sizeof(SDataWord_T) * CHAR_BIT)) {
		return R_FAILURE;
	}

	n_word = n_bit / (sizeof(SDataWord_T) * CHAR_BIT);
	n_bitpos = n_bit % (sizeof(SDataWord_T) * CHAR_BIT);

	bit_pattern <<= ((sizeof(SDataWord_T) * CHAR_BIT) - (n_bitpos + 1));

	if (bitstate) {
		ScriptModule.data_buf[n_buf]->data[n_word] |= bit_pattern;
	} else {
		ScriptModule.data_buf[n_buf]->data[n_word] &= ~bit_pattern;
	}

	return R_SUCCESS;
}

int SDATA_GetBit(int n_buf, SDataWord_T n_bit, int *bitstate) {
	int n_word;
	int n_bitpos;

	SDataWord_T bit_pattern = 0x01;

	if ((n_buf < 0) || (n_buf >= R_SCRIPT_DATABUF_NUM)) {
		return R_FAILURE;
	}

	if (n_bit >= (SDataWord_T) ScriptModule.data_buf[n_buf]->len * (sizeof(SDataWord_T) * CHAR_BIT)) {
		return R_FAILURE;
	}

	n_word = n_bit / (sizeof(SDataWord_T) * CHAR_BIT);
	n_bitpos = n_bit % (sizeof(SDataWord_T) * CHAR_BIT);

	bit_pattern <<= ((sizeof(SDataWord_T) * CHAR_BIT) - (n_bitpos + 1));

	*bitstate = (ScriptModule.data_buf[n_buf]->data[n_word] & bit_pattern) ? 1 : 0;

	return R_SUCCESS;
}

int SDATA_ReadWordS(SDataWord_T word) {
	uint16 u_int = word;
	int s_int;

	if (u_int & 0x8000U) {
		s_int = (int)(u_int - 0x8000U) - 0x7FFF - 1;
	} else {
		s_int = u_int;
	}

	return s_int;
}

uint16 SDATA_ReadWordU(SDataWord_T word) {
	uint16 u_int = (uint16) word;

	return u_int;
}

} // End of namespace Saga
