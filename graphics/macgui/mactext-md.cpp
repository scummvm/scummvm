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
	uint32 linkr = 0, linkg = 0, linkb = 0;
};

void render_blockcode(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, const Common::SDDataBuffer *lang, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockcode(%s)", PR(text));
}

void render_blockquote(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockquote(%s)", PR(text));
}

void render_blockhtml(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_blockhtml(%s)", PR(text));
}

void render_header(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, int level, void *opaque) {
	if (!text)
		return;

	debug(1, "render_header(%s)", PR(text));

	Common::String res = Common::String::format("\016+00%01x0" "%s" "\001\016-00f0\n", level, Common::String((const char *)text->data , text->size).c_str());

	sd_bufput(ob, res.c_str(), res.size());
}

void render_hrule(Common::SDDataBuffer *ob, void *opaque) {
	warning("STUB: render_hrule()");
}

void render_list_start(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	mdstate->listNum.push_back(flags & MKD_LIST_ORDERED ? 1 : -1);

	sd_bufput(ob, "\016+0001", 6);

	debug(1, "render_list_start(%s, %d)", PR(text), flags);
}

void render_list(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	mdstate->listNum.pop_back();

	sd_bufput(ob, text->data, text->size);
	sd_bufput(ob, "\n\016-0001", 7);

	debug(1, "render_list(%s, %d)", PR(text), flags);
}

void render_listitem(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, int flags, void *opaque) {
	MDState *mdstate = (MDState *)opaque;

	int listNum = mdstate->listNum.back();
	int depth = mdstate->listNum.size();

	for (int i = 0; i < depth; i++)
		sd_bufput(ob, "  ", 2);

	Common::String prefix;
	if (flags & MKD_LIST_ORDERED) {
		prefix = Common::String::format("%d. ", listNum);

		mdstate->listNum.back()++;
	} else {
		prefix = "* ";
	}

	Common::String res = Common::String::format("\016*%02x%s", prefix.size(), prefix.c_str());

	sd_bufput(ob, res.c_str(), res.size());

	sd_bufput(ob, prefix.c_str(), prefix.size());

	sd_bufput(ob, text->data, text->size);

	debug(1, "render_listitem(%s, %d)", PR(text), flags);
}

void render_paragraph(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return;

	debug(1, "render_paragraph(%s)", PR(text));

	sd_bufput(ob, text->data, text->size);
	sd_bufput(ob, "\n\n", 2);
}

void render_table(Common::SDDataBuffer *ob, const Common::SDDataBuffer *header, const Common::SDDataBuffer *body, void *opaque) {
	if (!body)
		return;

	warning("STUB: render_table(%s, %s)", header ? PR(header) : 0, PR(body));
}

void render_table_row(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_table_row(%s)", PR(text));
}

void render_table_cell(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, int flags, void *opaque) {
	if (!text)
		return;

	warning("STUB: render_table_cell(%s)", PR(text));
}

int render_autolink(Common::SDDataBuffer *ob, const Common::SDDataBuffer *link, Common::MKDAutolink type, void *opaque) {
	if (!link)
		return 0;

	warning("STUB: render_autolink(%s)", PR(link));
	return 1;
}

int render_codespan(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_codespan(%s)", PR(text));
	return 1;
}

int render_double_emphasis(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text || !text->size)
		return 0;

	debug(1, "render_double_emphasis(%s)", PR(text));

	Common::String res = Common::String::format("\001\016+%02x00" "%s" "\001\016-%02x00", kMacFontBold, Common::String((const char *)text->data , text->size).c_str(), kMacFontBold);

	sd_bufput(ob, res.c_str(), res.size());
	return 1;
}

int render_emphasis(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text || !text->size)
		return 0;

	debug(1, "render_emphasis(%s)", PR(text));

	Common::String res = Common::String::format("\001\016+%02x00" "%s" "\001\016-%02x00", kMacFontItalic, Common::String((const char *)text->data , text->size).c_str(), kMacFontItalic);

	sd_bufput(ob, res.c_str(), res.size());
	return 1;
}

int render_image(Common::SDDataBuffer *ob, const Common::SDDataBuffer *link, const Common::SDDataBuffer *title, const Common::SDDataBuffer *alt, void *opaque) {
	if (!link)
		return 0;

	Common::String res = Common::String::format("\001" "\016i%02x" "%02x%s",
			50, (uint)link->size, Common::String((const char *)link->data, link->size).c_str());

	if (alt)
		res += Common::String::format("%02x%s", (uint)alt->size, Common::String((const char *)alt->data, alt->size).c_str());
	else
		res += "00";

	if (title)
		res += Common::String::format("%02x%s\n", (uint)title->size, Common::String((const char *)title->data, title->size).c_str());
	else
		res += "00\n";

	sd_bufput(ob, res.c_str(), res.size());

	debug(1, "render_image(%s, %s, %s)", PR(link), title ? PR(title) : 0, alt ? PR(alt) : 0);
	return 1;
}

int render_linebreak(Common::SDDataBuffer *ob, void *opaque) {
	debug(1, "render_linebreak()");

	sd_bufput(ob, "\n", 1);
	return 1;
}

int render_link(Common::SDDataBuffer *ob, const Common::SDDataBuffer *link, const Common::SDDataBuffer *title, const Common::SDDataBuffer *content, void *opaque) {
	if (!link)
		return 0;

	MDState *mdstate = (MDState *)opaque;
	const Common::SDDataBuffer *text = content ? content : link;

	Common::String res = Common::String::format("\001" "\016+%02x00" "\001\016[%04x%04x%04x"
		"%s" "\001\016]" "\001\016-%02x00", kMacFontUnderline, mdstate->linkr, mdstate->linkg, mdstate->linkb,
		Common::String((const char *)text->data , text->size).c_str(), kMacFontUnderline);

	sd_bufput(ob, res.c_str(), res.size());

	debug(1, "render_link(%s, %s, %s)", PR(link), title ? PR(title) : 0, content ? PR(content) : 0);
	return 1;
}

int render_raw_html_tag(Common::SDDataBuffer *ob, const Common::SDDataBuffer *tag, void *opaque) {
	if (!tag)
		return 0;

	warning("STUB: render_raw_html_tag(%s)", PR(tag));
	return 1;
}

int render_triple_emphasis(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_triple_emphasis(%s)", PR(text));
	return 1;
}

int render_strikethrough(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
	if (!text)
		return 0;

	warning("STUB: render_strikethrough(%s)", PR(text));
	return 1;
}

int render_superscript(Common::SDDataBuffer *ob, const Common::SDDataBuffer *text, void *opaque) {
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

	MDState mdState;

	// Set link color to blue
	mdState.linkr = 0;
	mdState.linkg = 0;
	mdState.linkb = 0xff;

	Common::SDMarkdown md(0, 16, &cb, &mdState);
	Common::String rendered = md.render((const byte *)input.c_str(), input.size());

	setText(rendered);
}

} // End of namespace Graphics
