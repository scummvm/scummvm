#include "ags/lib/allegro/rotate.h"
#include "ags/lib/allegro/gfx.h"
#include "common/scummsys.h"

namespace AGS3 {

/* rotate_scale_coordinates:
 *  Calculates the coordinates for the rotated, scaled and flipped sprite,
 *  and passes them on to the given function.
 */
void rotate_scale_coordinates(fixed w, fixed h,
					fixed x, fixed y, fixed cx, fixed cy,
					fixed angle,
					fixed scale_x, fixed scale_y,
					fixed xs[4], fixed ys[4])
{
	// Setting angle to the range -180...180 degrees makes sin & cos more numerically stable.
	// (Yes, this does have an effect for big angles!)
	// Note that using "real" sin() and cos() gives much better precision than fixsin() and fixcos().
	angle = angle & 0xffffff;
	if (angle >= 0x800000)
		angle -= 0x1000000;

	double angle_radian = angle * (M_PI / (double)0x800000);
	double sin_angle = sin(angle_radian);
	double cos_angle = cos(angle_radian);

	fixed fix_cos, fix_sin;
	if (cos_angle >= 0)
		fix_cos = (int)(cos_angle * 0x10000 + 0.5);
	else
		fix_cos = (int)(cos_angle * 0x10000 - 0.5);
	if (sin_angle >= 0)
		fix_sin = (int)(sin_angle * 0x10000 + 0.5);
	else
		fix_sin = (int)(sin_angle * 0x10000 - 0.5);

	/* Decide what order to take corners in. */
	int tl = 0, tr = 1, bl = 3, br = 2;

	/* Calculate new coordinates of all corners. */
	w = fixmul(w, scale_x);
	h = fixmul(h, scale_y);
	cx = fixmul(cx, scale_x);
	cy = fixmul(cy, scale_y);

	fixed xofs = x - fixmul(cx, fix_cos) + fixmul(cy, fix_sin);
	fixed yofs = y - fixmul(cx, fix_sin) - fixmul(cy, fix_cos);

	xs[tl] = xofs;
	ys[tl] = yofs;
	xs[tr] = xofs + fixmul(w, fix_cos);
	ys[tr] = yofs + fixmul(w, fix_sin);
	xs[bl] = xofs - fixmul(h, fix_sin);
	ys[bl] = yofs + fixmul(h, fix_cos);

	xs[br] = xs[tr] + xs[bl] - xs[tl];
	ys[br] = ys[tr] + ys[bl] - ys[tl];
}

/* parallelogram_map:
 *  Worker routine for drawing rotated and/or scaled and/or flipped sprites:
 *  It actually maps the sprite to any parallelogram-shaped area of the
 *  bitmap. The top left corner is mapped to (xs[0], ys[0]), the top right to
 *  (xs[1], ys[1]), the bottom right to x (xs[2], ys[2]), and the bottom left
 *  to (xs[3], ys[3]). The corners are assumed to form a perfect
 *  parallelogram, i.e. xs[0]+xs[2] = xs[1]+xs[3]. The corners are given in
 *  fixed point format, so xs[] and ys[] are coordinates of the outer corners
 *  of corner pixels in clockwise order beginning with top left.
 *  All coordinates begin with 0 in top left corner of pixel (0, 0). So a
 *  rotation by 0 degrees of a sprite to the top left of a bitmap can be
 *  specified with coordinates (0, 0) for the top left pixel in source
 *  bitmap. With the default scanline drawer, a pixel in the destination
 *  bitmap is drawn if and only if its center is covered by any pixel in the
 *  sprite. The color of this covering sprite pixel is used to draw.
 */
void parallelogram_map(BITMAP *bmp, const BITMAP *spr, fixed xs[4], fixed ys[4]) {
	// Get index of topmost point.
	int top_index = 0;
	if (ys[1] < ys[0])
		top_index = 1;
	if (ys[2] < ys[top_index])
		top_index = 2;
	if (ys[3] < ys[top_index])
		top_index = 3;

	// Get direction of points: clockwise or anti-clockwise.
	int right_index = (double)(xs[(top_index+1) & 3] - xs[top_index]) *
		(double)(ys[(top_index-1) & 3] - ys[top_index]) >
		(double)(xs[(top_index-1) & 3] - xs[top_index]) *
		(double)(ys[(top_index+1) & 3] - ys[top_index]) ? 1 : -1;

	// Get coordinates of the corners.
	// Coordinates in bmp and sprite ordered as top-right-bottom-left.
	fixed corner_bmp_x[4], corner_bmp_y[4];
	fixed corner_spr_x[4], corner_spr_y[4];
	int index = top_index;
	for (int i = 0; i < 4; i++) {
		corner_bmp_x[i] = xs[index];
		corner_bmp_y[i] = ys[index];
		if (index < 2)
			corner_spr_y[i] = 0;
		else
			// Need `- 1' since otherwise it would be outside sprite.
			corner_spr_y[i] = (spr->h << 16) - 1;
		if ((index == 0) || (index == 3))
			corner_spr_x[i] = 0;
		else
			corner_spr_x[i] = (spr->w << 16) - 1;
		index = (index + right_index) & 3;
	}

	// Get scanline starts, ends and deltas, and clipping coordinates.
	#define top_bmp_y    corner_bmp_y[0]
	#define right_bmp_y  corner_bmp_y[1]
	#define bottom_bmp_y corner_bmp_y[2]
	#define left_bmp_y   corner_bmp_y[3]
	#define top_bmp_x    corner_bmp_x[0]
	#define right_bmp_x  corner_bmp_x[1]
	#define bottom_bmp_x corner_bmp_x[2]
	#define left_bmp_x   corner_bmp_x[3]
	#define top_spr_y    corner_spr_y[0]
	#define right_spr_y  corner_spr_y[1]
	#define bottom_spr_y corner_spr_y[2]
	#define left_spr_y   corner_spr_y[3]
	#define top_spr_x    corner_spr_x[0]
	#define right_spr_x  corner_spr_x[1]
	#define bottom_spr_x corner_spr_x[2]
	#define left_spr_x   corner_spr_x[3]

	// Calculate left and right clipping.
	fixed clip_left, clip_right;
	if (bmp->clip) {
		clip_left = bmp->cl << 16;
		clip_right = (bmp->cr << 16) - 1;
	} else {
		clip_left = 0;
		clip_right = (bmp->w << 16) - 1;
	}

	// Stop if we're totally outside.
	if ((left_bmp_x > clip_right) && (top_bmp_x > clip_right) && (bottom_bmp_x > clip_right))
		return;
	if ((right_bmp_x < clip_left) && (top_bmp_x < clip_left) && (bottom_bmp_x < clip_left))
		return;

	// Bottom clipping.
	int clip_bottom_i = (bottom_bmp_y + 0x8000) >> 16;
	if (bmp->clip) {
		if (clip_bottom_i > bmp->cb)
			clip_bottom_i = bmp->cb;
	}

	// Calculate y coordinate of first scanline.
	int bmp_y_i = (top_bmp_y + 0x8000) >> 16;
	if (bmp->clip) {
		if (bmp_y_i < bmp->ct)
			bmp_y_i = bmp->ct;
	}

	// Sprite is above or below bottom clipping area.
	if (bmp_y_i >= clip_bottom_i)
		return;

	// Vertical gap between top corner and centre of topmost scanline.
	fixed extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - top_bmp_y;
	// Calculate x coordinate of beginning of scanline in bmp.
	fixed l_bmp_dx = fixdiv(left_bmp_x - top_bmp_x, left_bmp_y - top_bmp_y);
	fixed l_bmp_x = top_bmp_x + fixmul(extra_scanline_fraction, l_bmp_dx);
	// Calculate x coordinate of beginning of scanline in spr.
	// note: all these are rounded down which is probably a Good Thing (tm)
	fixed l_spr_dx = fixdiv(left_spr_x - top_spr_x, left_bmp_y - top_bmp_y);
	fixed l_spr_x = top_spr_x + fixmul(extra_scanline_fraction, l_spr_dx);
	// Calculate y coordinate of beginning of scanline in spr.
	fixed l_spr_dy = fixdiv(left_spr_y - top_spr_y, left_bmp_y - top_bmp_y);
	fixed l_spr_y = top_spr_y + fixmul(extra_scanline_fraction, l_spr_dy);

	// Calculate left loop bound.
	int l_bmp_y_bottom_i = (left_bmp_y + 0x8000) >> 16;
	if (l_bmp_y_bottom_i > clip_bottom_i)
		l_bmp_y_bottom_i = clip_bottom_i;

	// Calculate x coordinate of end of scanline in bmp.
	fixed r_bmp_dx = fixdiv(right_bmp_x - top_bmp_x, right_bmp_y - top_bmp_y);
	fixed r_bmp_x = top_bmp_x + fixmul(extra_scanline_fraction, r_bmp_dx);

	// Calculate right loop bound.
	int r_bmp_y_bottom_i = (right_bmp_y + 0x8000) >> 16;

	// Get dx and dy, the offsets to add to the source coordinates as we move
	// one pixel rightwards along a scanline. This formula can be derived by
	// considering the 2x2 matrix that transforms the sprite to the
	// parallelogram.
	// We'd better use double to get this as exact as possible, since any
	// errors will be accumulated along the scanline.
	fixed spr_dx = (fixed)((ys[3] - ys[0]) * 65536.0 * (65536.0 * spr->w) /
		((xs[1] - xs[0]) * (double)(ys[3] - ys[0]) - (xs[3] - xs[0]) * (double)(ys[1] - ys[0])));
	fixed spr_dy = (fixed)((ys[1] - ys[0]) * 65536.0 * (65536.0 * spr->h) /
		((xs[3] - xs[0]) * (double)(ys[1] - ys[0]) - (xs[1] - xs[0]) * (double)(ys[3] - ys[0])));

	bool sameFormat = (spr->format == bmp->format);
	uint32 transColor = 0, alphaMask = 0xff;
	if (spr->format.bytesPerPixel != 1) {
		transColor = spr->format.ARGBToColor(0, 255, 0, 255);
		alphaMask = spr->format.ARGBToColor(255, 0, 0, 0);
		alphaMask = ~alphaMask;
	}

	// Loop through scanlines.
	while (1) {
		// Has beginning of scanline passed a corner?
		if (bmp_y_i >= l_bmp_y_bottom_i) {
			// Are we done?
			if (bmp_y_i >= clip_bottom_i)
				break;

			// Vertical gap between left corner and centre of scanline.
			extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - left_bmp_y;
			// Update x coordinate of beginning of scanline in bmp.
			l_bmp_dx = fixdiv(bottom_bmp_x - left_bmp_x, bottom_bmp_y - left_bmp_y);
			l_bmp_x = left_bmp_x + fixmul(extra_scanline_fraction, l_bmp_dx);
			// Update x coordinate of beginning of scanline in spr.
			l_spr_dx = fixdiv(bottom_spr_x - left_spr_x, bottom_bmp_y - left_bmp_y);
			l_spr_x = left_spr_x + fixmul(extra_scanline_fraction, l_spr_dx);
			// Update y coordinate of beginning of scanline in spr.
			l_spr_dy = fixdiv(bottom_spr_y - left_spr_y, bottom_bmp_y - left_bmp_y);
			l_spr_y = left_spr_y + fixmul(extra_scanline_fraction, l_spr_dy);

			// Update loop bound.
			l_bmp_y_bottom_i = (bottom_bmp_y + 0x8000) >> 16;
			if (l_bmp_y_bottom_i > clip_bottom_i)
				l_bmp_y_bottom_i = clip_bottom_i;
		}

		// Has end of scanline passed a corner?
		if (bmp_y_i >= r_bmp_y_bottom_i) {
			// Vertical gap between right corner and centre of scanline.
			extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - right_bmp_y;
			// Update x coordinate of end of scanline in bmp.
			r_bmp_dx = fixdiv(bottom_bmp_x - right_bmp_x, bottom_bmp_y - right_bmp_y);
			r_bmp_x = right_bmp_x + fixmul(extra_scanline_fraction, r_bmp_dx);

			// Update loop bound: We aren't supposed to use this any more, so
			// just set it to some big enough value.
			r_bmp_y_bottom_i = clip_bottom_i;
		}

		// Make left bmp coordinate be an integer and clip it.
		fixed l_bmp_x_rounded;
		l_bmp_x_rounded = (l_bmp_x + 0x8000) & ~0xffff;
		if (l_bmp_x_rounded < clip_left)
			l_bmp_x_rounded = clip_left;

		// ... and move starting point in sprite accordingly.
		fixed l_spr_x_rounded = l_spr_x + fixmul(l_bmp_x_rounded + 0x7fff - l_bmp_x, spr_dx);
		fixed l_spr_y_rounded = l_spr_y + fixmul(l_bmp_x_rounded + 0x7fff - l_bmp_x, spr_dy);

		// Make right bmp coordinate be an integer and clip it.
		fixed r_bmp_x_rounded = (r_bmp_x - 0x8000) & ~0xffff;
		if (r_bmp_x_rounded > clip_right)
			r_bmp_x_rounded = clip_right;

		// Draw!
		if (l_bmp_x_rounded <= r_bmp_x_rounded) {
			/* The bodies of these ifs are only reached extremely seldom,
				it's an ugly hack to avoid reading outside the sprite when
				the rounding errors are accumulated the wrong way. It would
				be nicer if we could ensure that this never happens by making
				all multiplications and divisions be rounded up or down at
				the correct places.
				I did try another approach: recalculate the edges of the
				scanline from scratch each scanline rather than incrementally.
				Drawing a sprite with that routine took about 25% longer time
				though.
				*/
			if ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w) {
				if (((l_spr_x_rounded < 0) && (spr_dx <= 0)) || ((l_spr_x_rounded > 0) && (spr_dx >= 0))) {
					// This can happen.
					goto skip_draw;
				} else {
					// I don't think this can happen, but I can't prove it.
					do {
						l_spr_x_rounded += spr_dx;
						l_bmp_x_rounded += 65536;
						if (l_bmp_x_rounded > r_bmp_x_rounded)
							goto skip_draw;
					} while ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w);
				}
			}
			int right_edge_test = l_spr_x_rounded + ((r_bmp_x_rounded - l_bmp_x_rounded) >> 16) * spr_dx;
			if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w) {
				if (((right_edge_test < 0) && (spr_dx <= 0)) || ((right_edge_test > 0) && (spr_dx >= 0))) {
					// This can happen.
					do {
						r_bmp_x_rounded -= 65536;
						right_edge_test -= spr_dx;
						if (l_bmp_x_rounded > r_bmp_x_rounded)
							goto skip_draw;
					} while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w);
				} else {
					// I don't think this can happen, but I can't prove it.
					goto skip_draw;
				}
			}
			if ((unsigned)(l_spr_y_rounded >> 16) >= (unsigned)spr->h) {
				if (((l_spr_y_rounded < 0) && (spr_dy <= 0)) || ((l_spr_y_rounded > 0) && (spr_dy >= 0))) {
					// This can happen.
					goto skip_draw;
				} else {
					// I don't think this can happen, but I can't prove it.
					do {
						l_spr_y_rounded += spr_dy;
						l_bmp_x_rounded += 65536;
						if (l_bmp_x_rounded > r_bmp_x_rounded)
							goto skip_draw;
					} while (((unsigned)l_spr_y_rounded >> 16) >= (unsigned)spr->h);
				}
			}
			right_edge_test = l_spr_y_rounded + ((r_bmp_x_rounded - l_bmp_x_rounded) >> 16) * spr_dy;
			if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h) {
				if (((right_edge_test < 0) && (spr_dy <= 0)) || ((right_edge_test > 0) && (spr_dy >= 0))) {
					// This can happen.
					do {
						r_bmp_x_rounded -= 65536;
						right_edge_test -= spr_dy;
						if (l_bmp_x_rounded > r_bmp_x_rounded)
							goto skip_draw;
					} while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h);
				} else {
					// I don't think this can happen, but I can't prove it.
					goto skip_draw;
				}
			}

			// draw scanline
			int r_bmp_x_i = (r_bmp_x_rounded >> 16);
			int l_bmp_x_i = (l_bmp_x_rounded >> 16);
			for (; l_bmp_x_i <= r_bmp_x_i; ++l_bmp_x_i) {
				uint32 c = (uint32)getpixel(spr, l_spr_x_rounded >> 16, l_spr_y_rounded >> 16);
				if ((c & alphaMask) != transColor) {
					if (!sameFormat) {
						uint8 a, r, g, b;
						spr->format.colorToARGB(c, a, r, g, b);
						c = bmp->format.ARGBToColor(a, r, g, b);
					}
					putpixel(bmp, l_bmp_x_i, bmp_y_i, c);
				}
				l_spr_x_rounded += spr_dx;
				l_spr_y_rounded += spr_dy;
			}
		}
		// I'm not going to apoligize for this label and its gotos.
		// to get rid of it would just make the code look worse.
		skip_draw:

		// Jump to next scanline.
		bmp_y_i++;
		// Update beginning of scanline.
		l_bmp_x += l_bmp_dx;
		l_spr_x += l_spr_dx;
		l_spr_y += l_spr_dy;
		// Update end of scanline.
		r_bmp_x += r_bmp_dx;
	}
}

void pivot_scaled_sprite(BITMAP *bmp, const BITMAP *sprite, fixed x, fixed y, fixed cx, fixed cy, fixed angle, fixed scale) {
	fixed xs[4], ys[4];
	rotate_scale_coordinates(sprite->w << 16, sprite->h << 16,
		x, y, cx, cy, angle, scale, scale, xs, ys);
	parallelogram_map(bmp, sprite, xs, ys);
}

} // namespace AGS3
