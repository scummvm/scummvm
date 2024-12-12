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

#include "got/game/image.h"
#include "got/events.h"
#include "got/utils/file.h"
#include "got/got.h"
#include "got/vars.h"

namespace Got {

void xcopys2d(int SourceStartX, int SourceStartY,
		int SourceEndX, int SourceEndY, int DestStartX,
		int DestStartY, byte *SourcePtr, unsigned int DestPageBase,
		int SourceBitmapWidth, int DestBitmapWidth) {
	error("TODO: Refactor this to use surfaces");
}


uint make_mask(MASK_IMAGE *new_image, uint page_start, byte *Image,
		int image_width, int image_height) {
	uint page_offset, size;
	int align, set;
	ALIGNED_MASK_IMAGE *work_ami;
	int scan_line, bit_num, temp_image_width;
	unsigned char mask_temp;
	byte *new_mask_ptr;
	byte *old_mask_ptr;

	page_offset = page_start;
	set = 0;

	for (align = 0; align < 3; align += 2) {
		work_ami = new_image->alignments[set++] = (ALIGNED_MASK_IMAGE *)_G(ami_buff);
		_G(ami_buff) += sizeof(ALIGNED_MASK_IMAGE);

		work_ami->image_width = (image_width + align + 3) / 4;
		work_ami->image_ptr = page_offset;	// Image dest

		// Download this alignment of the image
		xcopys2d(0, 0, image_width, image_height, align, 0,
			Image, page_offset, image_width,
			work_ami->image_width * 4);

		// Calculate the number of bytes needed to store the mask in
		// nibble (Map Mask-ready) form, then allocate that space */
		size = work_ami->image_width * image_height;
		work_ami->mask_ptr = (byte *)_G(mask_buff);
		_G(mask_buff) += size;

		/* Generate this nibble oriented (Map Mask-ready) alignment of
		   the mask, one scan line at a time */
		old_mask_ptr = Image;
		new_mask_ptr = work_ami->mask_ptr;
		for (scan_line = 0; scan_line < image_height; scan_line++) {
			bit_num = align;
			mask_temp = 0;
			temp_image_width = image_width;
			do {
				// Set the mask bit for next pixel according to its alignment
				mask_temp |= (*old_mask_ptr != 15 && *old_mask_ptr != 0) << bit_num;
				old_mask_ptr++;
				if (++bit_num > 3) {
					*new_mask_ptr = mask_temp;
					new_mask_ptr++;
					mask_temp = bit_num = 0;
				}
			} while (--temp_image_width);


			// Set any partial final mask on this scan line
			if (bit_num != 0) {
				*new_mask_ptr = mask_temp;
				new_mask_ptr++;
			}
		}
		page_offset += size; // Mark off the space we just used
	}
	return page_offset - page_start;
}

void setup_actor(ACTOR *actr, char num, char dir, int x, int y) {

	actr->next = 0;                    // Next frame to be shown
	actr->frame_count = actr->frame_speed;
	actr->dir = dir;                   // Direction of travel
	actr->last_dir = dir;              // Last direction of travel
	if (actr->directions == 1) actr->dir = 0;
	if (actr->directions == 2) actr->dir &= 1;
	if (actr->directions == 4 && actr->frames == 1) {
		actr->dir = 0;
		actr->next = dir;
	}

	actr->x = x;                       // Actual X coor
	actr->y = y;                       // Actual Y coor
	actr->width = 16;                  // Actual X coor
	actr->height = 16;                 // Actual Y coor
	actr->center = 0;                  // Center of object
	actr->last_x[0] = x;               // Last X coor on each page
	actr->last_x[1] = x;
	actr->last_y[0] = y;               // Last Y coor on each page
	actr->last_y[1] = y;
	actr->used = 1;                    // 1=active, 0=not active
	actr->speed_count = 8;             // Count down to movement
	actr->vunerable = STAMINA;         // Count down to vunerability
	actr->shot_cnt = 20;               // Count down to another shot
	actr->num_shots = 0;               // # of shots currently on screen
	actr->creator = 0;                 // which actor # created this actor
	actr->pause = 0;                   // Pause must be 0 to move
	actr->show = 0;
	actr->actor_num = num;
	actr->counter = 0;
	actr->move_counter = 0;
	actr->edge_counter = 20;
	actr->hit_thor = 0;
	actr->rand = g_engine->getRandomNumber(99);
	actr->temp1 = 0;
	actr->init_health = actr->health;
}

void make_actor_mask(ACTOR *actr) {
	int d, f;

	for (d = 0; d < actr->directions; d++) {
		for (f = 0; f < actr->frames; f++) {
			make_mask(&actr->pic[d][f], _G(latch_mem), &_G(tmp_buff)[256 * ((d * 4) + f)], 16, 16);
			_G(latch_mem) += 144;
			if (_G(latch_mem) > 65421u) {
				error("Too Many Actor Frames=");
			}
		}
	}
}

int load_standard_actors() {
	_G(latch_mem) = 50160u;
	_G(mask_buff) = _G(mask_buff_start);
	_G(ami_buff) = _G(abuff);

	load_actor(0, 100 + _G(thor_info).armor);   // Load thor
	memcpy(&_G(actor)[0], (_G(tmp_buff) + 5120), 40);
	setup_actor(&_G(actor)[0], 0, 0, 100, 100);
	_G(thor) = &_G(actor)[0];

	_G(ami_store1) = _G(ami_buff);
	_G(mask_store1) = _G(mask_buff);
	make_actor_mask(&_G(actor)[0]);

	_G(thor_x1) = _G(thor)->x + 2;
	_G(thor_y1) = _G(thor)->y + 2;
	_G(thor_x2) = _G(thor)->x + 14;
	_G(thor_y2) = _G(thor)->y + 14;

	load_actor(0, 103 + _G(thor_info).armor);   // Load hammer
	memcpy(&_G(actor)[1], (_G(tmp_buff) + 5120), 40);
	setup_actor(&_G(actor)[1], 1, 0, 100, 100);
	_G(actor)[1].used = 0;
	_G(hammer) = &_G(actor)[1];

	_G(ami_store2) = _G(ami_buff);
	_G(mask_store2) = _G(mask_buff);
	make_actor_mask(&_G(actor)[1]);

	load_actor(0, 106);   // Load sparkle
	memcpy(&_G(sparkle), (_G(tmp_buff) + 5120), 40);
	setup_actor(&_G(sparkle), 20, 0, 100, 100);
	_G(sparkle).used = 0;
	make_actor_mask(&_G(sparkle));

	load_actor(0, 107);   // Load explosion
	memcpy(&_G(explosion), (_G(tmp_buff) + 5120), 40);
	setup_actor(&_G(explosion), 21, 0, 100, 100);
	_G(explosion).used = 0;
	make_actor_mask(&_G(explosion));

	load_actor(0, 108);   // Load tornado
	memcpy(&_G(magic_item)[0], (_G(tmp_buff) + 5120), 40);
	memcpy(&_G(magic_pic)[0], _G(tmp_buff), 1024);
	setup_actor(&_G(magic_item)[0], 20, 0, 0, 0);
	_G(magic_item)[0].used = 0;

	load_actor(0, 1099);   // Load shield
	memcpy(&_G(magic_item)[1], (_G(tmp_buff) + 5120), 40);
	memcpy(&_G(magic_pic)[1], _G(tmp_buff), 1024);
	setup_actor(&_G(magic_item)[1], 20, 0, 0, 0);
	_G(magic_item)[1].used = 0;

	_G(actor)[2].used = 0;
	_G(magic_ami) = _G(ami_buff);
	_G(magic_mask_buff) = _G(mask_buff);

	make_actor_mask(&_G(magic_item)[0]);  // To fool next lines

	_G(enemy_mb) = _G(mask_buff);
	_G(enemy_ami) = _G(ami_buff);
	_G(enemy_lm) = _G(latch_mem);

	return 1;
}

void show_enemies() {
	int i, d, r;

	for (i = 3; i < MAX_ACTORS; i++)
		_G(actor)[i].used = 0;
	for (i = 0; i < MAX_ENEMIES; i++)
		_G(enemy_type)[i] = 0;

	_G(latch_mem) = _G(enemy_lm);
	_G(mask_buff) = _G(enemy_mb);
	_G(ami_buff) = _G(enemy_ami);

	for (i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn).actor_type[i] > 0) {
			r = load_enemy(_G(scrn).actor_type[i]);
			if (r >= 0) {
				memcpy(&_G(actor)[i + 3], &_G(enemy)[r], sizeof(ACTOR));
				d = _G(scrn).actor_dir[i];
				//       _G(scrn).actor_type[i] &= 0x3f;
				setup_actor(&_G(actor)[i + 3], i + 3, d, (_G(scrn).actor_loc[i] % 20) * 16,
					(_G(scrn).actor_loc[i] / 20) * 16);
				_G(actor)[i + 3].init_dir = _G(scrn).actor_dir[i];
				_G(actor)[i + 3].pass_value = _G(scrn).actor_value[i];
				if (_G(actor)[i + 3].move == 23) {
					// Spinball
					if (_G(actor)[i + 3].pass_value & 1)
						_G(actor)[i + 3].move = 24;
				}
				if (_G(scrn).actor_invis[i]) _G(actor)[i + 3].used = 0;
			}

			_G(etype)[i] = r;
		}
	}
}

int load_enemy(int type) {
	int i, f, d, e;
	ACTOR *enm;

	for (i = 0; i < MAX_ENEMIES; i++) if (_G(enemy_type)[i] == type) return i;

	if (!load_actor(1, type)) {
		return -1;
	}
	enm = (ACTOR *)(_G(tmp_buff) + 5120);

	e = -1;
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (!_G(enemy_type)[i]) {
			e = i;
			break;
		}
	}
	if (e == -1) return -1;

	memcpy(&_G(enemy)[e], enm, sizeof(ACTOR_NFO));

	make_actor_mask(&_G(enemy)[e]);
	_G(enemy_type)[e] = type;
	_G(enemy)[e].shot_type = 0;

	if (_G(enemy)[e].shots_allowed) {
		_G(enemy)[e].shot_type = e + 1;
		enm = (ACTOR *)(_G(tmp_buff) + 5160);
		memcpy(&_G(shot)[e], enm, sizeof(ACTOR_NFO));
		for (d = 0; d < _G(shot)[e].directions; d++) {
			for (f = 0; f < _G(shot)[e].frames; f++) {
				if (_G(shot)[e].directions < _G(shot)[e].frames)
					make_mask(&_G(shot)[e].pic[d][f], _G(latch_mem),
						&_G(tmp_buff)[4096 + (256 * ((d * 4) + f))], 16, 16);
				else
					make_mask(&_G(shot)[e].pic[f][d], _G(latch_mem),
						&_G(tmp_buff)[4096 + (256 * ((f * 4) + d))], 16, 16);

				_G(latch_mem) += 144;
				if (_G(latch_mem) > 65421u)
					error("Too Many Actor Frames");
			}
		}
	}

	return e;
}

int actor_visible(int invis_num) {
	int i, d;

	for (i = 0; i < MAX_ENEMIES; i++) {
		if (_G(scrn).actor_invis[i] == invis_num) {
			if (_G(etype)[i] >= 0 && !_G(actor)[i + 3].used) {
				memcpy(&_G(actor)[i + 3], &_G(enemy)[_G(etype)[i]], sizeof(ACTOR));
				d = _G(scrn).actor_dir[i];
				//       _G(scrn).actor_type[i] &= 0x3f;
				setup_actor(&_G(actor)[i + 3], i + 3, d, (_G(scrn).actor_loc[i] % 20) * 16,
					(_G(scrn).actor_loc[i] / 20) * 16);
				_G(actor)[i + 3].init_dir = _G(scrn).actor_dir[i];
				_G(actor)[i + 3].pass_value = _G(scrn).actor_value[i];
				return i;
			} else return -1;
		}
	}
	return -1;
}

void setup_magic_item(int item) {
	int i;
	byte *ami;
	byte *mb;


	mb = _G(mask_buff);
	_G(mask_buff) = _G(magic_mask_buff);

	ami = _G(ami_buff);
	_G(ami_buff) = _G(magic_ami);

	for (i = 0; i < 4; i++) {
		make_mask(&_G(magic_item)[item].pic[i / 4][i % 4], 55200u + (144 * i), &_G(magic_pic)[item][256 * i], 16, 16);
	}
	_G(ami_buff) = ami;
	_G(mask_buff) = mb;
}

void load_new_thor() {
	int rep;
	byte *ami;
	byte *mb;


	mb = _G(mask_buff);
	ami = _G(ami_buff);

	_G(mask_buff) = _G(mask_store1);
	_G(ami_buff) = _G(ami_store1);

	load_actor(0, 100 + _G(thor_info).armor);   // Load thor
	for (rep = 0; rep < 16; rep++) {
		make_mask(&_G(actor)[0].pic[rep / 4][rep % 4], PAGE3 + (144 * rep), &_G(tmp_buff)[256 * rep], 16, 16);
	}
	_G(mask_buff) = _G(mask_store2);
	_G(ami_buff) = _G(ami_store2);
	load_actor(0, 103 + _G(thor_info).armor);   // Load hammer
	for (rep = 0; rep < 16; rep++) {
		make_mask(&_G(actor)[1].pic[rep / 4][rep % 4], 52464u + (144 * rep), &_G(tmp_buff)[256 * rep], 16, 16);
	}

	_G(ami_buff) = ami;
	_G(mask_buff) = mb;
}

} // namespace Got
