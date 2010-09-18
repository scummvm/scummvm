#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_render_aa.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rgb.h>

#define SAVING 1

#define WIDTH 1500
#define HEIGHT 1500
#define BYTES_PER_PIXEL 4 /* 24 packed rgb bits */
#define ROWSTRIDE (WIDTH*BYTES_PER_PIXEL)


static unsigned char *render_path (void);

#if SAVING
static void save_buffer (unsigned char *buffer, const char *filename);

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <png.h>
#include <stdio.h>

/**
 * pixbuf_save_to_file:
 * @pixbuf: pixel buffer to save.
 * @filename: file name to save the pixel buffer into.
 *
 * Saves the pixel buffer in the given filename under
 * the .png format. Returns TRUE is succesful and FALSE
 * otherwise.
 *
 * Copyright is to Iain Holmes and Eazel, inc for this function.
 * It was stolen from nautilus-gdk-pixbuf-extensions.c but
 * was originally coming from gnome-iconedit by Iain Holmes
 * It was hacked by Ramiro Estrugo for Eazel, inc.
 */
static gboolean
pixbuf_save_to_file (const GdkPixbuf *pixbuf, const char *file_name)
{
    FILE *handle;
    unsigned char *buffer;
    gboolean has_alpha;
    int width, height, depth, rowstride;
    guchar *pixels;
    png_structp png_ptr;
    png_infop info_ptr;
    png_text text[2];
    int i;

    g_return_val_if_fail (pixbuf != NULL, FALSE);
    g_return_val_if_fail (file_name != NULL, FALSE);
    g_return_val_if_fail (file_name[0] != '\0', FALSE);

    handle = fopen (file_name, "wb");
    if (handle == NULL) {
        return FALSE;
    }

    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose (handle);
        return FALSE;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
        fclose (handle);
        return FALSE;
    }

    if (setjmp (png_ptr->jmpbuf)) {
        png_destroy_write_struct (&png_ptr, &info_ptr);
        fclose (handle);
        return FALSE;
    }

    png_init_io (png_ptr, handle);

    has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);
    depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);

    png_set_IHDR (png_ptr, info_ptr, width, height,
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
    png_set_text (png_ptr, info_ptr, text, 2);

    /* Write header data */
    png_write_info (png_ptr, info_ptr);

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
            png_write_row (png_ptr, row_pointer);
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
            png_write_row (png_ptr, (png_bytep) buffer);
        }
        pixels += rowstride;
    }

    png_write_end (png_ptr, info_ptr);
    png_destroy_write_struct (&png_ptr, &info_ptr);

    g_free (buffer);

    fclose (handle);
    return TRUE;
}

static void
save_buffer (unsigned char *buffer, const char *filename)
{
    GdkPixbuf *pixbuf;

    g_type_init();

    pixbuf = gdk_pixbuf_new_from_data (buffer,
                                       GDK_COLORSPACE_RGB,
                                       TRUE, 8,
                                       WIDTH, HEIGHT,
                                       ROWSTRIDE,
                                       NULL, NULL);

    pixbuf_save_to_file (pixbuf, filename);

    gdk_pixbuf_unref (pixbuf);
}

#endif

void
art_rgb_fill_run1 (art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, gint n)
{
    int i;

    if (r == g && g == b && r == 255)
    {
        memset (buf, g, n + n + n + n);
    }
    else
    {
        art_u32 *alt = (art_u32 *)buf;
        art_u32 color = (r << 24) | (g << 16) | (b << 8) | 0xff;
        for (i = 0; i < n; i++)
            *alt++ = color;
    }
}

/**
 * art_rgb_run_alpha: Render semitransparent color over RGB buffer.
 * @buf: Buffer for rendering.
 * @r: Red, range 0..255.
 * @g: Green, range 0..255.
 * @b: Blue, range 0..255.
 * @alpha: Alpha, range 0..256.
 * @n: Number of RGB triples to render.
 *
 * Renders a sequential run of solid (@r, @g, @b) color over @buf with
 * opacity @alpha.
 **/
void
art_rgb_run_alpha1 (art_u8 *buf, art_u8 r, art_u8 g, art_u8 b, int alpha, int n)
{
    int i;
    int v;

    for (i = 0; i < n; i++)
    {
        v = *buf;
        *buf++ = v + (((r - v) * alpha + 0x80) >> 8);
        v = *buf;
        *buf++ = v + (((g - v) * alpha + 0x80) >> 8);
        v = *buf;
        *buf++ = v + (((b - v) * alpha + 0x80) >> 8);
        v = *buf + alpha;
        if (v > 255)
            v = 255;
        *buf++ = v;
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
art_rgb_svp_alpha_callback1 (void *callback_data, int y,
                             int start, ArtSVPRenderAAStep *steps, int n_steps)
{
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

    if (n_steps > 0)
    {
        run_x1 = steps[0].x;
        if (run_x1 > x0)
        {
            alpha = (running_sum >> 16) & 0xff;
            if (alpha)
                art_rgb_run_alpha1 (linebuf,
                                    r, g, b, alphatab[alpha],
                                    run_x1 - x0);
        }

        for (k = 0; k < n_steps - 1; k++)
        {
            running_sum += steps[k].delta;
            run_x0 = run_x1;
            run_x1 = steps[k + 1].x;
            if (run_x1 > run_x0)
            {
                alpha = (running_sum >> 16) & 0xff;
                if (alpha)
                    art_rgb_run_alpha1 (linebuf + (run_x0 - x0) * 4,
                                        r, g, b, alphatab[alpha],
                                        run_x1 - run_x0);
            }
        }
        running_sum += steps[k].delta;
        if (x1 > run_x1)
        {
            alpha = (running_sum >> 16) & 0xff;
            if (alpha)
                art_rgb_run_alpha1 (linebuf + (run_x1 - x0) * 4,
                                    r, g, b, alphatab[alpha],
                                    x1 - run_x1);
        }
    }
    else
    {
        alpha = (running_sum >> 16) & 0xff;
        if (alpha)
            art_rgb_run_alpha1 (linebuf,
                                r, g, b, alphatab[alpha],
                                x1 - x0);
    }

    data->buf += data->rowstride;
}

static void
art_rgb_svp_alpha_opaque_callback1 (void *callback_data, int y,
                                    int start,
                                    ArtSVPRenderAAStep *steps, int n_steps)
{
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

    if (n_steps > 0)
    {
        run_x1 = steps[0].x;
        if (run_x1 > x0)
        {
            alpha = running_sum >> 16;
            if (alpha)
            {
                if (alpha >= 255)
                    art_rgb_fill_run1 (linebuf,
                                       r, g, b,
                                       run_x1 - x0);
                else
                    art_rgb_run_alpha1 (linebuf,
                                        r, g, b, alphatab[alpha],
                                        run_x1 - x0);
            }
        }

        for (k = 0; k < n_steps - 1; k++)
        {
            running_sum += steps[k].delta;
            run_x0 = run_x1;
            run_x1 = steps[k + 1].x;
            if (run_x1 > run_x0)
            {
                alpha = running_sum >> 16;
                if (alpha)
                {
                    if (alpha >= 255)
                        art_rgb_fill_run1 (linebuf + (run_x0 - x0) * 4,
                                           r, g, b,
                                           run_x1 - run_x0);
                    else
                        art_rgb_run_alpha1 (linebuf + (run_x0 - x0) * 4,
                                            r, g, b, alphatab[alpha],
                                            run_x1 - run_x0);
                }
            }
        }
        running_sum += steps[k].delta;
        if (x1 > run_x1)
        {
            alpha = running_sum >> 16;
            if (alpha)
            {
                if (alpha >= 255)
                    art_rgb_fill_run1 (linebuf + (run_x1 - x0) * 4,
                                       r, g, b,
                                       x1 - run_x1);
                else
                    art_rgb_run_alpha1 (linebuf + (run_x1 - x0) * 4,
                                        r, g, b, alphatab[alpha],
                                        x1 - run_x1);
            }
        }
    }
    else
    {
        alpha = running_sum >> 16;
        if (alpha)
        {
            if (alpha >= 255)
                art_rgb_fill_run1 (linebuf,
                                   r, g, b,
                                   x1 - x0);
            else
                art_rgb_run_alpha1 (linebuf,
                                    r, g, b, alphatab[alpha],
                                    x1 - x0);
        }
    }

    data->buf += data->rowstride;
}

/**
 * art_rgb_svp_alpha: Alpha-composite sorted vector path over RGB buffer.
 * @svp: The source sorted vector path.
 * @x0: Left coordinate of destination rectangle.
 * @y0: Top coordinate of destination rectangle.
 * @x1: Right coordinate of destination rectangle.
 * @y1: Bottom coordinate of destination rectangle.
 * @rgba: Color in 0xRRGGBBAA format.
 * @buf: Destination RGB buffer.
 * @rowstride: Rowstride of @buf buffer.
 * @alphagamma: #ArtAlphaGamma for gamma-correcting the compositing.
 *
 * Renders the shape specified with @svp over the @buf RGB buffer.
 * @x1 - @x0 specifies the width, and @y1 - @y0 specifies the height,
 * of the rectangle rendered. The new pixels are stored starting at
 * the first byte of @buf. Thus, the @x0 and @y0 parameters specify
 * an offset within @svp, and may be tweaked as a way of doing
 * integer-pixel translations without fiddling with @svp itself.
 *
 * The @rgba argument specifies the color for the rendering. Pixels of
 * entirely 0 winding number are left untouched. Pixels of entirely
 * 1 winding number have the color @rgba composited over them (ie,
 * are replaced by the red, green, blue components of @rgba if the alpha
 * component is 0xff). Pixels of intermediate coverage are interpolated
 * according to the rule in @alphagamma, or default to linear if
 * @alphagamma is NULL.
 **/
void
art_rgb_svp_alpha1 (const ArtSVP *svp,
                    int x0, int y0, int x1, int y1,
                    art_u32 rgba,
                    art_u8 *buf, int rowstride,
                    ArtAlphaGamma *alphagamma)
{
    ArtRgbSVPAlphaData data;
    int r, g, b, alpha;
    int i;
    int a, da;

    r = rgba >> 24;
    g = (rgba >> 16) & 0xff;
    b = (rgba >> 8) & 0xff;
    alpha = rgba & 0xff;

    data.r = r;
    data.g = g;
    data.b = b;
    data.alpha = alpha;

    a = 0x8000;
    da = (alpha * 66051 + 0x80) >> 8; /* 66051 equals 2 ^ 32 / (255 * 255) */

    for (i = 0; i < 256; i++)
    {
        data.alphatab[i] = a >> 16;
        a += da;
    }

    data.buf = buf;
    data.rowstride = rowstride;
    data.x0 = x0;
    data.x1 = x1;
    if (alpha == 255)
        art_svp_render_aa (svp, x0, y0, x1, y1, art_rgb_svp_alpha_opaque_callback1,
                           &data);
    else
        art_svp_render_aa (svp, x0, y0, x1, y1, art_rgb_svp_alpha_callback1, &data);
}



static unsigned char *
render_path ()
{
    art_u8 *buffer = NULL;
    art_u32 color1 = (0xFF << 24) | (0x00 <<16) | (0x00<<8) | (0xFF) ; /* RRGGBBAA */
    art_u32 color2 = (0x00 << 24) | (0x00 <<16) | (0xFF<<8) | (0xFF) ; /* RRGGBBAA */

    ArtBpath *bez1 = NULL;
    ArtVpath *vec = NULL;
    ArtSVP *svp1 = NULL;
    ArtSVP *svp2 = NULL;

    bez1 = art_new (ArtBpath, 30);
    bez1[0].code = ART_MOVETO;
    bez1[0].x3 = 352.000000; bez1[0].y3 = 59.000000;
    bez1[1].code = ART_CURVETO;
    bez1[1].x1 = 396.000000; bez1[1].y1 = 41.000000;
    bez1[1].x2 = 434.333333; bez1[1].y2 = 57.666667;
    bez1[1].x3 = 467.000000; bez1[1].y3 = 109.000000;
    bez1[2].code = ART_CURVETO;
    bez1[2].x1 = 481.666667; bez1[2].y1 = 137.666667;
    bez1[2].x2 = 487.666667; bez1[2].y2 = 172.000000;
    bez1[2].x3 = 485.000000; bez1[2].y3 = 212.000000;
    bez1[3].code = ART_LINETO;
    bez1[3].x3 = 485.000000; bez1[3].y3 = 216.000000;
    bez1[4].code = ART_LINETO;
    bez1[4].x3 = 483.000000; bez1[4].y3 = 231.000000;
    bez1[5].code = ART_LINETO;
    bez1[5].x3 = 468.000000; bez1[5].y3 = 291.000000;
    bez1[6].code = ART_LINETO;
    bez1[6].x3 = 466.000000; bez1[6].y3 = 299.000000;
    bez1[7].code = ART_LINETO;
    bez1[7].x3 = 446.000000; bez1[7].y3 = 349.000000;
    bez1[8].code = ART_LINETO;
    bez1[8].x3 = 445.000000; bez1[8].y3 = 351.000000;
    bez1[9].code = ART_CURVETO;
    bez1[9].x1 = 442.333333; bez1[9].y1 = 376.333333;
    bez1[9].x2 = 450.000000; bez1[9].y2 = 394.000000;
    bez1[9].x3 = 468.000000; bez1[9].y3 = 404.000000;
    bez1[10].code = ART_CURVETO;
    bez1[10].x1 = 373.333333; bez1[10].y1 = 428.666667;
    bez1[10].x2 = 294.666667; bez1[10].y2 = 426.666667;
    bez1[10].x3 = 232.000000; bez1[10].y3 = 398.000000;
    bez1[11].code = ART_CURVETO;
    bez1[11].x1 = 249.333333; bez1[11].y1 = 380.666667;
    bez1[11].x2 = 257.666667; bez1[11].y2 = 365.000000;
    bez1[11].x3 = 257.000000; bez1[11].y3 = 351.000000;
    bez1[12].code = ART_LINETO;
    bez1[12].x3 = 255.000000; bez1[12].y3 = 345.000000;
    bez1[13].code = ART_CURVETO;
    bez1[13].x1 = 244.333333; bez1[13].y1 = 326.333333;
    bez1[13].x2 = 236.000000; bez1[13].y2 = 307.666667;
    bez1[13].x3 = 230.000000; bez1[13].y3 = 289.000000;
    bez1[14].code = ART_CURVETO;
    bez1[14].x1 = 223.333333; bez1[14].y1 = 270.333333;
    bez1[14].x2 = 219.000000; bez1[14].y2 = 252.000000;
    bez1[14].x3 = 217.000000; bez1[14].y3 = 234.000000;
    bez1[15].code = ART_LINETO;
    bez1[15].x3 = 216.000000; bez1[15].y3 = 233.000000;
    bez1[16].code = ART_LINETO;
    bez1[16].x3 = 215.000000; bez1[16].y3 = 229.000000;
    bez1[17].code = ART_CURVETO;
    bez1[17].x1 = 201.666667; bez1[17].y1 = 187.666667;
    bez1[17].x2 = 210.666667; bez1[17].y2 = 144.000000;
    bez1[17].x3 = 242.000000; bez1[17].y3 = 98.000000;
    bez1[18].code = ART_CURVETO;
    bez1[18].x1 = 274.000000; bez1[18].y1 = 56.000000;
    bez1[18].x2 = 310.333333; bez1[18].y2 = 43.333333;
    bez1[18].x3 = 351.000000; bez1[18].y3 = 60.000000;
    bez1[19].code = ART_LINETO;
    bez1[19].x3 = 352.000000; bez1[19].y3 = 59.000000;
    bez1[20].code = ART_END;

    vec = art_bez_path_to_vec (bez1, 0.5);
    svp1 = art_svp_from_vpath (vec);

    int penWidth = 5;
    svp2 = art_svp_vpath_stroke(vec, ART_PATH_STROKE_JOIN_ROUND, ART_PATH_STROKE_CAP_ROUND, penWidth, 1.0, 0.5);


    buffer = art_new (art_u8, WIDTH*HEIGHT*BYTES_PER_PIXEL);
    memset(buffer, 0, WIDTH*HEIGHT*BYTES_PER_PIXEL);
    art_rgb_run_alpha (buffer, 0, 0, 0, 0, WIDTH*HEIGHT);
    art_rgb_svp_alpha1 (svp1, 0, 0, WIDTH, HEIGHT, color1, buffer, ROWSTRIDE, NULL);
    art_rgb_svp_alpha1 (svp2, 0, 0, WIDTH, HEIGHT, color2, buffer, ROWSTRIDE, NULL);

    art_free(svp2);
    art_free(svp1);
    art_free(vec);
    art_free(bez1);

    return (unsigned char *) buffer;
}

int main (int argc, char *argv[])
{
    unsigned char *buffer;

    buffer = render_path ();

#if SAVING
    save_buffer (buffer, "foo.png");
#endif

    return 0;
}







