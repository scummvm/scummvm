#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_render_aa.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rgb.h>

#define SAVING 1

#define WIDTH 3000
#define HEIGHT 3000
#define BYTES_PER_PIXEL 3 /* 24 packed rgb bits */
#define ROWSTRIDE (WIDTH*BYTES_PER_PIXEL)


static unsigned char *render_path(void);

#if SAVING
static void save_buffer(unsigned char *buffer, const char *filename);

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
		buffer = g_malloc(4 * width);
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
save_buffer(unsigned char *buffer, const char *filename) {
	GdkPixbuf *pixbuf;

	g_type_init();

	pixbuf = gdk_pixbuf_new_from_data(buffer,
	                                  GDK_COLORSPACE_RGB,
#if BYTES_PER_PIXEL == 4
	                                  TRUE,
#else
	                                  FALSE,
#endif
									  8,
	                                  WIDTH, HEIGHT,
	                                  ROWSTRIDE,
	                                  NULL, NULL);

	pixbuf_save_to_file(pixbuf, filename);

	gdk_pixbuf_unref(pixbuf);
}

#endif

void
art_rgb_fill_run1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, gint n) {
	int i;

	if (r == g && g == b && r == 255) {
		memset(buf, g, n + n + n + n);
	} else {
		art_u32 *alt = (art_u32 *)buf;
		//art_u32 color = (r << 24) | (g << 16) | (b << 8) | 0xff;
		art_u32 color = (r << 0) | (g << 8) | (b << 16) | (0xff << 24);
		for (i = 0; i < n; i++)
			*alt++ = color;
	}
}

void
art_rgb_run_alpha1(art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n) {
	int i;
	int v;

	for (i = 0; i < n; i++) {
		v = *buf;
		*buf++ = v + (((r - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((g - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((b - v) * alpha + 0x80) >> 8);
		v = *buf;
		*buf++ = v + (((alpha - v) * alpha + 0x80) >> 8);
	}
}

typedef struct _ArtRgbSVPAlphaData ArtRgbSVPAlphaData;

struct _ArtRgbSVPAlphaData {
	int alphatab[256];
	art_u8 r, g, b, alpha;
	art_u8 *buf;
	int rowstride;
	int x0, x1;
};

static void
art_rgb_svp_alpha_callback1(void *callback_data, int y,
                            int start, ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = (running_sum >> 16) & 0xff;
				if (alpha)
					art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = (running_sum >> 16) & 0xff;
			if (alpha)
				art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
		}
	} else {
		alpha = (running_sum >> 16) & 0xff;
		if (alpha)
			art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
	}

	data->buf += data->rowstride;
}

static void
art_rgb_svp_alpha_opaque_callback1(void *callback_data, int y,
                                   int start,
                                   ArtSVPRenderAAStep *steps, int n_steps) {
	ArtRgbSVPAlphaData *data = (ArtRgbSVPAlphaData *)callback_data;
	art_u8 *linebuf;
	int run_x0, run_x1;
	art_u32 running_sum = start;
	int x0, x1;
	int k;
	art_u8 r, g, b;
	int *alphatab;
	int alpha;

	linebuf = data->buf;
	x0 = data->x0;
	x1 = data->x1;

	r = data->r;
	g = data->g;
	b = data->b;
	alphatab = data->alphatab;

	if (n_steps > 0) {
		run_x1 = steps[0].x;
		if (run_x1 > x0) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf, r, g, b, run_x1 - x0);
				else
					art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], run_x1 - x0);
			}
		}

		for (k = 0; k < n_steps - 1; k++) {
			running_sum += steps[k].delta;
			run_x0 = run_x1;
			run_x1 = steps[k + 1].x;
			if (run_x1 > run_x0) {
				alpha = running_sum >> 16;
				if (alpha) {
					if (alpha >= 255)
						art_rgb_fill_run1(linebuf + (run_x0 - x0) * 4, r, g, b, run_x1 - run_x0);
					else
						art_rgb_run_alpha1(linebuf + (run_x0 - x0) * 4, r, g, b, alphatab[alpha], run_x1 - run_x0);
				}
			}
		}
		running_sum += steps[k].delta;
		if (x1 > run_x1) {
			alpha = running_sum >> 16;
			if (alpha) {
				if (alpha >= 255)
					art_rgb_fill_run1(linebuf + (run_x1 - x0) * 4, r, g, b, x1 - run_x1);
				else
					art_rgb_run_alpha1(linebuf + (run_x1 - x0) * 4, r, g, b, alphatab[alpha], x1 - run_x1);
			}
		}
	} else {
		alpha = running_sum >> 16;
		if (alpha) {
			if (alpha >= 255)
				art_rgb_fill_run1(linebuf, r, g, b, x1 - x0);
			else
				art_rgb_run_alpha1(linebuf, r, g, b, alphatab[alpha], x1 - x0);
		}
	}

	data->buf += data->rowstride;
}

void
art_rgb_svp_alpha1(const ArtSVP *svp,
                   int x0, int y0, int x1, int y1,
                   art_u32 rgba,
                   art_u8 *buf, int rowstride,
                   ArtAlphaGamma *alphagamma) {
	ArtRgbSVPAlphaData data;
	int r, g, b, alpha;
	int i;
	int a, da;

	alpha = rgba >> 24;
	r = (rgba >> 16) & 0xff;
	g = (rgba >> 8) & 0xff;
	b = rgba & 0xff;

	data.r = r;
	data.g = g;
	data.b = b;
	data.alpha = alpha;

	a = 0x8000;
	da = (alpha * 66051 + 0x80) >> 8; /* 66051 equals 2 ^ 32 / (255 * 255) */

	for (i = 0; i < 256; i++) {
		data.alphatab[i] = a >> 16;
		a += da;
	}

	data.buf = buf;
	data.rowstride = rowstride;
	data.x0 = x0;
	data.x1 = x1;
	if (alpha == 255)
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_opaque_callback1, &data);
	else
		art_svp_render_aa(svp, x0, y0, x1, y1, art_rgb_svp_alpha_callback1, &data);
}

static int art_vpath_len (ArtVpath * a) {
  int i;

  for (i = 0; a[i].code != ART_END; i++);
  return i;
}

ArtVpath *art_vpath_reverse(ArtVpath * a) {
  ArtVpath *dest;
  ArtVpath it;
  int len;
  int state = 0;
  int i;

  len = art_vpath_len(a);
  dest = g_malloc((len + 1) * sizeof(ArtVpath));

  for (i = 0; i < len; i++) {
    it = a[len - i - 1];
    if (state) {
      it.code = ART_LINETO;
    } else {
      it.code = ART_MOVETO_OPEN;
      state = 1;
    }
    if (a[len - i - 1].code == ART_MOVETO ||
        a[len - i - 1].code == ART_MOVETO_OPEN) {
      state = 0;
    }
    dest[i] = it;
  }
  dest[len] = a[len];

  return dest;
}

ArtVpath *art_vpath_reverse_free(ArtVpath *a) {
  ArtVpath *dest;

  dest = art_vpath_reverse(a);
  art_free(a);

  return dest;
}

void art_svp_make_convex (ArtSVP *svp) {
  int i;

  if (svp->n_segs > 0 && svp->segs[0].dir == 0) {
    for (i = 0; i < svp->n_segs; i++) {
      svp->segs[i].dir = !svp->segs[i].dir;
    }
  }
}


void drawBez(ArtBpath *bez, art_u8 *buffer, double scaleX, double scaleY, double penWidth, unsigned int color) {
	ArtVpath *vec = NULL;
	ArtSVP *svp = NULL;

	vec = art_bez_path_to_vec(bez, 0.5);

	if (scaleX != 1.0 || scaleY != 1.0) {
		ArtVpath *vec1;
		double matrix[6];

		matrix[0] = scaleX;
		matrix[1] = 0;
		matrix[2] = 0;
		matrix[3] = scaleY;
		matrix[4] = matrix[5] = 0;

		vec1 = art_vpath_affine_transform(vec, matrix);
		art_free(vec);

		vec = vec1;
	}

	if (penWidth != -1) {
		svp = art_svp_vpath_stroke(vec, ART_PATH_STROKE_JOIN_ROUND, ART_PATH_STROKE_CAP_ROUND, penWidth, 1.0, 0.5);
	} else {
		svp = art_svp_from_vpath(vec);
	}

#if BYTES_PER_PIXEL == 4
	art_rgb_svp_alpha1(svp, 0, 0, WIDTH, HEIGHT, color, buffer, ROWSTRIDE, NULL);
#else
	color <<= 8;
	color |= 0xff;
	art_rgb_svp_alpha(svp, 0, 0, WIDTH, HEIGHT, color, buffer, ROWSTRIDE, NULL);
#endif
	art_free(svp);

	art_free(vec);
}

static unsigned char *render_path() {
	art_u8 *buffer = NULL;

	ArtBpath *bez = NULL;

	buffer = art_new(art_u8, WIDTH * HEIGHT * BYTES_PER_PIXEL);
#if BYTES_PER_PIXEL == 4
	memset(buffer, 0, WIDTH*HEIGHT*BYTES_PER_PIXEL);
#else
	memset(buffer, 0xff, WIDTH*HEIGHT*BYTES_PER_PIXEL);
#endif
	bez = art_new(ArtBpath, 100);

#include "image.c"

	art_free(bez);

	return (unsigned char *) buffer;
}

int main(int argc, char *argv[]) {
	unsigned char *buffer;

	buffer = render_path();

#if SAVING
	save_buffer(buffer, "foo.png");
#endif

	return 0;
}
