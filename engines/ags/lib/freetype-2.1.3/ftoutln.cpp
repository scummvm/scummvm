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

/***************************************************************************/
/*                                                                         */
/*  ftoutln.c                                                              */
/*    FreeType outline management (body).                                  */
/*                                                                         */
/***************************************************************************/

#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftoutln.h"
#include "engines/ags/lib/freetype-2.1.3/ftobjs.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_outline

namespace AGS3 {
namespace FreeType213 {


static const FT_Outline null_outline = {0, 0, 0, 0, 0, 0};

FT_EXPORT_DEF(FT_Error)
FT_Outline_Decompose(FT_Outline *outline, const FT_Outline_Funcs *func_interface, void *user) {
#undef SCALED
#define SCALED(x) (((x) << shift) - delta)

	FT_Vector v_last;
	FT_Vector v_control;
	FT_Vector v_start;

	FT_Vector *point;
	FT_Vector *limit;
	char *tags;

	FT_Error error;

	FT_Int n;      /* index of contour in outline     */
	FT_UInt first; /* index of first point in contour */
	FT_Int tag;    /* current point's state           */

	FT_Int shift;
	FT_Pos delta;

	if (!outline || !func_interface)
		return FT2_1_3_Err_Invalid_Argument;

	shift = func_interface->shift;
	delta = func_interface->delta;
	first = 0;

	for (n = 0; n < outline->n_contours; n++) {
		FT_Int last; /* index of last point in contour */

		last = outline->contours[n];
		limit = outline->points + last;

		v_start = outline->points[first];
		v_last = outline->points[last];

		v_start.x = SCALED(v_start.x);
		v_start.y = SCALED(v_start.y);
		v_last.x = SCALED(v_last.x);
		v_last.y = SCALED(v_last.y);

		v_control = v_start;

		point = outline->points + first;
		tags = outline->tags + first;
		tag = FT_CURVE_TAG(tags[0]);

		/* A contour cannot start with a cubic control point! */
		if (tag == FT_CURVE_TAG_CUBIC)
			goto Invalid_Outline;

		/* check first point to determine origin */
		if (tag == FT_CURVE_TAG_CONIC) {
			/* first point is conic control.  Yes, this happens. */
			if (FT_CURVE_TAG(outline->tags[last]) == FT_CURVE_TAG_ON) {
				/* start at last point if it is on the curve */
				v_start = v_last;
				limit--;
			} else {
				/* if both first and last points are conic,         */
				/* start at their middle and record its position    */
				/* for closure                                      */
				v_start.x = (v_start.x + v_last.x) / 2;
				v_start.y = (v_start.y + v_last.y) / 2;

				v_last = v_start;
			}
			point--;
			tags--;
		}

		error = func_interface->move_to(&v_start, user);
		if (error)
			goto Exit;

		while (point < limit) {
			point++;
			tags++;

			tag = FT_CURVE_TAG(tags[0]);
			switch (tag) {
			case FT_CURVE_TAG_ON: { /* emit a single line_to */
				FT_Vector vec;

				vec.x = SCALED(point->x);
				vec.y = SCALED(point->y);

				error = func_interface->line_to(&vec, user);
				if (error)
					goto Exit;
				continue;
			}

			case FT_CURVE_TAG_CONIC: /* consume conic arcs */
				v_control.x = SCALED(point->x);
				v_control.y = SCALED(point->y);

			Do_Conic:
				if (point < limit) {
					FT_Vector vec;
					FT_Vector v_middle;

					point++;
					tags++;
					tag = FT_CURVE_TAG(tags[0]);

					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);

					if (tag == FT_CURVE_TAG_ON) {
						error = func_interface->conic_to(&v_control, &vec, user);
						if (error)
							goto Exit;
						continue;
					}

					if (tag != FT_CURVE_TAG_CONIC)
						goto Invalid_Outline;

					v_middle.x = (v_control.x + vec.x) / 2;
					v_middle.y = (v_control.y + vec.y) / 2;

					error = func_interface->conic_to(&v_control, &v_middle, user);
					if (error)
						goto Exit;

					v_control = vec;
					goto Do_Conic;
				}

				error = func_interface->conic_to(&v_control, &v_start, user);
				goto Close;

			default: { /* FT_CURVE_TAG_CUBIC */
				FT_Vector vec1, vec2;

				if (point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC)
					goto Invalid_Outline;

				point += 2;
				tags += 2;

				vec1.x = SCALED(point[-2].x);
				vec1.y = SCALED(point[-2].y);
				vec2.x = SCALED(point[-1].x);
				vec2.y = SCALED(point[-1].y);

				if (point <= limit) {
					FT_Vector vec;

					vec.x = SCALED(point->x);
					vec.y = SCALED(point->y);

					error = func_interface->cubic_to(&vec1, &vec2, &vec, user);
					if (error)
						goto Exit;
					continue;
				}

				error = func_interface->cubic_to(&vec1, &vec2, &v_start, user);
				goto Close;
			}
			}
		}

		/* close the contour with a line segment */
		error = func_interface->line_to(&v_start, user);

	Close:
		if (error)
			goto Exit;

		first = last + 1;
	}

	return 0;

Exit:
	return error;

Invalid_Outline:
	return FT2_1_3_Err_Invalid_Outline;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_New_Internal(FT_Memory memory, FT_UInt numPoints, FT_Int numContours, FT_Outline *anoutline) {
	FT_Error error;

	if (!anoutline || !memory)
		return FT2_1_3_Err_Invalid_Argument;

	*anoutline = null_outline;

	if (FT_NEW_ARRAY(anoutline->points, numPoints * 2L) || FT_NEW_ARRAY(anoutline->tags, numPoints) || FT_NEW_ARRAY(anoutline->contours, numContours))
		goto Fail;

	anoutline->n_points = (FT_UShort)numPoints;
	anoutline->n_contours = (FT_Short)numContours;
	anoutline->flags |= FT_OUTLINE_OWNER;

	return FT2_1_3_Err_Ok;

Fail:
	anoutline->flags |= FT_OUTLINE_OWNER;
	FT_Outline_Done_Internal(memory, anoutline);

	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_New(FT_Library library, FT_UInt numPoints, FT_Int numContours, FT_Outline *anoutline) {
	if (!library)
		return FT2_1_3_Err_Invalid_Library_Handle;

	return FT_Outline_New_Internal(library->memory, numPoints, numContours, anoutline);
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Check(FT_Outline *outline) {
	if (outline) {
		FT_Int n_points = outline->n_points;
		FT_Int n_contours = outline->n_contours;
		FT_Int end0, end;
		FT_Int n;

		/* empty glyph? */
		if (n_points == 0 && n_contours == 0)
			return 0;

		/* check point and contour counts */
		if (n_points <= 0 || n_contours <= 0)
			goto Bad;

		end0 = end = -1;
		for (n = 0; n < n_contours; n++) {
			end = outline->contours[n];

			/* note that we don't accept empty contours */
			if (end <= end0 || end >= n_points)
				goto Bad;

			end0 = end;
		}

		if (end != n_points - 1)
			goto Bad;

		/* XXX: check the tags array */
		return 0;
	}

Bad:
	return FT2_1_3_Err_Invalid_Argument;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Copy(FT_Outline *source, FT_Outline *target) {
	FT_Int is_owner;

	if (!source || !target || source->n_points != target->n_points || source->n_contours != target->n_contours)
		return FT2_1_3_Err_Invalid_Argument;

	FT2_1_3_MEM_COPY(target->points, source->points, source->n_points * sizeof(FT_Vector));
	FT2_1_3_MEM_COPY(target->tags, source->tags, source->n_points * sizeof(FT_Byte));
	FT2_1_3_MEM_COPY(target->contours, source->contours, source->n_contours * sizeof(FT_Short));

	/* copy all flags, except the `FT_OUTLINE_OWNER' one */
	is_owner = target->flags & FT_OUTLINE_OWNER;
	target->flags = source->flags;

	target->flags &= ~FT_OUTLINE_OWNER;
	target->flags |= is_owner;

	return FT2_1_3_Err_Ok;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Done_Internal(FT_Memory memory, FT_Outline *outline) {
	if (outline) {
		if (outline->flags & FT_OUTLINE_OWNER) {
			FT2_1_3_FREE(outline->points);
			FT2_1_3_FREE(outline->tags);
			FT2_1_3_FREE(outline->contours);
		}
		*outline = null_outline;

		return FT2_1_3_Err_Ok;
	} else
		return FT2_1_3_Err_Invalid_Argument;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Done(FT_Library library, FT_Outline *outline) {
	/* check for valid `outline' in FT_Outline_Done_Internal() */

	if (!library)
		return FT2_1_3_Err_Invalid_Library_Handle;

	return FT_Outline_Done_Internal(library->memory, outline);
}

FT_EXPORT_DEF(void)
FT_Outline_Get_CBox(FT_Outline *outline, FT_BBox *acbox) {
	FT_Pos xMin, yMin, xMax, yMax;

	if (outline && acbox) {
		if (outline->n_points == 0) {
			xMin = 0;
			yMin = 0;
			xMax = 0;
			yMax = 0;
		} else {
			FT_Vector *vec = outline->points;
			FT_Vector *limit = vec + outline->n_points;

			xMin = xMax = vec->x;
			yMin = yMax = vec->y;
			vec++;

			for (; vec < limit; vec++) {
				FT_Pos x, y;

				x = vec->x;
				if (x < xMin)
					xMin = x;
				if (x > xMax)
					xMax = x;

				y = vec->y;
				if (y < yMin)
					yMin = y;
				if (y > yMax)
					yMax = y;
			}
		}
		acbox->xMin = xMin;
		acbox->xMax = xMax;
		acbox->yMin = yMin;
		acbox->yMax = yMax;
	}
}

FT_EXPORT_DEF(void)
FT_Outline_Translate(FT_Outline *outline, FT_Pos xOffset, FT_Pos yOffset) {
	FT_UShort n;
	FT_Vector *vec = outline->points;

	for (n = 0; n < outline->n_points; n++) {
		vec->x += xOffset;
		vec->y += yOffset;
		vec++;
	}
}

FT_EXPORT_DEF(void)
FT_Outline_Reverse(FT_Outline *outline) {
	FT_UShort n;
	FT_Int first, last;

	first = 0;

	for (n = 0; n < outline->n_contours; n++) {
		last = outline->contours[n];

		/* reverse point table */
		{
			FT_Vector *p = outline->points + first;
			FT_Vector *q = outline->points + last;
			FT_Vector swap;

			while (p < q) {
				swap = *p;
				*p = *q;
				*q = swap;
				p++;
				q--;
			}
		}

		/* reverse tags table */
		{
			char *p = outline->tags + first;
			char *q = outline->tags + last;
			char swap;

			while (p < q) {
				swap = *p;
				*p = *q;
				*q = swap;
				p++;
				q--;
			}
		}

		first = last + 1;
	}

	outline->flags ^= FT_OUTLINE_REVERSE_FILL;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Render(FT_Library library, FT_Outline *outline, FT_Raster_Params *params) {
	FT_Error error;
	FT_Bool update = 0;
	FT_Renderer renderer;
	FT_ListNode node;

	if (!library)
		return FT2_1_3_Err_Invalid_Library_Handle;

	if (!params)
		return FT2_1_3_Err_Invalid_Argument;

	renderer = library->cur_renderer;
	node = library->renderers.head;

	params->source = (void *)outline;

	error = FT2_1_3_Err_Cannot_Render_Glyph;
	while (renderer) {
		error = renderer->raster_render(renderer->raster, params);
		if (!error || FT_ERROR_BASE(error) != FT2_1_3_Err_Cannot_Render_Glyph)
			break;

		/* FT2_1_3_Err_Cannot_Render_Glyph is returned if the render mode   */
		/* is unsupported by the current renderer for this glyph image */
		/* format                                                      */

		/* now, look for another renderer that supports the same */
		/* format                                                */
		renderer = FT_Lookup_Renderer(library, FT_GLYPH_FORMAT_OUTLINE, &node);
		update = 1;
	}

	/* if we changed the current renderer for the glyph image format */
	/* we need to select it as the next current one                  */
	if (!error && update && renderer)
		FT_Set_Renderer(library, renderer, 0, 0);

	return error;
}

FT_EXPORT_DEF(FT_Error)
FT_Outline_Get_Bitmap(FT_Library library, FT_Outline *outline, FT_Bitmap *abitmap) {
	FT_Raster_Params params;

	if (!abitmap)
		return FT2_1_3_Err_Invalid_Argument;

	/* other checks are delayed to FT_Outline_Render() */

	params.target = abitmap;
	params.flags = 0;

	if (abitmap->pixel_mode == FT_PIXEL_MODE_GRAY || abitmap->pixel_mode == FT_PIXEL_MODE_LCD || abitmap->pixel_mode == FT_PIXEL_MODE_LCD_V)
		params.flags |= FT_RASTER_FLAG_AA;

	return FT_Outline_Render(library, outline, &params);
}

FT_EXPORT_DEF(void)
FT2_1_3_Vector_Transform(FT_Vector *vector, FT_Matrix *matrix) {
	FT_Pos xz, yz;

	if (!vector || !matrix)
		return;

	xz = FT2_1_3_MulFix(vector->x, matrix->xx) +
		 FT2_1_3_MulFix(vector->y, matrix->xy);

	yz = FT2_1_3_MulFix(vector->x, matrix->yx) +
		 FT2_1_3_MulFix(vector->y, matrix->yy);

	vector->x = xz;
	vector->y = yz;
}

FT_EXPORT_DEF(void)
FT_Outline_Transform(FT_Outline *outline, FT_Matrix *matrix) {
	FT_Vector *vec 	 = outline->points;
	FT_Vector *limit = vec + outline->n_points;

	for (; vec < limit; vec++)
		FT2_1_3_Vector_Transform(vec, matrix);
}

} // End of namespace FreeType213
} // End of namespace AGS3
