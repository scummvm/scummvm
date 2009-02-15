/***************************************************************************
 bdftofont.c Copyright (C) 2003 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <bdf.h>
#include <config.h>
#include <stdlib.h>
#include <stdio.h>


bdf_options_t bdf_opts = {
	0, /* ttf_hint */
	0, /* correct_metrics */
	0, /* keep_unencoded */
	0, /* keep_comments */
	0, /* pad_cells */
	0, /* font_spacing */
	0, /* point_size */
	0, /* resolution_x */
	0, /* resolution_y */
	0, /* bits_per_pixel */
	BDF_UNIX_EOL /* eol */
};


#define GLYPH(n) glyphs[(((n) > 31)? ((n) - 31) : 0)]

void
convert_font(FILE *in_file, FILE *out_file)
{
	bdf_font_t *font =
		bdf_load_font(in_file,
			      &bdf_opts,
			      NULL, NULL);

	int chars_nr = font->glyphs_used;
	int width = font->monowidth;
	int bytes_per_row = (width + 7) >> 3;
	int line_height;
	int char_height = 0;
	int char_byte_size;
	int i;
	bdf_glyph_t *glyphs = font->glyphs;

	fclose(in_file);

	if (!font) {
		fprintf(stderr, "Not a BDF file? Aborting!\n");
		exit(1);
	}

	printf("Res = %d/%d; pointsize = %d, spacing = %d, width=%d, asc=%ld, desc=%ld, glyphs=%ld\n",
	       font->resolution_x,
	       font->resolution_y,
	       font->point_size,
	       font->spacing,
	       font->monowidth,
	       font->font_ascent,
	       font->font_descent,
	       chars_nr);

	if (chars_nr > 256)
		chars_nr = 256;

	for (i = 0; i < chars_nr; i++) {
		int rh = GLYPH(i).bbx.height;

		if (rh > char_height)
			char_height = rh;
	}

		

	line_height = char_height + 1;
	char_byte_size = bytes_per_row * char_height;

	fprintf(out_file, "# %d %d\n", chars_nr, char_height + 1);

	for (i = 0; i < chars_nr; i++) {
		int rh = GLYPH(i).bbx.height;
		int rw = GLYPH(i).bbx.width;
		int xoff = 0; /* GLYPH(i).bbx.x_offset; */
		int yoff = 0; /* GLYPH(i).bbx.y_offset; */
		int j, k;
		int top_pad = yoff;
		int bot_pad = (char_height - rh) - top_pad;
		int bytes_to_read = (GLYPH(i).bytes) / rh;
		unsigned char *data = GLYPH(i).bitmap;

		if (bytes_to_read <= 0) {
			fprintf(stderr, "No bytes per row: bytes=%d, w=%d, h=%d\n",
				GLYPH(i).bytes, rw, rh);
			exit(1);
		}

		if (bot_pad < 0) {
			fprintf(stderr, "Bottom padding <0: height=%d/%d, top_pad=%d\n",
				rh, char_height, yoff);
			exit(1);
		}

		/* First, pad everything */
		for (j = 0; j < top_pad; j++) {
			for (k = 0; k < rw; k++)
				fprintf(out_file, ".");
			fprintf(out_file, "\n");
		}

		/* Write char data */
		for (j = 0; j < rh; j++) {
			unsigned int b = 0;
			unsigned int oldb;
			for (k = 0; k < bytes_to_read; k++) {
				int shift_offset = 8 * (bytes_to_read - 1 - k);
				b |= (*data++ << shift_offset);
			}

			oldb = b;

			for (k = 0; k < rw; k++)
				fprintf(out_file, (oldb & (1 << ((bytes_per_row * 8) - 1 - k)))? "#":".");

			fprintf(out_file, "\n");
		}

		/* Pad bottom */
		for (j = 0; j < bot_pad; j++) {
			for (k = 0; k < rw; k++)
				fprintf(out_file, ".");
			fprintf(out_file, "\n");
		}
		fprintf(out_file,"----\t 0x%02x ('%c') */\n", i, ((i>31)&&(i<0x7f))?i:'.');
	}
	fprintf(out_file, "\n\n");

	fclose(out_file);
}


int
main(int argc, char **argv)
{
	FILE *f = NULL;
	bdf_setup();

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <font.bdf> <outfile.font>\n ", argv[0]);
		exit(1);
	}

	f = fopen(argv[1], "r");
	if (f)
		convert_font(f, fopen(argv[2], "w"));
	else
		perror(argv[1]);

	bdf_cleanup();
}
