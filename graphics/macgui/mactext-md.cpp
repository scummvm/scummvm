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

#define PR(x) (x->data ? Common::String((const char *)(x)->data , (x)->size).c_str() : "(null)")

struct MDState {
	Common::List<int> listNum;
};


void render_blockcode(Common::DataBuffer *ob, const Common::DataBuffer *text, const Common::DataBuffer *lang, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockcode(%s)", PR(text));
}

void render_blockquote(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockquote(%s)", PR(text));
}

void render_blockhtml(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockhtml(%s)", PR(text));
}

void render_header(Common::DataBuffer *ob, const Common::DataBuffer *text, int level, void *opaque) {
	if (!text)
		return;

	debug(1, "render_header(%s)", PR(text));

	Common::String res = Common::String::format("\016+00%01x%s\001\016-00f\n", level, Common::String((const char *)text->data , text->size).c_str());

	bufput(ob, res.c_str(), res.size());
}

void render_hrule(Common::DataBuffer *ob, void *opaque) {
	warning("STUB: render_hrule()");
}

void render_list_start(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	mdstate->listNum.push_back(flags & MKD_LIST_ORDERED ? 1 : -1);

	debug(1, "render_list_start(%s, %d)", PR(text), flags);
}

void render_list(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	mdstate->listNum.pop_back();

	bufput(ob, text->data, text->size);
	bufput(ob, "\n", 1);

	debug(1, "render_list(%s, %d)", PR(text), flags);
}

void render_listitem(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	int listNum = mdstate->listNum.back();
	int depth = mdstate->listNum.size();

	for (int i = 0; i < depth; i++)
		bufput(ob, "  ", 2);

	if (flags & MKD_LIST_ORDERED) {
		Common::String prefix = Common::String::format("%d. ", listNum);

		bufput(ob, prefix.c_str(), prefix.size());

		mdstate->listNum.back()++;
	} else {
		bufput(ob, "* ", 2);
	}

	bufput(ob, text->data, text->size);

	debug(1, "render_listitem(%s, %d)", PR(text), flags);
}

void render_paragraph(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	debug(1, "render_paragraph(%s)", PR(text));

	bufput(ob, text->data, text->size);
	bufput(ob, "\n\n", 2);
}

void render_table(Common::DataBuffer *ob, const Common::DataBuffer *header, const Common::DataBuffer *body, void *opaque) {
	if (!body)
		return;

	warning("STUB: render_table(%s, %s)", header ? PR(header) : 0, PR(body));
}

void render_table_row(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_table_row(%s)", PR(text));
}

void render_table_cell(Common::DataBuffer *ob, const Common::DataBuffer *text, int flags, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_table_cell(%s)", PR(text));
}

int render_autolink(Common::DataBuffer *ob, const Common::DataBuffer *link, Common::MKDAutolink type, void *opaque) {
	if (!link)
		return 0;

	warning("STUB: render_autolink(%s)", PR(link));
	return 1;
}

int render_codespan(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_codespan(%s)", PR(text));
	return 1;
}

int render_double_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text || !text->size)
		return 0;

	debug(1, "render_double_emphasis(%s)", PR(text));

	Common::String res = Common::String::format("\001\016+%02x0%s\001\016-%02x0", kMacFontBold, Common::String((const char *)text->data , text->size).c_str(), kMacFontBold);

	bufput(ob, res.c_str(), res.size());
	return 1;
}

int render_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text || !text->size)
		return 0;

	debug(1, "render_emphasis(%s)", PR(text));

	Common::String res = Common::String::format("\001\016+%02x0%s\001\016-%02x0", kMacFontItalic, Common::String((const char *)text->data , text->size).c_str(), kMacFontItalic);

	bufput(ob, res.c_str(), res.size());
	return 1;
}

int render_image(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *alt, void *opaque) {
	if (!link)
		return 0;

	warning("STUB: render_image(%s, %s, %s)", PR(link), title ? PR(title) : 0, alt ? PR(alt) : 0);
	return 1;
}

int render_linebreak(Common::DataBuffer *ob, void *opaque) {
	debug(1, "render_linebreak()");

	bufput(ob, "\n", 1);
	return 1;
}

int render_link(Common::DataBuffer *ob, const Common::DataBuffer *link, const Common::DataBuffer *title, const Common::DataBuffer *content, void *opaque) {
	if (!link)
		return 0;

	warning("STUB: render_link(%s, %s, %s)", PR(link), title ? PR(title) : 0, content ? PR(content) : 0);
	return 1;
}

int render_raw_html_tag(Common::DataBuffer *ob, const Common::DataBuffer *tag, void *opaque) {
	if (!tag)
		return 0;

	warning("STUB: render_raw_html_tag(%s)", tag->data);
	return 1;
}

int render_triple_emphasis(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_triple_emphasis(%s)", PR(text));
	return 1;
}

int render_strikethrough(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_strikethrough(%s)", PR(text));
	return 1;
}

int render_superscript(Common::DataBuffer *ob, const Common::DataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_superscript(%s)", PR(text));
	return 1;
}

void MacText::setMarkdownText(const Common::U32String &str) {

	const Common::SDCallbacks cb = {
		/* block level callbacks - NULL skips the block */
		render_blockcode,
		render_blockquote,
		render_blockhtml,
		render_header,
		render_hrule,
		render_list_start,
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
		NULL,
		NULL,

		/* header and footer */
		NULL,
		NULL,
	};

	Common::String input = str.encode(); // Encode to UTF8

	Common::DataBuffer *ib = Common::bufnew(input.size());
	Common::bufgrow(ib, input.size());
	ib->size = input.size();
	memcpy(ib->data, input.c_str(), input.size());

	Common::DataBuffer *ob = Common::bufnew(1024);

	MDState mdState;

	Common::SDMarkdown *md = sd_markdown_new(0, 16, &cb, &mdState);
	sd_markdown_render(ob, ib->data, ib->size, md);
	sd_markdown_free(md);

	warning("%zu bytes: %s", ob->size, toPrintable(Common::String((const char *)ob->data, ob->size)).c_str());

	//setDefaultFormatting(kMacFontChicago, kMacFontRegular, 40, 0, 0, 0);
	setText(Common::String((const char *)ob->data, ob->size));
}

} // End of namespace Graphics
