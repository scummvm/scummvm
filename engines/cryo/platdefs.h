#pragma once

#if 1
#include "common/file.h"

typedef Common::File file_t;

struct filespec_t {
char	create;
char	name[260];
};

#if 1
const int subtitles_x_margin = 16;			//PC
const int subtitles_x_scr_margin = 16;
const int space_width = 6;
#define FAKE_DOS_VERSION
#else
const int subtitles_x_margin = 16;			//MAC
const int subtitles_x_scr_margin = 16;			//MAC
const int space_width = 4;
#endif
const int subtitles_x_width = (320 - subtitles_x_margin * 2);
const int subtitles_x_center = subtitles_x_width / 2;

#endif
