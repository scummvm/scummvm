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

#include "common/textconsole.h"
#include "m4/adv_r/chunk_ops.h"
#include "m4/vars.h"

namespace M4 {

conv_chunk *get_conv(Conv *c, long cSize) {
	char *s = nullptr;
	conv_chunk *c_v = nullptr;

	s = &(c->conv[c->myCNode]);
	c_v = (conv_chunk *)&s[cSize];

	return c_v;
}

char *conv_ops_get_entry(long i, long *next, long *tag, Conv *c) {
	error("TODO: conv_ops_get_entry");
}

decl_chunk *get_decl(Conv *c, long cSize) {
	char *s = nullptr;
	decl_chunk *d = nullptr;

	s = c->conv;
	d = (decl_chunk *)&s[cSize];

	return d;
}

char *get_string(Conv *c, long cSize) {
	char *s = nullptr;
	char *c_s = nullptr;

	s = c->conv;
	c_s = (char *)&s[cSize];
	return c_s;
}

text_chunk *get_text(Conv *c, long cSize) {
	char *s = NULL;
	text_chunk *t = NULL;

	s = &(c->conv[c->myCNode]);
	t = (text_chunk *)&s[cSize];

	return t;
}

long conv_ops_text_strlen(char *s) {
	long len = 0;
	len = strlen(s) + 1; //added +1 for null char.

	if ((len % 4) == 0)
		return len;

	len += 4 - (len % 4);
	return len;
}

void conv_swap_words(Conv *c) {
	error("TODO: conv_swap_words");
}

} // End of namespace M4
