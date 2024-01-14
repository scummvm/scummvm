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

#include "twp/twp.h"
#include "twp/tsv.h"
#include "twp/squtil.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

void TextDb::parseTsv(Common::SeekableReadStream &stream) {
	stream.readLine();
	while (!stream.eos()) {
		Common::String line = stream.readLine();
		int pos = line.find('\t', 0);
		int id = atoi(line.c_str());
		Common::String s = line.substr(pos + 1);
		_texts[id] = s;
		debug("%d: %s", id, _texts[id].c_str());
	}
}

Common::String TextDb::getText(int id) {
	Common::String result;
	if (_texts.contains(id)) {
		result = _texts[id];
		if (result.hasSuffix("#M") || result.hasSuffix("#F"))
			result = result.substr(0, result.size() - 2);
		// replace \" by ";
		result = Twp::replace(result, "\\\"", "\"");
	} else {
		result = Common::String::format("Text %d not found", id);
		error("Text %d not found", id);
	}
	return result;
}

Common::String TextDb::getText(const Common::String &text) {
	HSQUIRRELVM v = g_engine->getVm();
	if (text.size() > 0) {
		if (text[0] == '@') {
			int id = atoi(text.c_str() + 1);
			return getText(id);
		} else if (text[0] == '^') {
			return text.substr(1);
		} else if (text[0] == '$') {
			Common::String txt;
			SQInteger top = sq_gettop(v);
			sq_pushroottable(v);
			Common::String code = Common::String::format("return %s", text.substr(1, text.size() - 2).c_str());
			if (SQ_FAILED(sq_compilebuffer(v, code.c_str(), code.size(), "execCode", SQTrue))) {
				error("Error executing code %s", code.c_str());
			} else {
				sq_push(v, -2);
				// call
				if (SQ_FAILED(sq_call(v, 1, SQTrue, SQTrue))) {
					error("Error calling code %s", code.c_str());
				} else {
					sqget(v, -1, txt);
					sq_settop(v, top);
					return getText(txt);
				}
			}
		}
	}
	return text;
}

} // namespace Twp
