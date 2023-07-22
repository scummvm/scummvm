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
/*  ftgloadr.c                                                             */
/*    The FreeType glyph loader (body).                                    */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftgloadr.h"
#include "engines/ags/lib/freetype-2.1.3/ftmemory.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gloader

namespace AGS3 {
namespace FreeType213 {


/* create a new glyph loader */
FT_BASE_DEF(FT_Error)
FT_GlyphLoader_New(FT_Memory memory, FT_GlyphLoader *aloader) {
	FT_GlyphLoader loader;
	FT_Error error;

	if (!FT_NEW(loader)) {
		loader->memory = memory;
		*aloader = loader;
	}
	return error;
}


/* rewind the glyph loader - reset counters to 0 */
FT_BASE_DEF(void)
FT_GlyphLoader_Rewind(FT_GlyphLoader loader) {
	FT_GlyphLoad base = &loader->base;
	FT_GlyphLoad current = &loader->current;

	base->outline.n_points = 0;
	base->outline.n_contours = 0;
	base->num_subglyphs = 0;

	*current = *base;
}


/* reset the glyph loader, frees all allocated tables */
/* and starts from zero                               */
FT_BASE_DEF(void)
FT_GlyphLoader_Reset(FT_GlyphLoader loader) {
	FT_Memory memory = loader->memory;

	FT2_1_3_FREE(loader->base.outline.points);
	FT2_1_3_FREE(loader->base.outline.tags);
	FT2_1_3_FREE(loader->base.outline.contours);
	FT2_1_3_FREE(loader->base.extra_points);
	FT2_1_3_FREE(loader->base.subglyphs);

	loader->max_points = 0;
	loader->max_contours = 0;
	loader->max_subglyphs = 0;

	FT_GlyphLoader_Rewind(loader);
}


/* delete a glyph loader */
FT_BASE_DEF(void)
FT_GlyphLoader_Done(FT_GlyphLoader loader) {
	if (loader) {
		FT_Memory memory = loader->memory;

		FT_GlyphLoader_Reset(loader);
		FT2_1_3_FREE(loader);
	}
}


/* re-adjust the `current' outline fields */
static void FT_GlyphLoader_Adjust_Points(FT_GlyphLoader loader) {
	FT_Outline *base = &loader->base.outline;
	FT_Outline *current = &loader->current.outline;

	current->points = base->points + base->n_points;
	current->tags = base->tags + base->n_points;
	current->contours = base->contours + base->n_contours;

	/* handle extra points table - if any */
	if (loader->use_extra)
		loader->current.extra_points = loader->base.extra_points + base->n_points;
}


FT_BASE_DEF(FT_Error)
FT_GlyphLoader_CreateExtra(FT_GlyphLoader loader) {
	FT_Error error;
	FT_Memory memory = loader->memory;

	if (!FT_NEW_ARRAY(loader->base.extra_points, loader->max_points)) {
		loader->use_extra = 1;
		FT_GlyphLoader_Adjust_Points(loader);
	}
	return error;
}


/* re-adjust the `current' subglyphs field */
static void FT_GlyphLoader_Adjust_Subglyphs(FT_GlyphLoader loader) {
	FT_GlyphLoad base = &loader->base;
	FT_GlyphLoad current = &loader->current;

	current->subglyphs = base->subglyphs + base->num_subglyphs;
}


/* Ensure that we can add `n_points' and `n_contours' to our glyph. this */
/* function reallocates its outline tables if necessary.  Note that it   */
/* DOESN'T change the number of points within the loader!                */
/*                                                                       */
FT_BASE_DEF(FT_Error)
FT_GlyphLoader_CheckPoints(FT_GlyphLoader loader, FT_UInt n_points, FT_UInt n_contours) {
	FT_Memory memory = loader->memory;
	FT_Error error = FT2_1_3_Err_Ok;
	FT_Outline *base = &loader->base.outline;
	FT_Outline *current = &loader->current.outline;
	FT_Bool adjust = 1;

	FT_UInt new_max, old_max;

	/* check points & tags */
	new_max = base->n_points + current->n_points + n_points;
	old_max = loader->max_points;

	if (new_max > old_max) {
		new_max = (new_max + 7) & -8;

		if (FT2_1_3_RENEW_ARRAY(base->points, old_max, new_max) || FT2_1_3_RENEW_ARRAY(base->tags, old_max, new_max))
			goto Exit;

		if (loader->use_extra && FT2_1_3_RENEW_ARRAY(loader->base.extra_points, old_max, new_max))
			goto Exit;

		adjust = 1;
		loader->max_points = new_max;
	}

	/* check contours */
	old_max = loader->max_contours;
	new_max = base->n_contours + current->n_contours + n_contours;
	if (new_max > old_max) {
		new_max = (new_max + 3) & -4;
		if (FT2_1_3_RENEW_ARRAY(base->contours, old_max, new_max))
			goto Exit;

		adjust = 1;
		loader->max_contours = new_max;
	}

	if (adjust)
		FT_GlyphLoader_Adjust_Points(loader);

Exit:
	return error;
}


/* Ensure that we can add `n_subglyphs' to our glyph. this function */
/* reallocates its subglyphs table if necessary.  Note that it DOES */
/* NOT change the number of subglyphs within the loader!            */
/*                                                                  */
FT_BASE_DEF(FT_Error)
FT_GlyphLoader_CheckSubGlyphs(FT_GlyphLoader loader, FT_UInt n_subs) {
	FT_Memory memory = loader->memory;
	FT_Error error = FT2_1_3_Err_Ok;
	FT_UInt new_max, old_max;

	FT_GlyphLoad base = &loader->base;
	FT_GlyphLoad current = &loader->current;

	new_max = base->num_subglyphs + current->num_subglyphs + n_subs;
	old_max = loader->max_subglyphs;
	if (new_max > old_max) {
		new_max = (new_max + 1) & -2;
		if (FT2_1_3_RENEW_ARRAY(base->subglyphs, old_max, new_max))
			goto Exit;

		loader->max_subglyphs = new_max;

		FT_GlyphLoader_Adjust_Subglyphs(loader);
	}

Exit:
	return error;
}


/* prepare loader for the addition of a new glyph on top of the base one */
FT_BASE_DEF(void)
FT_GlyphLoader_Prepare(FT_GlyphLoader loader) {
	FT_GlyphLoad current = &loader->current;

	current->outline.n_points = 0;
	current->outline.n_contours = 0;
	current->num_subglyphs = 0;

	FT_GlyphLoader_Adjust_Points(loader);
	FT_GlyphLoader_Adjust_Subglyphs(loader);
}


/* add current glyph to the base image - and prepare for another */
FT_BASE_DEF(void)
FT_GlyphLoader_Add(FT_GlyphLoader loader) {
	FT_GlyphLoad base = &loader->base;
	FT_GlyphLoad current = &loader->current;

	FT_UInt n_curr_contours = current->outline.n_contours;
	FT_UInt n_base_points = base->outline.n_points;
	FT_UInt n;

	base->outline.n_points = (short)(base->outline.n_points + current->outline.n_points);
	base->outline.n_contours = (short)(base->outline.n_contours + current->outline.n_contours);

	base->num_subglyphs += current->num_subglyphs;

	/* adjust contours count in newest outline */
	for (n = 0; n < n_curr_contours; n++)
		current->outline.contours[n] =
			(short)(current->outline.contours[n] + n_base_points);

	/* prepare for another new glyph image */
	FT_GlyphLoader_Prepare(loader);
}


FT_BASE_DEF(FT_Error)
FT_GlyphLoader_CopyPoints(FT_GlyphLoader target, FT_GlyphLoader source) {
	FT_Error error;
	FT_UInt num_points = source->base.outline.n_points;
	FT_UInt num_contours = source->base.outline.n_contours;

	error = FT_GlyphLoader_CheckPoints(target, num_points, num_contours);
	if (!error) {
		FT_Outline *out = &target->base.outline;
		FT_Outline *in = &source->base.outline;

		FT2_1_3_MEM_COPY(out->points, in->points, num_points * sizeof(FT_Vector));
		FT2_1_3_MEM_COPY(out->tags, in->tags, num_points * sizeof(char));
		FT2_1_3_MEM_COPY(out->contours, in->contours, num_contours * sizeof(short));

		/* do we need to copy the extra points? */
		if (target->use_extra && source->use_extra)
			FT2_1_3_MEM_COPY(target->base.extra_points, source->base.extra_points, num_points * sizeof(FT_Vector));

		out->n_points = (short)num_points;
		out->n_contours = (short)num_contours;

		FT_GlyphLoader_Adjust_Points(target);
	}

	return error;
}

} // End of namespace FreeType213
} // End of namespace AGS3
