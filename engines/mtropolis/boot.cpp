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

#include "common/crc.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/compression/stuffit.h"
#include "common/compression/vise.h"
#include "common/formats/winexe.h"
#include "common/compression/installshieldv3_archive.h"
#include "common/compression/installshield_cab.h"

#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

#include "mtropolis/boot.h"
#include "mtropolis/detection.h"
#include "mtropolis/runtime.h"
#include "mtropolis/subtitles.h"
#include "mtropolis/vfs.h"

#include "mtropolis/plugin/mti.h"
#include "mtropolis/plugin/obsidian.h"
#include "mtropolis/plugin/standard.h"
#include "mtropolis/plugins.h"

namespace MTropolis {

namespace Boot {

class GameDataHandler;
class BootScriptContext;

struct ManifestSubtitlesDef {
	Common::String speakerTablePath;
	Common::String linesTablePath;
	Common::String assetMappingTablePath;
	Common::String modifierMappingTablePath;
};

struct Game {
	MTropolisGameBootID bootID;
	void (BootScriptContext::*bootFunction)();
};

template<class T>
struct GameDataHandlerFactory {
	static GameDataHandler *create(const Boot::Game &game, const MTropolisGameDescription &desc) {
		return new T(game, desc);
	}
};

template<class T>
class PersistentResource : public ProjectPersistentResource {
public:
	explicit PersistentResource(const Common::SharedPtr<T> &item);
	const Common::SharedPtr<T> &getItem();

	static Common::SharedPtr<ProjectPersistentResource> wrap(const Common::SharedPtr<T> &item);

private:
	Common::SharedPtr<T> _item;
};

template<class T>
PersistentResource<T>::PersistentResource(const Common::SharedPtr<T> &item) : _item(item) {
}

template<class T>
const Common::SharedPtr<T> &PersistentResource<T>::getItem() {
	return _item;
}

template<class T>
Common::SharedPtr<ProjectPersistentResource> PersistentResource<T>::wrap(const Common::SharedPtr<T> &item) {
	return Common::SharedPtr<ProjectPersistentResource>(new PersistentResource<T>(item));
}

class ObsidianGameDataHandler  {
public:
	static Common::SharedPtr<MTropolis::PlugIn> loadPlugIn(Common::Archive &fs, const Common::Path &pluginsLocation, bool isMac, bool isRetail, bool isEnglish);

private:
	static Common::SharedPtr<Obsidian::WordGameData> loadWinWordGameData(Common::SeekableReadStream *stream);
	static Common::SharedPtr<Obsidian::WordGameData> loadMacWordGameData(Common::SeekableReadStream *stream);
};

Common::SharedPtr<MTropolis::PlugIn> ObsidianGameDataHandler::loadPlugIn(Common::Archive &fs, const Common::Path &pluginsLocation, bool isMac, bool isRetail, bool isEnglish) {
	Common::SharedPtr<Obsidian::WordGameData> wgData;

	if (isRetail && isEnglish) {
		if (isMac) {
			Common::MacResManager resMan;

			Common::SharedPtr<Common::SeekableReadStream> dataStream(resMan.openFileOrDataFork(pluginsLocation.appendComponent("RSGKit.rPP"), fs));

			if (!dataStream)
				error("Failed to open word game data");

			wgData = loadMacWordGameData(dataStream.get());
		} else {
			Common::SharedPtr<Common::SeekableReadStream> stream(fs.createReadStreamForMember(pluginsLocation.appendComponent("RSGKit.r95")));

			if (!stream)
				error("Failed to open word game data");

			wgData = loadWinWordGameData(stream.get());
		}

	}

	Common::SharedPtr<Obsidian::ObsidianPlugIn> obsidianPlugIn(new Obsidian::ObsidianPlugIn(wgData));
	return obsidianPlugIn.staticCast<MTropolis::PlugIn>();
}

Common::SharedPtr<Obsidian::WordGameData> ObsidianGameDataHandler::loadMacWordGameData(Common::SeekableReadStream *stream) {
	Common::SharedPtr<Obsidian::WordGameData> wgData(new Obsidian::WordGameData());

	Obsidian::WordGameLoadBucket buckets[] = {
		{0, 0},             // 0 letters
		{0xD7C8, 0xD7CC},   // 1 letter
		{0xD7CC, 0xD84D},   // 2 letter
		{0xD84D, 0xE25D},   // 3 letter
		{0x1008C, 0x12AA8}, // 4 letter
		{0x14C58, 0x19614}, // 5 letter
		{0x1C73C, 0x230C1}, // 6 letter
		{0x26D10, 0x2EB98}, // 7 letter
		{0x32ADC, 0x3AA0E}, // 8 letter
		{0x3E298, 0x45B88}, // 9 letter
		{0x48BE8, 0x4E0D0}, // 10 letter
		{0x4FFB0, 0x53460}, // 11 letter
		{0x545F0, 0x56434}, // 12 letter
		{0x56D84, 0x57CF0}, // 13 letter
		{0x58158, 0x58833}, // 14 letter
		{0x58A08, 0x58CD8}, // 15 letter
		{0x58D8C, 0x58EAD}, // 16 letter
		{0x58EF4, 0x58F72}, // 17 letter
		{0x58F90, 0x58FDC}, // 18 letter
		{0, 0},             // 19 letter
		{0x58FEC, 0x59001}, // 20 letter
		{0x59008, 0x59034}, // 21 letter
		{0x5903C, 0x59053}, // 22 letter
	};

	if (!wgData->load(stream, buckets, 23, 1, false))
		error("Failed to load word game data");

	return wgData;
}

Common::SharedPtr<Obsidian::WordGameData> ObsidianGameDataHandler::loadWinWordGameData(Common::SeekableReadStream *stream) {
	Common::SharedPtr<Obsidian::WordGameData> wgData(new Obsidian::WordGameData());

	Obsidian::WordGameLoadBucket buckets[] = {
		{0, 0},             // 0 letters
		{0x63D54, 0x63D5C}, // 1 letter
		{0x63BF8, 0x63CA4}, // 2 letter
		{0x627D8, 0x631E8}, // 3 letter
		{0x5C2C8, 0x60628}, // 4 letter
		{0x52F4C, 0x5919C}, // 5 letter
		{0x47A64, 0x4F2FC}, // 6 letter
		{0x3BC98, 0x43B20}, // 7 letter
		{0x2DA78, 0x38410}, // 8 letter
		{0x218F8, 0x2AA18}, // 9 letter
		{0x19D78, 0x1FA18}, // 10 letter
		{0x15738, 0x18BE8}, // 11 letter
		{0x128A8, 0x14DE8}, // 12 letter
		{0x1129C, 0x1243C}, // 13 letter
		{0x10974, 0x110C4}, // 14 letter
		{0x105EC, 0x108BC}, // 15 letter
		{0x10454, 0x105A8}, // 16 letter
		{0x103A8, 0x10434}, // 17 letter
		{0x10348, 0x10398}, // 18 letter
		{0, 0},             // 19 letter
		{0x10328, 0x10340}, // 20 letter
		{0x102EC, 0x1031C}, // 21 letter
		{0x102D0, 0x102E8}, // 22 letter
	};

	if (!wgData->load(stream, buckets, 23, 4, true)) {
		error("Failed to load word game data file");
		return nullptr;
	}

	return wgData;
}

static void loadCursorsMac(Common::Archive &archive, const Common::Path &path, CursorGraphicCollection &cursorGraphics) {
	Common::MacResManager resMan;

	if (!resMan.open(path, archive))
		return;

	const uint32 bwType = MKTAG('C', 'U', 'R', 'S');
	const uint32 colorType = MKTAG('c', 'r', 's', 'r');

	Common::MacResIDArray bwIDs = resMan.getResIDArray(bwType);
	Common::MacResIDArray colorIDs = resMan.getResIDArray(colorType);

	Common::MacResIDArray bwOnlyIDs;
	for (Common::MacResIDArray::const_iterator bwIt = bwIDs.begin(), bwItEnd = bwIDs.end(); bwIt != bwItEnd; ++bwIt) {
		bool hasColor = false;
		for (Common::MacResIDArray::const_iterator colorIt = colorIDs.begin(), colorItEnd = colorIDs.end(); colorIt != colorItEnd; ++colorIt) {
			if ((*colorIt) == (*bwIt)) {
				hasColor = true;
				break;
			}
		}

		if (!hasColor)
			bwOnlyIDs.push_back(*bwIt);
	}

	int numCursorsLoaded = 0;
	for (int cti = 0; cti < 2; cti++) {
		const uint32 resType = (cti == 0) ? bwType : colorType;
		const bool isBW = (cti == 0);
		const Common::MacResIDArray &resArray = (cti == 0) ? bwOnlyIDs : colorIDs;

		for (size_t i = 0; i < resArray.size(); i++) {
			Common::SharedPtr<Common::SeekableReadStream> resData(resMan.getResource(resType, resArray[i]));
			if (!resData) {
				warning("Failed to open cursor resource");
				return;
			}

			Common::SharedPtr<Graphics::MacCursor> cursor(new Graphics::MacCursor());
			// Some CURS resources are 72 bytes instead of the expected 68, make sure they load as the correct format
			if (!cursor->readFromStream(*resData, isBW, 0xff, isBW)) {
				warning("Failed to load cursor resource");
				return;
			}

			cursorGraphics.addMacCursor(resArray[i], cursor);
			numCursorsLoaded++;
		}
	}
}

static bool loadCursorsWin(Common::Archive &archive, const Common::Path &path, CursorGraphicCollection &cursorGraphics) {
	Common::SharedPtr<Common::SeekableReadStream> stream(archive.createReadStreamForMember(path));

	if (!stream)
		error("Failed to open file '%s'", path.toString(archive.getPathSeparator()).c_str());

	Common::SharedPtr<Common::WinResources> winRes(Common::WinResources::createFromEXE(stream.get()));
	if (!winRes)
		return false;

	int numCursorGroupsLoaded = 0;
	Common::Array<Common::WinResourceID> cursorGroupIDs = winRes->getIDList(Common::kWinGroupCursor);
	for (Common::Array<Common::WinResourceID>::const_iterator it = cursorGroupIDs.begin(), itEnd = cursorGroupIDs.end(); it != itEnd; ++it) {
		const Common::WinResourceID &id = *it;

		Common::SharedPtr<Graphics::WinCursorGroup> cursorGroup(Graphics::WinCursorGroup::createCursorGroup(winRes.get(), *it));
		if (!winRes) {
			warning("Couldn't load cursor group");
			return false;
		}

		if (cursorGroup->cursors.size() == 0) {
			// Empty?
			continue;
		}

		cursorGraphics.addWinCursorGroup(id.getID(), cursorGroup);
		numCursorGroupsLoaded++;
	}

	return true;
}

class BootScriptParser {
public:
	enum TokenType {
		kTokenTypeBooleanConstant,
		kTokenTypeOctalConstant,
		kTokenTypeHexConstant,
		kTokenTypeFloatConstant,
		kTokenTypeDecimalConstant,
		kTokenTypeIdentifier,
		kTokenTypePunctuation,
		kTokenTypeString,
		kTokenTypeChar,
	};

	enum ExprType {
		kExprTypeIdentifier,
		kExprTypeIntegral,
		kExprTypeFloat,
		kExprTypeString,
		kExprTypeChar,
		kExprTypePunctuation,
		kExprTypeBoolean,
	};

	explicit BootScriptParser(Common::ReadStream &stream);

	bool readToken(Common::String &outToken);

	void expect(const char *token);

	static TokenType classifyToken(const Common::String &token);
	static ExprType tokenTypeToExprType(TokenType tt);

	static Common::String evalString(const Common::String &token);
	static uint evalIntegral(const Common::String &token);

private:
	bool readChar(char &c);
	void requeueChar(char c);

	void skipLineComment();
	bool skipBlockComment();

	bool parseNumber(char firstChar, Common::String &outToken);
	bool parseIdentifier(Common::String &outToken);
	bool parseQuotedString(char quoteChar, Common::String &outToken);
	bool parseFloatFractionalPart(Common::String &outToken);	// Parses the part after the '.'
	bool parseFloatExponentPart(Common::String &outToken); // Parses the part after the 'e'
	bool parseHexDigits(Common::String &outToken);	// Must parse at least 1
	bool parseOctalDigits(Common::String &outToken);
	bool checkFloatSuffix();

	static bool isIdentifierInitialChar(char c);
	static bool isIdentifierChar(char c);
	static bool isDigit(char c);
	static bool isAlpha(char c);

	static uint evalOctalIntegral(const Common::String &token);
	static uint evalDecimalIntegral(const Common::String &token);
	static uint evalHexIntegral(const Common::String &token);

	static char evalEscapeSequence(const Common::String &token, uint startPos, uint maxEndPos, uint &outLength);
	static char evalOctalEscapeSequence(const Common::String &token, uint startPos, uint maxEndPos, uint &outLength);
	static char evalHexEscapeSequence(const Common::String &token, uint startPos, uint maxEndPos, uint &outLength);

	Common::ReadStream &_stream;
	char _requeuedChars[2];
	int _numRequeuedChars;
	bool _isEOS;
};


BootScriptParser::BootScriptParser(Common::ReadStream &stream) : _stream(stream), _requeuedChars{0, 0}, _numRequeuedChars(0), _isEOS(false) {
}

bool BootScriptParser::readToken(Common::String &outToken) {
	// Skip whitespace
	char firstChar = 0;
	char secondChar = 0;

	for (;;) {
		if (!readChar(firstChar))
			return false;

		if (firstChar == '/') {
			if (!readChar(secondChar)) {
				outToken = "/";
				return true;
			}

			if (secondChar == '/') {
				skipLineComment();
			} else if (secondChar == '*') {
				if (!skipBlockComment())
					return false;
			} else {
				requeueChar(secondChar);
				return true;
			}

			continue;
		}

		// Ignore whitespace
		if (firstChar >= 0 && firstChar <= 32)
			continue;

		if (isDigit(firstChar))
			return parseNumber(firstChar, outToken);

		if (isIdentifierInitialChar(firstChar)) {
			requeueChar(firstChar);
			return parseIdentifier(outToken);
		}

		if (firstChar == '\'' || firstChar == '\"')
			return parseQuotedString(firstChar, outToken);

		if (firstChar == '.') {
			if (readChar(secondChar)) {
				if (secondChar == '.') {
					char thirdChar = 0;
					if (readChar(thirdChar)) {
						if (thirdChar == '.') {
							outToken = "...";
							return true;
						} else {
							requeueChar(thirdChar);
						}
					}
				} else if (isDigit(secondChar)) {
					Common::String fractionalPart;
					if (!parseFloatFractionalPart(fractionalPart))
						return false;

					outToken = Common::String('.') + fractionalPart;
					return true;
				} else {
					requeueChar(secondChar);
				}
			}
			outToken = ".";
			return true;
		}

		if (firstChar == ':') {
			if (readChar(secondChar)) {
				if (secondChar == ':') {
					outToken = "::";
					return true;
				} else {
					requeueChar(secondChar);
				}
			}

			outToken = ":";
			return true;
		}

		switch (firstChar) {
		case '+':
		case '-':
		case '=':
		case '<':
		case '>':
		case '|':
		case '&':
			if (!readChar(secondChar)) {
				outToken = Common::String(firstChar);
				return true;
			} else if (secondChar == firstChar || secondChar == '=') {
				char constructedString[2] = {firstChar, secondChar};
				outToken = Common::String(constructedString, 2);
				return true;
			} else {
				requeueChar(secondChar);
				outToken = Common::String(firstChar);
			}

			return true;

		case '*':
		case '/':
		case '%':
		case '!':
		case '^':
			if (!readChar(secondChar)) {
				outToken = Common::String(firstChar);
				return true;
			} else if (secondChar == '=') {
				char constructedString[2] = {firstChar, secondChar};
				outToken = Common::String(constructedString, 2);
				return true;
			} else {
				requeueChar(secondChar);
				outToken = Common::String(firstChar);
			}

			return true;
		case ',':
		case ';':
		case '?':
		case '[':
		case ']':
		case '(':
		case ')':
		case '{':
		case '}':
			outToken = Common::String(firstChar);
			return true;
		default:
			error("Unrecognized token in boot script: %c", firstChar);
			return false;
		};
	}
}

void BootScriptParser::expect(const char *expectedToken) {
	Common::String token;
	if (!readToken(token))
		error("Expected '%s' but found EOF", expectedToken);

	if (token != expectedToken)
		error("Expected '%s' but found '%s'", expectedToken, token.c_str());
}

BootScriptParser::TokenType BootScriptParser::classifyToken(const Common::String &token) {
	if (token.size() == 0 || token == "." || token == "...")
		return kTokenTypePunctuation;

	if (token[0] == '.')
		return kTokenTypeFloatConstant;

	if (isDigit(token[0])) {
		if (token.size() > 1 && (token[1] == 'x' || token[1] == 'X'))
			return kTokenTypeHexConstant;

		for (char c : token) {
			if (c == '.' || c == 'f' || c == 'F' || c == 'e' || c == 'E')
				return kTokenTypeFloatConstant;
		}

		if (token[0] == '0')
			return kTokenTypeOctalConstant;

		return kTokenTypeDecimalConstant;
	}

	if (isIdentifierInitialChar(token[0])) {
		if (token == "true" || token == "false")
			return kTokenTypeBooleanConstant;

		return kTokenTypeIdentifier;
	}

	if (token[0] == '\'')
		return kTokenTypeChar;

	if (token[0] == '\"')
		return kTokenTypeString;

	return kTokenTypePunctuation;
}

BootScriptParser::ExprType BootScriptParser::tokenTypeToExprType(BootScriptParser::TokenType tt) {
	switch (tt) {
	case kTokenTypeBooleanConstant:
		return kExprTypeBoolean;
	case kTokenTypeChar:
		return kExprTypeChar;
	case kTokenTypeDecimalConstant:
	case kTokenTypeOctalConstant:
	case kTokenTypeHexConstant:
		return kExprTypeIntegral;
	case kTokenTypeFloatConstant:
		return kExprTypeFloat;
	case kTokenTypeString:
		return kExprTypeString;
	default:
		return kExprTypePunctuation;
	}

	return kExprTypeString;
}

Common::String BootScriptParser::evalString(const Common::String &token) {
	assert(token.size() >= 2);
	assert(token[0] == '\"');
	assert(token[token.size() - 1] == '\"');

	uint endPos = token.size() - 1;

	Common::Array<char> chars;
	chars.resize(token.size() - 2);

	uint numChars = 0;

	for (uint i = 1; i < endPos; i++) {
		char c = token[i];
		if (c == '\\') {
			uint escapeLength = 0;

			c = evalEscapeSequence(token, i + 1, endPos, escapeLength);
			i += escapeLength;
		}

		chars[numChars++] = c;
	}

	if (numChars == 0)
		return "";

	return Common::String(&chars[0], numChars);
}

uint BootScriptParser::evalIntegral(const Common::String &token) {
	if (token.size() == 1)
		return evalDecimalIntegral(token);

	if (token[1] == 'x' || token[1] == 'X')
		return evalHexIntegral(token);

	if (token[0] == '0')
		return evalOctalIntegral(token);

	return evalDecimalIntegral(token);
}

bool BootScriptParser::readChar(char &c) {
	if (_numRequeuedChars > 0) {
		_numRequeuedChars--;
		c = _requeuedChars[_numRequeuedChars];
		return true;
	}

	if (_isEOS)
		return false;

	if (_stream.read(&c, 1) == 0) {
		_isEOS = true;
		return false;
	}

	return true;
}

void BootScriptParser::requeueChar(char c) {
	assert(_numRequeuedChars < static_cast<int>(sizeof(_requeuedChars)));
	_requeuedChars[_numRequeuedChars++] = c;
}

void BootScriptParser::skipLineComment() {
	char ch = 0;

	while (readChar(ch)) {
		if (ch == '\r') {
			if (readChar(ch)) {
				if (ch != '\n')
					requeueChar(ch);
			}

			return;
		}

		if (ch == '\n')
			return;
	}
}

bool BootScriptParser::skipBlockComment() {
	char ch = 0;

	while (readChar(ch)) {
		if (ch == '*') {
			if (readChar(ch)) {
				if (ch == '/')
					return true;
				else
					requeueChar(ch);
			}
		}
	}

	warning("Unexpected EOF in boot script block comment!");
	return false;
}

bool BootScriptParser::parseNumber(char firstChar, Common::String &outToken) {
	char ch = 0;

	if (firstChar == '0') {
		bool mightBeOctal = true;

		if (readChar(ch)) {
			if (ch == 'x' || ch == 'X') {
				char prefix[2] = {firstChar, ch};
				Common::String hexDigits;
				if (!parseHexDigits(hexDigits))
					return false;

				outToken = Common::String(prefix, 2) + hexDigits;
				return true;
			} else if (ch == '.' || ch == 'e' || ch == 'E') {
				mightBeOctal = false;
				requeueChar(ch);
			} else
				requeueChar(ch);
		}

		if (mightBeOctal) {
			Common::String octalDigits;
			if (!parseOctalDigits(octalDigits))
				return false;

			outToken = Common::String('0') + octalDigits;
			return true;
		}
	}

	outToken = Common::String(firstChar);

	// Decimal number
	for (;;) {
		if (!readChar(ch))
			return true;

		if (ch >= '0' && ch <= '9') {
			outToken += ch;
			continue;
		}

		if (ch == '.') {
			outToken += ch;

			Common::String fractionalPart;
			if (!parseFloatFractionalPart(fractionalPart))
				return false;

			outToken += fractionalPart;
			return true;
		}

		if (ch == 'e' || ch == 'E') {
			outToken += ch;

			Common::String exponentPart;
			if (!parseFloatExponentPart(exponentPart))
				return false;

			outToken += exponentPart;
			return true;
		}

		if (isAlpha(ch)) {
			warning("Invalid floating point constant in boot script");
			return false;
		}

		requeueChar(ch);
		return true;
	}
}

bool BootScriptParser::parseIdentifier(Common::String &outToken) {
	outToken.clear();

	char ch = 0;
	while (readChar(ch)) {
		if (isIdentifierChar(ch))
			outToken += ch;
		else {
			requeueChar(ch);
			break;
		}
	}

	return true;
}

bool BootScriptParser::parseQuotedString(char quoteChar, Common::String &outToken) {
	outToken = Common::String(quoteChar);

	char ch = 0;
	while (readChar(ch)) {
		if (ch == '\r' || ch == '\n')
			break;

		outToken += ch;

		if (ch == '\\') {
			if (!readChar(ch))
				break;

			outToken += ch;
		} else if (ch == quoteChar)
			return true;
	}

	error("Unterminated quoted string/char in boot script");

	return false;
}

bool BootScriptParser::parseFloatFractionalPart(Common::String &outToken) {
	for (;;) {
		char ch = 0;
		if (!readChar(ch))
			return true;

		if (ch == 'e' || ch == 'E') {
			outToken += ch;

			Common::String expPart;
			if (!parseFloatExponentPart(expPart))
				return false;

			outToken += expPart;
			return true;
		}

		if (isDigit(ch))
			outToken += ch;
		else if (ch == 'f' || ch == 'F') {
			outToken += ch;
			if (!checkFloatSuffix())
				return false;

			return true;
		} else if (isAlpha(ch)) {
			error("Invalid characters in floating point constant");
		} else
			return true;
	}
}

bool BootScriptParser::parseFloatExponentPart(Common::String &outToken) {
	char ch = 0;

	if (!readChar(ch)) {
		error("Missing digit sequence in floating point constant");
		return false;
	}

	if (ch == '-' || ch == '+') {
		outToken += ch;
		if (!readChar(ch)) {
			error("Missing digit sequence in floating point constant");
			return false;
		}
	}

	if (!isDigit(ch)) {
		error("Missing digit sequence in floating point constant");
		return false;
	}

	for (;;) {
		if (isDigit(ch))
			outToken += ch;
		else if (ch == 'f' || ch == 'F') {
			outToken += ch;
			if (!checkFloatSuffix())
				return false;

			return true;
		} else if (isAlpha(ch)) {
			error("Invalid characters in floating point constant");
			return false;
		} else
			return true;
	}
}

bool BootScriptParser::parseHexDigits(Common::String &outToken) {
	char ch = 0;

	if (!readChar(ch)) {
		error("Missing hex digits in boot script constant");
		return false;
	}

	for (;;) {
		if (isDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
			outToken += ch;
		else if (isAlpha(ch)) {
			error("Invalid characters in hex constant");
			return false;
		} else
			return true;
	}
}

bool BootScriptParser::parseOctalDigits(Common::String &outToken) {
	char ch = 0;

	for (;;) {
		if (ch >= '0' && ch <= '7')
			outToken += ch;
		else if (isAlpha(ch) || isDigit(ch)) {
			error("Invalid characters in octal constant");
			return false;
		} else
			return true;
	}
}

bool BootScriptParser::checkFloatSuffix() {
	char ch = 0;

	if (readChar(ch)) {
		if (isIdentifierChar(ch)) {
			error("Invalid characters after floating point suffix");
			return false;
		}

		requeueChar(ch);
	}

	return true;
}

bool BootScriptParser::isIdentifierInitialChar(char c) {
	return isAlpha(c) || (c == '_');
}

bool BootScriptParser::isIdentifierChar(char c) {
	return isDigit(c) || isIdentifierInitialChar(c);
}

bool BootScriptParser::isDigit(char c) {
	return c >= '0' && c <= '9';
}

bool BootScriptParser::isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

uint BootScriptParser::evalOctalIntegral(const Common::String &token) {
	uint result = 0;
	uint maxBeforeMul = std::numeric_limits<uint>::max() / 8;

	for (uint i = 0; i < token.size(); i++) {
		if (result > maxBeforeMul)
			error("Integer overflow evaluating octal value %s", token.c_str());

		char c = token[i];
		if (c >= '0' && c <= '7')
			result = result * 8u + static_cast<uint>(c - '0');
		else
			error("Invalid character in octal constant %s", token.c_str());
	}

	return result;
}

uint BootScriptParser::evalDecimalIntegral(const Common::String &token) {
	uint result = 0;
	uint maxBeforeMul = std::numeric_limits<uint>::max() / 10;

	for (uint i = 0; i < token.size(); i++) {
		if (result > maxBeforeMul)
			error("Integer overflow evaluating octal value %s", token.c_str());

		char c = token[i];
		if (c >= '0' && c <= '9')
			result = result * 10u + static_cast<uint>(c - '0');
		else
			error("Invalid character in octal constant %s", token.c_str());
	}

	return result;
}

uint BootScriptParser::evalHexIntegral(const Common::String &token) {
	uint result = 0;
	uint maxBeforeMul = std::numeric_limits<uint>::max() / 16;

	for (uint i = 2; i < token.size(); i++) {
		if (result > maxBeforeMul)
			error("Integer overflow evaluating octal value %s", token.c_str());

		char c = token[i];
		if (c >= '0' && c <= '9')
			result = result * 16u + static_cast<uint>(c - '0');
		else if (c >= 'a' && c <= 'f')
			result = result * 16u + static_cast<uint>(c - 'a' + 0xa);
		else if (c >= 'A' && c <= 'F')
			result = result * 16u + static_cast<uint>(c - 'A' + 0xA);
		else
			error("Invalid character in hex constant %s", token.c_str());
	}

	return result;
}

char BootScriptParser::evalEscapeSequence(const Common::String &token, uint startPos, uint maxEndPos, uint &outLength) {
	if (startPos == maxEndPos)
		error("Unexpectedly terminated escape sequence in token %s", token.c_str());

	char firstEscapeChar = token[startPos];

	if (firstEscapeChar == 'x') {
		uint hexLength = 0;
		char c = evalHexEscapeSequence(token, startPos + 1, maxEndPos, hexLength);
		outLength = hexLength + 1;
		return c;
	}

	if (firstEscapeChar >= '0' && firstEscapeChar <= '7')
		return evalOctalEscapeSequence(token, startPos, maxEndPos, outLength);

	if (firstEscapeChar == '\'')
		return '\'';
	if (firstEscapeChar == '\"')
		return '\"';
	if (firstEscapeChar == '\?')
		return '\?';
	if (firstEscapeChar == '\\')
		return '\\';
	if (firstEscapeChar == '\a')
		return '\a';
	if (firstEscapeChar == '\b')
		return '\b';
	if (firstEscapeChar == '\f')
		return '\f';
	if (firstEscapeChar == '\n')
		return '\n';
	if (firstEscapeChar == '\r')
		return '\r';
	if (firstEscapeChar == '\t')
		return '\t';
	if (firstEscapeChar == '\v')
		return '\v';

	error("Unknown escape character in %s", token.c_str());
	return '\0';
}

char BootScriptParser::evalOctalEscapeSequence(const Common::String &token, uint pos, uint maxEndPos, uint &outLength) {
	uint length = 0;
	uint result = 0;
	while (length < 3 && pos < maxEndPos) {
		char c = token[pos];
		if (c < '0' || c > '7')
			break;

		result = result * 8u + (c - '0');
		pos++;
		length++;
	}

	if (result > 255)
		error("Overflowed octal character escape in token %s", token.c_str());

	outLength = length;
	return static_cast<char>(static_cast<unsigned char>(result));
}

char BootScriptParser::evalHexEscapeSequence(const Common::String &token, uint pos, uint maxEndPos, uint &outLength) {
	uint length = 0;
	uint result = 0;
	while (pos < maxEndPos) {
		char c = token[pos];
		if (c >= '0' && c <= '9')
			result = result * 16u + (c - '0');
		else if (c >= 'a' && c <= 'f')
			result = result * 16u + (c - 'a' + 0xa);
		else if (c >= 'A' && c <= 'F')
			result = result * 16u + (c - 'A' + 0xA);

		if (result > 255)
			error("Overflowed octal character escape in token %s", token.c_str());

		pos++;
		length++;
	}

	outLength = length;
	return static_cast<char>(static_cast<unsigned char>(result));
}


class BootScriptContext {
public:
	enum PlugIn {
		kPlugInMTI,
		kPlugInStandard,
		kPlugInObsidian,
		kPlugInMIDI,
		kPlugInFTTS,
		kPlugInRWC,
		kPlugInKnowWonder,
		kPlugInAxLogic,
		kPlugInHoologic,
		kPlugInMLine,
		kPlugInThereware,
	};

	enum BitDepth {
		kBitDepthAuto,

		kBitDepth8,
		kBitDepth16,
		kBitDepth32
	};

	enum RuntimeVersion {
		kRuntimeVersionAuto,

		kRuntimeVersion100,

		kRuntimeVersion110,
		kRuntimeVersion111,
		kRuntimeVersion112,

		kRuntimeVersion200,
	};

	explicit BootScriptContext(bool isMac);

	void bootObsidianRetailMacEn();
	void bootObsidianRetailMacJp();
	void bootObsidianGeneric();
	void bootObsidianRetailWinDe();
	void bootMTIRetailMac();
	void bootMTIGeneric();
	void bootMTIRetailWinRu();
	void bootSPQRMac();
	void bootSPQRWin();
	void bootPurpleMoonWin();
	void bootDilbertWin();
	void bootEasyBakeWin();
	void bootC9SamplerWin();
	void bootFTTSWin();
	void bootArchitectureWin();
	void bootDrawMarvelWin();
	void bootDinosaurFinderWin();
	void bootAnimalDoctorWin();
	void bootIvoclarWin();
	void bootBeatrixWin();
	void bootPoserWin();
	void bootRWCWin();
	void bootAngelicaWin();
	void bootAlbertWin();
	void bootWhitetailWin();
	void bootNotebookWin();
	void bootMsbAnimalWin();
	void bootMsbBugsWin();
	void bootMsbConcertWin();
	void bootMsbFlightWin();
	void bootMsbMarsWin();
	void bootMsbVolcanoWin();
	void bootMsbWhalesWin();
	void bootTelemedWin();
	void bootWorldBrokeWin();
	void bootFreeWillyWin();
	void bootHerculesWin();
	void bootMindGymWin();
	void bootStarTrekWin();


	void bootGeneric();
	void bootUsingBootScript();

	void finalize();

	const Common::Array<Common::SharedPtr<Common::Archive> > &getPersistentArchives() const;
	const Common::Array<PlugIn> &getPlugIns() const;
	const VirtualFileSystemLayout &getVFSLayout() const;
	const ManifestSubtitlesDef &getSubtitlesDef() const;
	const Common::String &getMainSegmentFileOverride() const;

	BitDepth getBitDepth() const;
	BitDepth getEnhancedBitDepth() const;
	RuntimeVersion getRuntimeVersion() const;
	const Common::Point &getResolution() const;
	bool getWantPrintVFS() const;

private:
	enum ArchiveType {
		kArchiveTypeMacVISE,
		kArchiveTypeStuffIt,
		kArchiveTypeInstallShieldV3,
		kArchiveTypeInstallShieldCab,
	};

	struct EnumBinding {
		const char *name;
		uint value;
	};

	void addPlugIn(PlugIn plugIn);
	void addArchive(ArchiveType archiveType, const Common::String &mountPoint, const Common::String &archivePath);
	void addJunction(const Common::String &virtualPath, const Common::String &physicalPath);
	void addSubtitles(const Common::String &linesFile, const Common::String &speakersFile, const Common::String &assetMappingFile, const Common::String &modifierMappingFile);
	void addExclusion(const Common::String &virtualPath);
	void setResolution(uint width, uint height);
	void setBitDepth(BitDepth bitDepth);
	void setEnhancedBitDepth(BitDepth bitDepth);
	void setRuntimeVersion(RuntimeVersion version);
	void setMainSegmentFile(const Common::String &mainSegmentFilePath);
	void printVFS();

	void executeFunction(const Common::String &functionName, const Common::Array<Common::String> &paramTokens);

	void checkParams(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint expectedCount);
	void parseEnumSized(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, const EnumBinding *bindings, uint numBindings, uint &outValue);
	void parseString(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, Common::String &outValue);
	void parseUInt(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, uint &outValue);

	template<uint TSize>
	void parseEnum(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, const EnumBinding (&bindings)[TSize], uint &outValue) {
		parseEnumSized(functionName, paramTokens, paramIndex, bindings, TSize, outValue);
	}

	VirtualFileSystemLayout _vfsLayout;
	Common::Array<PlugIn> _plugIns;

	ManifestSubtitlesDef _subtitlesDef;

	Common::Array<Common::SharedPtr<Common::Archive> > _persistentArchives;
	Common::String _mainSegmentFileOverride;
	bool _isMac;
	bool _wantPrintVFS;
	Common::Point _preferredResolution;
	BitDepth _bitDepth;
	BitDepth _enhancedBitDepth;
	RuntimeVersion _runtimeVersion;
};

BootScriptContext::BootScriptContext(bool isMac)
	: _isMac(isMac), _preferredResolution(0, 0)
	, _bitDepth(kBitDepthAuto), _enhancedBitDepth(kBitDepthAuto), _runtimeVersion(kRuntimeVersionAuto)
	, _wantPrintVFS(false) {
	_vfsLayout._pathSeparator = isMac ? ':' : '/';

	VirtualFileSystemLayout::ArchiveJunction fsJunction;
	fsJunction._archive = &SearchMan;
	fsJunction._archiveName = "fs";

	_vfsLayout._archiveJunctions.push_back(fsJunction);
}

void BootScriptContext::addPlugIn(PlugIn plugIn) {
	if (Common::find(_plugIns.begin(), _plugIns.end(), plugIn) != _plugIns.end())
		error("Duplicated plug-in");

	_plugIns.push_back(plugIn);
}

void BootScriptContext::addArchive(ArchiveType archiveType, const Common::String &mountPoint, const Common::String &archivePath) {
	for (const VirtualFileSystemLayout::ArchiveJunction &junction : _vfsLayout._archiveJunctions) {
		Common::String prefix = junction._archiveName + _vfsLayout._pathSeparator;

		if (archivePath.hasPrefixIgnoreCase(prefix)) {
			Common::Path path(archivePath.substr(prefix.size()), _vfsLayout._pathSeparator);

			Common::SeekableReadStream *stream = nullptr;

			bool isSingleStreamArchive = (archiveType != kArchiveTypeInstallShieldCab);

			if (isSingleStreamArchive) {
				if (_isMac)
					stream = Common::MacResManager::openFileOrDataFork(path, *junction._archive);
				else
					stream = junction._archive->createReadStreamForMember(path);

				if (!stream)
					error("Couldn't mount archive from path %s", archivePath.c_str());
			}

			Common::Archive *archive = nullptr;

			switch (archiveType) {
			case kArchiveTypeMacVISE:
				archive = Common::createMacVISEArchive(stream);
				break;
			case kArchiveTypeInstallShieldV3: {
					Common::InstallShieldV3 *isa = new Common::InstallShieldV3();
					if (isa->open(stream))
						archive = isa;
					else
						delete isa;
				}
				break;
			case kArchiveTypeInstallShieldCab: {
					archive = Common::makeInstallShieldArchive(path, *junction._archive);
				}
				break;
			case kArchiveTypeStuffIt:
				archive = Common::createStuffItArchive(stream, false);
				break;
			default:
				error("Unknown archive type");
			}

			if (!archive)
				error("Couldn't open archive %s", archivePath.c_str());

			_persistentArchives.push_back(Common::SharedPtr<Common::Archive>(archive));

			VirtualFileSystemLayout::ArchiveJunction newJunction;
			newJunction._archive = archive;
			newJunction._archiveName = mountPoint;

			_vfsLayout._archiveJunctions.push_back(newJunction);

			break;
		}
	}
}

void BootScriptContext::addJunction(const Common::String &virtualPath, const Common::String &physicalPath) {
	VirtualFileSystemLayout::PathJunction pathJunction;
	pathJunction._srcPath = (virtualPath.size() == 0) ? "workspace" : (Common::String(_isMac ? "workspace:" : "workspace/") + virtualPath);
	pathJunction._destPath = physicalPath;

	_vfsLayout._pathJunctions.push_back(pathJunction);
}

void BootScriptContext::addSubtitles(const Common::String &linesFile, const Common::String &speakersFile, const Common::String &assetMappingFile, const Common::String &modifierMappingFile) {
	_subtitlesDef.linesTablePath = linesFile;
	_subtitlesDef.assetMappingTablePath = assetMappingFile;
	_subtitlesDef.speakerTablePath = speakersFile;
	_subtitlesDef.modifierMappingTablePath = modifierMappingFile;
}

void BootScriptContext::addExclusion(const Common::String &virtualPath) {
	_vfsLayout._exclusions.push_back(Common::String(_isMac ? "workspace:" : "workspace/") + virtualPath);
}

void BootScriptContext::setResolution(uint width, uint height) {
	_preferredResolution = Common::Point(width, height);
}

void BootScriptContext::setBitDepth(BitDepth bitDepth) {
	_bitDepth = bitDepth;
}

void BootScriptContext::setEnhancedBitDepth(BitDepth bitDepth) {
	_enhancedBitDepth = bitDepth;
}

void BootScriptContext::setRuntimeVersion(RuntimeVersion version) {
	_runtimeVersion = version;
}

void BootScriptContext::setMainSegmentFile(const Common::String &mainSegmentFilePath) {
	_mainSegmentFileOverride = mainSegmentFilePath;
}

void BootScriptContext::printVFS() {
	_wantPrintVFS = true;
}

void BootScriptContext::bootObsidianRetailMacEn() {
	addPlugIn(kPlugInObsidian);
	addPlugIn(kPlugInMIDI);
	addPlugIn(kPlugInStandard);

	addArchive(kArchiveTypeStuffIt, "installer", "fs:Obsidian Installer");

	addJunction("", "installer:Obsidian \xc4");
	addJunction("Obsidian Data 1", "installer:Obsidian Data 1");
	addJunction("Obsidian Data 2", "fs:Obsidian Data 2");
	addJunction("Obsidian Data 3", "fs:Obsidian Data 3");
	addJunction("Obsidian Data 4", "fs:Obsidian Data 4");
	addJunction("Obsidian Data 5", "fs:Obsidian Data 5");
	addJunction("Obsidian Data 6", "fs:Obsidian Data 6");

	addExclusion("Obsidian Data 0");

	addSubtitles("subtitles_lines_obsidian_en.csv", "subtitles_speakers_obsidian_en.csv", "subtitles_asset_mapping_obsidian_en.csv", "subtitles_modifier_mapping_obsidian_en.csv");
}

void BootScriptContext::bootObsidianRetailMacJp() {
	addPlugIn(kPlugInObsidian);
	addPlugIn(kPlugInMIDI);
	addPlugIn(kPlugInStandard);

	addArchive(kArchiveTypeStuffIt, "installer", "fs:xn--u9j9ecg0a2fsa1io6k6jkdc2k");

	addJunction("", "installer");
	addJunction("Obsidian Data 2", "fs:Obsidian Data 2");
	addJunction("Obsidian Data 3", "fs:Obsidian Data 3");
	addJunction("Obsidian Data 4", "fs:Obsidian Data 4");
	addJunction("Obsidian Data 5", "fs:Obsidian Data 5");
	addJunction("Obsidian Data 6", "fs:Obsidian Data 6");

	addExclusion("Obsidian Data 0");
	addExclusion("Obsidian \xc4:Resource:\xcaIgorServer.rPP");	// Steam/ZOOM release wipes this file
}

void BootScriptContext::bootObsidianGeneric() {
	addPlugIn(kPlugInObsidian);
	addPlugIn(kPlugInMIDI);
	addPlugIn(kPlugInStandard);

	addSubtitles("subtitles_lines_obsidian_en.csv", "subtitles_speakers_obsidian_en.csv", "subtitles_asset_mapping_obsidian_en.csv", "subtitles_modifier_mapping_obsidian_en.csv");
}

void BootScriptContext::bootObsidianRetailWinDe() {
	addPlugIn(kPlugInObsidian);
	addPlugIn(kPlugInMIDI);
	addPlugIn(kPlugInStandard);

	addArchive(kArchiveTypeInstallShieldV3, "installer", "_SETUP.1");

	addJunction("workspace/Obsidian.exe", "installer/Group1/Obsidian.exe");
	addJunction("workspace/Resource/Obsidian.c95", "installer/Group2/Obsidian.c95");
	addJunction("workspace/Resource/MCURSORS.C95", "installer/Group2/MCURSORS.C95");

	addJunction("workspace", "fs");
}

void BootScriptContext::bootMTIRetailMac() {
	addPlugIn(kPlugInMTI);
	addPlugIn(kPlugInStandard);

	addJunction("mPlayer PPC", "fs:MPlayer PPC");
	addJunction("mPlayer PPC:Resource", "fs:MPlayer PPC:Resource");
	addJunction("MTI1", "fs:xn--MTI1-8b7a");
	addJunction("MTI2", "fs:MTI2");
	addJunction("MTI3", "fs:MTI3");
	addJunction("MTI4", "fs:MTI4");

	addJunction("VIDEO", "fs:VIDEO");
}

void BootScriptContext::bootMTIGeneric() {
	addPlugIn(kPlugInMTI);
	addPlugIn(kPlugInStandard);
}

void BootScriptContext::bootMTIRetailWinRu() {
	addPlugIn(kPlugInMTI);
	addPlugIn(kPlugInStandard);

	addArchive(kArchiveTypeInstallShieldCab, "installer", "fs/data1.cab");
	addJunction("", "installer");

	addJunction("", "fs");
}

void BootScriptContext::bootSPQRMac() {
	addPlugIn(kPlugInStandard);

	addArchive(kArchiveTypeMacVISE, "installer", "fs:Install.vct");

	addJunction("", "fs:GAME");
	addJunction("", "installer");
}

void BootScriptContext::bootSPQRWin() {
	addPlugIn(kPlugInStandard);
}

void BootScriptContext::bootPurpleMoonWin() {
	addPlugIn(kPlugInStandard);
	setMainSegmentFile("workspace/0Sampler.mpl");
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootDilbertWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	addPlugIn(kPlugInThereware);
	addPlugIn(kPlugInAxLogic);
	addPlugIn(kPlugInHoologic);
	setMainSegmentFile("workspace/HIJINKS/Hijinks1.mpl");
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootEasyBakeWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/EB/INTRO.MFX");
}

void BootScriptContext::bootC9SamplerWin() {
	addPlugIn(kPlugInStandard);
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion100);
	setMainSegmentFile("workspace/RUNSAMP.C9A");
}

void BootScriptContext::bootFTTSWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInFTTS);
	addPlugIn(kPlugInHoologic);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootArchitectureWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInHoologic);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootDrawMarvelWin() {
	addPlugIn(kPlugInStandard);
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion100);
	setMainSegmentFile("workspace/MDRAW.C9A");
}

void BootScriptContext::bootDinosaurFinderWin() {
	addPlugIn(kPlugInStandard);
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion100);
	setMainSegmentFile("workspace/WBDFR1.C9A");
}

void BootScriptContext::bootAnimalDoctorWin() {
	addPlugIn(kPlugInStandard);
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion100);
	setMainSegmentFile("workspace/VET.C9A");
}

void BootScriptContext::bootIvoclarWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInMLine);
}

void BootScriptContext::bootBeatrixWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInHoologic);
	//Force V112 mode for PlugInModifier::load
	//Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootPoserWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInThereware);
}

void BootScriptContext::bootRWCWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInThereware);
	addPlugIn(kPlugInRWC);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootAngelicaWin() {
	addPlugIn(kPlugInStandard);
	setMainSegmentFile("workspace/mfx/Splash.mfx");
}

void BootScriptContext::bootAlbertWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInHoologic);
	addPlugIn(kPlugInThereware);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootWhitetailWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInThereware);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootNotebookWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/YN/Hallway.mfx");
}

void BootScriptContext::bootMsbAnimalWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/Startup.mfw");
}

void BootScriptContext::bootMsbBugsWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignIn.mfx");
}

void BootScriptContext::bootMsbConcertWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignInC.mfx");
}

void BootScriptContext::bootMsbFlightWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignInF.mfx");
}

void BootScriptContext::bootMsbMarsWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignInM.mfx");
}

void BootScriptContext::bootMsbVolcanoWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignInV.mfx");
}

void BootScriptContext::bootMsbWhalesWin() {
	addPlugIn(kPlugInStandard);
	addPlugIn(kPlugInKnowWonder);
	setMainSegmentFile("workspace/MSB/SignInW.mfx");
}

void BootScriptContext::bootTelemedWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootWorldBrokeWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootFreeWillyWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootHerculesWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootMindGymWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootStarTrekWin() {
	addPlugIn(kPlugInStandard);
	// Force V112 mode for PlugInModifier::load
	// Autodetected V100 causes error there
	setRuntimeVersion(RuntimeVersion::kRuntimeVersion112);
}

void BootScriptContext::bootGeneric() {
	addPlugIn(kPlugInStandard);
}

void BootScriptContext::bootUsingBootScript() {
	const char *bootFileName = _isMac ? MTROPOLIS_MAC_BOOT_SCRIPT_NAME : MTROPOLIS_WIN_BOOT_SCRIPT_NAME;

	Common::File f;
	if (!f.open(bootFileName))
		error("Couldn't open boot script '%s'", bootFileName);

	BootScriptParser parser(f);

	Common::String functionName;
	while (parser.readToken(functionName)) {
		parser.expect("(");

		Common::Array<Common::String> paramTokens;

		{
			Common::String paramToken;
			if (!parser.readToken(paramToken))
				error("Unexpected EOF or error when reading parameter token");

			if (paramToken != ")") {
				paramTokens.push_back(paramToken);

				for (;;) {
					if (!parser.readToken(paramToken))
						error("Unexpected EOF or error when reading parameter token");

					if (paramToken == ")")
						break;

					if (paramToken != ",")
						error("Unexpected token %s while reading parameter list", paramToken.c_str());

					if (!parser.readToken(paramToken))
						error("Unexpected EOF or error when reading parameter token");

					paramTokens.push_back(paramToken);
				}
			}
		}

		parser.expect(";");

		executeFunction(functionName, paramTokens);
	}
}

#define ENUM_BINDING(name) \
	{ #name, name }

void BootScriptContext::executeFunction(const Common::String &functionName, const Common::Array<Common::String> &paramTokens) {
	const EnumBinding plugInEnum[] = {ENUM_BINDING(kPlugInMTI),
									  ENUM_BINDING(kPlugInStandard),
									  ENUM_BINDING(kPlugInObsidian),
									  ENUM_BINDING(kPlugInMIDI),
									  ENUM_BINDING(kPlugInFTTS),
									  ENUM_BINDING(kPlugInRWC),
									  ENUM_BINDING(kPlugInKnowWonder),
									  ENUM_BINDING(kPlugInAxLogic),
									  ENUM_BINDING(kPlugInHoologic),
									  ENUM_BINDING(kPlugInMLine),
									  ENUM_BINDING(kPlugInThereware),};

	const EnumBinding bitDepthEnum[] = {ENUM_BINDING(kBitDepthAuto),
										ENUM_BINDING(kBitDepth8),
										ENUM_BINDING(kBitDepth16),
										ENUM_BINDING(kBitDepth32)};

	const EnumBinding archiveTypeEnum[] = {ENUM_BINDING(kArchiveTypeMacVISE),
										   ENUM_BINDING(kArchiveTypeStuffIt),
										   ENUM_BINDING(kArchiveTypeInstallShieldV3),
										   ENUM_BINDING(kArchiveTypeInstallShieldCab)};

	const EnumBinding runtimeVersionEnum[] = {ENUM_BINDING(kRuntimeVersionAuto),
											  ENUM_BINDING(kRuntimeVersion100),
											  ENUM_BINDING(kRuntimeVersion110),
											  ENUM_BINDING(kRuntimeVersion111),
											  ENUM_BINDING(kRuntimeVersion112),
											  ENUM_BINDING(kRuntimeVersion200)};


	Common::String str1, str2, str3, str4;
	uint ui1 = 0;
	uint ui2 = 0;

	if (functionName == "addPlugIn") {
		checkParams(functionName, paramTokens, 1);
		parseEnum(functionName, paramTokens, 0, plugInEnum, ui1);

		addPlugIn(static_cast<PlugIn>(ui1));
	} else if (functionName == "addArchive") {
		checkParams(functionName, paramTokens, 3);
		parseEnum(functionName, paramTokens, 0, archiveTypeEnum, ui1);
		parseString(functionName, paramTokens, 1, str1);
		parseString(functionName, paramTokens, 2, str2);

		addArchive(static_cast<ArchiveType>(ui1), str1, str2);
	} else if (functionName == "addJunction") {
		checkParams(functionName, paramTokens, 2);
		parseString(functionName, paramTokens, 0, str1);
		parseString(functionName, paramTokens, 1, str2);

		addJunction(str1, str2);
	} else if (functionName == "addSubtitles") {
		checkParams(functionName, paramTokens, 4);
		parseString(functionName, paramTokens, 0, str1);
		parseString(functionName, paramTokens, 1, str2);
		parseString(functionName, paramTokens, 2, str3);
		parseString(functionName, paramTokens, 3, str4);

		addSubtitles(str1, str2, str3, str4);
	} else if (functionName == "addExclusion") {
		checkParams(functionName, paramTokens, 1);
		parseString(functionName, paramTokens, 0, str1);

		addExclusion(str1);
	} else if (functionName == "setResolution") {
		checkParams(functionName, paramTokens, 2);
		parseUInt(functionName, paramTokens, 0, ui1);
		parseUInt(functionName, paramTokens, 1, ui2);

		setResolution(ui1, ui2);
	} else if (functionName == "setBitDepth") {
		checkParams(functionName, paramTokens, 1);
		parseEnum(functionName, paramTokens, 0, bitDepthEnum, ui1);

		setBitDepth(static_cast<BitDepth>(ui1));
	} else if (functionName == "setEnhancedBitDepth") {
		checkParams(functionName, paramTokens, 1);
		parseEnum(functionName, paramTokens, 0, bitDepthEnum, ui1);

		setEnhancedBitDepth(static_cast<BitDepth>(ui1));
	} else if (functionName == "setRuntimeVersion") {
		checkParams(functionName, paramTokens, 1);
		parseEnum(functionName, paramTokens, 0, runtimeVersionEnum, ui1);

		setRuntimeVersion(static_cast<RuntimeVersion>(ui1));
	} else if (functionName == "setMainSegmentFile") {
		checkParams(functionName, paramTokens, 1);

		parseString(functionName, paramTokens, 0, str1);
		setMainSegmentFile(str1);
	} else if (functionName == "printVFS") {
		checkParams(functionName, paramTokens, 0);

		printVFS();
	} else {
		error("Unknown function '%s'", functionName.c_str());
	}
}

#undef ENUM_BINDING

void BootScriptContext::checkParams(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint expectedCount) {
	if (expectedCount != paramTokens.size())
		error("Expected %u parameters for function %s", paramTokens.size(), functionName.c_str());
}

void BootScriptContext::parseEnumSized(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, const EnumBinding *bindings, uint numBindings, uint &outValue) {
	const Common::String &param = paramTokens[paramIndex];

	if (BootScriptParser::classifyToken(param) != BootScriptParser::kTokenTypeIdentifier)
		error("Expected identifier for parameter %u of function %s", paramIndex, functionName.c_str());

	for (uint i = 0; i < numBindings; i++) {
		if (param == bindings[i].name) {
			outValue = bindings[i].value;
			return;
		}
	}

	error("Couldn't resolve enum value %s for parameter %u of function %s", param.c_str(), paramIndex, functionName.c_str());
}

void BootScriptContext::parseString(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, Common::String &outValue) {
	const Common::String &param = paramTokens[paramIndex];

	if (BootScriptParser::classifyToken(param) != BootScriptParser::kTokenTypeString)
		error("Expected string for parameter %u of function %s", paramIndex, functionName.c_str());

	outValue = BootScriptParser::evalString(param);
}

void BootScriptContext::parseUInt(const Common::String &functionName, const Common::Array<Common::String> &paramTokens, uint paramIndex, uint &outValue) {
	const Common::String &param = paramTokens[paramIndex];

	BootScriptParser::TokenType tt = BootScriptParser::classifyToken(param);

	if (tt != BootScriptParser::kTokenTypeDecimalConstant && tt != BootScriptParser::kTokenTypeOctalConstant && tt != BootScriptParser::kTokenTypeHexConstant)
		error("Expected integral constant for parameter %u of function %s", paramIndex, functionName.c_str());

	outValue = BootScriptParser::evalIntegral(param);
}

void BootScriptContext::finalize() {
	if (_vfsLayout._pathJunctions.size() == 0) {
		VirtualFileSystemLayout::PathJunction pathJunction;
		pathJunction._srcPath = "workspace";
		pathJunction._destPath = "fs";

		_vfsLayout._pathJunctions.push_back(pathJunction);
	}
}

const Common::Array<Common::SharedPtr<Common::Archive> > &BootScriptContext::getPersistentArchives() const {
	return _persistentArchives;
}

const Common::Array<BootScriptContext::PlugIn> &BootScriptContext::getPlugIns() const {
	return _plugIns;
}

const VirtualFileSystemLayout &BootScriptContext::getVFSLayout() const {
	return _vfsLayout;
}

const ManifestSubtitlesDef &BootScriptContext::getSubtitlesDef() const {
	return _subtitlesDef;
}

const Common::String &BootScriptContext::getMainSegmentFileOverride() const {
	return _mainSegmentFileOverride;
}

BootScriptContext::BitDepth BootScriptContext::getBitDepth() const {
	return _bitDepth;
}

BootScriptContext::BitDepth BootScriptContext::getEnhancedBitDepth() const {
	return _enhancedBitDepth;
}

BootScriptContext::RuntimeVersion BootScriptContext::getRuntimeVersion() const {
	return _runtimeVersion;
}

const Common::Point &BootScriptContext::getResolution() const {
	return _preferredResolution;
}

bool BootScriptContext::getWantPrintVFS() const {
	return _wantPrintVFS;
}

namespace Games {



const Game games[] = {
	// Boot script
	{
		MTBOOT_USE_BOOT_SCRIPT,
		&BootScriptContext::bootUsingBootScript
	},

	// Obsidian - Retail - Macintosh - English
	{
		MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
		&BootScriptContext::bootObsidianRetailMacEn
	},
	// Obsidian - Retail - Macintosh - Japanese
	{
		MTBOOT_OBSIDIAN_RETAIL_MAC_JP,
		&BootScriptContext::bootObsidianRetailMacJp
	},
	// Obsidian - Retail - Windows - English
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_EN,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Retail - Windows - German - Installed
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE_INSTALLED,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Retail - Windows - German - Disc
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE_DISC,
		&BootScriptContext::bootObsidianRetailWinDe
	},
	// Obsidian - Retail - Windows - Italian
	{
		MTBOOT_OBSIDIAN_RETAIL_WIN_IT,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Macintosh - English
	{
		MTBOOT_OBSIDIAN_DEMO_MAC_EN,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 1
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_1,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 2
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_2,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 3
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_3,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 4
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_4,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 5
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_5,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 6
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_6,
		&BootScriptContext::bootObsidianGeneric
	},
	// Obsidian - Demo - Windows - English - Variant 7
	{
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,
		&BootScriptContext::bootObsidianGeneric
	},
	// Muppet Treasure Island - Retail - Macintosh - Multiple languages
	{
		MTBOOT_MTI_RETAIL_MAC,
		&BootScriptContext::bootMTIRetailMac
	},
	// Muppet Treasure Island - Retail - Windows - Multiple languages
	{
		MTBOOT_MTI_RETAIL_WIN,
		&BootScriptContext::bootMTIGeneric
	},
	// Muppet Treasure Island - Retail - Windows - Russian - Installed
	{
		MTBOOT_MTI_RETAIL_WIN_RU_INSTALLED,
		&BootScriptContext::bootMTIGeneric
	},
	// Muppet Treasure Island - Retail - Windows - Russian
	{
		MTBOOT_MTI_RETAIL_WIN_RU_DISC,
		&BootScriptContext::bootMTIRetailWinRu
	},
	// Muppet Treasure Island - Demo - Windows
	{
		MTBOOT_MTI_DEMO_WIN,
		&BootScriptContext::bootMTIGeneric
	},
	// Uncle Albert's Magical Album - German - Windows
	{
		MTBOOT_ALBERT1_WIN_DE,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Magical Album - English - Windows
	{
		MTBOOT_ALBERT1_WIN_EN,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Magical Album - French - Windows
	{
		MTBOOT_ALBERT1_WIN_FR,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Magical Album - Dutch - Windows
	{
		MTBOOT_ALBERT1_WIN_NL,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Fabulous Voyage - German - Windows
	{
		MTBOOT_ALBERT2_WIN_DE,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Fabulous Voyage - English - Windows
	{
		MTBOOT_ALBERT2_WIN_EN,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Fabulous Voyage - English - Windows
	{
		MTBOOT_ALBERT2_WIN_FR,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Fabulous Voyage - Dutch - Windows
	{
		MTBOOT_ALBERT2_WIN_NL,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Mysterious Island - German - Windows
	{
		MTBOOT_ALBERT3_WIN_DE,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Mysterious Island - English - Windows
	{
		MTBOOT_ALBERT3_WIN_EN,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Mysterious Island - French - Windows
	{
		MTBOOT_ALBERT3_WIN_FR,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Mysterious Island - Dutch - Windows
	{
		MTBOOT_ALBERT3_WIN_NL,
		&BootScriptContext::bootAlbertWin
	},
	// Uncle Albert's Mysterious Island - Catalan - Windows
	{
		MTBOOT_ALBERT3_WIN_CA,
		&BootScriptContext::bootAlbertWin
	},
	// SPQR: The Empire's Darkest Hour - Retail - Windows - English
	{
		MTBOOT_SPQR_RETAIL_WIN,
		&BootScriptContext::bootSPQRWin
	},
	// SPQR: The Empire's Darkest Hour - Retail - Macintosh - English
	{
		MTBOOT_SPQR_RETAIL_MAC,
		&BootScriptContext::bootSPQRMac
	},
	// Star Trek: The Game Show - Retail - Windows
	{
		MTBOOT_STTGS_RETAIL_WIN,
		&BootScriptContext::bootStarTrekWin
	},
	// Star Trek: The Game Show - Demo - Windows
	{
		MTBOOT_STTGS_DEMO_WIN,
		&BootScriptContext::bootStarTrekWin
	},
	// Unit: Rebooted
	{
		MTBOOT_UNIT_REBOOTED_WIN,
		&BootScriptContext::bootGeneric
	},
	// Mind Gym - Windows - English
	{
		MTBOOT_MINDGYM_WIN_EN,
		&BootScriptContext::bootMindGymWin
	},
	// Mind Gym - Windows - German
	{
		MTBOOT_MINDGYM_WIN_DE,
		&BootScriptContext::bootMindGymWin
	},
	// Fun With Architecture - Windows - English
	{
		MTBOOT_ARCHITECTURE_EN,
	 	&BootScriptContext::bootArchitectureWin
	},
	// The Magic World of Beatrix Potter - Windows - English
	{
		MTBOOT_BEATRIX_EN,
	 	&BootScriptContext::bootBeatrixWin
	},
	// The Magic World of Beatrix Potter - Demo - Windows - English
	{
		MTBOOT_BEATRIX_DEMO_EN,
		&BootScriptContext::bootBeatrixWin
	},
	// Whitetail Impact - Windows - English
	{
		MTBOOT_WT_IMPACT_EN,
	 	&BootScriptContext::bootWhitetailWin
	},
	// The Day The World Broke - Windows - English
	{
		MTBOOT_WORLDBROKE_EN,
	 	&BootScriptContext::bootWorldBrokeWin
	},
	// The Totally Techie World of Young Dilbert: Hi-Tech Hijinks - Windows - English
	{
		MTBOOT_DILBERT_WIN_EN,
	 	&BootScriptContext::bootDilbertWin
	},
	// Free Willy Activity Center - Windows - English
	{
		MTBOOT_FREEWILLY_WIN_EN,
	 	&BootScriptContext::bootFreeWillyWin
	},
	// Hercules & Xena Learning Adventure: Quest for the Scrolls - Windows - English
	{
		MTBOOT_HERCULES_WIN_EN,
	 	&BootScriptContext::bootHerculesWin
	},
	// I Can Be a Dinosaur Finder - Retail - Windows - English
	{
		MTBOOT_IDINO_RETAIL_EN,
	 	&BootScriptContext::bootDinosaurFinderWin
	},
	// I Can Be an Animal Doctor - Retail - Windows - English
	{
		MTBOOT_IDOCTOR_RETAIL_EN,
	 	&BootScriptContext::bootAnimalDoctorWin
	},
	// How to Draw the Marvel Way - Windows - English
	{
		MTBOOT_DRAWMARVELWAY_WIN_EN,
		&BootScriptContext::bootDrawMarvelWin,
	},
	// FairyTale: A True Story - Activity Center - Windows - English
	{
		MTBOOT_FTTS_WIN_EN,
	 	&BootScriptContext::bootFTTSWin
	},
	// Purple Moon Sampler - Demo - Windows - English
	{
		MTBOOT_PURPLEMOON_WIN_EN,
	 	&BootScriptContext::bootPurpleMoonWin
	},
	// Chomp! The Video Game - Retail - Windows - English
	{
		MTBOOT_CHOMP_RETAIL_WIN_EN,
	 	&BootScriptContext::bootGeneric
	},
	// Chomp! The Video Game - Demo - Windows - English
	{
		MTBOOT_CHOMP_DEMO_WIN_EN,
	 	&BootScriptContext::bootGeneric
	},
	// 24 Hours in Cyberspace - Windows - English
	{
		MTBOOT_CYBER24_WIN_EN,
	 	&BootScriptContext::bootGeneric
	},
	// IVOCLAR - Windows - English
	{
		MTBOOT_IVOCLAR_WIN_EN,
	 	&BootScriptContext::bootIvoclarWin
	},
	// Real Wild Child! Australian Rock Music 1950s-90s - Windows - English
	{
		MTBOOT_REALWILD_WIN_EN,
		&BootScriptContext::bootRWCWin
	},
	// How to Build a Telemedicine Program - Windows - English
	{
		MTBOOT_TELEMED_WIN_EN,
	 	&BootScriptContext::bootTelemedWin
	},
	// Rugrats: Totally Angelica Boredom Buster - Windows - English
	{
		MTBOOT_ANGELICA_WIN_EN,
		&BootScriptContext::bootAngelicaWin
	},
	// Babe and Friends: Animated Early Reader - Windows - English
	{
		MTBOOT_BABE_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// Biologia Cellulare Evoluzione E Variet� Della Vita - Windows - Italian
	{
		MTBOOT_BIOCELLEVO_WIN_IT,
		&BootScriptContext::bootGeneric
	},
	// Easy-Bake Kitchen - Windows - English
	{
		MTBOOT_EASYBAKE_WIN_EN,
		&BootScriptContext::bootEasyBakeWin
	},
	// The Forgotten: It Begins - Windows - English
	{
		MTBOOT_FORGOTTEN_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// The Mystery at Greveholm 2: The Journey to Planutus - Windows - Swedish
	{
		MTBOOT_GREVEHOLM2_WIN_SE,
		&BootScriptContext::bootGeneric
	},
	// Itacante: La Cit� des Robots - Windows - French
	{
		MTBOOT_ITACANTE_WIN_FR,
		&BootScriptContext::bootGeneric
	},
	// King of Dragon Pass - Windows - English
	{
		MTBOOT_KINGOFDRAGONPASS_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// The Times Key Stage 1 English - Windows - English
	{
		MTBOOT_KS1ENG_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// Maisy's Playhouse - Windows - English
	{
		MTBOOT_MAISY_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// The Magic School Bus Explores the World of Animals - Windows - English
	{
		MTBOOT_MSB_ANIMAL_WIN_EN,
		&BootScriptContext::bootMsbAnimalWin
	},
	// The Magic School Bus Explores Bugs - Windows - English
	{
		MTBOOT_MSB_BUGS_WIN_EN,
		&BootScriptContext::bootMsbBugsWin
	},
	// The Magic School Bus In Concert - Windows - English
	{
		MTBOOT_MSB_CONCERT_WIN_EN,
		&BootScriptContext::bootMsbConcertWin
	},
	// The Magic School Bus Discovers Flight - Windows - English
	{
		MTBOOT_MSB_FLIGHT_WIN_EN,
		&BootScriptContext::bootMsbFlightWin
	},
	// The Magic School Bus Lands on Mars - Windows - English
	{
		MTBOOT_MSB_MARS_WIN_EN,
		&BootScriptContext::bootMsbMarsWin
	},
	// The Magic School Bus Volcano Adventure - Windows - English
	{
		MTBOOT_MSB_VOLCANO_WIN_EN,
		&BootScriptContext::bootMsbVolcanoWin
	},
	// The Magic School Bus Whales & Dolphins - Windows - English
	{
		MTBOOT_MSB_WHALES_WIN_EN,
		&BootScriptContext::bootMsbWhalesWin
	},
	// Mykropolis Planet der Roboter - Windows - German
	{
		MTBOOT_MYKROPOLIS_WIN_DE,
		&BootScriptContext::bootGeneric
	},
	// Your Notebook (with help from Amelia) - Windows - English
	{
		MTBOOT_NOTEBOOK_WIN_EN,
		&BootScriptContext::bootNotebookWin
	},
	// Pferd & Pony Lass uns reiten	- Windows - English
	{
		MTBOOT_PFERDPONY_WIN_DE,
		&BootScriptContext::bootGeneric
	},
	// Pinnacle Systems miroVideo Studio DC10 Plus - Windows - English
	{
		MTBOOT_MIRODC10_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// Pinnacle Systems miroVideo Studio DC10 Plus - Windows - German
	{
		MTBOOT_MIRODC10_WIN_DE,
		&BootScriptContext::bootGeneric
	},
	// Poser 3 Content Sampler - Windows - English
	{
		MTBOOT_POSER3_SAMPLER_ZYGOTE_WIN_EN,
		&BootScriptContext::bootPoserWin
	},
	// Another Poser 3 content sampler - Windows - English
	{
		MTBOOT_POSER3_ZYGOTE_WIN_EN,
		&BootScriptContext::bootPoserWin
	},
	// Whitetail Extreme - Windows - English
	{
		MTBOOT_WT_EXTREME_WIN_EN,
		&BootScriptContext::bootWhitetailWin
	},
	// Cloud 9 CD Sampler Volume 2 - Windows - English
	{
		MTBOOT_C9SAMPLER_WIN_EN,
		&BootScriptContext::bootC9SamplerWin
	},
	// Adobe 24 Hours Tools Sampler - Windows - English
	{
		MTBOOT_ADOBE24_WIN_EN,
		&BootScriptContext::bootGeneric
	},
	// Byzantine: The Betrayal - Demo - Windows - English
	{
		MTBOOT_BYZANTINE_DEMO_WIN_EN,
		&BootScriptContext::bootGeneric
	},
};

} // End of namespace Games

Common::SharedPtr<MTropolis::PlugIn> loadStandardPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> standardPlugIn = PlugIns::createStandard();
	static_cast<Standard::StandardPlugIn *>(standardPlugIn.get())->getHacks().allowGarbledListModData = true;
	return standardPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadMIDIPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> midiPlugIn = PlugIns::createMIDI();
	return midiPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadObsidianPlugIn(const MTropolisGameDescription &gameDesc, Common::Archive &fs, const Common::Path &pluginsLocation) {
	bool isMac = (gameDesc.desc.platform == Common::kPlatformMacintosh);
	bool isRetail = ((gameDesc.desc.flags & ADGF_DEMO) == 0);
	bool isEnglish = (gameDesc.desc.language == Common::EN_ANY);

	return ObsidianGameDataHandler::loadPlugIn(fs, pluginsLocation, isMac, isRetail, isEnglish);
}

Common::SharedPtr<MTropolis::PlugIn> loadMTIPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> mtiPlugIn(PlugIns::createMTI());
	return mtiPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadFTTSPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> fttsPlugIn(PlugIns::createFTTS());
	return fttsPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadRWCPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> rwcPlugIn(PlugIns::createRWC());
	return rwcPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadKnowWonderPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> kwPlugIn(PlugIns::createKnowWonder());
	return kwPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadAXLogicPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> axPlugIn(PlugIns::createAXLogic());
	return axPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadHoologicPlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> hlPlugIn(PlugIns::createHoologic());
	return hlPlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadMLinePlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> mlinePlugIn(PlugIns::createMLine());
	return mlinePlugIn;
}

Common::SharedPtr<MTropolis::PlugIn> loadTherewarePlugIn(const MTropolisGameDescription &gameDesc) {
	Common::SharedPtr<MTropolis::PlugIn> twPlugIn(PlugIns::createThereware());
	return twPlugIn;
}

enum PlayerType {
	kPlayerTypeNone,

	kPlayerTypeWin16,
	kPlayerTypeWin32,

	kPlayerTypeMac68k,
	kPlayerTypeMacPPC,
	kPlayerTypeMacFatBinary,
};

PlayerType evaluateWinPlayer(Common::ArchiveMember &archiveMember, bool mustBePE) {
	Common::SharedPtr<Common::SeekableReadStream> stream(archiveMember.createReadStream());

	if (!stream)
		return kPlayerTypeNone;

	// Largest known mPlayer executable is slightly over 1MB
	// Smallest known mPlayer executable is 542kb
	// By excluding ~2MB files we also ignore QT32.EXE QuickTime installers
	if (stream->size() < 512 * 1024 || stream->size() > 3 * 1024 * 1024 / 2)
		return kPlayerTypeNone;

	if (!stream->seek(0x3c))
		return kPlayerTypeNone;

	uint32 peOffset = stream->readUint32LE();
	if (stream->eos() || stream->err())
		return kPlayerTypeNone;

	bool isPE = false;
	if (stream->size() - 4 >= peOffset) {
		if (stream->seek(peOffset)) {
			uint32 possiblePEHeader = stream->readUint32LE();
			if (!stream->eos() && !stream->err() && possiblePEHeader == 0x00004550)
				isPE = true;
		}
	}

	stream->clearErr();

	// If we already found a Win32 player, ignore Win16 players
	if (mustBePE && !isPE)
		return kPlayerTypeNone;

	const char *signature = "mTropolis Windows Player";
	uint signatureLength = strlen(signature);

	if (!stream->seek(0))
		return kPlayerTypeNone;

	Common::Array<char> fileContents;
	fileContents.resize(stream->size());

	if (stream->read(&fileContents[0], fileContents.size()) != fileContents.size())
		return kPlayerTypeNone;

	stream.reset();

	// Look for signature
	uint lastStartPos = fileContents.size() - signatureLength * (isPE ? 2 : 1);

	for (uint i = 0; i < lastStartPos; i++) {
		bool isMatch = true;

		for (uint j = 0; j < signatureLength; j++) {
			if (isPE) {
				if (fileContents[i + j * 2] != signature[j] || fileContents[i + j * 2 + 1] != '\0') {
					isMatch = false;
					break;
				}
			} else {
				if (fileContents[i + j] != signature[j]) {
					isMatch = false;
					break;
				}
			}
		}

		if (isMatch)
			return isPE ? kPlayerTypeWin32 : kPlayerTypeWin16;
	}

	return kPlayerTypeNone;
}

void findWindowsPlayer(Common::Archive &fs, Common::Path &resolvedPath, PlayerType &resolvedPlayerType) {
	Common::ArchiveMemberList executableFiles;

	fs.listMatchingMembers(executableFiles, "*.exe", true);

	if (executableFiles.size() == 0)
		error("No executable files were found");

	Common::ArchiveMemberPtr bestPlayer;
	PlayerType bestPlayerType = kPlayerTypeNone;
	uint numPlayersInCategory = 0;

	for (const Common::ArchiveMemberPtr &archiveMember : executableFiles) {
		PlayerType playerType = evaluateWinPlayer(*archiveMember, bestPlayerType == kPlayerTypeWin32);

		debug(1, "Evaluated possible player executable %s as quality %i", archiveMember->getPathInArchive().toString(fs.getPathSeparator()).c_str(), static_cast<int>(playerType));

		if (playerType > bestPlayerType) {
			bestPlayerType = playerType;
			bestPlayer = archiveMember;
			numPlayersInCategory = 1;
		} else if (playerType == bestPlayerType)
			numPlayersInCategory++;
	}

	if (numPlayersInCategory == 0 || bestPlayerType == kPlayerTypeNone)
		error("Couldn't find any mTropolis Player executables");

	if (numPlayersInCategory != 1)
		error("Found multiple mTropolis Player executables of the same quality");

	resolvedPath = bestPlayer->getPathInArchive();
	resolvedPlayerType = bestPlayerType;
}

PlayerType evaluateMacPlayer(Common::Archive &fs, Common::ArchiveMember &archiveMember) {
	Common::Path path = archiveMember.getPathInArchive();

	Common::MacFinderInfo finderInfo;
	if (Common::MacResManager::getFileFinderInfo(path, fs, finderInfo)) {
		if (finderInfo.type[0] != 'A' || finderInfo.type[1] != 'P' || finderInfo.type[2] != 'P' || finderInfo.type[3] != 'L')
			return kPlayerTypeNone;
	}

	Common::MacResManager resMan;
	if (!resMan.open(path, fs))
		return kPlayerTypeNone;

	if (!resMan.hasResFork())
		return kPlayerTypeNone;

	Common::ScopedPtr<Common::SeekableReadStream> strStream(resMan.getResource(MKTAG('S', 'T', 'R', '#'), 200));
	if (!strStream)
		return kPlayerTypeNone;

	uint8 strInitialBytes[12];

	if (strStream->size() < static_cast<int64>(sizeof(strInitialBytes)))
		return kPlayerTypeNone;

	if (strStream->read(strInitialBytes, sizeof(strInitialBytes)) != sizeof(strInitialBytes))
		return kPlayerTypeNone;

	if (memcmp(strInitialBytes + 2, "\x09mTropolis", 10))
		return kPlayerTypeNone;

	bool is68k = resMan.getResIDArray(MKTAG('C', 'O', 'D', 'E')).size() > 0;
	bool isPPC = resMan.getResIDArray(MKTAG('c', 'f', 'r', 'g')).size() > 0;

	if (is68k) {
		if (isPPC)
			return kPlayerTypeMacFatBinary;
		else
			return kPlayerTypeMac68k;
	} else {
		if (isPPC)
			return kPlayerTypeMacPPC;
		else
			return kPlayerTypeNone;
	}
}

void findMacPlayer(Common::Archive &fs, Common::Path &resolvedPath, PlayerType &resolvedPlayerType) {
	Common::ArchiveMemberList allFiles;

	fs.listMembers(allFiles);

	Common::ArchiveMemberPtr bestPlayer;
	PlayerType bestPlayerType = kPlayerTypeNone;
	uint numPlayersInCategory = 0;

	for (const Common::ArchiveMemberPtr &archiveMember : allFiles) {
		PlayerType playerType = evaluateMacPlayer(fs, *archiveMember);

		debug(1, "Evaluated possible player executable %s as quality %i", archiveMember->getPathInArchive().toString(fs.getPathSeparator()).c_str(), static_cast<int>(playerType));

		if (playerType > bestPlayerType) {
			bestPlayerType = playerType;
			bestPlayer = archiveMember;
			numPlayersInCategory = 1;
		} else if (playerType == bestPlayerType)
			numPlayersInCategory++;
	}

	if (numPlayersInCategory == 0 || bestPlayerType == kPlayerTypeNone)
		error("Couldn't find any mTropolis Player applications");

	if (numPlayersInCategory != 1)
		error("Found multiple mTropolis Player applications of the same quality");

	if (bestPlayerType == kPlayerTypeMacFatBinary)
		bestPlayerType = kPlayerTypeMacPPC;

	resolvedPath = bestPlayer->getPathInArchive();
	resolvedPlayerType = bestPlayerType;
}

void findWindowsMainSegment(Common::Archive &fs, const BootScriptContext &bootScriptContext, Common::Path &resolvedPath, bool &resolvedIsV2) {
	Common::ArchiveMemberList allFiles;
	Common::ArchiveMemberList filteredFiles;

	const char *mainSegmentSuffixes[] = {".mpl", ".mfw", ".mfx"};

	if (bootScriptContext.getMainSegmentFileOverride().empty()) {
		fs.listMembers(allFiles);

		for (const Common::ArchiveMemberPtr &archiveMember : allFiles) {
			Common::String fileName = archiveMember->getFileName();

			for (const char *suffix : mainSegmentSuffixes) {
				if (fileName.hasSuffixIgnoreCase(suffix)) {
					filteredFiles.push_back(archiveMember);
					debug(4, "Identified possible main segment file %s", archiveMember->getPathInArchive().toString(fs.getPathSeparator()).c_str());
					break;
				}
			}
		}

		allFiles.clear();
	} else {
		const Common::String &pathStr = bootScriptContext.getMainSegmentFileOverride();

		Common::ArchiveMemberPtr mainSegmentFile = fs.getMember(Common::Path(pathStr, fs.getPathSeparator()));

		if (!mainSegmentFile)
			error("Couldn't find main segment '%s' in VFS", pathStr.c_str());

		filteredFiles.push_back(mainSegmentFile);

		bool hasRecognizedSuffix = false;
		
		for (const char *suffix : mainSegmentSuffixes) {
			if (pathStr.hasSuffixIgnoreCase(suffix)) {
				hasRecognizedSuffix = true;
				break;
			}
		}

		if (!hasRecognizedSuffix && bootScriptContext.getRuntimeVersion() == BootScriptContext::kRuntimeVersionAuto)
			error("Main segment has an unknown suffix, you must set a runtime version with setRuntimeVersion");
	}

	if (filteredFiles.size() == 0)
		error("Couldn't find any main segment files");

	if (filteredFiles.size() != 1)
		error("Found multiple main segment files");

	resolvedPath = filteredFiles.front()->getPathInArchive();
	resolvedIsV2 = !filteredFiles.front()->getFileName().hasSuffixIgnoreCase(".mpl");
}

bool getMacFileType(Common::Archive &fs, const Common::Path &path, uint32 &outTag) {
	Common::MacFinderInfo finderInfo;

	if (!Common::MacResManager::getFileFinderInfo(path, fs, finderInfo))
		return false;

	outTag = MKTAG(finderInfo.type[0], finderInfo.type[1], finderInfo.type[2], finderInfo.type[3]);
	return true;
}

enum SegmentSignatureType {
	kSegmentSignatureUnknown,

	kSegmentSignatureMacV1,
	kSegmentSignatureWinV1,
	kSegmentSignatureCrossV1,
	kSegmentSignatureMacV2,
	kSegmentSignatureWinV2,
	kSegmentSignatureCrossV2,
};

const uint kSignatureHeaderSize = 10;

SegmentSignatureType identifyStreamBySignature(byte (&header)[kSignatureHeaderSize]) {
	const byte macV1Signature[kSignatureHeaderSize] = {0, 0, 0xaa, 0x55, 0xa5, 0xa5, 0, 0, 0, 0};
	const byte winV1Signature[kSignatureHeaderSize] = {1, 0, 0xa5, 0xa5, 0x55, 0xaa, 0, 0, 0, 0};
	const byte crossV1Signature[kSignatureHeaderSize] = {8, 0, 0xa5, 0xa5, 0x55, 0xaa, 0, 0, 0, 0};
	const byte macV2Signature[kSignatureHeaderSize] = {0, 0, 0xaa, 0x55, 0xa5, 0xa5, 2, 0, 0, 0};
	const byte winV2Signature[kSignatureHeaderSize] = {1, 0, 0xa5, 0xa5, 0x55, 0xaa, 0, 0, 0, 2};
	const byte crossV2Signature[kSignatureHeaderSize] = {8, 0, 0xa5, 0xa5, 0x55, 0xaa, 0, 0, 0, 2};

	const byte *signatures[6] = {macV1Signature, winV1Signature, crossV1Signature, macV2Signature, winV2Signature, crossV2Signature};

	for (int i = 0; i < ARRAYSIZE(signatures); i++) {
		const byte *signature = signatures[i];

		if (!memcmp(signature, header, kSignatureHeaderSize))
			return static_cast<SegmentSignatureType>(i + kSegmentSignatureMacV1);
	}

	return kSegmentSignatureUnknown;
}

SegmentSignatureType identifyMacFileBySignature(Common::Archive &fs, const Common::Path &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(Common::MacResManager::openFileOrDataFork(path, fs));

	if (!stream)
		return kSegmentSignatureUnknown;

	byte header[kSignatureHeaderSize];
	if (stream->read(header, kSignatureHeaderSize) != kSignatureHeaderSize)
		return kSegmentSignatureUnknown;

	stream.reset();

	return identifyStreamBySignature(header);
}

void findMacMainSegment(Common::Archive &fs, Common::Path &resolvedPath, bool &resolvedIsV2) {
	Common::ArchiveMemberList allFiles;
	Common::ArchiveMemberList mfmmFiles;
	Common::ArchiveMemberList mfmxFiles;
	Common::ArchiveMemberList mfxmFiles;
	Common::ArchiveMemberList mfxxFiles;

	Common::ArchiveMemberList filteredFiles;


	fs.listMembers(allFiles);

	bool isV2 = false;

	// This is a somewhat tricky scenario because the main segment type may be either MFmx or MFmm, but MFmx
	// is NOT the main segment for mTropolis 1.x projects.  For those projects, we expect a MFmm.
	//
	// MT1 Mac: MFmm[+MFmx]
	// MT2 Mac: MFmm[+MFxm]
	// MT2 Cross: MFmx[+MFxx]

	for (const Common::ArchiveMemberPtr &archiveMember : allFiles) {
		uint32 fileTag = 0;
		if (getMacFileType(fs, archiveMember->getPathInArchive(), fileTag)) {
			switch (fileTag) {
			case MKTAG('M', 'F', 'm', 'm'):
				mfmmFiles.push_back(archiveMember);
				break;
			case MKTAG('M', 'F', 'm', 'x'):
				mfmxFiles.push_back(archiveMember);
				break;
			case MKTAG('M', 'F', 'x', 'm'):
				mfxmFiles.push_back(archiveMember);
				break;
			case MKTAG('M', 'F', 'x', 'x'):
				mfxxFiles.push_back(archiveMember);
				break;
			default:
				break;
			}
		}
	}

	if (mfmmFiles.size() > 0)
		filteredFiles = mfmmFiles;
	else if (mfxxFiles.size() > 0) {
		filteredFiles = mfmxFiles;
		isV2 = true;
	} else {
		// No MFmm files and no MFxx files, so if there are MFmx files, they could be the main segment of
		// a mTropolis 2.x project or additional files belonging to
		for (const Common::ArchiveMemberPtr &mfmxFile : mfmxFiles) {
			SegmentSignatureType signatureType = identifyMacFileBySignature(fs, mfmxFile->getPathInArchive());

			if (signatureType == kSegmentSignatureCrossV2) {
				filteredFiles.push_back(mfmxFile);
				isV2 = true;
			}
		}
	}

	if (filteredFiles.size() == 0) {
		warning("Didn't find main segment by Finder type, inspecting all files manually.  This is slow, you should use a format that preserves Finder info");

		// Didn't find any file that looks like a main segment by type, need to inspect all untagged files by signature.
		for (const Common::ArchiveMemberPtr &archiveMember : allFiles) {
			Common::Path path = archiveMember->getPathInArchive();
			uint32 tag = 0;

			if (!getMacFileType(fs, path, tag)) {
				SegmentSignatureType signatureType = identifyMacFileBySignature(fs, archiveMember->getPathInArchive());

				if (signatureType != kSegmentSignatureUnknown) {
					filteredFiles.push_back(archiveMember);

					if (signatureType == kSegmentSignatureMacV2 || signatureType == kSegmentSignatureWinV2 || signatureType == kSegmentSignatureCrossV2)
						isV2 = true;
				}
			}
		}
	}

	allFiles.clear();

	if (filteredFiles.size() == 0)
		error("Couldn't find any main segment files");

	if (filteredFiles.size() != 1) {
		for (const Common::ArchiveMemberPtr &archiveMember : filteredFiles)
			warning("Possible main segment file: '%s'", archiveMember->getPathInArchive().toString(fs.getPathSeparator()).c_str());

		error("Found multiple main segment files");
	}

	resolvedPath = filteredFiles.front()->getPathInArchive();
	resolvedIsV2 = isV2;
}

bool sortPathFileName(const Common::Path &a, const Common::Path &b) {
	Common::String aFileName = a.getLastComponent().toString();
	Common::String bFileName = b.getLastComponent().toString();

	return aFileName.compareToIgnoreCase(bFileName) < 0;
}

uint32 readEndian32(Common::ReadStream &stream, bool isBE) {
	return isBE ? stream.readUint32BE() : stream.readUint32LE();
}

uint16 readEndian16(Common::ReadStream &stream, bool isBE) {
	return isBE ? stream.readUint16BE() : stream.readUint16LE();
}

void safeResolveBitDepthAndResolutionFromPresentationSettings(Common::SeekableReadStream &mainSegmentStream, bool isMac, uint8 &outBitDepth, uint16 &outWidth, uint16 &outHeight) {
	byte header[kSignatureHeaderSize];

	if (mainSegmentStream.read(header, kSignatureHeaderSize) != kSignatureHeaderSize)
		error("Failed to read main segment header");

	SegmentSignatureType sigType = identifyStreamBySignature(header);

	if (sigType == kSegmentSignatureUnknown)
		error("Unknown main segment signature");

	bool isBE = (sigType == kSegmentSignatureMacV1 || sigType == kSegmentSignatureMacV2);
	bool isRuntimeV2 = (sigType == kSegmentSignatureMacV2 || sigType == kSegmentSignatureWinV2 || sigType == kSegmentSignatureCrossV2);

	Data::DataReader catReader(kSignatureHeaderSize, mainSegmentStream, isBE ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, isRuntimeV2 ? kRuntimeVersion200 : kRuntimeVersion100, true);

	uint32 hdrUnknown = 0;

	uint32 phTypeID = 0;
	uint16 phRevision = 0;
	uint32 phPersistFlags = 0;
	uint32 phSizeIncludingTag = 0;
	uint16 phUnknown1 = 0;
	uint32 phCatalogPosition = 0;

	if (!catReader.readMultiple(hdrUnknown, phTypeID, phRevision, phPersistFlags, phSizeIncludingTag, phUnknown1, phCatalogPosition) || phTypeID != 1002 || phRevision != 0)
		error("Failed to read project header from main segment");

	if (!mainSegmentStream.seek(phCatalogPosition))
		error("Failed to seek to catalog");

	uint32 catTypeID = 0;
	uint16 catRevision = 0;

	if (!catReader.readMultiple(catTypeID, catRevision) || catTypeID != 1000 || (catRevision != 2 && catRevision != 3))
		error("Failed to read catalog header");

	uint32 catPersistFlags = 0;
	uint32 catSizeOfStreamAndSegmentDescs = 0;
	uint16 catNumStreams = 0;
	uint16 catUnknown1 = 0;
	uint16 catUnknown2 = 0;
	uint16 catNumSegments = 0;

	if (!catReader.readMultiple(catPersistFlags, catSizeOfStreamAndSegmentDescs, catNumStreams, catUnknown1, catUnknown2, catNumSegments))
		error("Failed to read stream descs from catalog header");

	uint32 bootStreamPos = 0;
	uint32 bootStreamSize = 0;

	for (uint i = 0; i < catNumStreams; i++) {
		char streamType[25];
		streamType[24] = 0;

		uint32 winPosition = 0;
		uint32 winSize = 0;
		uint32 macPosition = 0;
		uint32 macSize = 0;

		mainSegmentStream.read(streamType, 24);

		uint16 segmentIndexPlusOne = readEndian16(mainSegmentStream, isBE);

		if (catRevision >= 3) {
			macPosition = readEndian32(mainSegmentStream, isBE);
			macSize = readEndian32(mainSegmentStream, isBE);
			winPosition = readEndian32(mainSegmentStream, isBE);
			winSize = readEndian32(mainSegmentStream, isBE);
		} else {
			winPosition = macPosition = readEndian32(mainSegmentStream, isBE);
			winSize = macSize = readEndian32(mainSegmentStream, isBE);
		}

		if (mainSegmentStream.eos() || mainSegmentStream.err())
			error("Error reading stream description");

		if (!strcmp(streamType, "bootstream") || !strcmp(streamType, "bootStream")) {
			bootStreamPos = (isMac ? macPosition : winPosition);
			bootStreamSize = (isMac ? macSize : winSize);

			if (segmentIndexPlusOne != 1)
				error("Boot stream isn't in segment 1");

			break;
		}
	}

	if (!bootStreamSize)
		error("Failed to resolve boot stream");

	if (!mainSegmentStream.seek(bootStreamPos))
		error("Failed to seek to boot stream");

	// NOTE: Endianness switches from isBE to isMac here!
	Data::DataReader streamReader(bootStreamPos, mainSegmentStream, isMac ? Data::kDataFormatMacintosh : Data::kDataFormatWindows, catReader.getRuntimeVersion(), catReader.isVersionAutoDetect());

	uint32 shTypeID = 0;
	uint16 shRevision = 0;
	uint32 shPersistFlags = 0;
	uint32 shSizeIncludingTag = 0;

	if (!streamReader.readMultiple(shTypeID, shRevision, shPersistFlags, shSizeIncludingTag) || shTypeID != 1001 || shRevision != 0 || shSizeIncludingTag < 14)
		error("Failed to read boot stream header");

	if (!mainSegmentStream.skip(shSizeIncludingTag - 14))
		error("Failed to skip stream header");

	uint32 psTypeID = 0;
	uint16 psRevision = 0;
	uint32 psPersistFlags = 0;
	uint32 psSizeIncludingTag = 0;
	uint16 psUnknown1 = 0;

	uint32 psResolution = 0;
	uint16 psBitsPerPixel = 0;

	if (!streamReader.readMultiple(psTypeID, psRevision, psPersistFlags, psSizeIncludingTag, psUnknown1, psResolution, psBitsPerPixel) || psTypeID != 1004 || (psRevision != 2 && psRevision != 3))
		error("Failed to read presentation settings");

	outHeight = ((psResolution >> 16) & 0xffff);
	outWidth = (psResolution & 0xffff);

	switch (psBitsPerPixel) {
	case 1:
		outBitDepth = 1;
		break;
	case 2:
		outBitDepth = 2;
		break;
	case 4:
		outBitDepth = 4;
		break;
	case 8:
		outBitDepth = 8;
		break;
	case 16:
		outBitDepth = 16;
		break;
	case 32:
		outBitDepth = 32;
		break;
	default:
		error("Unknown bit depth mode in presentation settings");
	}
}

void resolveBitDepthAndResolutionFromPresentationSettings(Common::SeekableReadStream &mainSegmentStream, bool isMac, uint8 &outBitDepth, uint16 &outWidth, uint16 &outHeight) {
	if (!mainSegmentStream.seek(0))
		error("Couldn't reset main segment stream to start");

	safeResolveBitDepthAndResolutionFromPresentationSettings(mainSegmentStream, isMac, outBitDepth, outWidth, outHeight);

	if (!mainSegmentStream.seek(0))
		error("Couldn't reset main segment stream to start");
}

} // End of namespace Boot



BootConfiguration::BootConfiguration() : _bitDepth(0), _enhancedBitDepth(0), _width(0), _height(0) {
}

bool stableSortCaseInsensitive(const Common::String &a, const Common::String &b) {
	uint shorterPath = a.size();
	if (b.size() < shorterPath)
		shorterPath = b.size();

	bool aLessSensitive = false;
	bool bLessSensitive = false;
	for (uint i = 0; i < shorterPath; i++) {
		char ca = a[i];
		char cb = b[i];

		char ccia = invariantToLower(ca);
		char ccib = invariantToLower(cb);

		if (ccia < ccib)
			return true;
		if (ccib < ccia)
			return false;

		if (ca < cb)
			aLessSensitive = true;
		if (cb < ca)
			bLessSensitive = true;
	}

	if (aLessSensitive)
		return true;
	if (bLessSensitive)
		return false;

	return a.size() < b.size();
}

struct StablePathInArchiveSorter {
	explicit StablePathInArchiveSorter(char pathSeparator);

	bool operator()(const Common::ArchiveMember *a, const Common::ArchiveMember *b) const {
		Common::String aPathStr = a->getPathInArchive().toString(_pathSeparator);
		Common::String bPathStr = b->getPathInArchive().toString(_pathSeparator);

		return stableSortCaseInsensitive(aPathStr, bPathStr);
	}

private:
	char _pathSeparator;
};

StablePathInArchiveSorter::StablePathInArchiveSorter(char pathSeparator) : _pathSeparator(pathSeparator) {
}


BootConfiguration bootProject(const MTropolisGameDescription &gameDesc) {
	BootConfiguration bootConfig;

	Common::SharedPtr<ProjectDescription> &desc = bootConfig._projectDesc;

	Common::Array<Common::SharedPtr<ProjectPersistentResource>> persistentResources;
	Common::Array<Common::SharedPtr<PlugIn> > plugIns;

	Common::SharedPtr<Boot::GameDataHandler> gameDataHandler;

	Common::SharedPtr<SubtitleAssetMappingTable> subsAssetMappingTable;
	Common::SharedPtr<SubtitleModifierMappingTable> subsModifierMappingTable;
	Common::SharedPtr<SubtitleSpeakerTable> subsSpeakerTable;
	Common::SharedPtr<SubtitleLineTable> subsLineTable;

	Common::String speakerTablePath;
	Common::String linesTablePath;
	Common::String assetMappingTablePath;
	Common::String modifierMappingTablePath;

	const Boot::Game *bootGame = nullptr;
	for (const Boot::Game &bootGameCandidate : Boot::Games::games) {
		if (bootGameCandidate.bootID == gameDesc.bootID) {
			// Multiple manifests should not have the same manifest ID!
			assert(!bootGame);
			bootGame = &bootGameCandidate;
		}
	}

	if (!bootGame)
		error("Couldn't boot mTropolis game, don't have a file manifest for manifest ID %i", static_cast<int>(gameDesc.bootID));

	Boot::BootScriptContext bootScriptContext(gameDesc.desc.platform == Common::kPlatformMacintosh);

	void (Boot::BootScriptContext::*bootFunc)() = bootGame->bootFunction;
	(bootScriptContext.*bootFunc)();

	for (const Common::SharedPtr<Common::Archive> &arc : bootScriptContext.getPersistentArchives())
		persistentResources.push_back(Boot::PersistentResource<Common::Archive>::wrap(arc));

	bootScriptContext.finalize();

	if (bootScriptContext.getWantPrintVFS()) {
		const VirtualFileSystemLayout &vfsLayout = bootScriptContext.getVFSLayout();

		char pathSeparator = vfsLayout._pathSeparator;

		debug("VFS layout:");

		debug("Workspace root: %s", vfsLayout._workspaceRoot.toString(pathSeparator).c_str());

		debug("Archive junctions:");

		for (const VirtualFileSystemLayout::ArchiveJunction &arcJunction : vfsLayout._archiveJunctions) {
			debug("Physical paths from archive '%s':", arcJunction._archiveName.c_str());

			Common::ArchiveMemberList memberList;
			arcJunction._archive->listMembers(memberList);

			Common::Array<Common::ArchiveMember *> sortedArchiveMembers;

			for (const Common::ArchiveMemberPtr &archiveMember : memberList)
				sortedArchiveMembers.push_back(archiveMember.get());

			Common::sort(sortedArchiveMembers.begin(), sortedArchiveMembers.end(), StablePathInArchiveSorter(pathSeparator));

			for (const Common::ArchiveMember *archiveMember : sortedArchiveMembers) {
				debug("    %s%c%s", arcJunction._archiveName.c_str(), pathSeparator, archiveMember->getPathInArchive().toString(pathSeparator).c_str());
			}

			debug("%s", "");
		}

		debug("Virtual-to-physical path mappings:");
		for (const VirtualFileSystemLayout::PathJunction &pathJunction : vfsLayout._pathJunctions) {
			debug("    %s -> %s", pathJunction._srcPath.c_str(), pathJunction._destPath.c_str());
		}
		debug("%s", "");

		debug("Exclusions:");

		Common::StringArray exclusions = vfsLayout._exclusions;
		Common::sort(exclusions.begin(), exclusions.end(), stableSortCaseInsensitive);

		for (const Common::String &str : vfsLayout._exclusions)
			debug("    %s", str.c_str());

		debug("%s", "");
	}

	Common::SharedPtr<VirtualFileSystem> vfs(new VirtualFileSystem(bootScriptContext.getVFSLayout()));

	if (bootScriptContext.getWantPrintVFS()) {
		debug("Files in VFS:");
		Common::ArchiveMemberList memberList;
		vfs->listMembers(memberList);

		Common::Array<Common::ArchiveMember *> sortedArchiveMembers;

		for (const Common::ArchiveMemberPtr &archiveMember : memberList)
			sortedArchiveMembers.push_back(archiveMember.get());

		Common::sort(sortedArchiveMembers.begin(), sortedArchiveMembers.end(), StablePathInArchiveSorter(vfs->getPathSeparator()));

		for (const Common::ArchiveMember *archiveMember : sortedArchiveMembers) {
			debug("    %s", archiveMember->getPathInArchive().toString(vfs->getPathSeparator()).c_str());
		}
	}

	Common::Path playerLocation;
	Common::Path mainSegmentLocation;
	Common::Path mainSegmentDirectory;
	Common::Path playerDirectory;
	Common::Path pluginsLocation;
	Boot::PlayerType playerType = Boot::kPlayerTypeNone;
	bool isV2Project = false;

	persistentResources.push_back(Boot::PersistentResource<VirtualFileSystem>::wrap(vfs));

	if (gameDesc.desc.platform == Common::kPlatformMacintosh) {
		Boot::findMacPlayer(*vfs, playerLocation, playerType);
		Boot::findMacMainSegment(*vfs, mainSegmentLocation, isV2Project);
	} else if (gameDesc.desc.platform == Common::kPlatformWindows) {
		Boot::findWindowsPlayer(*vfs, playerLocation, playerType);
		Boot::findWindowsMainSegment(*vfs, bootScriptContext,  mainSegmentLocation, isV2Project);
	}

	if (bootScriptContext.getRuntimeVersion() != Boot::BootScriptContext::kRuntimeVersionAuto)
		isV2Project = (bootScriptContext.getRuntimeVersion() >= Boot::BootScriptContext::kRuntimeVersion200);

	{
		Common::StringArray pathComponents = playerLocation.splitComponents();
		pathComponents.pop_back();
		playerDirectory = Common::Path::joinComponents(pathComponents);

		pathComponents = mainSegmentLocation.splitComponents();
		pathComponents.pop_back();
		mainSegmentDirectory = Common::Path::joinComponents(pathComponents);
	}

	if (isV2Project)
		pluginsLocation = playerDirectory.appendComponent("mplugins");
	else
		pluginsLocation = playerDirectory.appendComponent("resource");

	{
		const Boot::ManifestSubtitlesDef &subtitlesDef = bootScriptContext.getSubtitlesDef();

		linesTablePath = subtitlesDef.linesTablePath;
		speakerTablePath = subtitlesDef.speakerTablePath;
		assetMappingTablePath = subtitlesDef.assetMappingTablePath;
		modifierMappingTablePath = subtitlesDef.modifierMappingTablePath;
	}

	Common::SharedPtr<CursorGraphicCollection> cursorGraphics(new CursorGraphicCollection());

	// Load plug-ins
	{
		Common::ArchiveMemberList pluginFiles;
		Common::Array<Common::Path> pluginPathsSorted;

		const char *plugInSuffix = nullptr;

		switch (playerType) {
		case Boot::kPlayerTypeMac68k:
			plugInSuffix = "68";
			break;
		case Boot::kPlayerTypeMacPPC:
			plugInSuffix = "pp";
			break;
		case Boot::kPlayerTypeWin32:
			plugInSuffix = isV2Project ? "32" : "95";
			break;
		case Boot::kPlayerTypeWin16:
			plugInSuffix = isV2Project ? "16" : "31";
			break;
		default:
			error("Unknown player type");
			break;
		}

		vfs->listMatchingMembers(pluginFiles, pluginsLocation.appendComponent("*"));

		
		debug(4, "Looking for plug-in files in %s", pluginsLocation.toString(vfs->getPathSeparator()).c_str());
		for (const Common::ArchiveMemberPtr &pluginFile : pluginFiles) {
			Common::String fileName = pluginFile->getFileName();
			uint fnameLen = fileName.size();

			if (fnameLen >= 4 && fileName[fnameLen - 4] == '.' && invariantToLower(fileName[fnameLen - 2]) == plugInSuffix[0] && invariantToLower(fileName[fnameLen - 1]) == plugInSuffix[1])
				pluginPathsSorted.push_back(pluginFile->getPathInArchive());
		}

		// This is possibly not optimal - Sort order on MacOS is based on the MacRoman encoded file name,
		// and possibly case-sensitive too.  Sort order on Windows is case-insensitive.  However, we don't
		// want to rely on the filenames having the correct case on the user machine.
		Common::sort(pluginPathsSorted.begin(), pluginPathsSorted.end(), Boot::sortPathFileName);

		for (const Common::Path &plugInPath : pluginPathsSorted) {
			debug(4, "Found plug-in %s", plugInPath.toString(vfs->getPathSeparator()).c_str());
		}

		if (gameDesc.desc.platform == Common::kPlatformMacintosh) {
			Boot::loadCursorsMac(*vfs, playerLocation, *cursorGraphics);

			for (const Common::Path &plugInPath : pluginPathsSorted)
				Boot::loadCursorsMac(*vfs, plugInPath, *cursorGraphics);
		} else if (gameDesc.desc.platform == Common::kPlatformWindows) {
			Boot::loadCursorsWin(*vfs, playerLocation, *cursorGraphics);

			for (const Common::Path &plugInPath : pluginPathsSorted)
				Boot::loadCursorsWin(*vfs, plugInPath, *cursorGraphics);
		}
	}

	// Add ScummVM plug-ins from the boot script
	for (Boot::BootScriptContext::PlugIn plugIn : bootScriptContext.getPlugIns()) {
		switch (plugIn) {
		case Boot::BootScriptContext::kPlugInStandard:
			plugIns.push_back(Boot::loadStandardPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInMIDI:
			plugIns.push_back(Boot::loadMIDIPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInObsidian:
			plugIns.push_back(Boot::loadObsidianPlugIn(gameDesc, *vfs, pluginsLocation));
			break;
		case Boot::BootScriptContext::kPlugInMTI:
			plugIns.push_back(Boot::loadMTIPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInFTTS:
			plugIns.push_back(Boot::loadFTTSPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInRWC:
			plugIns.push_back(Boot::loadRWCPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInKnowWonder:
			plugIns.push_back(Boot::loadKnowWonderPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInAxLogic:
			plugIns.push_back(Boot::loadAXLogicPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInHoologic:
			plugIns.push_back(Boot::loadHoologicPlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInMLine:
			plugIns.push_back(Boot::loadMLinePlugIn(gameDesc));
			break;
		case Boot::BootScriptContext::kPlugInThereware:
			plugIns.push_back(Boot::loadTherewarePlugIn(gameDesc));
			break;
		default:
			error("Unknown plug-in ID");
		}
	}

	ProjectPlatform projectPlatform = (gameDesc.desc.platform == Common::kPlatformMacintosh) ? kProjectPlatformMacintosh : kProjectPlatformWindows;

	RuntimeVersion runtimeVersion = kRuntimeVersion100;
	bool isAutoVersion = false;

	switch (bootScriptContext.getRuntimeVersion()) {
	case Boot::BootScriptContext::kRuntimeVersionAuto:
		if (isV2Project)
			runtimeVersion = kRuntimeVersion200;
		isAutoVersion = true;
		break;
	case Boot::BootScriptContext::kRuntimeVersion100:
		runtimeVersion = kRuntimeVersion100;
		break;
	case Boot::BootScriptContext::kRuntimeVersion110:
		runtimeVersion = kRuntimeVersion110;
		break;
	case Boot::BootScriptContext::kRuntimeVersion111:
		runtimeVersion = kRuntimeVersion111;
		break;
	case Boot::BootScriptContext::kRuntimeVersion112:
		runtimeVersion = kRuntimeVersion112;
		break;
	case Boot::BootScriptContext::kRuntimeVersion200:
		runtimeVersion = kRuntimeVersion200;
		break;
	default:
		error("Boot script runtime version was not handled");
	}

	desc.reset(new ProjectDescription(projectPlatform, runtimeVersion, isAutoVersion, vfs.get(), mainSegmentDirectory));
	desc->setCursorGraphics(cursorGraphics);

	for (const Common::SharedPtr<PlugIn> &plugIn : plugIns)
		desc->addPlugIn(plugIn);

	Common::SharedPtr<Common::SeekableReadStream> mainSegmentStream;

	if (gameDesc.desc.platform == Common::kPlatformMacintosh)
		mainSegmentStream.reset(Common::MacResManager::openFileOrDataFork(mainSegmentLocation, *vfs));
	else if (gameDesc.desc.platform == Common::kPlatformWindows)
		mainSegmentStream.reset(vfs->createReadStreamForMember(mainSegmentLocation));

	if (!mainSegmentStream)
		error("Failed to open main segment");

	persistentResources.push_back(Boot::PersistentResource<Common::SeekableReadStream>::wrap(mainSegmentStream));

	desc->addSegment(0, mainSegmentStream.get());
	desc->setLanguage(gameDesc.desc.language);

	Common::Point resolution = bootScriptContext.getResolution();

	if (bootScriptContext.getBitDepth() == Boot::BootScriptContext::kBitDepthAuto || resolution.x == 0 || resolution.y == 0) {
		uint16 width = 0;
		uint16 height = 0;
		Boot::resolveBitDepthAndResolutionFromPresentationSettings(*mainSegmentStream, gameDesc.desc.platform == Common::kPlatformMacintosh, bootConfig._bitDepth, width, height);

		if (resolution.x == 0)
			resolution.x = width;

		if (resolution.y == 0)
			resolution.y = height;
	}

	bootConfig._width = resolution.x;
	bootConfig._height = resolution.y;

	switch (bootScriptContext.getBitDepth()) {
	case Boot::BootScriptContext::kBitDepthAuto:
		break;
	case Boot::BootScriptContext::kBitDepth8:
		bootConfig._bitDepth = 8;
		break;
	case Boot::BootScriptContext::kBitDepth16:
		bootConfig._bitDepth = 16;
		break;
	case Boot::BootScriptContext::kBitDepth32:
		bootConfig._bitDepth = 32;
		break;
	default:
		error("Invalid bit depth in boot script");
	};

	switch (bootScriptContext.getEnhancedBitDepth()) {
	case Boot::BootScriptContext::kBitDepthAuto:
		bootConfig._enhancedBitDepth = bootConfig._bitDepth;
		bootConfig._enhancedBitDepth = 32;
		break;
	case Boot::BootScriptContext::kBitDepth8:
		bootConfig._enhancedBitDepth = 8;
		break;
	case Boot::BootScriptContext::kBitDepth16:
		bootConfig._enhancedBitDepth = 16;
		break;
	case Boot::BootScriptContext::kBitDepth32:
		bootConfig._enhancedBitDepth = 32;
		break;
	default:
		error("Invalid bit depth in boot script");
	};

	if (bootConfig._enhancedBitDepth < bootConfig._bitDepth)
		bootConfig._enhancedBitDepth = bootConfig._bitDepth;

	Common::SharedPtr<ProjectResources> resources(new ProjectResources());
	resources->persistentResources = persistentResources;

	desc->setResources(resources);

	if (assetMappingTablePath.size() > 0 && linesTablePath.size() > 0) {
		subsAssetMappingTable.reset(new SubtitleAssetMappingTable());
		subsModifierMappingTable.reset(new SubtitleModifierMappingTable());
		subsSpeakerTable.reset(new SubtitleSpeakerTable());
		subsLineTable.reset(new SubtitleLineTable());

		Common::ErrorCode assetMappingError = subsAssetMappingTable->load(assetMappingTablePath);
		Common::ErrorCode modifierMappingError = subsModifierMappingTable->load(modifierMappingTablePath);
		Common::ErrorCode speakerError = subsSpeakerTable->load(speakerTablePath);
		Common::ErrorCode linesError = speakerError;

		if (speakerError == Common::kNoError)
			linesError = subsLineTable->load(linesTablePath, *subsSpeakerTable);

		if (assetMappingError != Common::kNoError || modifierMappingError != Common::kNoError || linesError != Common::kNoError) {
			// If all sub files are missing, then the user hasn't installed them
			if (assetMappingError != Common::kPathDoesNotExist || modifierMappingError != Common::kPathDoesNotExist || linesError != Common::kPathDoesNotExist) {
				warning("Failed to load subtitles data");
			}

			subsAssetMappingTable.reset();
			subsModifierMappingTable.reset();
			subsLineTable.reset();
			subsSpeakerTable.reset();
		}
	}

	SubtitleTables subTables;
	subTables.assetMapping = subsAssetMappingTable;
	subTables.lines = subsLineTable;
	subTables.modifierMapping = subsModifierMappingTable;
	subTables.speakers = subsSpeakerTable;

	desc->setSubtitles(subTables);

	return bootConfig;
}

void bootAddSearchPaths(const Common::FSNode &gameDataDir, const MTropolisGameDescription &gameDesc) {

	const Boot::Game *bootGame = nullptr;
	for (const Boot::Game &bootGameCandidate : Boot::Games::games) {
		if (bootGameCandidate.bootID == gameDesc.bootID) {
			// Multiple manifests should not have the same manifest ID!
			assert(!bootGame);
			bootGame = &bootGameCandidate;
		}
	}

	if (!bootGame)
		error("Couldn't boot mTropolis game, don't have a file manifest for manifest ID %i", static_cast<int>(gameDesc.bootID));
}

} // End of namespace MTropolis
