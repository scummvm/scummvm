/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/remastered/commentary.h"
#include "engines/grim/resource.h"
#include "engines/grim/textsplit.h"
#include "engines/grim/localize.h"

#include "common/textconsole.h"

namespace Grim {

struct CommentLine {
	int _id;
	Common::String _line;
	int _start;
	int _end;
};

class Comment {
	Common::String _name;
	Common::String _filename;
	Common::Array<CommentLine> _lines;
	bool _hasHeard; //TODO: Should be saved
public:
	Comment(const Common::String &name, const Common::String &filename);
	Common::String getName() const;
	void addLine(int id, const Common::String &text, int start, int end);
	void play();
	bool hasHeard() const { return _hasHeard; }
};

Comment::Comment(const Common::String &name, const Common::String &filename) : _name(name), _filename(filename), _hasHeard(false) {
}

void Comment::addLine(int id, const Common::String &text, int start, int end) {
	CommentLine line;
	line._id = id;
	line._line = text;
	line._start = start;
	line._end = end;
	_lines.push_back(line);
}

Common::String Comment::getName() const {
	return _name;
}

void Comment::play() {
	for (int i = 0; i < _lines.size(); i++) {
		Common::String text = g_localizer->localize(_lines[i]._line.c_str());
		warning("Line: %d Start: %d End: %d Id: %d Text: %s", i, _lines[i]._start, _lines[i]._end, _lines[i]._id, text.c_str());
	}
	_hasHeard = true;
}

Commentary::Commentary() : _currentCommentary(nullptr) {
	loadCommentary();
}

Commentary::~Commentary() {
	Common::HashMap<Common::String, Comment*>::iterator it = _comments.begin();
	for (; it != _comments.end(); ++it) {
		delete it->_value;
	}
	_comments.clear();
}

void Commentary::loadCommentary() {
	Common::String defFilename = "commentary_def.txt";
	Common::SeekableReadStream *f = g_resourceloader->openNewStreamFile(defFilename);
	if (!f) {
		error("Commentary::loadCommentary: Unable to find commentary definition (%s)", defFilename.c_str());
		return;
	}
	TextSplitter ts(defFilename, f);

	while (!ts.isEof()) {
		// Skip comments
		while (ts.checkString("#")) {
			ts.nextLine();
		}
		// Skip blank lines and trim input
		Common::String currentLine = ts.getCurrentLine();
		currentLine.trim();
		while (currentLine.size() == 0) {
			currentLine = ts.nextLine();
			currentLine.trim();
		}

		Common::String name = currentLine;
		name.trim();
		Common::String filename = ts.nextLine();
		ts.nextLine();
		filename.trim();

		Comment *c = new Comment(name, filename);

		int numLines = 0;
		ts.scanString("\t%d", 1, &numLines);

		char str[20] = {0};
		for (int i = 0; i < numLines; i++) {
			int id = 0;
			int start = 0;
			int end = 0;
			ts.scanString("%d\t%s\t%d\t%d", 4, &id, str, &start, &end);
			c->addLine(id, str, start, end);
		}

		_comments.setVal(name, c);
	}
}

Comment *Commentary::findCommentary(const Common::String &name) {
	Common::String lowerName = name;
	lowerName.toLowercase();
	if (!_comments.contains(lowerName)) {
		return nullptr;
	} else {
		return _comments.getVal(lowerName);
	}
}

void Commentary::playCurrentCommentary() {
	if (_currentCommentary != nullptr) {
		warning("Commentary::playCurrentCommentary, current is %s", _currentCommentary->getName().c_str());
			_currentCommentary->play();
	} else {
		warning("Commentary::playCurrentCommentary, no current commentary");
	}
}
 
void Commentary::setCurrentCommentary(const Common::String &name) {
	warning("Commentary::setCurrentCommentary(%s)", name.c_str());
	_currentCommentary = findCommentary(name);
	if (_currentCommentary == nullptr) {
		warning("Commentary::setCurrentCommentary(%s) could not find commentary", name.c_str());
	}
}

bool Commentary::hasHeardCommentary(const Common::String &name) const {
	Common::String lowerName = name;
	lowerName.toLowercase();
	if (!_comments.contains(lowerName)) {
		warning("Commentary::hasHeardCommentary(%s) could not find commentary", name.c_str());
		return false;
	} else {
		return _comments.getVal(lowerName)->hasHeard();
	}
}

} // end of namespace Grim
