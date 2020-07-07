/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Is the whole interpolation and ALR passes thing right?  There's no
 *   documentation on it, and it's not intuitively implemented in Adrift.
 *
 * o Is dissecting HTML tags the right thing to do?
 */

/* Assorted definitions and constants. */
static const sc_uint PRINTFILTER_MAGIC = 0xb4736417;
enum {
	BUFFER_GROW_INCREMENT = 32,
	ITERATION_LIMIT = 32
};
static const sc_char NUL = '\0';
static const sc_char LESSTHAN = '<';
static const sc_char GREATERTHAN = '>';
static const sc_char PERCENT = '%';
static const sc_char *const ENTITY_LESSTHAN = "&lt;",
                            *const ENTITY_GREATERTHAN = "&gt;",
                                   *const ENTITY_PERCENT = "+percent+";
enum {
	ENTITY_LENGTH = 4,
	PERCENT_LENGTH = 9
};
static const sc_char *const ESCAPES = "<>%&+";
static const sc_char *const WHITESPACE = "\t\n\v\f\r ";

/* Trace flag, set before running. */
static sc_bool pf_trace = FALSE;


/*
 * Table tying HTML-like tag strings to enumerated tag types.  Since it's
 * scanned sequentially by strncmp(), it's ordered so that longer strings
 * come before shorter ones.  The <br> tag is missing because this is
 * handled separately, as a simple put of '\n'.
 */
struct sc_html_tags_t {
	const sc_char *const name;
	const sc_int length;
	const sc_int tag;
};

static const sc_html_tags_t HTML_TAGS_TABLE[] = {
	{"bgcolour", 8, SC_TAG_BGCOLOR}, {"bgcolor", 7, SC_TAG_BGCOLOR},
	{"waitkey", 7, SC_TAG_WAITKEY},
	{"center", 6, SC_TAG_CENTER}, {"/center", 7, SC_TAG_ENDCENTER},
	{"centre", 6, SC_TAG_CENTER}, {"/centre", 7, SC_TAG_ENDCENTER},
	{"right", 5, SC_TAG_RIGHT}, {"/right", 6, SC_TAG_ENDRIGHT},
	{"font", 4, SC_TAG_FONT}, {"/font", 5, SC_TAG_ENDFONT},
	{"wait", 4, SC_TAG_WAIT}, {"cls", 3, SC_TAG_CLS},
	{"i", 1, SC_TAG_ITALICS}, {"/i", 2, SC_TAG_ENDITALICS},
	{"b", 1, SC_TAG_BOLD}, {"/b", 2, SC_TAG_ENDBOLD},
	{"u", 1, SC_TAG_UNDERLINE}, {"/u", 2, SC_TAG_ENDUNDERLINE},
	{"c", 1, SC_TAG_COLOR}, {"/c", 2, SC_TAG_ENDCOLOR},
	{NULL, 0, SC_TAG_UNKNOWN}
};

/*
 * Printfilter structure definition.  It defines a buffer for output,
 * associated size and length, a note of any conversion to apply to the next
 * buffered character, and a flag to let the filter ignore incoming text.
 */
struct sc_filter_s {
	sc_uint magic;
	sc_int buffer_length;
	sc_int buffer_allocation;
	sc_char *buffer;
	sc_bool new_sentence;
	sc_bool is_muted;
	sc_bool needs_filtering;
};
typedef sc_filter_s sc_filter_t;


/*
 * pf_is_valid()
 *
 * Return TRUE if pointer is a valid printfilter, FALSE otherwise.
 */
static sc_bool pf_is_valid(sc_filterref_t filter) {
	return filter && filter->magic == PRINTFILTER_MAGIC;
}


/*
 * pf_create()
 *
 * Create and return a new printfilter.
 */
sc_filterref_t pf_create(void) {
	static sc_bool initialized = FALSE;

	sc_filterref_t filter;

	/* On first call only, verify the string lengths in the table. */
	if (!initialized) {
		const sc_html_tags_t *entry;

		/* Compare table lengths with string lengths. */
		for (entry = HTML_TAGS_TABLE; entry->name; entry++) {
			if (entry->length != (sc_int) strlen(entry->name)) {
				sc_fatal("pf_create:"
				         " table string length is wrong for \"%s\"\n",
				         entry->name);
			}
		}

		initialized = TRUE;
	}

	/* Create a new printfilter. */
	filter = (sc_filterref_t)sc_malloc(sizeof(*filter));
	filter->magic = PRINTFILTER_MAGIC;
	filter->buffer_length = 0;
	filter->buffer_allocation = 0;
	filter->buffer = NULL;
	filter->new_sentence = FALSE;
	filter->is_muted = FALSE;
	filter->needs_filtering = FALSE;

	return filter;
}


/*
 * pf_destroy()
 *
 * Destroy a printfilter and free its allocated memory.
 */
void pf_destroy(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	/* Free buffer space, and poison and free the printfilter. */
	sc_free(filter->buffer);
	memset(filter, 0xaa, sizeof(*filter));
	sc_free(filter);
}


/*
 * pf_interpolate_vars()
 *
 * Replace %...% elements in a string by their variable values.  If any
 * variables were interpolated, returns an allocated string with replacements
 * done, otherwise returns NULL.
 *
 * If a %...% element exists that is not a variable, then it's left in as is.
 * Similarly, an unmatched (single) % in a string is also left as is.  There
 * appears to be no facility in the file format for escaping literal '%'
 * characters, and since some games have strings with this character in them,
 * this is probably all that can be done.
 */
static sc_char *pf_interpolate_vars(const sc_char *string, sc_var_setref_t vars) {
	sc_char *buffer, *name;
	const sc_char *cursor;
	const sc_char *marker;
	sc_bool is_interpolated;

	/*
	 * Begin with NULL buffer and name strings for lazy allocation, and clear
	 * interpolation detection flag.
	 */
	buffer = NULL;
	name = NULL;
	is_interpolated = FALSE;

	/* Run through the string looking for variables. */
	marker = string;
	for (cursor = (const sc_char *)strchr(marker, PERCENT);
	        cursor; cursor = (const sc_char *)strchr(marker, PERCENT)) {
		sc_int type;
		sc_vartype_t vt_rvalue;
		sc_char close;

		/*
		 * If not yet allocated, allocate a buffer for the return string and
		 * copy up to the percent character into it; otherwise append to buffer
		 * up to percent character.  And if not yet done, allocate a name
		 * buffer guaranteed long enough.
		 */
		if (!buffer) {
			buffer = (sc_char *)sc_malloc(cursor - marker + 1);
			memcpy(buffer, marker, cursor - marker);
			buffer[cursor - marker] = NUL;
		} else {
			buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + cursor - marker + 1);
			strncat(buffer, marker, cursor - marker);
		}
		if (!name)
			name = (sc_char *)sc_malloc(strlen(string) + 1);

		/*
		 * Get the variable name, and from that, the value.  If we encounter a
		 * mismatched '%' or unknown variable, skip it.
		 */
		if (sscanf(cursor, "%%%[^%]%c", name, &close) != 2
		        || close != PERCENT
		        || !var_get(vars, name, &type, &vt_rvalue)) {
			buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + 2);
			strncat(buffer, cursor, 1);
			marker = cursor + 1;
			continue;
		}

		/* Get variable value and append to the string. */
		switch (type) {
		case VAR_INTEGER: {
			sc_char value[32];

			sprintf(value, "%ld", vt_rvalue.integer);
			buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + strlen(value) + 1);
			strcat(buffer, value);
			break;
		}

		case VAR_STRING:
			buffer = (sc_char *)sc_realloc(buffer,
			                               strlen(buffer) + strlen(vt_rvalue.string) + 1);
			strcat(buffer, vt_rvalue.string);
			break;

		default:
			sc_fatal("pf_interpolate_vars: invalid variable type, %ld\n", type);
		}

		/* Advance over the %...% variable name, and note success. */
		marker = cursor + strlen(name) + 2;
		is_interpolated = TRUE;
	}

	/*
	 * If we allocated a buffer and interpolated into it, append the remainder
	 * of the string.  If we didn't interpolate successfully (the input contained
	 * a rogue '%' character), throw out the buffer as it will be the same as
	 * our input.
	 */
	if (buffer) {
		if (is_interpolated) {
			buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + strlen(marker) + 1);
			strcat(buffer, marker);
		} else {
			sc_free(buffer);
			buffer = NULL;
		}
	}

	/* Clean up, and return either the updated string or NULL. */
	sc_free(name);
	return buffer;
}


/*
 * pf_replace_alr()
 *
 * Helper for pf_replace_alrs().  Replace one ALR found in the string with
 * its equivalent, updating the buffer at the address passed in, including
 * reallocating if necessary.  Return TRUE if the buffer was changed.
 */
static sc_bool pf_replace_alr(const sc_char *string, sc_char **buffer, sc_int alr, sc_prop_setref_t bundle) {
	sc_vartype_t vt_key[3];
	const sc_char *marker, *cursor, *original, *replacement;
	sc_char *buffer_ = *buffer;

	/* Retrieve the ALR original string, set replacement to NULL for now. */
	vt_key[0].string = "ALRs";
	vt_key[1].integer = alr;
	vt_key[2].string = "Original";
	original = prop_get_string(bundle, "S<-sis", vt_key);
	replacement = NULL;

	/* Ignore pathological empty originals. */
	if (original[0] == NUL)
		return FALSE;

	/* Run through the marker string looking for things to replace. */
	marker = string;
	for (cursor = strstr(marker, original);
	        cursor; cursor = strstr(marker, original)) {
		/* Optimize by retrieving the replacement string only on demand. */
		if (!replacement) {
			vt_key[2].string = "Replacement";
			replacement = prop_get_string(bundle, "S<-sis", vt_key);
		}

		/*
		 * If not yet allocated, allocate a buffer for the return string and
		 * copy; else append to the existing buffer: basic copy-on-write.
		 */
		if (!buffer_) {
			buffer_ = (sc_char *)sc_malloc(cursor - marker + strlen(replacement) + 1);
			memcpy(buffer_, marker, cursor - marker);
			buffer_[cursor - marker] = NUL;
			strcat(buffer_, replacement);
		} else {
			buffer_ = (sc_char *)sc_realloc(buffer_, strlen(buffer_) +
			                                cursor - marker + strlen(replacement) + 1);
			strncat(buffer_, marker, cursor - marker);
			strcat(buffer_, replacement);
		}

		/* Advance over the original. */
		marker = cursor + strlen(original);
	}

	/* If any pending text, append it to the buffer. */
	if (replacement) {
		buffer_ = (sc_char *)sc_realloc(buffer_, strlen(buffer_) + strlen(marker) + 1);
		strcat(buffer_, marker);
	}

	/* Write back buffer, and if replacement set, the buffer was altered. */
	*buffer = buffer_;
	return replacement != NULL;
}


/*
 * pf_replace_alrs()
 *
 * Replace any ALRs found in the string with their equivalents.  If any
 * ALRs were replaced, returns an allocated string with replacements done,
 * otherwise returns NULL.
 */
static sc_char *pf_replace_alrs(const sc_char *string, sc_prop_setref_t bundle,
		sc_bool alr_applied[], sc_int alr_count) {
	sc_int index_;
	sc_char *buffer1, *buffer2, **buffer;
	const sc_char *marker;

	/*
	 * Begin with NULL buffers and alternate for lazy allocation.  To avoid a
	 * lot of allocation and copying, we use two buffers to help with repeated
	 * ALR replacement.
	 */
	buffer1 = buffer2 = NULL;
	buffer = &buffer1;

	/* Run through each ALR that exists. */
	marker = string;
	for (index_ = 0; index_ < alr_count; index_++) {
		sc_vartype_t vt_key[3];
		sc_int alr;

		/*
		 * Ignore ALR indexes that have already been applied.  This prevents
		 * endless loops in ALR replacement.
		 */
		if (alr_applied[index_])
			continue;

		/*
		 * Get the actual ALR number for the ALR.  This comes from the index
		 * that we sorted earlier by length of original string.  Try replacing
		 * that ALR in the current marker string.
		 */
		vt_key[0].string = "ALRs2";
		vt_key[1].integer = index_;
		vt_key[2].string = "ALRIndex";
		alr = prop_get_integer(bundle, "I<-sis", vt_key);

		if (pf_replace_alr(marker, buffer, alr, bundle)) {
			/*
			 * The current buffer in use has been altered.  This means that we
			 * have to switch the marker string to the buffer containing the
			 * replacement, and switch 'buffer' to the other one for the next
			 * ALR iteration.
			 */
			marker = *buffer;
			buffer = (buffer == &buffer1) ? &buffer2 : &buffer1;

			/* Discard any content in the buffer switched to above. */
			if (*buffer)
				(*buffer)[0] = NUL;

			/* Note this ALR as "used up", and unavailable for future passes. */
			alr_applied[index_] = TRUE;
		}
	}

	/*
	 * If marker points to one or other of the buffers, that buffer is the
	 * return string, and the other is garbage, and should now be freed (or
	 * was never used, in which case it is NULL).
	 */
	if (marker == buffer1) {
		sc_free(buffer2);
		return buffer1;
	} else if (marker == buffer2) {
		sc_free(buffer1);
		return buffer2;
	} else
		return NULL;
}


/*
 * pf_output_text()
 *
 * Edit the tag-stripped text element passed in, substituting &lt; &gt;
 * +percent+ with < > %, then send to the OS-specific output functions.
 */
static void pf_output_text(const sc_char *string) {
	sc_int index_, b_index;
	sc_char *buffer;

	/* Optimize away the allocation and copy if possible. */
	if (!(strstr(string, ENTITY_LESSTHAN)
	        || strstr(string, ENTITY_GREATERTHAN)
	        || strstr(string, ENTITY_PERCENT))) {
		if_print_string(string);
		return;
	}

	/*
	 * Copy characters from the string into the buffer, replacing any &..;
	 * elements by their single-character equivalents.  We also replace any
	 * +percent+ elements by percent characters; apparently an undocumented
	 * Adrift Runner extension.
	 */
	buffer = (sc_char *)sc_malloc(strlen(string) + 1);
	for (index_ = 0, b_index = 0;
	        string[index_] != NUL; index_++, b_index++) {
		if (sc_strncasecmp(string + index_,
		                   ENTITY_LESSTHAN, ENTITY_LENGTH) == 0) {
			buffer[b_index] = LESSTHAN;
			index_ += ENTITY_LENGTH - 1;
		} else if (sc_strncasecmp(string + index_,
		                          ENTITY_GREATERTHAN, ENTITY_LENGTH) == 0) {
			buffer[b_index] = GREATERTHAN;
			index_ += ENTITY_LENGTH - 1;
		} else if (sc_strncasecmp(string + index_,
		                          ENTITY_PERCENT, PERCENT_LENGTH) == 0) {
			buffer[b_index] = PERCENT;
			index_ += PERCENT_LENGTH - 1;
		} else
			buffer[b_index] = string[index_];
	}

	/* Terminate, print, and free the buffer. */
	buffer[b_index] = NUL;
	if_print_string(buffer);
	sc_free(buffer);
}


/*
 * pf_output_tag()
 *
 * Output an HTML-like tag element to the OS-specific tag handling function.
 */
static void pf_output_tag(const sc_char *contents) {
	const sc_html_tags_t *entry;
	const sc_char *argument;

	/* For a simple <br> tag, just print out a newline. */
	if (sc_compare_word(contents, "br", 2)) {
		if_print_character('\n');
		return;
	}

	/*
	 * Search for the name in the HTML tags table.  It should be a full match,
	 * that is, the character after the matched name must be space or NUL.
	 * The <bgcolour="xyz"> tag is the exception; here the terminator is '='.
	 */
	for (entry = HTML_TAGS_TABLE; entry->name; entry++) {
		if (sc_strncasecmp(contents, entry->name, entry->length) == 0) {
			sc_char next;

			next = contents[entry->length];
			if (next == NUL || sc_isspace(next)
			        || (entry->tag == SC_TAG_BGCOLOR && next == '='))
				break;
		}
	}

	/* If not matched, output an unknown tag with contents as its argument. */
	if (!entry->name) {
		if_print_tag(SC_TAG_UNKNOWN, contents);
		return;
	}

	/*
	 * Find the argument by skipping the tag name and any spaces.  Again, for
	 * <bgcolour="xyz">, make a special case, passing the complete contents as
	 * argument (to match <font colour=...> for the client.
	 */
	argument = contents;
	argument += (entry->tag != SC_TAG_BGCOLOR) ? entry->length : 0;
	while (sc_isspace(argument[0]))
		argument++;
	if_print_tag(entry->tag, argument);
}


/*
 * pf_output_untagged()
 *
 * Break apart HTML-like string into normal text elements, and HTML-like
 * tags.
 */
static void pf_output_untagged(const sc_char *string) {
	sc_char *temporary, *untagged, *contents;
	const sc_char *cursor;
	const sc_char *marker;

	/*
	 * Optimize away the allocation and copy if possible.  We need to check
	 * here both for tags and for entities; only if neither occurs is it safe
	 * to output the string directly.
	 */
	if (!strchr(string, LESSTHAN)
	        && !(strstr(string, ENTITY_LESSTHAN)
	             || strstr(string, ENTITY_GREATERTHAN)
	             || strstr(string, ENTITY_PERCENT))) {
		if_print_string(string);
		return;
	}

	/*
	 * Create a general temporary string, and alias it to both untagged text
	 * and the tag name, for sharing inside the loop.
	 */
	temporary = (sc_char *)sc_malloc(strlen(string) + 1);
	untagged = contents = temporary;

	/* Run through the string looking for <...> tags. */
	marker = string;
	for (cursor = (const sc_char *)strchr(marker, LESSTHAN);
	        cursor; cursor = (const sc_char *)strchr(marker, LESSTHAN)) {
		sc_char close;

		/* Handle characters up to the tag start; untagged text. */
		memcpy(untagged, marker, cursor - marker);
		untagged[cursor - marker] = NUL;
		pf_output_text(untagged);

		/* Catch and ignore completely empty tags. */
		if (cursor[1] == GREATERTHAN) {
			marker = cursor + 2;
			continue;
		}

		/*
		 * Get the text within the tag, reusing the temporary buffer.  If this
		 * fails, allow the remainder of the line to be delivered as a tag;
		 * unknown, probably.
		 */
		if (sscanf(cursor, "<%[^>]%c", contents, &close) != 2
		        || close != GREATERTHAN) {
			if (sscanf(cursor, "<%[^>]", contents) != 1) {
				sc_error("pf_output_untagged: mismatched '%c'\n", LESSTHAN);
				if_print_character(LESSTHAN);
				marker = cursor + 1;
				continue;
			}
		}

		/* Output tag, and advance marker over the <...> tag. */
		if (!sc_strempty(contents))
			pf_output_tag(contents);
		marker = cursor + strlen(contents) + 1;
		marker += (marker[0] == GREATERTHAN) ? 1 : 0;
	}

	/* Output any remaining string text, and free the temporary buffer. */
	pf_output_text(marker);
	sc_free(temporary);
}


/*
 * pf_filter_internal()
 *
 * Filters an output string, interpolating variables and replacing ALR's.  If
 * any filtering was done, returns an allocated string that the caller needs
 * to free; otherwise, return NULL.
 *
 * Bundle may be NULL, requesting that the function suppress ALR replacements,
 * and do only variables; used for game info strings.
 *
 * The way Adrift does this is somewhat obscure, but the following seems to
 * replicate it well enough for most practical purposes (it's unlikely that
 * any game assumes or relies on anything not covered by this):
 *
 *  repeat some number of times
 *    repeat some number of times
 *      interpolate variables
 *    repeat [some number of times?]
 *      for each ALR unused so far this pass
 *        search the current string for the ALR original
 *        if found
 *          replace this ALR in the current string
 *          mark this ALR as used
 *    until no more changes in the current string
 *
 */
static sc_char *pf_filter_internal(const sc_char *string, sc_var_setref_t vars, sc_prop_setref_t bundle) {
	sc_int alr_count, iteration;
	sc_char *current;
	sc_bool *alr_applied;
	assert(string && vars);

	if (pf_trace)
		sc_trace("Printfilter: initial \"%s\"\n", string);

	/* If including ALRs, create a common set of ALR application flags. */
	if (bundle) {
		sc_vartype_t vt_key;

		/* Obtain a count of ALRs. */
		vt_key.string = "ALRs";
		alr_count = prop_get_child_count(bundle, "I<-s", &vt_key);

		/*
		 * Create a new set of ALR application flags.  These are used to ensure
		 * that a given ALR is applied only once on a given pass.  If the game
		 * has no ALRs, don't create a flag set.
		 */
		if (alr_count > 0) {
			alr_applied = (sc_bool *)sc_malloc(alr_count * sizeof(*alr_applied));
			memset(alr_applied, FALSE, alr_count * sizeof(*alr_applied));
		} else
			alr_applied = NULL;
	} else {
		/* Not including ALRs, so set alr count to 0, and flags to NULL. */
		alr_count = 0;
		alr_applied = NULL;
	}

	/* Loop for a sort-of arbitrary number of passes; probably enough. */
	current = NULL;
	for (iteration = 0; iteration < ITERATION_LIMIT; iteration++) {
		sc_int inner_iteration;
		const sc_char *initial;
		sc_char *intermediate;

		/* Note the initial string, so we can check for no change. */
		initial = current;

		for (inner_iteration = 0;
		        inner_iteration < ITERATION_LIMIT; inner_iteration++) {
			/*
			 * Interpolate variables.  If any changes were made, advance current
			 * to the interpolated version, and free the old current if required.
			 * Work on the current string, if any, otherwise the input string.
			 */
			intermediate = pf_interpolate_vars(current ? current : string, vars);
			if (intermediate) {
				sc_free(current);
				current = intermediate;
				if (pf_trace) {
					sc_trace("Printfilter: interpolated [%ld,%ld] \"%s\"\n",
					         iteration, inner_iteration, current);
				}
			} else
				break;
		}

		/* If we have ALRs to process, search out and replace all findable. */
		if (alr_count > 0) {
			/* Replace ALRs until no more ALRs can be found. */
			inner_iteration = 0;
			while (TRUE) {
				/*
				 * Replace ALRs, and advance current as for variables above.
				 * Leave the loop when ALR replacements stop.  Again, work on
				 * the current string if any, otherwise the input string.
				 */
				intermediate = pf_replace_alrs(current ? current : string,
				                               bundle, alr_applied, alr_count);
				if (intermediate) {
					sc_free(current);
					current = intermediate;
					if (pf_trace) {
						sc_trace("Printfilter: replaced [%ld,%ld] \"%s\"\n",
						         iteration, inner_iteration, current);
					}
				} else
					break;
				inner_iteration++;
			}
		}

		/* If nothing changed this iteration, stop now. */
		if (current == initial)
			break;
	}

	/* Free any ALR application flags, and return current, NULL if no change. */
	sc_free(alr_applied);
	return current;
}


/*
 * pf_filter()
 *
 * A facet of pf_filter_internal().  Filter an output string, interpolating
 * variables and replacing ALR's.  Returns an allocated string that the caller
 * needs to free.
 */
sc_char *pf_filter(const sc_char *string, sc_var_setref_t vars, sc_prop_setref_t bundle) {
	sc_char *current;

	/* Filter this string, including ALRs replacements. */
	current = pf_filter_internal(string, vars, bundle);

	/* Our contract is to return an allocated string; copy if required. */
	if (!current) {
		current = (sc_char *)sc_malloc(strlen(string) + 1);
		strcpy(current, string);
	}

	return current;
}


/*
 * pf_filter_for_info()
 *
 * A facet of pf_filter_internal().  Filters output, interpolating variables
 * only (no ALR replacement), and returns the resulting string to the caller.
 * Used on informational strings such as the game title and author.  Returns
 * an allocated string that the caller needs to free.
 */
sc_char *pf_filter_for_info(const sc_char *string, sc_var_setref_t vars) {
	sc_char *current;

	/* Filter this string, excluding ALRs replacements. */
	current = pf_filter_internal(string, vars, NULL);

	/* Our contract is to return an allocated string; copy if required. */
	if (!current) {
		current = (sc_char *)sc_malloc(strlen(string) + 1);
		strcpy(current, string);
	}

	return current;
}


/*
 * pf_flush()
 *
 * Filter buffered data, interpolating variables and replacing ALR's, and
 * send the resulting string to the output channel.
 */
void pf_flush(sc_filterref_t filter, sc_var_setref_t vars, sc_prop_setref_t bundle) {
	assert(pf_is_valid(filter));
	assert(vars && bundle);

	/* See if there is any buffered data to flush. */
	if (filter->buffer_length > 0) {
		/*
		 * Filter the buffered string, then print it untagged.  Remember to free
		 * the filtered version.  If filtering made no difference, or if the
		 * buffer was already filtered by, say, checkpointing, just print the
		 * original buffer untagged instead.
		 */
		if (filter->needs_filtering) {
			sc_char *filtered;

			filtered = pf_filter_internal(filter->buffer, vars, bundle);
			if (filtered) {
				pf_output_untagged(filtered);
				sc_free(filtered);
			} else
				pf_output_untagged(filter->buffer);
		} else
			pf_output_untagged(filter->buffer);

		/* Remove buffered data by resetting length to zero. */
		filter->buffer_length = 0;
		filter->needs_filtering = FALSE;
	}

	/* Reset new sentence and mute flags. */
	filter->new_sentence = FALSE;
	filter->is_muted = FALSE;
}


/*
 * pf_append_string()
 *
 * Append a string to the filter buffer.
 */
static void pf_append_string(sc_filterref_t filter, const sc_char *string) {
	sc_int length, required;

	/*
	 * Calculate the required buffer size to append string.  Remember to add
	 * one for the terminating NUL.
	 */
	length = strlen(string);
	required = filter->buffer_length + length + 1;

	/* If this is more than the current buffer allocation, resize it. */
	if (required > filter->buffer_allocation) {
		sc_int new_allocation;

		/* Calculate the new malloc size, in increment chunks. */
		new_allocation = ((required + BUFFER_GROW_INCREMENT - 1)
		                  / BUFFER_GROW_INCREMENT) * BUFFER_GROW_INCREMENT;

		/* Grow the buffer. */
		filter->buffer = (sc_char *)sc_realloc(filter->buffer, new_allocation);
		filter->buffer_allocation = new_allocation;
	}

	/* If empty, put a NUL into the buffer to permit strcat. */
	if (filter->buffer_length == 0)
		filter->buffer[0] = NUL;

	/* Append the string to the buffer and extend length. */
	strcat(filter->buffer, string);
	filter->buffer_length += length;
}


/*
 * pf_checkpoint()
 *
 * Filter buffered data, interpolating variables and replacing ALR's, and
 * store the result back in the buffer.  This allows a string to be inter-
 * polated in between main flushes; used to update buffered text with variable
 * values before those values are updated by task actions.
 */
void pf_checkpoint(sc_filterref_t filter, sc_var_setref_t vars, sc_prop_setref_t bundle) {
	assert(pf_is_valid(filter));
	assert(vars && bundle);

	/* See if there is any buffered data to filter. */
	if (filter->buffer_length > 0) {
		/*
		 * Filter the buffered string, and place the filtered result, if any,
		 * back into the filter buffer.  We do this by setting the buffer length
		 * back to zero, then appending the filtered string; this keeps the
		 * grown buffer intact.
		 */
		if (filter->needs_filtering) {
			sc_char *filtered;

			filtered = pf_filter_internal(filter->buffer, vars, bundle);
			if (filtered) {
				filter->buffer_length = 0;
				pf_append_string(filter, filtered);
				sc_free(filtered);
			}
		}

		/* Note the buffer as filtered, to avoid pointless filtering. */
		filter->needs_filtering = FALSE;
	}
}


/*
 * pf_get_buffer()
 * pf_transfer_buffer()
 *
 * Return the raw, unfiltered, buffered text.  Returns NULL if no buffered
 * data available.  Transferring the buffer transfers ownership of the buffer
 * string to the caller, who is then responsible for freeing it.
 *
 * The second function is an optimization to avoid allocations and copying
 * in client code.
 */
const sc_char *pf_get_buffer(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	/*
	 * Return buffer if filter length is greater than zero.  Note that this
	 * assumes that the buffer is a nul-terminated string.
	 */
	if (filter->buffer_length > 0) {
		assert(filter->buffer[filter->buffer_length] == NUL);
		return filter->buffer;
	} else
		return NULL;
}

sc_char *pf_transfer_buffer(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	/*
	 * If the filter length is greater than zero, pass out the buffer (a nul-
	 * terminated string) and zero our length, allocation, and set the buffer
	 * back to NULL; an empty in all except the free-ing.
	 */
	if (filter->buffer_length > 0) {
		sc_char *retval;

		/* Set the return value to be the buffered text. */
		assert(filter->buffer[filter->buffer_length] == NUL);
		retval = filter->buffer;

		/* Clear all filter fields down to empty values. */
		filter->buffer_length = 0;
		filter->buffer_allocation = 0;
		filter->buffer = NULL;
		filter->new_sentence = FALSE;
		filter->is_muted = FALSE;
		filter->needs_filtering = FALSE;

		/* Return the allocated buffered text. */
		return retval;
	} else
		return NULL;
}


/*
 * pf_empty()
 *
 * Empty any text currently buffered in the filter.
 */
void pf_empty(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	/* Free any allocation, and return the filter to initialization state. */
	filter->buffer_length = 0;
	filter->buffer_allocation = 0;
	sc_free(filter->buffer);
	filter->buffer = NULL;
	filter->new_sentence = FALSE;
	filter->is_muted = FALSE;
	filter->needs_filtering = FALSE;
}


/*
 * pf_buffer_string()
 * pf_buffer_character()
 *
 * Add a string, and a single character, to the printfilter buffer.  If muted,
 * these functions do nothing.
 */
void pf_buffer_string(sc_filterref_t filter, const sc_char *string) {
	assert(pf_is_valid(filter));
	assert(string);

	/* Ignore the call if the printfilter is muted. */
	if (!filter->is_muted) {
		sc_int noted;

		/* Note append start, then append the string to the buffer. */
		noted = filter->buffer_length;
		pf_append_string(filter, string);

		/* Adjust the first character of the appended string if flagged. */
		if (filter->new_sentence)
			filter->buffer[noted] = sc_toupper(filter->buffer[noted]);

		/* Clear new sentence, and note as currently needing filtering. */
		filter->needs_filtering = TRUE;
		filter->new_sentence = FALSE;
	}
}

void pf_buffer_character(sc_filterref_t filter, sc_char character) {
	sc_char buffer[2];
	assert(pf_is_valid(filter));

	buffer[0] = character;
	buffer[1] = NUL;
	pf_buffer_string(filter, buffer);
}


/*
 * pf_prepend_string()
 *
 * Add a string to the front of the printfilter buffer, rather than to the
 * end.  Generally less efficient than an append, these are for use by task
 * running code, which needs to run task actions and then prepend the task's
 * completion text.  If muted, this function does nothing.
 */
void pf_prepend_string(sc_filterref_t filter, const sc_char *string) {
	assert(pf_is_valid(filter));
	assert(string);

	/* Ignore the call if the printfilter is muted. */
	if (!filter->is_muted) {
		if (filter->buffer_length > 0) {
			sc_char *copy;

			/* Take a copy of the current buffered string. */
			assert(filter->buffer[filter->buffer_length] == NUL);
			copy = (sc_char *)sc_malloc(filter->buffer_length + 1);
			strcpy(copy, filter->buffer);

			/*
			 * Now restart buffering with the input string passed in.  Removing
			 * the current content by zeroing the length preserves the grown
			 * allocation of the main buffer.
			 */
			filter->buffer_length = 0;
			pf_append_string(filter, string);

			/* Append the string saved above and then free it. */
			pf_append_string(filter, copy);
			sc_free(copy);

			/* Adjust the first character of the prepended string if flagged. */
			if (filter->new_sentence)
				filter->buffer[0] = sc_toupper(filter->buffer[0]);

			/* Clear new sentence, and note as currently needing filtering. */
			filter->needs_filtering = TRUE;
			filter->new_sentence = FALSE;
		} else
			/* No data, so the call is equivalent to a normal buffer. */
			pf_buffer_string(filter, string);
	}
}


/*
 * pf_new_sentence()
 *
 * Tells the printfilter to force the next non-space character to uppercase.
 * Ignored if the printfilter is muted.
 */
void pf_new_sentence(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	if (!filter->is_muted)
		filter->new_sentence = TRUE;
}


/*
 * pf_mute()
 * pf_clear_mute()
 *
 * A muted printfilter ignores all new text additions.
 */
void pf_mute(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	filter->is_muted = TRUE;
}

void pf_clear_mute(sc_filterref_t filter) {
	assert(pf_is_valid(filter));

	filter->is_muted = FALSE;
}


/*
 * pf_buffer_tag()
 *
 * Insert an HTML-like tag into the buffered output data.  The call is ignored
 * if the printfilter is muted.
 */
void pf_buffer_tag(sc_filterref_t filter, sc_int tag) {
	const sc_html_tags_t *entry;
	assert(pf_is_valid(filter));

	/* Search the tags table for this tag. */
	for (entry = HTML_TAGS_TABLE; entry->name; entry++) {
		if (tag == entry->tag)
			break;
	}

	/* If found, output the equivalent string, enclosed in '<>' characters. */
	if (entry->name) {
		pf_buffer_character(filter, LESSTHAN);
		pf_buffer_string(filter, entry->name);
		pf_buffer_character(filter, GREATERTHAN);
	} else
		sc_error("pf_buffer_tag: invalid tag, %ld\n", tag);
}


/*
 * pf_strip_tags_common()
 *
 * Strip HTML-like tags from a string.  Used to process strings used in ways
 * other than being passed to if_print_string(), for example room names and
 * status lines.  It ignores all tags except <br>, which it replaces with
 * a newline if requested by allow_newlines.
 */
static void pf_strip_tags_common(sc_char *string, sc_bool allow_newlines) {
	sc_char *marker, *cursor;

	/* Run through the string looking for <...> tags. */
	marker = string;
	for (cursor = strchr(marker, LESSTHAN);
	        cursor; cursor = strchr(marker, LESSTHAN)) {
		sc_char *tag_end;

		/* Locate tag end, and break if unterminated. */
		tag_end = strchr(cursor, GREATERTHAN);
		if (!tag_end)
			break;

		/* If the tag is <br>, replace with newline if requested. */
		if (allow_newlines) {
			if (tag_end - cursor == 3
			        && sc_strncasecmp(cursor + 1, "br", 2) == 0)
				*cursor++ = '\n';
		}

		/* Remove the tag from the string, then advance input. */
		memmove(cursor, tag_end + 1, strlen(tag_end));
		marker = cursor;
	}
}


/*
 * pf_strip_tags()
 * pf_strip_tags_for_hints()
 *
 * Public interfaces to pf_strip_tags_common().  The hints version will
 * allow <br> tags to map into newlines in hints strings.
 */
void pf_strip_tags(sc_char *string) {
	pf_strip_tags_common(string, FALSE);
}

void pf_strip_tags_for_hints(sc_char *string) {
	pf_strip_tags_common(string, TRUE);
}


/*
 * pf_escape()
 *
 * Escape <, >, and % characters in the input string.  Used to filter player
 * input prior to storing in referenced text.
 *
 * Adrift offers no escapes for & and + escapes, so for these we convert to
 * the character itself followed by a space.  The return string is malloc'ed,
 * so the caller needs to remember to free it.
 */
sc_char *pf_escape(const sc_char *string) {
	const sc_char *marker, *cursor;
	sc_char *buffer;

	/* Start with an empty return buffer. */
	buffer = (sc_char *)sc_malloc(strlen(string) + 1);
	buffer[0] = NUL;

	/* Run through the string looking for <, >, %, or other escapes. */
	marker = string;
	for (cursor = marker + strcspn(marker, ESCAPES);
	        cursor[0] != NUL; cursor = marker + strcspn(marker, ESCAPES)) {
		const sc_char *escape;
		sc_char escape_buffer[3];

		/* Extend buffer to hold the string so far. */
		if (cursor > marker) {
			buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + cursor - marker + 1);
			buffer[strlen(buffer) + cursor - marker] = NUL;
			memcpy(buffer + strlen(buffer), marker, cursor - marker);
		}

		/* Determine the appropriate character escape. */
		if (cursor[0] == LESSTHAN)
			escape = ENTITY_LESSTHAN;
		else if (cursor[0] == GREATERTHAN)
			escape = ENTITY_GREATERTHAN;
		else if (cursor[0] == PERCENT)
			escape = ENTITY_PERCENT;
		else {
			/*
			 * No real escape available, so fake, badly, by appending a space
			 * for cases where we've encountered a character entity; leave
			 * others untouched.
			 */
			escape_buffer[0] = cursor[0];
			if (sc_strncasecmp(cursor,
			                   ENTITY_LESSTHAN, ENTITY_LENGTH) == 0
			        || sc_strncasecmp(cursor,
			                          ENTITY_GREATERTHAN, ENTITY_LENGTH) == 0
			        || sc_strncasecmp(cursor,
			                          ENTITY_PERCENT, PERCENT_LENGTH) == 0) {
				escape_buffer[1] = ' ';
				escape_buffer[2] = NUL;
			} else
				escape_buffer[1] = NUL;
			escape = escape_buffer;
		}

		buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + strlen(escape) + 1);
		strcat(buffer, escape);

		/* Pass over character escaped and continue. */
		cursor++;
		marker = cursor;
	}

	/* Add all remaining characters to the buffer. */
	if (cursor > marker) {
		buffer = (sc_char *)sc_realloc(buffer, strlen(buffer) + cursor - marker + 1);
		buffer[strlen(buffer) + cursor - marker] = NUL;
		memcpy(buffer + strlen(buffer), marker, cursor - marker);
	}

	return buffer;
}


/*
 * pf_compare_words()
 *
 * Matches multiple words from words in string.  Returns the extent of
 * the match if the string matched, 0 otherwise.
 */
static sc_int pf_compare_words(const sc_char *string, const sc_char *words) {
	sc_int word_posn, posn;

	/* None expected, but skip leading space. */
	for (word_posn = 0; sc_isspace(words[word_posn]) && words[word_posn] != NUL;)
		word_posn++;

	/* Match characters from words with the string at position. */
	posn = 0;
	while (TRUE) {
		/* Any character mismatch means no words match. */
		if (sc_tolower(words[word_posn]) != sc_tolower(string[posn]))
			return 0;

		/* Move to next character in each. */
		word_posn++;
		posn++;

		/*
		 * If at space, advance over whitespace in words list.  Stop when we
		 * hit the end of the words list.
		 */
		while (sc_isspace(words[word_posn]) && words[word_posn] != NUL)
			word_posn++;
		if (words[word_posn] == NUL)
			break;

		/*
		 * About to match another word, so advance over whitespace in the
		 * current string too.
		 */
		while (sc_isspace(string[posn]) && string[posn] != NUL)
			posn++;
	}

	/*
	 * We reached the end of words.  If we're at the end of the match string,
	 * or at spaces, we've matched.
	 */
	if (sc_isspace(string[posn]) || string[posn] == NUL)
		return posn;

	/* More text after the match, so it's not quite a match. */
	return 0;
}


/*
 * pf_filter_input()
 *
 * Applies synonym changes to a player input string, and returns the resulting
 * string to the caller, or NULL if no synonym changes were needed.  The
 * return string is malloc'ed, so the caller needs to remember to free it.
 */
sc_char *pf_filter_input(const sc_char *string, sc_prop_setref_t bundle) {
	sc_vartype_t vt_key[3];
	sc_int synonym_count, buffer_allocation;
	sc_char *buffer;
	const sc_char *current;
	assert(string && bundle);

	if (pf_trace)
		sc_trace("Printfilter: input \"%s\"\n", string);

	/* Obtain a count of synonyms. */
	vt_key[0].string = "Synonyms";
	synonym_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Begin with a NULL buffer for lazy allocation. */
	buffer_allocation = 0;
	buffer = NULL;

	/* Loop over each word in the string. */
	current = string + strspn(string, WHITESPACE);
	while (current[0] != NUL) {
		sc_int index_, extent;

		/* Search for a synonym match at this index into the buffer. */
		extent = 0;
		for (index_ = 0; index_ < synonym_count; index_++) {
			const sc_char *original;

			/* Retrieve the synonym original string. */
			vt_key[0].string = "Synonyms";
			vt_key[1].integer = index_;
			vt_key[2].string = "Original";
			original = prop_get_string(bundle, "S<-sis", vt_key);

			/* Compare the original at this point. */
			extent = pf_compare_words(current, original);
			if (extent > 0)
				break;
		}

		/*
		 * If a synonym found was, index_ indicates it, and extent shows how
		 * much of the buffer to replace with it.
		 */
		if (index_ < synonym_count && extent > 0) {
			const sc_char *replacement;
			sc_char *position;
			sc_int length, final_length;

			/*
			 * If not yet allocated, allocate a buffer now, and copy the input
			 * string into it.  Then switch current to the equivalent location
			 * in the allocated buffer.  More basic copy-on-write.
			 */
			if (!buffer) {
				buffer_allocation = strlen(string) + 1;
				buffer = (sc_char *)sc_malloc(buffer_allocation);
				strcpy(buffer, string);
				current = buffer + (current - string);
			}

			/* Find the replacement text for this synonym. */
			vt_key[0].string = "Synonyms";
			vt_key[1].integer = index_;
			vt_key[2].string = "Replacement";
			replacement = prop_get_string(bundle, "S<-sis", vt_key);
			length = strlen(replacement);

			/*
			 * If necessary, grow the output buffer for the replacement,
			 * remembering to adjust current for the new buffer allocated.
			 * At the same time, note the last character index for the move.
			 */
			if (length > extent) {
				sc_int offset;

				offset = current - buffer;
				buffer_allocation += length - extent;
				buffer = (sc_char *)sc_realloc(buffer, buffer_allocation);
				current = buffer + offset;
				final_length = length;
			} else
				final_length = extent;

			/* Insert the replacement string into the buffer. */
			position = buffer + (current - buffer);
			memmove(position + length,
			        position + extent,
			        buffer_allocation - (current - buffer) - final_length);
			memcpy(position, replacement, length);

			/* Adjust current to skip over the replacement. */
			current += length;

			if (pf_trace)
				sc_trace("Printfilter: synonym \"%s\"\n", buffer);
		} else {
			/* If no match, advance current over the unmatched word. */
			current += strcspn(current, WHITESPACE);
		}

		/* Set current to the next word start. */
		current += strspn(current, WHITESPACE);
	}

	/* Return the final string, or NULL if no synonym replacements. */
	return buffer;
}


/*
 * pf_debug_trace()
 *
 * Set filter tracing on/off.
 */
void pf_debug_trace(sc_bool flag) {
	pf_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
