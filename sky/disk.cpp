/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/engine.h"
#include "common/file.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/rnc_deco.h"

#define no_of_files_hd	1600
#define no_of_files_cd	5200
#define max_files_in_list		60

const char *data_file_name = "sky.dsk";
const char *dinner_file_name = "sky.dnr";
uint8 *dinner_table_area, *fixed_dest, *file_dest, *comp_dest;
uint32 dinner_table_entries, file_flags, file_offset, file_size, decomp_size, comp_file;
uint16 build_list[max_files_in_list];
uint32 loaded_file_list[max_files_in_list];

File *data_disk_handle = new File();
File *dnr_handle = new File();

void SkyState::initialise_disk()
{
	uint32 entries_read;

	dnr_handle->open(dinner_file_name, _gameDataPath);
	if (dnr_handle->isOpen() == false)
			error("Could not open %s%s!\n", _gameDataPath, dinner_file_name);

	if (!(dinner_table_entries = dnr_handle->readUint32LE()))
		error("Error reading from sky.dnr!\n"); //even though it was opened correctly?!
	
	debug(1, "Entries in dinner table: %d", dinner_table_entries);

	if (dinner_table_entries > 1600) 
		_isCDVersion = true;
	else
		_isCDVersion = false;
		
	dinner_table_area = (uint8 *)malloc(dinner_table_entries * 8);
	entries_read = dnr_handle->read(dinner_table_area, 8 * dinner_table_entries) / 8;

	if (entries_read != dinner_table_entries)
		warning("bytes_read != dinner_table_entries. [%d/%d]\n", entries_read, dinner_table_entries);

	data_disk_handle->open(data_file_name, _gameDataPath);
	if (data_disk_handle->isOpen() == false) 
		error("Error opening %s%s!\n", _gameDataPath, data_file_name);
}

//load in file file_nr to address dest
//if dest == NULL, then allocate memory for this file
uint16 *SkyState::load_file(uint16 file_nr, uint8 *dest)
{
	uint8 cflag;
	int32 bytes_read;
	uint8 *file_ptr, *inputPtr, *outputPtr;
	dataFileHeader file_header;

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	comp_file = file_nr;
	debug(1, "load file %d,%d (%d)", (file_nr >> 11), (file_nr & 2047), file_nr); 

	file_ptr = (uint8 *)get_file_info(file_nr);
	if (file_ptr == NULL) {
		printf("File %d not found!\n", file_nr);
		return NULL;
	}

	file_flags = READ_LE_UINT32((file_ptr + 5));
	file_size = file_flags & 0x03fffff;

	file_offset = READ_LE_UINT32((file_ptr + 2)) & 0x0ffffff;

	cflag = (uint8)((file_offset >> (23)) & 0x1);
	file_offset = (((1 << (23)) ^ 0xFFFFFFFF) & file_offset);

	if (cflag)
		file_offset <<= 4;

	fixed_dest = dest;
	file_dest = dest;
	comp_dest = dest;

	if (dest == NULL) //we need to allocate memory for this file
		file_dest = (uint8 *)malloc(file_size);

	data_disk_handle->seek(file_offset, SEEK_SET);

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	//now read in the data
	bytes_read = data_disk_handle->read(file_dest, 1*file_size);

	if (bytes_read != (int32)file_size)
		printf("ERROR: Unable to read %d bytes from datadisk (%d bytes read)\n", file_size, bytes_read);

	cflag = (uint8)((file_flags >> (23)) & 0x1);

	//if cflag == 0 then file is compressed, 1 == uncompressed

	if (!cflag) {
		debug(1, "File is compressed...");

		memcpy(&file_header, file_dest, sizeof(struct dataFileHeader));
		if ( (uint8)((FROM_LE_16(file_header.flag) >> 7) & 0x1)	 ) {
			debug(1, "with RNC!");

			decomp_size = (FROM_LE_16(file_header.flag) & 0xFFFFFF00) << 8;
			decomp_size |= FROM_LE_16((uint16)file_header.s_tot_size);

			if (fixed_dest == NULL) // is this valid?
				comp_dest = (uint8 *)malloc(decomp_size);

			inputPtr = file_dest;
			outputPtr = comp_dest;

			if ( (uint8)(file_flags >> (22) & 0x1) ) //do we include the header?
				inputPtr += sizeof(struct dataFileHeader);
			else {
				memcpy(outputPtr, inputPtr, sizeof(struct dataFileHeader));
				inputPtr += sizeof(struct dataFileHeader);
				outputPtr += sizeof(struct dataFileHeader);
			}

			RncDecoder rncDecoder;
			int32 unPackLen = rncDecoder.unpackM1(inputPtr, outputPtr, 0);

			debug(2, "UnpackM1 returned: %d", unPackLen);

			if (unPackLen == 0) { //Unpack returned 0: file was probably not packed.
				if (fixed_dest == NULL)
					free(comp_dest);
			
				return (uint16 *)file_dest;
			}

			if (! (uint8)(file_flags >> (22) & 0x1) ) { // include header?
				unPackLen += sizeof(struct dataFileHeader);

				if (unPackLen != (int32)decomp_size) {
					debug(1, "ERROR: invalid decomp size! (was: %d, should be: %d)", unPackLen, decomp_size);
				}
			}

			if (fixed_dest == NULL)
				free(file_dest);

		} else
			debug(1, "but not with RNC! (?!)");
	} else
		return (uint16 *)file_dest;

	return (uint16 *)comp_dest;
}

uint16 *SkyState::get_file_info(uint16 file_nr)
{
	uint16 i;
	uint16 *dnr_tbl_16_ptr = (uint16 *)dinner_table_area;

	for (i = 0; i < dinner_table_entries; i++) {
		if (READ_LE_UINT16(dnr_tbl_16_ptr + (i * 4)) == file_nr) {
			debug(1, "file %d found!", file_nr);
			return (dnr_tbl_16_ptr + (i * 4));
		}
	}

	// if file is speech file then return NULL if not found
	printf("get_file_info() - speech file support not implemented yet!\n");
	return (uint16 *)NULL;
}

