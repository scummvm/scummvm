/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.

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

#include "common/formats/markdown.h"
#include "graphics/macgui/mactext.h"

namespace Graphics {

void render_blockcode(Common::DataBuffer *ob, const Common::DataBuffer *text, const Common::DataBuffer *lang, void *opaque) {
}

void render_blockquote(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
}

void render_blockhtml(Common::DataBuffer *ob,const  Common::DataBuffer *text, void *opaque) {
}

void render_header(Common::DataBuffer *ob, const Common::DataBuffer *text, int level, void *opaque) {
}

void render_hrule(Common::DataBuffer *ob, void *opaque) {
}

void render_list(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
}

void render_listitem(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
}

void render_paragraph(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
}

void render_table(Common::DataBuffer *ob, const Common::DataBuffer *header, const Common::DataBuffer *body, void *opaque) {
}

void render_table_row(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
}

void render_table_cell(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
}

int render_autolink(Common::DataBuffer *ob, const Common::DataBuffer *link, Common::MKDAutolink type, void *opaque) {
	return 0;
}

int render_codespan(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

int render_double_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

int render_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

int render_image(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *alt, void *opaque) {
	return 0;
}

int render_linebreak(Common::DataBuffer *ob, void *opaque) {
	return 0;
}

int render_link(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *content, void *opaque) {
	return 0;
}

int render_raw_html_tag(Common::DataBuffer *ob, const Common::DataBuffer *tag, void *opaque) {
	return 0;
}

int render_triple_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

int render_strikethrough(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

int render_superscript(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	return 0;
}

void render_entity(Common::DataBuffer *ob, const Common::DataBuffer *entity, void *opaque) {
}

void render_normal_text(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
}

void render_doc_header(Common::DataBuffer *ob, void *opaque) {
}

void render_doc_footer(Common::DataBuffer *ob, void *opaque) {
}


void MacText::setMarkdownText(const Common::U32String &str) {

	const Common::SDCallbacks cb = {
		/* block level callbacks - NULL skips the block */
		render_blockcode,
		render_blockquote,
		render_blockhtml,
		render_header,
		render_hrule,
		render_list,
		render_listitem,
		render_paragraph,
		render_table,
		render_table_row,
		render_table_cell,


		/* span level callbacks - NULL or return 0 prints the span verbatim */
		render_autolink,
		render_codespan,
		render_double_emphasis,
		render_emphasis,
		render_image,
		render_linebreak,
		render_link,
		render_raw_html_tag,
		render_triple_emphasis,
		render_strikethrough,
		render_superscript,

		/* low level callbacks - NULL copies input directly into the output */
		render_entity,
		render_normal_text,

		/* header and footer */
		render_doc_header,
		render_doc_footer,
	};

	Common::String input = str.encode(); // Encode to UTF8

	Common::DataBuffer *ib = Common::bufnew(input.size());


}

} // End of namespace Graphics
