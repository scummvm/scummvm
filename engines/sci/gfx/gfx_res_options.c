/***************************************************************************
 gfx_res_options.c  Copyright (C) 2002 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <gfx_system.h>
#include <gfx_options.h>
#include <gfx_resmgr.h>

#include <ctype.h>

/*#define DEBUG*/

static gfx_res_pattern_list_t*
pattern_list_insert(gfx_res_pattern_list_t *list, int min, int max)
{
	gfx_res_pattern_list_t *retval = (gfx_res_pattern_list_t*)sci_malloc(sizeof(gfx_res_pattern_list_t));
	retval->pattern.min = min;
	retval->pattern.max = max;
	retval->next = list;

	return retval;
}

static int
pattern_list_len(gfx_res_pattern_list_t *list)
{
	int v = 0;
	while (list) {
		++v;
		list = list->next;
	}

	return v;
}

static void
pattern_list_flatten(gfx_res_pattern_t *dest, gfx_res_pattern_list_t *list)
{
	while (list) {
		*dest++ = list->pattern;
		list = list->next;
	}
}

static inline void
pattern_list_free(gfx_res_pattern_list_t *list)
{
	if (list)
		pattern_list_free(list->next);

	free(list);
}


static inline int
extract_pattern(gfx_res_pattern_list_t **destp,
		char *src, int offset)
{
	char *src_orig = src - offset;
	int final = 0;
	int wildcard = 0;

	while (!final) {
		char *end = strchr(src, ',');
		if (end)
			*end = 0;
		else
			final = 1;

		while (*src && isblank(*src))
			++src;

		if (*src == '*'
		    || *src == '_') {
			wildcard = 1;
			++src;
		} else if (*src == '.' || isdigit(*src)) {
			char *endp;
			int start = strtol(src, &endp, 0);

			if (*src == '.'
			    && src[1] == '.') {
				start = GFX_RES_PATTERN_MIN;
				endp = src;
			}

			src = endp;

			while (*src && isblank(*src))
				++src;

			if (*src) {
				int stop;
				if (*src == '.'
				    && src[1] == '.') {

					src += 2;
					while (*src && isblank(*src))
						++src;

					if (!*src)
						stop = GFX_RES_PATTERN_MAX;
					else if (!isdigit(*src)) {
						if (end)
							*end = ',';
						goto lexical_error_label;
					}

					stop = strtol(src, &endp, 0);
					src = endp;

					*destp = pattern_list_insert(*destp,
								     start, stop);

				} else { /* No ellipsis */
					if (end)
						*end = ',';
					goto lexical_error_label;
				}
			} else /* End of sub-pattern */
				*destp = pattern_list_insert(*destp,
							     start, start);

			while (*src && isblank(*src))
				++src;

			if (*src) {
				if (end)
					*end = ',';
				goto lexical_error_label;
			}

		} else {
			if (end)
				*end = ',';
			sciprintf("[gfx-conf] Unexpected character '%c'\n",
				  *src);
			goto lexical_error_label;
		}
		
		if (!final) {
			*end = ',';
			src = end + 1;
		}
	}

	if (wildcard) {
		pattern_list_free(*destp);
		*destp = NULL;
	}

	return 0;

 lexical_error_label:
	sciprintf("[gfx-conf] Lexical error in pattern at offset %d\n",
		  src - src_orig);
	return 1;
}

static int
extract_mod_rule(char *src, gfx_res_mod_t *rule)
{
	char *orig_src = src;
	char *endp;
	float f[3];
	int i;

	rule->type = GFX_RES_MULTIPLY_FIXED;

	if (isdigit(*src) || *src == '.') {
		f[0] = f[1] = f[2] = strtod(src, &endp);

		if (*endp)
			goto mod_error_label;
	} else if (*src == '(') {
		i = 0;
		++src;

		do {
			while (*src && isblank(*src))
				++src;
			if (!*src || !(isdigit(*src) || *src == '.')) {
				sciprintf("[gfx-conf] Unexpected character '%c'\n",
					  *src);
				goto mod_error_label;
			}
			f[i++] = strtod(src, &endp);

			src = endp;

			while (*src && isblank(*src))
				++src;

			if ((i == 3) && *src != ')') {
				sciprintf("[gfx-conf] Error: Expected ')' at end of modification rule\n");
				goto mod_error_label;
			} else if (i<3 && !isdigit(*src) && *src != '.' && *src != ',') {
				sciprintf("[gfx-conf] Error: Expected ',' as separator in modification rule, not '%c'\n",
					  *src);
				goto mod_error_label;
			}
			++src;
		} while (i < 3);

		if (*src) {
			sciprintf("[gfx-conf] Error: Trailing garbage after modification rule\n");
			goto mod_error_label;
		}

	} else
		goto mod_error_label;

	for (i = 0; i < 3; i++) {
		int v = (int)(f[i] * 16.0);
		rule->mod.factor[i] = (v > 255) ? 255 : v;
	}

	return 0;
 mod_error_label:
	sciprintf("[gfx-conf] Ill-formed modification rule '%s'\n",
		  orig_src);
	return 1;
}


extern gfx_pixmap_color_t gfx_sci0_image_colors[][GFX_SCI0_IMAGE_COLORS_NR];

#define PREDEFINED_PALETTES_NR 4
static int
extract_assign_rule(char *src, gfx_res_assign_t *rule)
{
	/*char *orig_src = src;*/
	struct {
		const char *name;
		int colors_nr;
		gfx_pixmap_color_t *colors;
	} predefined_palettes[PREDEFINED_PALETTES_NR] = {
		{"default", 16, (gfx_pixmap_color_t *) &(gfx_sci0_image_colors[0])},
		{"amiga", 16, (gfx_pixmap_color_t *) &(gfx_sci0_image_colors[1])},
		{"gray", 16, (gfx_pixmap_color_t *) &(gfx_sci0_image_colors[2])},
		{"grey", 16, (gfx_pixmap_color_t *) &(gfx_sci0_image_colors[2])},
	};
	int i;

	rule->type = GFX_RES_ASSIGN_TYPE_PALETTE;

	for (i = 0; i < PREDEFINED_PALETTES_NR; i++)
		if (!strcmp(src, predefined_palettes[i].name)) {
			rule->assign.palette.colors_nr =
				predefined_palettes[i].colors_nr;
			rule->assign.palette.colors =
				predefined_palettes[i].colors;
			return 0;
		}

	sciprintf("[gfx-conf] Unknown palette '%s'\n", src);
	return 1;
	/*
 assign_error_label:
	sciprintf("[gfx-conf] Ill-formed assignment rule '%s'\n",
		  orig_src);
	return 1;
	*/
}

#define CLASSES_NR 3
int
gfx_update_conf(gfx_options_t *options, char *str)
{
	int total_patterns;
	int mod = 0; /* Modifier or assignment rule? */
	char *orig_str = str;
	char *sem_end;

	int fields_nr; /* Number of fields a restriction is possible by.
		       ** cursors:1, pics:2, views:3. */
	struct {
		const char *class_name;
		int class_id;
		int fields_nr;
	} classes[CLASSES_NR] = {
		{"view", GFX_RESOURCE_TYPE_VIEW, 3},
		{"pic", GFX_RESOURCE_TYPE_PIC, 2},
		{"cursor", GFX_RESOURCE_TYPE_CURSOR, 1},
	};
	gfx_res_conf_t *conf = (gfx_res_conf_t*)sci_malloc(sizeof(gfx_res_conf_t));
	gfx_res_pattern_list_t *patterns = NULL;
	gfx_res_pattern_list_t *loops = NULL;
	gfx_res_pattern_list_t *cels = NULL;
	gfx_res_pattern_list_t **fields[3] = {
		&patterns, &loops, &cels
	};
	int i;
	int fieldcnt;
	const char *pat_name_str;

	/* Extract pattern(s) */
	while (*str && isblank(*str))
		++str;

	fields_nr = -1;
	for (i = 0; i < CLASSES_NR; i++) {
		int len = strlen(classes[i].class_name);

		if (!strncmp(str, classes[i].class_name, len)) {
			pat_name_str = classes[i].class_name;
			conf->type = classes[i].class_id;
			fields_nr = classes[i].fields_nr;
			str += len;
			break;
		}
	}

	if (fields_nr == -1) {
		sciprintf("[gfx-conf] Unexpected pattern class: Expected one of 'view', 'pic', 'cursor'\n");
		goto failure_label;
	}

	fieldcnt = 0;
	do {
		while (*str && isblank(*str))
			++str;

		if (!*str)
			goto unexpected_end;

		if (*str == '='
		    || *str == '*')
			break;

		if (*str == '(') {
			char *end = strchr(str, ')');

			if (fieldcnt >= fields_nr) {
				sciprintf("[gfx-conf] Error: Patterns of class '%s' may only be"
					  " constrained by %d arguments\n",
					  pat_name_str, fields_nr);
				goto failure_label;
			}

			if (!end) {
				sciprintf("[gfx-conf] Unmatched parentheses at offset %d\n",
					  str - orig_str);
				goto failure_label;
			}
			*end = 0;

			if (extract_pattern(fields[fieldcnt++],
					    str + 1,
					    str + 1 - orig_str))
				goto failure_label;

			*end = ')';
			str = end + 1;

			continue;
		}

		sciprintf("[gfx-conf] Lexical error in pattern at offset %d: Unexpected '%c'\n",
			  str - orig_str, *str);
		goto failure_label;
	} while (1);
		    

	/* Flatten patterns */
	conf->patterns = NULL;
	total_patterns = conf->patterns_nr = pattern_list_len(patterns);
	total_patterns += (conf->loops_nr = pattern_list_len(loops));
	total_patterns += (conf->cels_nr = pattern_list_len(cels));

	conf->patterns = (gfx_res_pattern_t*)sci_malloc(1 + (sizeof(gfx_res_pattern_t) * total_patterns));
	pattern_list_flatten(conf->patterns, patterns);
	pattern_list_flatten(conf->patterns + conf->patterns_nr, loops);
	pattern_list_flatten(conf->patterns + conf->patterns_nr + conf->loops_nr, cels);

	pattern_list_free(patterns);
	patterns = NULL;
	pattern_list_free(loops);
	loops = NULL;
	pattern_list_free(cels);
	cels = NULL;

	/* Parse remainder */
	if (*str == '*') {
		mod = 1;
		++str;
	}

	if (*str != '=') {
		sciprintf("[gfx-conf] Expected '='\n");
		goto failure_label;
	}

	do { ++str; }
	while (*str && isblank(*str));

	sem_end = strchr(str, ';');
	if (!sem_end) {
		sciprintf("[gfx-conf] Expected ';' at end of rule\n");
		goto failure_label;
	}
	do { *sem_end-- = 0; }
	while (sem_end >= str
	       && isblank(*sem_end));

	if (mod) {
		if (extract_mod_rule(str, &conf->conf.mod))
			goto failure_label;
	} else {
		if (extract_assign_rule(str, &conf->conf.assign))
			goto failure_label;
	}

	/* Write back into options */
	if (mod) {
		conf->next = options->res_conf.mod[conf->type];
		options->res_conf.mod[conf->type] = conf;
	} else {
		conf->next = options->res_conf.assign[conf->type];
		options->res_conf.assign[conf->type] = conf;
	}

	return 0;

	/* Error handling */
unexpected_end:
	sciprintf("[gfx-conf] Unexpected end of pattern encountered\n");
 failure_label:
	sciprintf("[gfx-conf] Error occured in: '%s'\n", orig_str);
	pattern_list_free(patterns);
	pattern_list_free(loops);
	pattern_list_free(cels);
	if (conf->patterns)
		free(conf->patterns);
	free(conf);
	return 1;
}

static inline int
matches_patternlist(gfx_res_pattern_t *patterns, int nr, int val)
{
	int i;
	for (i = 0; i < nr; i++)
		if (patterns[i].min <= val
		    && patterns[i].max >= val)
			return 1;

	return 0;
}

#ifdef DEBUG
static void
print_pattern(gfx_res_pattern_t *pat)
{
	fprintf(stderr, "[%d..%d]",
		pat->min, pat->max);
}
#endif

static inline int
resource_matches_patternlists(gfx_res_conf_t *conf,
			      int type, int nr, int loop, int cel)
{
	int loc;
#ifdef DEBUG
	int i;
	fprintf(stderr, "[DEBUG:gfx-res] Trying to match against %d/%d/%d choices\n",
		conf->patterns_nr, conf->loops_nr, conf->cels_nr);
	for (i = 0; i < conf->patterns_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Pat #%d: ", i);
		print_pattern(conf->patterns + i);
		fprintf(stderr, "\n");
	}
	loc = conf->patterns_nr;
	for (i = 0; i < conf->loops_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Loop #%d: ", i);
		print_pattern(conf->patterns + i + loc);
		fprintf(stderr, "\n");
	}
	loc += conf->loops_nr;
	for (i = 0; i < conf->cels_nr; i++) {
		fprintf(stderr, "[DEBUG:gfx-res] Cel #%d: ", i);
		print_pattern(conf->patterns + i + loc);
		fprintf(stderr, "\n");
	}
#endif
	if (conf->patterns_nr &&
	    !matches_patternlist(conf->patterns,
				 conf->patterns_nr,
				 nr))
		return 0;

	if (type == GFX_RESOURCE_TYPE_CURSOR)
		return 1;

	/* Otherwise, we must match at least the loop (pic)
	** and, for views, the cel as well  */
	loc = conf->patterns_nr;
	if (conf->loops_nr &&
	    !matches_patternlist(conf->patterns + loc,
				 conf->loops_nr,
				 loop))
		return 0;

	if (type != GFX_RESOURCE_TYPE_VIEW)
		return 1;

	loc += conf->loops_nr;

	if (!conf->cels_nr)
		return 1;
	
	return matches_patternlist(conf->patterns + loc,
				   conf->cels_nr,
				   cel);
}

static inline gfx_res_conf_t *
find_match(gfx_res_conf_t *conflist,
	   int type, int nr, int loop, int cel)
{
	while (conflist) {
		if (resource_matches_patternlists(conflist,
						  type, nr, loop, cel)) {
#ifdef DEBUG
			fprintf(stderr, "[DEBUG:gfx-res] Found match!\n");
#endif
			return conflist;
		}

		conflist = conflist->next;
	}
	return NULL;
}

void
apply_assign(gfx_res_assign_t *conf, gfx_pixmap_t *pxm)
{
	/* Has a dynamically allocated palette? Must clean up */
	if (!(pxm->flags & GFX_PIXMAP_FLAG_EXTERNAL_PALETTE)) {
		if (pxm->colors)
			free(pxm->colors);
		pxm->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	}

	pxm->colors_nr = conf->assign.palette.colors_nr;
	pxm->colors = conf->assign.palette.colors;
}

void
apply_mod(gfx_res_mod_t *mod, gfx_pixmap_t *pxm)
{
	gfx_pixmap_color_t *pal = pxm->colors;
	int i, pal_size = pxm->colors_nr;

	/* Does not have a dynamically allocated palette? Must dup current one */
	if (pxm->flags & GFX_PIXMAP_FLAG_EXTERNAL_PALETTE) {
		int size = sizeof(gfx_pixmap_color_t) * pal_size;
		pxm->colors = (gfx_pixmap_color_t*)sci_malloc(size);
		memcpy(pxm->colors, pal, size);
		pal = pxm->colors;
		pxm->flags &= ~GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
		/* Flag for later deallocation */
	}

	switch (mod->type) {

	case GFX_RES_MULTIPLY_FIXED: {
		for (i = 0; i < pal_size; i++) {
			int v;

#define UPDATE_COL(nm, idx)                        \
			v = pal[i].nm;             \
			v *= mod->mod.factor[idx]; \
			v >>= 4;                   \
			pal[i].nm = (v > 255)? 255 : v;

			UPDATE_COL(r, 0);
			UPDATE_COL(g, 1);
			UPDATE_COL(b, 2);
#undef UPDATE_COL
		}
		break;
	}

	default:
		GFXERROR("Using unexpected visual resource modifier %d\n", mod->type);
	}
}

int
gfx_get_res_config(gfx_options_t *options, gfx_pixmap_t *pxm)
{
	int restype = GFXR_RES_TYPE(pxm->ID);
	int nr = GFXR_RES_NR(pxm->ID);
	int loop = pxm->loop;
	int cel = pxm->cel;

	gfx_res_conf_t *conf;

#ifdef DEBUG
	fprintf(stderr, "[DEBUG:gfx-res] Trying to conf %d/%d/%d/%d (ID=%d)\n",
		restype, nr, loop, cel, pxm->ID);
#endif

	if (pxm->ID < 0 || restype < 0 || restype >= GFX_RESOURCE_TYPES_NR)
		return 1; /* Not appropriate */

	conf = find_match(options->res_conf.assign[restype],
			  restype, nr, loop, cel);

	if (conf)
		apply_assign(&(conf->conf.assign), pxm);

	conf = options->res_conf.mod[restype];
	while (conf) {
		conf = find_match(conf,
				  restype, nr, loop, cel);
		if (conf) {
			apply_mod(&(conf->conf.mod), pxm);
			conf = conf->next;
		}
	}
	fflush(NULL);

	return 0;
}
