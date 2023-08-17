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
	if (!text)
		return;

	warning("render_blockcode(%s)", text->data);
}

void render_blockquote(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("render_blockquote(%s)", text->data);
}

void render_blockhtml(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("render_blockhtml(%s)", text->data);
}

void render_header(Common::DataBuffer *ob, const Common::DataBuffer *text, int level, void *opaque) {
	if (!text)
		return;

	warning("render_header(%s)", text->data);
}

void render_hrule(Common::DataBuffer *ob, void *opaque) {
	warning("render_hrule()");
}

void render_list(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	if (!text)
		return;

	warning("render_list(%s)", text->data);
}

void render_listitem(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	if (!text)
		return;

	warning("render_listitem(%s)", text->data);
}

void render_paragraph(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("render_paragraph(%s)", text->data);
}

void render_table(Common::DataBuffer *ob, const Common::DataBuffer *header, const Common::DataBuffer *body, void *opaque) {
	if (!body)
		return;

	warning("render_table(%s, %s)", header ? header->data : 0, body->data);
}

void render_table_row(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("render_table_row(%s)", text->data);
}

void render_table_cell(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	if (!text)
		return;

	warning("render_table_cell(%s)", text->data);
}

int render_autolink(Common::DataBuffer *ob, const Common::DataBuffer *link, Common::MKDAutolink type, void *opaque) {
	if (!link)
		return 0;

	warning("render_autolink(%s)", link->data);
	return 0;
}

int render_codespan(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_codespan(%s)", text->data);
	return 0;
}

int render_double_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_double_emphasis(%s)", text->data);
	return 0;
}

int render_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_emphasis(%s)", text->data);
	return 0;
}

int render_image(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *alt, void *opaque) {
	if (!link)
		return 0;

	warning("render_image(%s, %s, %s)", link->data, title ? title->data : 0, alt ? alt->data : 0);
	return 0;
}

int render_linebreak(Common::DataBuffer *ob, void *opaque) {
	warning("render_linebreak()");
	return 0;
}

int render_link(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *content, void *opaque) {
	if (!link)
		return 0;

	warning("render_link(%s, %s, %s)", link->data, title ? title->data : 0, content ? content->data : 0);
	return 0;
}

int render_raw_html_tag(Common::DataBuffer *ob, const Common::DataBuffer *tag, void *opaque) {
	if (!tag)
		return 0;

	warning("render_raw_html_tag(%s)", tag->data);
	return 0;
}

int render_triple_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_triple_emphasis(%s)", text->data);
	return 0;
}

int render_strikethrough(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_strikethrough(%s)", text->data);
	return 0;
}

int render_superscript(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("render_superscript(%s)", text->data);
	return 0;
}

void render_entity(Common::DataBuffer *ob, const Common::DataBuffer *entity, void *opaque) {
	if (!entity)
		return;

	warning("render_entity(%s)", entity->data);
}

void render_normal_text(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("render_normal_text(%s)", text->data);

	Common::bufgrow(ob, text->size);
	Common::bufputs(ob, (char *)text->data);
}

void render_doc_header(Common::DataBuffer *ob, void *opaque) {
	warning("render_doc_header()");
}

void render_doc_footer(Common::DataBuffer *ob, void *opaque) {
	warning("render_doc_footer()");
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
	Common::bufgrow(ib, input.size());
	memcpy(ib->data, input.c_str(), input.size());

	Common::DataBuffer *ob = Common::bufnew(1024);

	Common::SDMarkdown *md = sd_markdown_new(0, 16, &cb, this);
	sd_markdown_render(ob, ib->data, ib->size, md);
	sd_markdown_free(md);

	warning("%zu bytes: %s", ob->size, ob->data);
}

} // End of namespace Graphics
