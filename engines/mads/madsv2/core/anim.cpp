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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "mads/madsv2/core/anim.h"
#include "mads/madsv2/core/himem.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/color.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/lbm.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/mads.h"
#include "mads/madsv2/core/tile.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {

ShadowList anim_shadow = { 0 };
int anim_error;

void AnimFile::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(num_series, num_frames, num_images, num_speech,
		load_flags, font_auto_spacing, background_type, background_room);
	src->readMultipleLE(misc);
	src->read(background_name, 13);

	for (int i = 0; i < AA_MAX_SERIES; ++i)
		src->read(&series_name[i][0], 13);

	src->read(sound_file_name, 13);
	src->read(background_depth, 13);
	src->read(speech_file, 13);
	src->read(font_file, 13);
}

void Speech::load(Common::SeekableReadStream *src) {
	resource_id = src->readSint16LE();
	src->read(text, 60);
	src->read(misc, 3);
	src->readMultipleLE(sound, x, y, display_condition);

	for (int i = 0; i < 2; ++i)
		color[i].load(src);

	src->readMultipleLE(flags, speech_loops, non_speech_loops, segment_to_loop,
		first_frame, last_frame, first_image);
}

void ImageInter::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(flags, segment_id, series_id, sprite_id, x, y);
}

void Frame::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(sound, speech, ticks, view_x, view_y, yank_x, yank_y);
}

void SegmentInter::load(Common::SeekableReadStream *src) {
	src->readMultipleLE(probability, num_images, first_image, last_image, counter);
	src->read(spawn, AA_MAX_SPAWNED);
	src->readMultipleLE(spawn_frame);
	src->readMultipleLE(sound, sound_frame);
}


//====================================================================

int anim_load_background(AnimFile *anim_in, Buffer *this_orig,
		Buffer *this_depth, TileMapHeader *pictureMap, TileMapHeader *depthMap,
		TileResource *pictureResource, TileResource *depthResource,
		RoomPtr *room, CycleListPtr cycle_list, int load_flags, int star_search) {
	int error_flag;
	char temp_buf[80];
	RoomPtr my_room = NULL;

	error_flag = false;

	if (anim_in->background_type <= AA_FULLSIZE) {

		pal_activate_shadow(&anim_shadow);

		my_room = room_load(anim_in->background_room, 0, anim_in->background_name,
			this_orig, this_depth, NULL, NULL,
			pictureMap,
			depthMap,
			pictureResource,
			depthResource,
			-1,
			-1,
			load_flags);

		if (my_room == NULL) {
			error_flag = true;
		} else {
			if (cycle_list != NULL) {
				memcpy(cycle_list, &my_room->cycle_list, sizeof(CycleList));
			}
		}

	} else if (anim_in->background_type == AA_INTERFACE) {
		temp_buf[0] = 0;
		if (star_search) {
			Common::strcat_s(temp_buf, "*");
		}
		Common::strcat_s(temp_buf, anim_in->background_name);
		error_flag = inter_load_background(temp_buf, this_orig);
		if (cycle_list != NULL) {
			cycle_list->num_cycles = 0;
		}

		if (pictureMap != NULL) {
			tile_fake_map(TILE_PICTURE,
				pictureMap,
				this_orig,
				this_orig->x,
				this_orig->y);
		}

		if (depthMap != NULL) depthMap->map = NULL;

	} else {
		buffer_init_name(this_orig, video_x, video_y, "$anmorig");
		if (this_depth != NULL) buffer_init_name(this_depth, video_x, video_y >> 1, "$anmdpth");
		if (this_orig->data != NULL) {
			buffer_fill(*this_orig, 0);
		} else {
			error_flag = true;
		}

		if (this_depth != NULL) {
			if (this_depth->data != NULL) {
				buffer_fill(*this_depth, 0xff);
			}
		}

		if (pictureMap != NULL) {
			tile_fake_map(TILE_PICTURE,
				pictureMap,
				this_orig,
				this_orig->x, this_orig->y);
		}

		if (depthMap != NULL) {
			tile_fake_map(TILE_ATTRIBUTE,
				depthMap,
				this_depth,
				this_orig->x, this_orig->y);
		}

	}

	if (error_flag) {
		if (this_depth != NULL) buffer_free(this_depth);
		buffer_free(this_orig);
		if (pictureMap != NULL) tile_map_free(pictureMap);
		if (pictureMap != NULL) tile_map_free(depthMap);
	}

	if (room != NULL) *room = my_room;

	return error_flag;
}

void anim_unload(AnimPtr anim) {
	int count;

	if (anim != NULL) {
		if (anim->misc_any_packed) {
			matte_deallocate_series(anim->series_id[anim->misc_packed_series], true);
		}
		for (count = anim->num_series - 1; count >= 0; count--) {
			if (!anim->misc_any_packed || (count != anim->misc_packed_series)) {
				matte_deallocate_series(anim->series_id[count], true);
			}
		}
		if (anim->font != NULL) {
			mem_free(anim->font);
		}

		mem_free(anim);
	}
}

AnimPtr anim_load(const char *file_name, Buffer *orig, Buffer *depth,
		TileMapHeader *pictureMap, TileMapHeader *depthMap,
		TileResource *pictureResource, TileResource *depthResource,
		RoomPtr *room, CycleListPtr cycle_list, int load_flags) {
	int count;
	int error_flag = true;
	int star_search;
	char temp_buf[80];
	char block_name[20];
	char *mark;
	word *color_slaves = (word *)temp_buf;
	word num_color_slaves;
	long image_size, frame_size, speech_size, anim_size;
	AnimPtr anim = NULL;
	AnimInterPtr anim2 = NULL;
	AnimFile anim_in;
	Load load_handle;

	mem_last_alloc_loader = MODULE_ANIM_LOADER;

	load_handle.open = false;

	Common::strcpy_s(temp_buf, file_name);
	if (strchr(temp_buf, '.') == NULL) {
		Common::strcat_s(temp_buf, ".AA");
	}

	star_search = (temp_buf[0] == '*');

	anim_error = 1;
	if (loader_open(&load_handle, temp_buf, "rb", true))
		goto done;

	{
		anim_error = 2;
		byte buffer[AnimFile::SIZE];
		if (!loader_read(buffer, sizeof(AnimFile), 1, &load_handle))
			goto done;

		Common::MemoryReadStream src(buffer, AnimFile::SIZE);
		anim_in.load(&src);
	}

	if (anim_in.background_type == AA_INTERFACE) {
		load_flags |= PAL_MAP_RESERVED;
	}

	if (load_flags & ANIM_LOAD_BACKGROUND) {
		if (anim_load_background(&anim_in,
			orig, depth,
			pictureMap, depthMap,
			pictureResource, depthResource,
			room, cycle_list,
			load_flags, star_search)) {
			anim_error = 20000 + room_load_error;
			goto done;
		}
	}

	if (load_flags & ANIM_LOAD_BACK_ONLY) {
		anim_in.num_speech = 0;
		anim_in.num_images = 0;
		anim_in.num_frames = 0;
	}

	// get how much memory we need to load all the bits
	// AA_INTERFACE are like the fish in the interface in rex... (quasi supported)

	if (anim_in.background_type != AA_INTERFACE) {
		speech_size = (long)sizeof(Speech) * (long)anim_in.num_speech;
		image_size = (long)sizeof(Image) * (long)anim_in.num_images;
		frame_size = (long)sizeof(Frame) * (long)anim_in.num_frames;
		anim_size = (long)sizeof(Anim) + speech_size + image_size + frame_size;
	} else {
		speech_size = (long)sizeof(Speech) * (long)anim_in.num_speech;
		image_size = (long)sizeof(ImageInter) * (long)anim_in.num_images;
		frame_size = (long)sizeof(SegmentInter) * (long)anim_in.num_frames;
		anim_size = (long)sizeof(AnimInter) + speech_size + image_size + frame_size;
	}

	Common::strcpy_s(block_name, "A$");
	mark = temp_buf;
	if (*mark == '*') mark++;
	if ((*mark == 'R') && (*(mark + 1) == 'M')) {
		mark += 2;
	}
	strncat(block_name, mark, 6);

	anim = (AnimPtr)mem_get_name(anim_size, block_name);
	anim2 = (AnimInterPtr)anim;

	anim_error = 4;
	if (anim == NULL) goto done;

	anim->font = NULL;

	if (anim_in.background_type != AA_INTERFACE) {
		anim->speech = (SpeechPtr)mem_normalize(((char *)anim) + sizeof(Anim));
		anim->image = (ImagePtr)mem_normalize(((char *)anim->speech) + speech_size);
		anim->frame = (FramePtr)mem_normalize(((char *)anim->image) + image_size);
	} else {
		anim2->speech = (SpeechPtr)mem_normalize(((char *)anim2) + sizeof(AnimInter));
		anim2->image = (ImageInterPtr)mem_normalize(((char *)anim2->speech) + speech_size);
		anim2->segment = (SegmentInterPtr)mem_normalize(((char *)anim2->image) + image_size);
	}

	// get ready for loading some series
	Common::fill(anim->series, anim->series + AA_MAX_SERIES, (SeriesPtr)nullptr);
	Common::fill(anim->series_id, anim->series_id + AA_MAX_SERIES, -1);

	// we read the header before, now get it into our new structure
	memcpy(anim, &anim_in, sizeof(AnimFile));

	if (speech_size > 0) {
		anim_error = 5;

		size_t size = Speech::SIZE * anim_in.num_speech;
		byte *buffer = (byte *)malloc(size);
		if (!loader_read(buffer, speech_size, 1, &load_handle)) {
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, size);

		if (anim_in.background_type == AA_INTERFACE) {
			for (int i = 0; i < anim_in.num_speech; ++i)
				anim2->speech[i].load(&src);
		} else {
			for (int i = 0; i < anim_in.num_speech; ++i)
				anim->speech[i].load(&src);
		}

		free(buffer);
	}

	// x, y, depth, s, series and sprite id comprise an image
	// read in all images here. image_size is sizeof(Image)* number of images

	if (image_size > 0) {
		anim_error = 6;

		size_t size = (anim_in.background_type == AA_INTERFACE ? ImageInter::SIZE : Image::SIZE) * anim_in.num_images;
		byte *buffer = (byte *)malloc(size);
		if (!loader_read(buffer, size, 1, &load_handle)) {
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, size);

		if (anim_in.background_type == AA_INTERFACE) {
			for (int i = 0; i < anim_in.num_images; ++i)
				anim2->image[i].load(&src);
		} else {
			for (int i = 0; i < anim_in.num_images; ++i)
				anim->image[i].load(&src);
		}

		free(buffer);
	}

	// a frame contains timing information and sound effect information
	// images know which frame they are a part of

	if (frame_size > 0) {
		anim_error = 7;

		size_t size = (anim_in.background_type == AA_INTERFACE ? SegmentInter::SIZE : Frame::SIZE) * anim_in.num_frames;
		byte *buffer = (byte *)malloc(size);
		if (!loader_read(buffer, size, 1, &load_handle)) {
			free(buffer);
			goto done;
		}

		Common::MemoryReadStream src(buffer, size);

		if (anim_in.background_type == AA_INTERFACE) {
			for (int i = 0; i < anim_in.num_frames; ++i)
				anim2->segment[i].load(&src);
		} else {
			for (int i = 0; i < anim_in.num_frames; ++i)
				anim->frame[i].load(&src);
		}

		free(buffer);
	}

	loader_close(&load_handle);

	if (anim->load_flags & AA_LOAD_FONT) {
		temp_buf[0] = 0;
		if (star_search)
			Common::strcat_s(temp_buf, "*");

		Common::strcat_s(temp_buf, anim_in.font_file);
		anim->font = font_load(temp_buf);
		anim_error = 8;

		if (anim->font == NULL)
			goto done;
	}

	// we've loaded the anim file, now load the series

	for (count = 0; count < anim->num_series; count++) {
		if (!anim->misc_any_packed || (count != anim->misc_packed_series)) {
			temp_buf[0] = 0;
			if (star_search) {
				Common::strcat_s(temp_buf, "*");
			}
			Common::strcat_s(temp_buf, anim_in.series_name[count]);

			// matte_load_series will check to see if a series was
			// previously loaded before it goes and gets it from the disk
			// we'll get back a pointer to the actual sprite series for      
			// each id number

			// printf("temp_buf=%s",temp_buf); keys_get();
			anim->series_id[count] = matte_load_series(temp_buf, load_flags, 0);
			if (anim->series_id[count] < 0) {
				anim_error = 9;
				goto done;
			}
			anim->series[count] = series_list[anim->series_id[count]];

		}
	}

	// if a separate load was needed for some reason
	// if we're stream loading, we might need to allocate memory
	// last so that it is at the end of memory - decrease fragmentation
	// problems

	if (anim->misc_any_packed) {
		temp_buf[0] = 0;
		if (star_search) {
			Common::strcat_s(temp_buf, "*");
		}
		Common::strcat_s(temp_buf, anim_in.series_name[anim->misc_packed_series]);

		count = anim->misc_packed_series;
		anim->series_id[count] = matte_load_series(temp_buf, load_flags, 0);
		if (anim->series_id[count] < 0) {
			anim_error = 11;
			goto done;
		}

		anim->series[count] = series_list[anim->series_id[count]];
	}

	// compact all the colors in the series to a single color handle
	// so it doesn't take up lots of palette space

	num_color_slaves = 0;
	for (count = 1; count < anim->num_series; count++) {
		color_slaves[num_color_slaves] = anim->series[count]->color_handle;
		num_color_slaves++;
	}

	if (num_color_slaves > 0) {
		pal_compact(anim->series[0]->color_handle, num_color_slaves, color_slaves);
	}

	// translate the numbers we have into actual runtime numbers
	// in the file they were 0, 1, 2, ... When we translate them they'll
	// have to be moved not to conflict with series that we might already
	// have loaded

	if (anim_in.background_type != AA_INTERFACE) {
		for (count = 0; count < anim->num_images; count++) {
			anim->image[count].series_id = (byte)anim->series_id[anim->image[count].series_id];
		}
	} else {
		for (count = 0; count < anim2->num_images; count++) {
			anim2->image[count].series_id = (byte)anim2->series_id[anim2->image[count].series_id];
		}
	}

	error_flag = false;
	anim_error = 0;

done:
	if (error_flag) {
		anim_unload(anim);
		anim = NULL;

		if (load_flags & ANIM_LOAD_BACKGROUND) {
			if (depth != NULL)       buffer_free(depth);
			if (orig != NULL)       buffer_free(orig);
			if (depthMap != NULL)   tile_map_free(depthMap);
			if (pictureMap != NULL) tile_map_free(pictureMap);
		}
	}
	if (load_handle.open) loader_close(&load_handle);

	return (anim);
}

int anim_get_sound_info(char *file_name, char *sound_file_buffer, int *sound_load_flag) {
	int error_flag = true;
	AnimFile anim_in;
	Load load_handle;

	load_handle.open = false;

	if (loader_open(&load_handle, file_name, "rb", true)) goto done;

	if (!loader_read(&anim_in, sizeof(AnimFile), 1, &load_handle)) goto done;

	Common::strcpy_s(sound_file_buffer, 65536, anim_in.sound_file_name);
	*sound_load_flag = (anim_in.load_flags & AA_LOAD_SOUND);

	error_flag = false;

done:
	if (load_handle.open)
		loader_close(&load_handle);
	return error_flag;
}

int anim_get_header_info(char *file_name,
	AnimFile *anim_in) {
	int error_flag = true;
	Load load_handle;

	load_handle.open = false;

	if (loader_open(&load_handle, file_name, "rb", true))
		goto done;

	if (!loader_read(anim_in, sizeof(AnimFile), 1, &load_handle))
		goto done;

	error_flag = false;

done:
	if (load_handle.open)
		loader_close(&load_handle);

	return error_flag;
}

int anim_himem_preload(char *name, int level) {
	int error_flag = true;
	char temp_buf[80];
	AnimFile anim_in;
	int count2;
	int mads_mode;

	mads_mode = (name[0] == '*');

	himem_preload_series(name, level);

	if (anim_get_header_info(name, &anim_in)) goto done;

	if (anim_in.load_flags & AA_LOAD_FONT) {
		temp_buf[0] = 0;
		if (mads_mode) Common::strcat_s(temp_buf, "*");
		Common::strcat_s(temp_buf, anim_in.font_file);
		himem_preload_series(temp_buf, level);
	}

	for (count2 = 0; count2 < anim_in.num_series; count2++) {
		temp_buf[0] = 0;
		if (mads_mode) Common::strcpy_s(temp_buf, "*");
		Common::strcat_s(temp_buf, anim_in.series_name[count2]);
		himem_preload_series(temp_buf, level);
	}

	if (anim_in.background_type == AA_ROOM) {
		env_get_level_path(temp_buf, ROOM, ".DAT", 0, anim_in.background_room);
		himem_preload_series(temp_buf, level);
		env_get_level_path(temp_buf, ROOM, ".TT", 0, anim_in.background_room);
		himem_preload_series(temp_buf, level);
		env_get_level_path(temp_buf, ROOM, ".MM", 0, anim_in.background_room);
		himem_preload_series(temp_buf, level);
		env_get_level_path(temp_buf, ROOM, ".TT0", 0, anim_in.background_room);
		himem_preload_series(temp_buf, level);
		env_get_level_path(temp_buf, ROOM, ".MM0", 0, anim_in.background_room);
		himem_preload_series(temp_buf, level);
	} else if (anim_in.background_type == AA_INTERFACE) {
		temp_buf[0] = 0;
		if (mads_mode) Common::strcat_s(temp_buf, "*");
		Common::strcat_s(temp_buf, anim_in.background_name);
		himem_preload_series(temp_buf, level);
	}

	error_flag = false;

done:
	return error_flag;
}

} // namespace MADSV2
} // namespace MADS
