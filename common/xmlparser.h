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
 * $URL$
 * $Id$
 *
 */

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"
#include "common/xmlparser.h"
#include "common/stream.h"
#include "common/file.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"

namespace Common {

class XMLStream {
protected:
	SeekableReadStream *_stream;
	int _pos;

public:
	XMLStream() : _stream(0), _pos(0) {}

	~XMLStream() {
		delete _stream;
	}

	SeekableReadStream *stream() {
		return _stream;
	}

	char operator [](int idx) {
		assert(_stream && idx >= 0);

		if (_pos + 1 != idx)
			_stream->seek(idx, SEEK_SET);

		_pos = idx;

		return _stream->readByte();
	}

	void loadStream(SeekableReadStream *s) {
		delete _stream;
		_stream = s;
	}

	bool ready() {
		return _stream != 0;
	}
};

/**
 * The base XMLParser class implements generic functionality for parsing
 * XML-like files.
 *
 * In order to use it, it must be inherited with a child class that implements
 * the XMLParser::keyCallback() function.
 *
 * @see XMLParser::keyCallback()
 */
class XMLParser {

public:
	/**
	 * Parser constructor.
	 */
	XMLParser() {}

	virtual ~XMLParser() {
		while (!_activeKey.empty())
			delete _activeKey.pop();
	}

	/** Active state for the parser */
	enum ParserState {
		kParserNeedKey,
		kParserNeedKeyName,

		kParserNeedPropertyName,
		kParserNeedPropertyOperator,
		kParserNeedPropertyValue,

		kParserError
	};

	/** Struct representing a parsed node */
	struct ParserNode {
		Common::String name;
		Common::StringMap values;
		bool ignore;
		int depth;
	};

	/**
	 * Loads a file into the parser.
	 * Used for the loading of Theme Description files
	 * straight from the filesystem.
	 *
	 * @param filename Name of the file to load.
	 */
	virtual bool loadFile(Common::String filename) {
		Common::File *f = new Common::File;

		if (!f->open(filename, Common::File::kFileReadMode))
			return false;

		_fileName = filename;
		_text.loadStream(f);
		return true;
	}

	/**
	 * Loads a memory buffer into the parser.
	 * Used for loading the default theme fallback directly
	 * from memory if no themes can be found.
	 *
	 * @param buffer Pointer to the buffer.
	 * @param size Size of the buffer
	 * @param disposable Sets if the XMLParser owns the buffer,
	 *                   i.e. if it can be freed safely after it's
	 *                   no longer needed by the parser.
	 */
	virtual bool loadBuffer(const byte *buffer, uint32 size, bool disposable = false) {
		_text.loadStream(new MemoryReadStream(buffer, size, disposable));
		_fileName = "Memory Stream";
		return true;
	}

	/**
	 * The actual parsing function.
	 * Parses the loaded data stream, returns true if successful.
	 */
	virtual bool parse();

	/**
	 * Returns the active node being parsed (the one on top of
	 * the node stack).
	 */
	ParserNode *getActiveNode() {
		if (!_activeKey.empty())
			return _activeKey.top();

		return 0;
	}

	/**
	 * Returns the parent of a given node in the stack.
	 */
	ParserNode *getParentNode(ParserNode *child) {
		return child->depth > 0 ? _activeKey[child->depth - 1] : 0;
	}

protected:
	/**
	 * The keycallback function must be overloaded by inheriting classes
	 * to implement parser-specific functions.
	 *
	 * This function is called everytime a key has successfully been parsed.
	 * The keyName parameter contains the name of the key that has just been
	 * parsed; this same key is still on top of the Node Stack.
	 *
	 * Access the node stack to view this key's properties and its parents.
	 * Remember to leave the node stack _UNCHANGED_ in your own function. Removal
	 * of closed keys is done automatically.
	 *
	 * When parsing a key, one may chose to skip it, e.g. because it's not needed
	 * on the current configuration. In order to ignore a key, you must set
	 * the "ignore" field of its KeyNode struct to "true": The key and all its children
	 * will then be automatically ignored by the parser.
	 *
	 * Return true if the key was properly handled (this includes the case when the
	 * key is being ignored). False otherwise.
	 * See the sample implementation in GUI::ThemeParser.
	 */
	virtual bool keyCallback(Common::String keyName) {
		return false;
	}

	/**
	 * The closed key callback function must be overloaded by inheriting classes to
	 * implement parser-specific functions.
	 *
	 * The closedKeyCallback is issued once a key has been finished parsing, to let
	 * the parser verify that all the required subkeys, etc, were included.
	 *
	 * Returns true if the key was properly closed, false otherwise.
	 * By default, all keys are properly closed.
	 */
	virtual bool closedKeyCallback(Common::String keyName) {
		return true;
	}

	/**
	 * Parses the value of a given key. There's no reason to overload this.
	 */
	virtual bool parseKeyValue(Common::String keyName);

	/**
	 * Called once a key has been parsed. It handles the closing/cleanup of the
	 * node stack and calls the keyCallback.
	 * There's no reason to overload this.
	 */
	virtual bool parseActiveKey(bool closed);

	/**
	 * Prints an error message when parsing fails and stops the parser.
	 * Parser error always returns "false" so we can pass the return value directly
	 * and break down the parsing.
	 */
	virtual bool parserError(const char *errorString, ...) GCC_PRINTF(2, 3);

	/**
	 * Skips spaces/whitelines etc. Returns true if any spaces were skipped.
	 * Overload this if you want to make your parser depend on newlines or
	 * whatever.
	 */
	virtual bool skipSpaces() {
		if (!isspace(_text[_pos]))
			return false;

		while (_text[_pos] && isspace(_text[_pos]))
			_pos++;

		return true;
	}

	/**
	 * Skips comment blocks and comment lines.
	 * Returns true if any comments were skipped.
	 * Overload this if you want to disable comments on your XML syntax
	 * or to change the commenting syntax.
	 */
	virtual bool skipComments() {
		if (_text[_pos] == '/' && _text[_pos + 1] == '*') {
			_pos += 2;
			while (_text[_pos++]) {
				if (_text[_pos - 2] == '*' && _text[_pos - 1] == '/')
					break;
				if (_text[_pos] == 0)
					parserError("Comment has no closure.");
			}
			return true;
		}

		if (_text[_pos] == '/' && _text[_pos + 1] == '/') {
			_pos += 2;
			while (_text[_pos] && _text[_pos] != '\n' && _text[_pos] != '\r')
				_pos++;
			return true;
		}

		return false;
	}

	/**
	 * Check if a given character can be part of a KEY or VALUE name.
	 * Overload this if you want to support keys with strange characters
	 * in their name.
	 */
	virtual bool isValidNameChar(char c) {
		return isalnum(c) || c == '_';
	}

	/**
	 * Parses a the first textual token found.
	 * There's no reason to overload this.
	 */
	virtual bool parseToken() {
		_token.clear();
		while (isValidNameChar(_text[_pos]))
			_token += _text[_pos++];

		return isspace(_text[_pos]) != 0 || _text[_pos] == '>' || _text[_pos] == '=';
	}

	/**
	 * Parses the values inside an integer key.
	 * The count parameter specifies the number of values inside
	 * the key, which are expected to be separated with commas.
	 *
	 * Sample usage:
	 * parseIntegerKey("255, 255, 255", 3, &red, &green, &blue);
	 * [will parse each field into its own integer]
	 *
	 * parseIntegerKey("1234", 1, &number);
	 * [will parse the single number into the variable]
	 *
	 * @param key String containing the integers to be parsed.
	 * @param count Number of comma-separated ints in the string.
	 * @param ... Integer variables to store the parsed ints, passed
	 *            by reference.
	 * @returns True if the parsing succeeded.
	 */
	virtual bool parseIntegerKey(const char *key, int count, ...) {
		char *parseEnd;
		int *num_ptr;

		va_list args;
		va_start(args, count);

		while (count--) {
			while (isspace(*key))
				key++;

			num_ptr = va_arg(args, int*);
			*num_ptr = strtol(key, &parseEnd, 10);

			key = parseEnd;

			while (isspace(*key))
				key++;

			if (count && *key++ != ',')
				return false;
		}

		va_end(args);
		return (*key == 0);
	}

	/**
	 * Overload if your parser needs to support parsing the same file
	 * several times, so you can clean up the internal state of the 
	 * parser before each parse.
	 */
	virtual void cleanup() {}

	int _pos; /** Current position on the XML buffer. */
	XMLStream _text; /** Buffer with the text being parsed */
	Common::String _fileName;

	ParserState _state; /** Internal state of the parser */

	Common::String _error; /** Current error message */
	Common::String _token; /** Current text token */

	Common::Stack<ParserNode*> _activeKey; /** Node stack of the parsed keys */
};

}

#endif
