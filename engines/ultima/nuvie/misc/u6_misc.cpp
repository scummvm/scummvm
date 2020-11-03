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

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/misc.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/conf/configuration.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/str.h"

namespace Ultima {
namespace Nuvie {

using namespace Std;

bool find_casesensitive_path(Std::string path, Std::string filename, Std::string &new_path);
bool find_path(Std::string path, Std::string &dir_str);

void Tokenise(const Std::string &str, Std::vector<Std::string> &tokens, char delimiter = ' ') {
	Std::string delimiters(delimiter);

	// Skip delimiters at beginning.
	string::size_type lastPos = str.findFirstNotOf(delimiters, 0);

	for (string::size_type pos = str.findFirstOf(delimiters, lastPos) ;
	        string::npos != pos || string::npos != lastPos ;
	        pos = str.findFirstOf(delimiters, lastPos)) {
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.findFirstNotOf(delimiters, pos);
	}
}

Std::string config_get_game_key(Configuration *config) {
	Std::string game_key, game_name;

	config->value("config/GameName", game_name);

	game_key.assign("config/");
	game_key.append(game_name);

	return game_key;
}

const char *get_game_tag(int game_type) {
	switch (game_type) {
	case NUVIE_GAME_U6 :
		return "U6";
	case NUVIE_GAME_MD :
		return "MD";
	case NUVIE_GAME_SE :
		return "SE";
	}

	return "";
}

void config_get_path(Configuration *config, Std::string filename, Std::string &path) {
	Std::string key, game_name, game_dir, tmp_path;

	config->value("config/GameName", game_name);

	key.assign("config/");
	key.append(game_name);
	key.append("/gamedir");

	config->pathFromValue(key, "", game_dir);

	tmp_path = game_dir + filename;

	path = tmp_path;
}

bool find_casesensitive_path(Std::string path, Std::string filename, Std::string &new_path) {
	vector<string> directories;
	string tmp_path = path;

	Tokenise(filename, directories, U6PATH_DELIMITER);

	Std::vector<string>::iterator dir_iter;

	for (dir_iter = directories.begin(); dir_iter != directories.end();) {
		string dir = *dir_iter;

		::debug(1, "%s, ", dir.c_str());

		if (find_path(tmp_path, dir) == false)
			return false;

		dir_iter++;

		if (dir_iter != directories.end())
			dir += U6PATH_DELIMITER;

		tmp_path += dir;
	}

	new_path = tmp_path;

	::debug(1, "\nproper path = %s", new_path.c_str());
	return true;
}

bool find_path(Std::string path, Std::string &dir_str) {
	dir_str = path;
	return true;
#if 0
	DIR *dir;
	struct dirent *item;

	dir = opendir(path.c_str());
	if (dir == NULL)
		return false;

	for (item = readdir(dir); item != NULL; item = readdir(dir)) {
		debug("trying %s, want %s", item->d_name, dir_str.c_str());
		if (strlen(item->d_name) == dir_str.length() && Common::scumm_stricmp(item->d_name, dir_str.c_str()) == 0) {
			dir_str = item->d_name;
			return true;
		}
	}

	return false;
#endif
}

void stringToUpper(Std::string &str) {
	for (size_t i = 0; i < str.length(); ++i) {
		str[i] = toupper(str[i]);
	}
}

void stringToLower(Std::string &str) {
	for (size_t i = 0; i < str.length(); ++i) {
		str[i] = tolower(str[i]);
	}
}

int mkdir_recursive(Std::string path, int mode) {
#ifdef TODO
	vector<string> directories;
	string tmp_path;

	Tokenise(path, directories, U6PATH_DELIMITER);

	Std::vector<string>::iterator dir_iter;

	if (path.find(U6PATH_DELIMITER) == 0)
		tmp_path += U6PATH_DELIMITER;

	for (dir_iter = directories.begin(); dir_iter != directories.end();) {
		string dir = *dir_iter;

		debug("%s, ", dir.c_str());

		tmp_path += dir;
		tmp_path += U6PATH_DELIMITER;
		if (!directory_exists(tmp_path.c_str())) {
#if defined(WIN32)
			int ret = mkdir(tmp_path.c_str());
#else
			int ret = mkdir(tmp_path.c_str(), mode);
#endif
			if (ret != 0)
				return ret;
		}
		dir_iter++;
	}

	return 0;
#else
	error("TODO");
#endif
}

//return the uint8 game_type from a char string
uint8 get_game_type(const char *string) {
	if (string != NULL && strlen(string) >= 2) {
		if (strcmp("md", string) == 0 || strcmp("martian", string) == 0)
			return NUVIE_GAME_MD;
		if (strcmp("se", string) == 0 || strcmp("savage", string) == 0)
			return NUVIE_GAME_SE;
		if (strcmp("u6", string) == 0 || strcmp("ultima6", string) == 0)
			return NUVIE_GAME_U6;
	}

	return NUVIE_GAME_NONE;
}

nuvie_game_t get_game_type(Configuration *config) {
	int game_type;
	config->value("config/GameType", game_type);

	return (nuvie_game_t)game_type;
}

void build_path(Std::string path, Std::string filename, Std::string &full_path) {
	full_path = path;

	if (full_path.length() > 0 && full_path[full_path.length() - 1] != U6PATH_DELIMITER)
		full_path += U6PATH_DELIMITER + filename;
	else
		full_path += filename;

	return;
}

bool has_fmtowns_support(Configuration *config) {
	Std::string townsdir;
	config->value("config/townsdir", townsdir, "");
	if (townsdir != "" && directory_exists(townsdir.c_str()))
		return true;

	return false;
}

bool directory_exists(const char *directory) {
	Common::FSNode gameDir(ConfMan.get("path"));
	return Common::FSNode(directory).exists() || gameDir.getChild(directory).exists();
}

bool file_exists(const char *path) {
	return Common::File::exists(path);
}

void print_b(DebugLevelType level, uint8 num) {
	sint8 i;

	for (i = 7; i >= 0; i--) {
		if (num & (1 << i))
			DEBUG(1, level, "1");
		else
			DEBUG(1, level, "0");
	}

	return;
}

void print_b16(DebugLevelType level, uint16 num) {
	sint8 i;

	for (i = 15; i >= 0; i--) {
		if (num & (1 << i))
			DEBUG(1, level, "1");
		else
			DEBUG(1, level, "0");
	}

	return;
}

void print_indent(DebugLevelType level, uint8 indent) {
	uint16 i;

	for (i = 0; i < indent; i++)
		DEBUG(1, level, " ");

	return;
}


void print_bool(DebugLevelType level, bool state, const char *yes, const char *no) {
	DEBUG(1, level, "%s", state ? yes : no);
}


void print_flags(DebugLevelType level, uint8 num, const char *f[8]) {
	Std::string complete_flags = "";
	print_b(level, num);
	if (num != 0)
		complete_flags += "(";
	for (uint32 i = 0; i < 8; i++)
		if ((num & (1 << i)) && f[i])
			complete_flags += f[i];
	if (num != 0)
		complete_flags += ")";
	DEBUG(1, level, "%s", complete_flags.c_str());
}


/* Where rect1 and rect2 merge, subtract and copy that rect to sub_rect.
 * Returns false if the rectangles don't intersect.
 */
bool subtract_rect(Common::Rect *rect1, Common::Rect *rect2, Common::Rect *sub_rect) {
	uint16 rect1_x2 = rect1->right, rect1_y2 = rect1->bottom;
	uint16 rect2_x2 = rect2->right, rect2_y2 = rect2->bottom;
	uint16 x1, x2, y1, y2;

	if (line_in_rect(rect1->left, rect1->top, rect1_x2, rect1->top, rect2)
	        || line_in_rect(rect1_x2, rect1->top, rect1_x2, rect1_y2, rect2)
	        || line_in_rect(rect1->left, rect1->top, rect1->left, rect1_y2, rect2)
	        || line_in_rect(rect1->left, rect1_y2, rect1_x2, rect1_y2, rect2)) {
		x1 = rect2->left >= rect1->left ? rect2->left : rect1->left;
		y1 = rect2->top >= rect1->top ? rect2->top : rect1->top;
		x2 = rect2_x2 <= rect1_x2 ? rect2_x2 : rect1_x2;
		y2 = rect2_y2 <= rect1_y2 ? rect2_y2 : rect1_y2;
	} else
		return (false);
	if (sub_rect) { // you can perform test without returning a subtraction
		sub_rect->left = x1;
		sub_rect->top = y1;
		sub_rect->setWidth(x2 - x1);
		sub_rect->setHeight(y2 - y1);
	}
	return (true);
}

const char *get_direction_name(uint8 dir) {
	switch (dir) {
	case NUVIE_DIR_N:
		return ("north");
	case NUVIE_DIR_NE:
		return ("Northeast");
	case NUVIE_DIR_E:
		return ("East");
	case NUVIE_DIR_SE:
		return ("Southeast");
	case NUVIE_DIR_S:
		return ("South");
	case NUVIE_DIR_SW:
		return ("Southwest");
	case NUVIE_DIR_W:
		return ("West");
	case NUVIE_DIR_NW:
		return ("Northwest");
	default:
		break;
	}

	return ("nowhere");
}

/* Returns name of relative direction. 0,0 prints "nowhere".
 */
const char *get_direction_name(sint16 rel_x, sint16 rel_y) {
	return get_direction_name(get_direction_code(rel_x, rel_y));
}

/* Gets the nuvie direction code from the original u6 direction code. */
uint8 get_nuvie_dir_code(uint8 original_dir_code) {
	uint8 dir = NUVIE_DIR_NONE;
	//convert original direction into nuvie direction.
	//original
	// 701
	// 6 2
	// 543
	//
	// nuvie
	// 704
	// 3 1
	// 625
	switch (original_dir_code) {
	case 0:
		dir = NUVIE_DIR_N;
		break;
	case 1:
		dir = NUVIE_DIR_NE;
		break;
	case 2:
		dir = NUVIE_DIR_E;
		break;
	case 3:
		dir = NUVIE_DIR_SE;
		break;
	case 4:
		dir = NUVIE_DIR_S;
		break;
	case 5:
		dir = NUVIE_DIR_SW;
		break;
	case 6:
		dir = NUVIE_DIR_W;
		break;
	case 7:
		dir = NUVIE_DIR_NW;
		break;
	default:
		break;
	}

	return dir;
}

sint8 get_original_dir_code(uint8 nuvie_dir_code) {
	sint8 dir = -1;
	//convert nuvie direction into original direction.
	switch (nuvie_dir_code) {
	case NUVIE_DIR_N:
		dir = 0;
		break;
	case NUVIE_DIR_NE:
		dir = 1;
		break;
	case NUVIE_DIR_E:
		dir = 2;
		break;
	case NUVIE_DIR_SE:
		dir = 3;
		break;
	case NUVIE_DIR_S:
		dir = 4;
		break;
	case NUVIE_DIR_SW:
		dir = 5;
		break;
	case NUVIE_DIR_W:
		dir = 6;
		break;
	case NUVIE_DIR_NW:
		dir = 7;
		break;
	default:
		break;
	}

	return dir;
}
/* Returns direction code of relative direction.
 */
uint8 get_direction_code(sint16 rel_x, sint16 rel_y) {
	if (rel_x == 0 && rel_y < 0)
		return NUVIE_DIR_N;
	else if (rel_x > 0 && rel_y < 0)
		return NUVIE_DIR_NE;
	else if (rel_x > 0 && rel_y == 0)
		return NUVIE_DIR_E;
	else if (rel_x > 0 && rel_y > 0)
		return NUVIE_DIR_SE;
	else if (rel_x == 0 && rel_y > 0)
		return NUVIE_DIR_S;
	else if (rel_x < 0 && rel_y > 0)
		return NUVIE_DIR_SW;
	else if (rel_x < 0 && rel_y == 0)
		return NUVIE_DIR_W;
	else if (rel_x < 0 && rel_y < 0)
		return NUVIE_DIR_NW;

	return NUVIE_DIR_NONE;
}

uint8 get_reverse_direction(uint8 dir) {
	switch (dir) {
	case  NUVIE_DIR_N :
		return NUVIE_DIR_S;
	case NUVIE_DIR_E :
		return NUVIE_DIR_W;
	case NUVIE_DIR_S :
		return NUVIE_DIR_N;
	case NUVIE_DIR_W :
		return NUVIE_DIR_E;

	case NUVIE_DIR_NE :
		return NUVIE_DIR_SW;
	case NUVIE_DIR_SE :
		return NUVIE_DIR_NW;
	case NUVIE_DIR_SW :
		return NUVIE_DIR_NE;
	case NUVIE_DIR_NW :
		return NUVIE_DIR_SE;

	case NUVIE_DIR_NONE :
	default :
		break;
	}

	return NUVIE_DIR_NONE;
}

void get_relative_dir(uint8 dir, sint16 *rel_x, sint16 *rel_y) {
	switch (dir) {
	case  NUVIE_DIR_N :
		*rel_x = 0;
		*rel_y = -1;
		break;
	case NUVIE_DIR_E :
		*rel_x = 1;
		*rel_y = 0;
		break;
	case NUVIE_DIR_S :
		*rel_x = 0;
		*rel_y = 1;
		break;
	case NUVIE_DIR_W :
		*rel_x = -1;
		*rel_y = 0;
		break;

	case NUVIE_DIR_NE :
		*rel_x = 1;
		*rel_y = -1;
		break;
	case NUVIE_DIR_SE :
		*rel_x = 1;
		*rel_y = 1;
		break;
	case NUVIE_DIR_SW :
		*rel_x = -1;
		*rel_y = 1;
		break;
	case NUVIE_DIR_NW :
		*rel_x = -1;
		*rel_y = -1;
		break;

	case NUVIE_DIR_NONE :
	default :
		*rel_x = 0;
		*rel_y = 0;
		break;
	}
}

int str_bsearch(const char *str[], int max, const char *value) {
	int position;
	int begin = 0;
	int end = max - 1;
	int cond = 0;

	while (begin <= end) {
		position = (begin + end) / 2;
		if ((cond = strcmp(str[position], value)) == 0)
			return position;
		else if (cond < 0)
			begin = position + 1;
		else
			end = position - 1;
	}

	return -1;
}

#define LINE_FRACTION 65536L

void draw_line_8bit(int sx, int sy, int ex, int ey, uint8 col, uint8 *pixels, uint16 w, uint16 h) {
	uint16 pitch = w;
	int xinc = 1;
	int yinc = 1;

	if (sx == ex) {
		sx --;
		if (sy > ey) {
			yinc = -1;
			sy--;
		}
	} else {
		if (sx > ex) {
			sx--;
			xinc = -1;
		} else {
			ex--;
		}

		if (sy > ey) {
			yinc = -1;
			sy--;
			ey--;
		}
	}

	uint8 *pixptr = (uint8 *)(pixels + pitch * sy + sx);
	uint8 *pixend = (uint8 *)(pixels + pitch * ey + ex);
	pitch = pitch * yinc;

	int cury = sy;
	int curx = sx;
	int width = w;
	int height = h;
	bool no_clip = true;

	if (sx >= width && ex >= width) return;
	if (sy >= height && ey >= height) return;
	if (sx < 0 && ex < 0) return;
	if (sy < 0 && ey < 0) return;

	if (sy < 0 || sy >= height || sx < 0 || sx >= width) no_clip = false;
	if (ey < 0 || ey >= height || ex < 0 || ex >= width) no_clip = false;

	// vertical
	if (sx == ex) {
		//Std::cout << "Vertical" << Std::endl;
		// start is below end
		while (pixptr != pixend) {
			if (no_clip || (cury >= 0 && cury < height)) *pixptr = col;
			pixptr += pitch;
			cury += yinc;
		}
	}
	// Horizontal
	else if (sy == ey) {
		//Std::cout << "Horizontal" << Std::endl;
		while (pixptr != pixend) {
			if (no_clip || (curx >= 0 && curx < width)) *pixptr = col;
			pixptr += xinc;
			curx += xinc;
		}
	}
	// Diagonal xdiff >= ydiff
	else if (Std::labs(sx - ex) >= Std::labs(sy - ey)) {
		//Std::cout << "Diagonal 1" << Std::endl;
		uint32 fraction = Std::labs((LINE_FRACTION * (sy - ey)) / (sx - ex));
		uint32 ycounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col;
			pixptr += xinc;
			if (curx == ex) break;
			curx  += xinc;
			ycounter += fraction;

			// Need to work out if we need to change line
			if (ycounter > LINE_FRACTION) {
				ycounter -= LINE_FRACTION;
				pixptr += pitch;
				cury  += yinc;
			}
		}
	}
	// Diagonal ydiff > xdiff
	else {
		//Std::cout << "Diagonal 2" << Std::endl;
		uint32 fraction = Std::labs((LINE_FRACTION * (sx - ex)) / (sy - ey));
		uint32 xcounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col;
			pixptr += pitch;
			if (cury == ey) break;
			cury  += yinc;
			xcounter += fraction;

			// Need to work out if we need to change line
			if (xcounter > LINE_FRACTION) {
				xcounter -= LINE_FRACTION;
				pixptr += xinc;
				curx += xinc;
			}
		}
	}

}

bool string_i_compare(const Std::string &s1, const Std::string &s2) {
	return scumm_stricmp(s1.c_str(), s2.c_str()) == 0;
}

void *nuvie_realloc(void *ptr, size_t size) {
	void *new_ptr = realloc(ptr, size);
	if (!new_ptr)
		free(ptr);
	return new_ptr;
}


uint32 sdl_getpixel(Graphics::ManagedSurface *surface, int x, int y) {
	int bpp = surface->format.bytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	byte *p = (byte *)surface->getBasePtr(x, y);

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(uint16 *)p;
		break;

	case 3:
		error("TODO: RGB24 unsupported");
		break;

	case 4:
		return *(uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}


void scaleLine8Bit(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth) {
	int NumPixels = TgtWidth;
	int IntPart = SrcWidth / TgtWidth;
	int FractPart = SrcWidth % TgtWidth;
	int E = 0;

	while (NumPixels-- > 0) {
		*Target++ = *Source;
		Source += IntPart;
		E += FractPart;
		if (E >= TgtWidth) {
			E -= TgtWidth;
			Source++;
		} /* if */
	} /* while */
}

//Coarse 2D scaling from http://www.compuphase.com/graphic/scale.htm
void scale_rect_8bit(unsigned char *Source, unsigned char *Target, int SrcWidth, int SrcHeight,
                     int TgtWidth, int TgtHeight) {
	int NumPixels = TgtHeight;
	int IntPart = (SrcHeight / TgtHeight) * SrcWidth;
	int FractPart = SrcHeight % TgtHeight;
	int E = 0;
	unsigned char *PrevSource = NULL;

	while (NumPixels-- > 0) {
		if (Source == PrevSource) {
			memcpy(Target, Target - TgtWidth, TgtWidth * sizeof(*Target));
		} else {
			scaleLine8Bit(Target, Source, SrcWidth, TgtWidth);
			PrevSource = Source;
		} /* if */
		Target += TgtWidth;
		Source += IntPart;
		E += FractPart;
		if (E >= TgtHeight) {
			E -= TgtHeight;
			Source += SrcWidth;
		} /* if */
	} /* while */
}

bool has_file_extension(const char *filename, const char *extension) {
	if (strlen(filename) > strlen(extension) &&
			scumm_stricmp((const char *)&filename[strlen(filename) - 4], extension) == 0)
		return true;

	return false;
}

uint16 wrap_signed_coord(sint16 coord, uint8 level) {
	uint16 width = MAP_SIDE_LENGTH(level);
	if (coord < 0) {
		return (uint16)(width + coord);
	}

	return (uint16)coord % width;
}

sint8 get_wrapped_rel_dir(sint16 p1, sint16 p2, uint8 level) {
	uint16 stride = MAP_SIDE_LENGTH(level);

	sint16 ret = clamp(p1 - p2, -1, 1);

	if (abs(p1 - p2) > stride / 2) {
		return -ret;
	}

	return ret;
}

Std::string encode_xml_entity(const Std::string &s) {
	string  ret;

	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		switch (*it) {
		case '<':
			ret += "&lt;";
			break;
		case '>':
			ret += "&gt;";
			break;
		case '"':
			ret += "&quot;";
			break;
		case '\'':
			ret += "&apos;";
			break;
		case '&':
			ret += "&amp;";
			break;
		default:
			ret += *it;
		}
	}
	return ret;
}

} // End of namespace Nuvie
} // End of namespace Ultima
