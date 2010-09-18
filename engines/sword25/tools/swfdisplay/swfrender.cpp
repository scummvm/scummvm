#include "art.h"
#include "vectorimage.h"

#define SAVING 1

#if SAVING
static void save_buffer(unsigned char *buffer, uint width, uint height, const char *filename);

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <png.h>
#include <stdio.h>

static gboolean
pixbuf_save_to_file(const GdkPixbuf *pixbuf, const char *file_name) {
	FILE *handle;
	unsigned char *buffer;
	gboolean has_alpha;
	int width, height, depth, rowstride;
	guchar *pixels;
	png_structp png_ptr;
	png_infop info_ptr;
	png_text text[2];
	int i;

	g_return_val_if_fail(pixbuf != NULL, FALSE);
	g_return_val_if_fail(file_name != NULL, FALSE);
	g_return_val_if_fail(file_name[0] != '\0', FALSE);

	handle = fopen(file_name, "wb");
	if (handle == NULL) {
		return FALSE;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(handle);
		return FALSE;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(handle);
		return FALSE;
	}

	if (setjmp(png_ptr->jmpbuf)) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(handle);
		return FALSE;
	}

	png_init_io(png_ptr, handle);

	has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	depth = gdk_pixbuf_get_bits_per_sample(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);

	png_set_IHDR(png_ptr, info_ptr, width, height,
	             depth, PNG_COLOR_TYPE_RGB_ALPHA,
	             PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);

	/* Some text to go with the png image */
	text[0].key = "Title";
	text[0].text = (char *) file_name;
	text[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text[1].key = "Software";
	text[1].text = "Nautilus Thumbnail";
	text[1].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text(png_ptr, info_ptr, text, 2);

	/* Write header data */
	png_write_info(png_ptr, info_ptr);

	/* if there is no alpha in the data, allocate buffer to expand into */
	if (has_alpha) {
		buffer = NULL;
	} else {
		buffer = (unsigned char *)g_malloc(4 * width);
	}

	/* pump the raster data into libpng, one scan line at a time */
	for (i = 0; i < height; i++) {
		if (has_alpha) {
			png_bytep row_pointer = pixels;
			png_write_row(png_ptr, row_pointer);
		} else {
			/* expand RGB to RGBA using an opaque alpha value */
			int x;
			unsigned char *buffer_ptr = buffer;
			unsigned char *source_ptr = pixels;
			for (x = 0; x < width; x++) {
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = 255;
			}
			png_write_row(png_ptr, (png_bytep) buffer);
		}
		pixels += rowstride;
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	g_free(buffer);

	fclose(handle);
	return TRUE;
}

static void
save_buffer(unsigned char *buffer, uint width, uint height, const char *filename) {
	GdkPixbuf *pixbuf;

	g_type_init();

	pixbuf = gdk_pixbuf_new_from_data(buffer,
	                                  GDK_COLORSPACE_RGB,
	                                  TRUE,
									  8,
	                                  width, height,
	                                  width * 4,
	                                  NULL, NULL);

	pixbuf_save_to_file(pixbuf, filename);

	gdk_pixbuf_unref(pixbuf);
}

#endif


int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		exit(0);
	}

	int fnamelen = strlen(argv[1]);
	char *fname = (char *)malloc(fnamelen + 1);
	strcpy(fname, argv[1]);
	fname[fnamelen-3] = 'p';
	fname[fnamelen-2] = 'n';
	fname[fnamelen-1] = 'g';

	FILE *in = fopen(argv[1], "r");
	fseek(in, 0, SEEK_END);
	uint32 len = ftell(in);

	byte *buf = (byte *)malloc(len);
	fseek(in, 0, SEEK_SET);

	fread(buf, len, 1, in);
	fclose(in);

	bool success;
	Sword25::VectorImage img(buf, len, success);

	img.render(img.getWidth(), img.getHeight());

#if SAVING
	save_buffer(img.getPixelData(), img.getWidth(), img.getHeight(), fname);
#endif

	return 0;
}
