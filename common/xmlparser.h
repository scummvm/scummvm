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
#include "common/fs.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"

namespace Common {
	
/***********************************************
 **** XMLParser.cpp/h -- Generic XML Parser ****
 ***********************************************

	This is a simple implementation of a generic parser which is able to
	interpret a subset of the XML language.
	
	The XMLParser class is virtual, and must be derived into a child class,
	called a Custom Parser Class, which will manage the parsed data for your
	specific needs.
	
	Custom Parser Classes have two basic requirements:
	They must inherit directly the XMLParser class, and they must define the
	parsing layout of the XML file.
	
	Declaring the XML layout is done with the help of the CUSTOM_XML_PARSER()
	macro: this macro must appear once inside the Custom Parser Class 
	declaration, and takes a single parameter, the name of the Custom Parser
	Class.
	
	The macro must be followed by the actual layout of the XML files to be 
	parsed, and closed with the PARSER_END() macro. The layout of XML files
	is defined by the use of 3 helper macros: XML_KEY(), KEY_END() and 
	XML_PROP().
	
	Here's a sample of its usage:
	
	===========	===========	===========	===========	===========	===========
	
	CUSTOM_XML_PARSER(ThemeParser) {
		XML_KEY(render_info)
			XML_KEY(palette)
				XML_KEY(color)
					XML_PROP(name, true)
					XML_PROP(rgb, true)
				KEY_END()
			KEY_END()

			XML_KEY(fonts)
				XML_KEY(font)
					XML_PROP(id, true)
					XML_PROP(type, true)
					XML_PROP(color, true)
				KEY_END()
			KEY_END()

			XML_KEY(defaults)
				XML_PROP(stroke, false)
				XML_PROP(shadow, false)
				XML_PROP(factor, false)
				XML_PROP(fg_color, false)
				XML_PROP(bg_color, false)
				XML_PROP(gradient_start, false)
				XML_PROP(gradient_end, false)
				XML_PROP(gradient_factor, false)
				XML_PROP(fill, false)
			KEY_END()
		KEY_END()
	} PARSER_END()
			
	===========	===========	===========	===========	===========	===========
	
	The XML_KEY() macro takes a single argument, the name of the expected key.
	Inside the scope of each key, you may define properties for the given key
	with the XML_PROP() macro, which takes as parameters the name of the 
	property and whether it's optional or required. You might also define the 
	contained children keys, using the XML_KEY() macro again.
	The scope of a XML key is closed with the KEY_END() macro.
	
	Keys which may contain any kind of Property names may be defined with the
	XML_PROP_ANY() macro instead of the XML_PROP() macro. This macro takes no
	arguments.
	
	As an example, the following XML layout:
	
		XML_KEY(palette)
			XML_KEY(color)
				XML_PROP(name, true)
				XML_PROP(rgb, true)
				XML_PROP(optional_param, false)
			KEY_END()
		KEY_END()
		
	will expect to parse a syntax like this:
	
		<palette>
			<color name = "red" rgb = "255, 0, 0" />
			<color name = "blue" rgb = "0, 0, 255" optional_param = "565" />
		</palette>
		
	Once a layout has been defined, everytime a XML node (that is, a key and
	all its properties) has been parsed, a specific callback funcion is called,
	which should take care of managing the parsed data for the node.
	
	Callback functions must be explicitly declared with the following syntax:
	
		bool parserCallback_KEYNAME(ParserNode *node);
		
	A callback function is needed for each key that can be parsed, since they
	are called automatically; the function will receive a pointer to the XML
	Node that has been parsed. This XML Node has the following properties:
	
		- It's assured to be expected in the layout of the XML file (i.e. 
		  has the proper position and depth in the XML tree).
		
		- It's assured to contain all the required Properties that have 
		  been declared in the XML layout.
		
		- It's assured to contain NO unexpected properties (i.e. properties
		  which haven't been declared in the XML layout).
		
	Further validation of the Node's data may be performed inside the callback
	function. Once the node has been validated and its data has been parsed/
	managed, the callback function is expected to return true.
	
	If the data in the XML Node is corrupted or there was a problem when 
	parsing it, the callback function is expected to return false or, 
	preferably, to throw a parserError() using the following syntax:
	
		return parserError("There was a problem in key '%s'.", arg1, ...);
	
	Also, note that the XML parser doesn't take into account the actual order
	of the keys and properties in the XML layout definition, only its layout 
	and relationships.
	
	Lastly, when defining your own Custom XML Parser, further customization 
	may be accomplished _optionally_ by overloading several virtual functions
	of the XMLParser class.
	
	Check the API documentation of the following functions for more info:
		
		virtual bool closedKeyCallback(ParserNode *node);
		virtual bool skipComments();
		virtual bool isValidNameChar(char c);
		virtual void cleanup();
		
	Check the sample implementation of the GUI::ThemeParser custom parser
	for a working sample of a Custom XML Parser.
		
*/
			
#define XML_KEY(keyName) {\
		lay =  new CustomXMLKeyLayout;\
		lay->callback = (&kLocalParserName::parserCallback_##keyName);\
		layout.top()->children[#keyName] = lay;\
		layout.push(lay); \
		_layoutList.push_back(lay);\
		for (Common::List<XMLKeyLayout::XMLKeyProperty>::const_iterator p = globalProps.begin(); p != globalProps.end(); ++p){\
			layout.top()->properties.push_back(*p);}
		
#define XML_KEY_RECURSIVE(keyName) {\
			layout.top()->children[#keyName] = layout.top();\
			layout.push(layout.top());\
		}

#define KEY_END() layout.pop(); }

#define XML_PROP(propName, req) {\
		prop.name = #propName; \
		prop.required = req; \
		layout.top()->properties.push_back(prop); }
		
#define XML_GLOBAL_PROP(propName, req) {\
		prop.name = #propName; \
		prop.required = req;\
		globalProps.push_back(prop); }
		
	
#define CUSTOM_XML_PARSER(parserName) \
	protected: \
	typedef parserName kLocalParserName; \
	bool keyCallback(ParserNode *node) {return node->layout->doCallback(this, node); }\
	struct CustomXMLKeyLayout : public XMLKeyLayout {\
		typedef bool (parserName::*ParserCallback)(ParserNode *node);\
		ParserCallback callback;\
		bool doCallback(XMLParser *parent, ParserNode *node) {return ((kLocalParserName*)parent->*callback)(node);} };\
	virtual void buildLayout() { \
		Common::Stack<XMLKeyLayout*> layout; \
		CustomXMLKeyLayout *lay = 0; \
		XMLKeyLayout::XMLKeyProperty prop; \
		Common::List<XMLKeyLayout::XMLKeyProperty> globalProps; \
		_XMLkeys = new CustomXMLKeyLayout; \
		layout.push(_XMLkeys);
	
#define PARSER_END() layout.clear(); }

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
	XMLParser() : _XMLkeys(0) {}

	virtual ~XMLParser() {
		while (!_activeKey.empty())
			delete _activeKey.pop();

		delete _XMLkeys;

		for (Common::List<XMLKeyLayout*>::iterator i = _layoutList.begin();
			i != _layoutList.end(); ++i)
			delete *i;

		_layoutList.clear();
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
	
	struct XMLKeyLayout;
	struct ParserNode;
	
	typedef Common::HashMap<Common::String, XMLParser::XMLKeyLayout*, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ChildMap;
	
	/** nested struct representing the layout of the XML file */
	struct XMLKeyLayout {
		struct XMLKeyProperty {
			Common::String name;
			bool required;
		};
		
		Common::List<XMLKeyProperty> properties;
		ChildMap children;
		
		virtual bool doCallback(XMLParser *parent, ParserNode *node) = 0;
		
		virtual ~XMLKeyLayout() {
			properties.clear();
		}
	} *_XMLkeys;

	/** Struct representing a parsed node */
	struct ParserNode {
		Common::String name;
		Common::StringMap values;
		bool ignore;
		int depth;
		XMLKeyLayout *layout;
	};

	/**
	 * Loads a file into the parser.
	 * Used for the loading of Theme Description files
	 * straight from the filesystem.
	 *
	 * @param filename Name of the file to load.
	 */
	bool loadFile(const Common::String &filename) {
		Common::File *f = new Common::File;

		if (!f->open(filename)) {
			delete f;
			return false;
		}

		_fileName = filename;
		_text.loadStream(f);
		return true;
	}
	
	bool loadFile(const FilesystemNode &node) {
		Common::File *f = new Common::File;
		
		if (!f->open(node)) {
			delete f;
			return false;
		}
		
		_fileName = node.getName();
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
	bool loadBuffer(const byte *buffer, uint32 size, bool disposable = false) {
		_text.loadStream(new MemoryReadStream(buffer, size, disposable));
		_fileName = "Memory Stream";
		return true;
	}
	
	bool loadStream(MemoryReadStream *stream) {
		_text.loadStream(stream);
		_fileName = "Compressed File Stream";
		return true;
	}

	/**
	 * The actual parsing function.
	 * Parses the loaded data stream, returns true if successful.
	 */
	bool parse();

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
	 * The buildLayout function builds the layout for the parser to use
	 * based on a series of helper macros. This function is automatically
	 * generated by the CUSTOM_XML_PARSER() macro on custom parsers.
	 *
	 * See the documentation regarding XML layouts.
	 */
	virtual void buildLayout() = 0;
	
	/**
	 * The keycallback function is automatically overloaded on custom parsers
	 * when using the CUSTOM_XML_PARSER() macro. 
	 *
	 * Its job is to call the corresponding Callback function for the given node.
	 * A function for each key type must be declared separately. See the custom
	 * parser creation instructions.
	 *
	 * When parsing a key in such function, one may chose to skip it, e.g. because it's not needed
	 * on the current configuration. In order to ignore a key, you must set
	 * the "ignore" field of its KeyNode struct to "true": The key and all its children
	 * will then be automatically ignored by the parser.
	 *
	 * The callback function must return true if the key was properly handled (this includes the case when the
	 * key is being ignored). False otherwise. The return of keyCallback() is the same as
	 * the callback function's.
	 * See the sample implementation in GUI::ThemeParser.
	 */
	virtual bool keyCallback(ParserNode *node) = 0;

	/**
	 * The closed key callback function MAY be overloaded by inheriting classes to
	 * implement parser-specific functions.
	 *
	 * The closedKeyCallback is issued once a key has been finished parsing, to let
	 * the parser verify that all the required subkeys, etc, were included.
	 *
	 * Unlike the keyCallbacks(), there's just a closedKeyCallback() for all keys.
	 * Use "node->name" to distinguish between each key type.
	 *
	 * Returns true if the key was properly closed, false otherwise.
	 * By default, all keys are properly closed.
	 */
	virtual bool closedKeyCallback(ParserNode *node) {
		return true;
	}
	
	/**
	 * Called when a node is closed. Manages its cleanup and calls the
	 * closing callback function if needed.
	 */
	bool closeKey();

	/**
	 * Parses the value of a given key. There's no reason to overload this.
	 */
	bool parseKeyValue(Common::String keyName);

	/**
	 * Called once a key has been parsed. It handles the closing/cleanup of the
	 * node stack and calls the keyCallback.
	 */
	bool parseActiveKey(bool closed);

	/**
	 * Prints an error message when parsing fails and stops the parser.
	 * Parser error always returns "false" so we can pass the return value directly
	 * and break down the parsing.
	 */
	bool parserError(const char *errorString, ...) GCC_PRINTF(2, 3);

	/**
	 * Skips spaces/whitelines etc. Returns true if any spaces were skipped.
	 */
	bool skipSpaces() {
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
	bool parseToken() {
		_token.clear();
		while (isValidNameChar(_text[_pos]))
			_token += _text[_pos++];

		return isspace(_text[_pos]) != 0 || _text[_pos] == '>' || _text[_pos] == '=' || _text[_pos] == '/';
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
	bool parseIntegerKey(const char *key, int count, ...) {
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

	Common::List<XMLKeyLayout*> _layoutList;

private:
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
