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

#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {

byte Magnetic::init_gfx1(size_t size) {
	if (!(gfx_buf = new byte[MAX_PICTURE_SIZE]))
		return 1;

	if (!(gfx_data = new byte[size])) {
		delete[] gfx_buf;
		gfx_buf = nullptr;
		return 1;
	}

	if (_gfxFile.read(gfx_data, size) != size) {
		delete[] gfx_data;
		delete[] gfx_buf;
		gfx_data = gfx_buf = nullptr;
		return 1;
	}

	gfx_ver = 1;
	return 2;
}

byte Magnetic::init_gfx2(size_t size) {
	if (!(gfx_buf = new byte[MAX_PICTURE_SIZE])) {
		return 1;
	}

	gfx2_hsize = size;
	if (!(gfx2_hdr = new byte[gfx2_hsize])) {
		delete[] gfx_buf;
		gfx_buf = nullptr;
		return 1;
	}

	if (_gfxFile.read(gfx2_hdr, gfx2_hsize) != gfx2_hsize) {
		delete[] gfx_buf;
		delete[] gfx2_hdr;
		gfx_buf = nullptr;
		gfx2_hdr = nullptr;
		return 1;
	}

	gfx_ver = 2;
	return 2;
}

void Magnetic::ms_showpic(int c, byte mode) {
	// TODO
}

bool Magnetic::is_blank(uint16 line, uint16 width) const {
	int i;

	for (i = line * width; i < (line + 1) * width; i++)
		if (gfx_buf[i])
			return false;
	return true;
}

byte *Magnetic::ms_extract1(byte pic, uint16 * w, uint16 * h, uint16 * pal) {
	byte *table, *data, bit, val, *buffer;
	uint16 tablesize, count;
	uint32 i, j, upsize, offset;
	//uint32 datasize;

	offset = READ_LE_UINT32(gfx_data + 4 * pic);
	buffer = gfx_data + offset - 8;

	for (i = 0; i < 16; i++)
		pal[i] = READ_LE_UINT16(buffer + 0x1c + 2 * i);
	w[0] = (uint16)(READ_LE_UINT16(buffer + 4) - READ_LE_UINT16(buffer + 2));
	h[0] = READ_LE_UINT16(buffer + 6);

	tablesize = READ_LE_UINT16(buffer + 0x3c);
	//datasize = READ_LE_UINT32(buffer + 0x3e);
	table = buffer + 0x42;
	data = table + tablesize * 2 + 2;
	upsize = h[0] * w[0];

	for (i = 0, j = 0, count = 0, val = 0, bit = 7; i < upsize; i++, count--) {
		if (!count) {
			count = tablesize;
			while (count < 0x80) {
				if (data[j] & (1 << bit))
					count = table[2 * count];
				else
					count = table[2 * count + 1];
				if (!bit)
					j++;
				bit = (byte)(bit ? bit - 1 : 7);
			}
			count &= 0x7f;
			if (count >= 0x10)
				count -= 0x10;
			else
			{
				val = (byte)count;
				count = 1;
			}
		}
		gfx_buf[i] = val;
	}
	for (j = w[0]; j < upsize; j++)
		gfx_buf[j] ^= gfx_buf[j - w[0]];

	for (; h[0] > 0 && is_blank((uint16)(h[0] - 1), w[0]); h[0]--);
	for (i = 0; h[0] > 0 && is_blank((uint16)i, w[0]); h[0]--, i++);
	return gfx_buf + i * w[0];
}


byte *Magnetic::ms_extract2(const char *name, uint16 *w, uint16 *h, uint16 *pal, byte *is_anim) {
	struct picture main_pic;
	uint32 offset = 0, length = 0, i;
	int16 header_pos = -1;
	byte* anim_data;
	uint32 j;

	if (is_anim != 0)
		*is_anim = 0;
	gfx2_name = name;

	pos_table_size = 0;

	// Find the uppercase (no animation) version of the picture first
	header_pos = find_name_in_header(name, 1);

	if (header_pos < 0)
		header_pos = find_name_in_header(name, 0);
	if (header_pos < 0)
		return 0;

	offset = READ_LE_UINT32(gfx2_hdr + header_pos + 8);
	length = READ_LE_UINT32(gfx2_hdr + header_pos + 12);

	if (offset != 0) {
		if (gfx2_buf) {
			delete[] gfx2_buf;
			gfx2_buf = nullptr;
		}

		gfx2_buf = new byte[length];
		if (!gfx2_buf)
			return 0;

		if (!_gfxFile.seek(offset) || _gfxFile.read(gfx2_buf, length) != length) {
			delete[] gfx2_buf;
			gfx2_buf = nullptr;
			return 0;
		}

		for (i = 0; i < 16; i++)
			pal[i] = READ_LE_UINT16(gfx2_buf + 4 + (2 * i));

		main_pic.data = gfx2_buf + 48;
		main_pic.data_size = READ_LE_UINT32(gfx2_buf + 38);
		main_pic.width = READ_LE_UINT16(gfx2_buf + 42);
		main_pic.height = READ_LE_UINT16(gfx2_buf + 44);
		main_pic.wbytes = (uint16)(main_pic.data_size / main_pic.height);
		main_pic.plane_step = (uint16)(main_pic.wbytes / 4);
		main_pic.mask = (byte*)0;
		extract_frame(&main_pic);

		*w = main_pic.width;
		*h = main_pic.height;

		// Check for an animation
		anim_data = gfx2_buf + 48 + main_pic.data_size;
		if ((anim_data[0] != 0xD0) || (anim_data[1] != 0x5E)) {
			byte *current;
			uint16 frame_count;
			uint16 value1, value2;
			//uint16 command_count;

			if (is_anim != 0)
				*is_anim = 1;

			current = anim_data + 6;
			frame_count = READ_LE_UINT16(anim_data + 2);
			if (frame_count > MAX_ANIMS)
			{
				error("animation frame array too short");
				return 0;
			}

			/* Loop through each animation frame */
			for (i = 0; i < frame_count; i++)
			{
				anim_frame_table[i].data = current + 10;
				anim_frame_table[i].data_size = READ_LE_UINT32(current);
				anim_frame_table[i].width = READ_LE_UINT16(current + 4);
				anim_frame_table[i].height = READ_LE_UINT16(current + 6);
				anim_frame_table[i].wbytes = (uint16)(anim_frame_table[i].data_size / anim_frame_table[i].height);
				anim_frame_table[i].plane_step = (uint16)(anim_frame_table[i].wbytes / 4);
				anim_frame_table[i].mask = (byte*)0;

				current += anim_frame_table[i].data_size + 12;
				value1 = READ_LE_UINT16(current - 2);
				value2 = READ_LE_UINT16(current);

				/* Get the mask */
				if ((value1 == anim_frame_table[i].width) && (value2 == anim_frame_table[i].height))
				{
					uint16 skip;

					anim_frame_table[i].mask = (byte*)(current + 4);
					skip = READ_LE_UINT16(current + 2);
					current += skip + 6;
				}
			}

			/* Get the positioning tables */
			pos_table_size = READ_LE_UINT16(current - 2);
			if (pos_table_size > MAX_POSITIONS)
			{
				error("animation position array too short");
				return 0;
			}

			for (i = 0; i < pos_table_size; i++) {
				pos_table_count[i] = READ_LE_UINT16(current + 2);
				current += 4;

				if (pos_table_count[i] > MAX_ANIMS)
				{
					error("animation position array too short");
					return 0;
				}

				for (j = 0; j < pos_table_count[i]; j++)
				{
					pos_table[i][j].x = READ_LE_UINT16(current);
					pos_table[i][j].y = READ_LE_UINT16(current + 2);
					pos_table[i][j].number = READ_LE_UINT16(current + 4) - 1;
					current += 8;
				}
			}

			// Get the command sequence table
			//command_count = READ_LE_UINT16(current);
			command_table = current + 2;

			for (i = 0; i < MAX_POSITIONS; i++)
			{
				anim_table[i].flag = -1;
				anim_table[i].count = -1;
			}
			command_index = 0;
			anim_repeat = 0;
			pos_table_index = -1;
			pos_table_max = -1;
		}

		return gfx_buf;
	}

	return nullptr;
}

int16 Magnetic::find_name_in_header(const Common::String &name, bool upper) {
	int16 header_pos = 0;
	Common::String pic_name(name.c_str(), name.c_str() + 6);

	if (upper)
		pic_name.toUppercase();

	while (header_pos < gfx2_hsize) {
		const char *hname = (const char *)(gfx2_hdr + header_pos);
		if (strncmp(hname, pic_name.c_str(), 6) == 0)
			return header_pos;
		header_pos += 16;
	}

	return -1;
}

void Magnetic::extract_frame(const picture *pic) {
	uint32 i, x, y, bit_x, mask, ywb, yw, value, values[4];
	values[0] = values[1] = values[2] = values[3] = 0;

	if (pic->width * pic->height > MAX_PICTURE_SIZE) {
		error("picture too large");
		return;
	}

	for (y = 0; y < pic->height; y++) {
		ywb = y * pic->wbytes;
		yw = y * pic->width;

		for (x = 0; x < pic->width; x++) {
			if ((x % 8) == 0) {
				for (i = 0; i < 4; i++)
					values[i] = pic->data[ywb + (x / 8) + (pic->plane_step * i)];
			}

			bit_x = 7 - (x & 7);
			mask = 1 << bit_x;
			value = ((values[0] & mask) >> bit_x) << 0 |
				((values[1] & mask) >> bit_x) << 1 |
				((values[2] & mask) >> bit_x) << 2 |
				((values[3] & mask) >> bit_x) << 3;
			value &= 15;

			gfx_buf[yw + x] = (byte)value;
		}
	}
}

byte *Magnetic::ms_extract(uint32 pic, uint16 *w, uint16 *h, uint16 *pal, byte *is_anim) {
	if (is_anim)
		*is_anim = 0;

	if (gfx_buf) {
		switch (gfx_ver) {
		case 1:
			return ms_extract1((byte)pic, w, h, pal);
		case 2:
			return ms_extract2((const char *)(code + pic), w, h, pal, is_anim);
		}
	}

	return nullptr;
}

byte Magnetic::ms_animate(ms_position **positions, uint16 *count) {
	byte got_anim = 0;
	uint16 i, j, ttable;

	if ((gfx_buf == 0) || (gfx2_buf == 0) || (gfx_ver != 2))
		return 0;
	if ((pos_table_size == 0) || (command_index < 0))
		return 0;

	*count = 0;
	*positions = (struct ms_position*)0;

	while (got_anim == 0)
	{
		if (pos_table_max >= 0)
		{
			if (pos_table_index < pos_table_max)
			{
				for (i = 0; i < pos_table_size; i++)
				{
					if (anim_table[i].flag > -1)
					{
						if (*count >= MAX_FRAMES)
						{
							error("returned animation array too short");
							return 0;
						}

						pos_array[*count] = pos_table[i][anim_table[i].flag];

						(*count)++;

						if (anim_table[i].flag < (pos_table_count[i] - 1))
							anim_table[i].flag++;
						if (anim_table[i].count > 0)
							anim_table[i].count--;
						else
							anim_table[i].flag = -1;
					}
				}
				if (*count > 0)
				{
					*positions = pos_array;
					got_anim = 1;
				}
				pos_table_index++;
			}
		}

		if (got_anim == 0)
		{
			byte command = command_table[command_index];
			command_index++;

			pos_table_max = -1;
			pos_table_index = -1;

			switch (command)
			{
			case 0x00:
				command_index = -1;
				return 0;
			case 0x01:
				ttable = command_table[command_index];
				command_index++;

				if (ttable - 1 >= MAX_POSITIONS)
				{
					error("animation table too short");
					return 0;
				}

				anim_table[ttable - 1].flag = (int16)(command_table[command_index] - 1);
				command_index++;
				anim_table[ttable - 1].count = (int16)(command_table[command_index] - 1);
				command_index++;

				/* Workaround for Wonderland "catter" animation */
				if (v4_id == 0)
				{
					if (gfx2_name == "catter") {
						if (command_index == 96)
							anim_table[ttable - 1].count = 9;
						if (command_index == 108)
							anim_table[ttable - 1].flag = -1;
						if (command_index == 156)
							anim_table[ttable - 1].flag = -1;
					}
				}
				break;
			case 0x02:
				pos_table_max = command_table[command_index];
				pos_table_index = 0;
				command_index++;
				break;
			case 0x03:
				if (v4_id == 0)
				{
					command_index = -1;
					return 0;
				}
				else
				{
					command_index = 0;
					anim_repeat = 1;
					pos_table_index = -1;
					pos_table_max = -1;
					for (j = 0; j < MAX_POSITIONS; j++)
					{
						anim_table[j].flag = -1;
						anim_table[j].count = -1;
					}
				}
				break;

			case 0x04:
				command_index += 3;
				return 0;
			case 0x05:
				ttable = next_table;
				command_index++;

				anim_table[ttable - 1].flag = 0;
				anim_table[ttable - 1].count = command_table[command_index];

				pos_table_max = command_table[command_index];
				pos_table_index = 0;
				command_index++;
				command_index++;
				next_table++;
				break;
			default:
				error("unknown animation command");
				command_index = -1;
				return 0;
			}
		}
	}

	return got_anim;
}

byte *Magnetic::ms_get_anim_frame(int16 number, uint16 *width, uint16 *height, byte **mask) {
	if (number >= 0)
	{
		extract_frame(anim_frame_table + number);
		*width = anim_frame_table[number].width;
		*height = anim_frame_table[number].height;
		*mask = anim_frame_table[number].mask;
		return gfx_buf;
	}

	return nullptr;
}

} // End of namespace Magnetic
} // End of namespace Glk
