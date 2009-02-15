/***************************************************************************
 options.h Copyright (C) 2003-2004 Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

   Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#ifndef __OPTIONS_H
#define __OPTIONS_H

struct dc_option_t {
	char *name;	/* Option name */
	char *values;	/* Option values, each followed by `\0' */
};

/* 0 = stable, 1 = glutton */
#define DC_OPTIONS_TAG_MAJOR 1
/* Increase after every modification to the dc_options struct. */
#define DC_OPTIONS_TAG_MINOR 1
#define NUM_DC_OPTIONS 6

struct dc_option_t dc_options[NUM_DC_OPTIONS] = {
/* 0 */		{ "Video Mode", "Letterboxed\0Full-Screen\0" },
/* 1 */		{ "Dithering", "256 Color Dithering\0Interpolate\00016 Color Dithering\0" },
/* 2 */		{ "Antialiasing", "Off\0On\0" },
/* 3 */		{ "SCI Version", "Autodetect\0001.000.000\0001.000.200\0001.000.510\0" },
/* 4 */		{ "Resource Version", "Autodetect\0001\0002\0003\0004\0005\0006\0" },
/* 5 */		{ "Pic Port Bounds", "Default\0\"0, 0, 320, 200\"\0" }
};

#endif  /* __OPTIONS_H */
