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
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/rect.h"

#include "graphics/surface.h"
#include "graphics/primitives.h"

#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/scene.h"
#include "dgds/font.h"

namespace Dgds {

template<class S> Common::String _dumpStructList(const Common::String &indent, const Common::String &name, const Common::Array<S> &list) {
	if (list.empty())
		return "";

	const Common::String nextind = indent + "    ";
	Common::String str = Common::String::format("\n%s%s=", Common::String(indent + "  ").c_str(), name.c_str());
	for (const auto &s : list) {
		str += "\n";
		str += s.dump(nextind);
	}
	return str;
}


Common::String Rect::dump(const Common::String &indent) const {
	return Common::String::format("%sRect<%d,%d %d,%d>", indent.c_str(), x, y, width, height);
}


Common::String SceneStruct1::dump(const Common::String &indent) const {
	return Common::String::format("%sSceneStruct1<%d flg 0x%02x %d>", indent.c_str(), val1, flags, val3);
}


Common::String SceneStruct2::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneStruct2<%s %d %d",
			indent.c_str(), rect.dump("").c_str(), field1_0x8, field2_0xa);
	str += _dumpStructList(indent, "struct1list", struct1List);
	str += _dumpStructList(indent, "oplist1", opList1);
	str += _dumpStructList(indent, "oplist2", opList2);
	str += _dumpStructList(indent, "oplist3", opList3);
	str += "\n";
	str += indent + ">";
	return str;
}


static Common::String _sceneOpCodeName(SceneOpCode code) {
	switch (code) {
	case kSceneOpNone: 		  return "none";
	case kSceneOpChangeScene: return "changeScene";
	case kSceneOpNoop:		  return "noop";
	case kSceneOpMeanwhile:   return "meanwhile";
	default:
		return Common::String::format("sceneOp%d", (int)code);
	}
}

Common::String SceneOp::dump(const Common::String &indent) const {
	Common::String argsStr;
	if (_args.empty()) {
		argsStr = "[]";
	} else {
		argsStr = "[";
		for  (uint i : _args)
			argsStr += Common::String::format("%d ", i);
		argsStr.setChar(']', argsStr.size() - 1);
	}
	Common::String str = Common::String::format("%sSceneOp<op: %s args: %s", indent.c_str(), _sceneOpCodeName(_opCode).c_str(), argsStr.c_str());

	str += _dumpStructList(indent, "struct1list", struct1List);
	if (!struct1List.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}


Common::String GameItem::dump(const Common::String &indent) const {
	Common::String super = SceneStruct2::dump(indent + "  ");

	Common::String str = Common::String::format(
			"%sGameItem<\n%s\n%sunk10 %d icon %d unk12 %d unk13 %d unk14 %d",
			indent.c_str(), super.c_str(), indent.c_str(), field10_0x24,
			_iconNum, field12_0x28, field13_0x2a, field14_0x2c);
	str += _dumpStructList(indent, "oplist5", opList5);
	str += _dumpStructList(indent, "oplist6", opList6);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String MouseCursor::dump(const Common::String &indent) const {
	return Common::String::format("%sMouseCursor<%d %d>", indent.c_str(), _hotX, _hotY);
}


Common::String SceneStruct4::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneStruct4<%d %d", indent.c_str(), val1, val2);

	str += _dumpStructList(indent, "oplist", opList);
	str += "\n";
	str += indent + ">";
	return str;
}


Common::String SceneStruct7::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSceneStruct7<%d %d", indent.c_str(), val, field1_0x2);
	str += _dumpStructList(indent, "struct1list", struct1List);
	str += _dumpStructList(indent, "opList", sceneOpList);
	str += "\n";
	str += indent + ">";
	return str;
}


void Dialogue::draw(Graphics::Surface *dst, int mode) {
	switch (_frameType) {
	case 1: return drawType1(dst, mode);
	case 2: return drawType2(dst, mode);
	case 3: return drawType3(dst, mode);
	case 4: return drawType4(dst, mode);
	default: error("unexpected frame type %d for dialog %d", _frameType, _num);
	}
}

static void _drawPixel(int x, int y, int color, void *data) {
	Graphics::Surface *surface = (Graphics::Surface *)data;

	if (x >= 0 && x < surface->w && y >= 0 && y < surface->h)
		*((byte *)surface->getBasePtr(x, y)) = (byte)color;
}


//  box with simple frame
void Dialogue::drawType1(Graphics::Surface *dst, int stage) {
	//if (!_field15_0x22)
	//	return;
	if (stage == 1) {
		int x = _rect.x;
		int y = _rect.y;
		int w = _rect.width;
		int h = _rect.height;

		// TODO: Is this right?
		dst->fillRect(Common::Rect(x, y, x + w, y + h), _bgColor);
		dst->fillRect(Common::Rect(x + 1, y + 1, x + w - 1, y + h - 1), _fontColor);
		/*
		int uVar1 = _field15_0x22;
		*(int *)(uVar1 + 2) = x + 3;
		*(int *)(uVar1 + 4) = y + 3;
		*(int *)(uVar1 + 6) = width + -6;
		*(int *)(uVar1 + 8) = height + -6; */
	} else if (stage == 2) {
		drawStage2(dst);
	} else if (stage == 2) {
		drawStage2(dst);
	} else {
		drawStage4(dst);
	}
}

// box with fancy frame and optional title (everything before ":")
void Dialogue::drawType2(Graphics::Surface *dst, int stage) {
	// TODO: Implement me properly.
	Common::Rect drawRect(_rect.x, _rect.y, _rect.x + _rect.width, _rect.y + _rect.height);
	dst->fillRect(drawRect, _bgColor);
}

// comic baloon style box
void Dialogue::drawType3(Graphics::Surface *dst, int stage) {
	// TODO: Implement me properly.
	Common::Rect drawRect(_rect.x, _rect.y, _rect.x + _rect.width, _rect.y + _rect.height);
	dst->fillRect(drawRect, _bgColor);
}

// ellipse
void Dialogue::drawType4(Graphics::Surface *dst, int stage) {
	if (stage == 1) {
		int x = _rect.x;
		int y = _rect.y;
		int w = _rect.width;
		int h = _rect.height;

		int midy = (h - 1) / 2;
		//int radius = (midy * 5) / 4;

		byte fillcolor;
		byte fillbgcolor;
		if (!(_flags & 1)) {
			fillcolor = 0;
			fillbgcolor = 15;
		} else {
			fillcolor = _fontColor;
			fillbgcolor = _bgColor;
		}

		// This is not exactly the same as the original - might need some work to get pixel-perfect
		Common::Rect drawRect(x, y, x + w, y + h);
		Graphics::drawRoundRect(drawRect, midy, fillbgcolor, true, _drawPixel, dst);
		Graphics::drawRoundRect(drawRect, midy, fillcolor, false, _drawPixel, dst);
	} else if (stage == 2) {
		drawStage2(dst);
	} else if (stage == 2) {
		drawStage2(dst);
	} else {
		drawStage4(dst);
	}
}

void Dialogue::drawStage2(Graphics::Surface *dst) {
	// TODO: various text wrapping and alignment calculations happen here.
}

void Dialogue::drawStage3(Graphics::Surface *dst) {
	// TODO: various text wrapping and alignment calculations happen here.
}

void Dialogue::drawStage4(Graphics::Surface *dst) {
	DgdsEngine *engine = static_cast<DgdsEngine *>(g_engine);
	const FontManager *fontman = engine->getFontMan();
	FontManager::FontType fontType = FontManager::k6x6Font;
	if (_fontSize == 1)
		fontType = FontManager::k8x8Font;
	else if (_fontSize == 3)
		fontType = FontManager::k4x5Font;
	const Font *font = fontman->getFont(fontType);

	// TODO: some more text calcuations happen here.
	// This is where we actually draw the text.
	// For now do the simplest wrapping.
	Common::StringArray lines;
	const int h = font->getFontHeight();
	font->wordWrapText(_str, _rect.width, lines);

	int ystart = _rect.y + (_rect.height - lines.size() * h) / 2;
	for (uint i = 0; i < lines.size(); i++) {
		//const int w = font->getStringWidth(lines[i]);
		font->drawString(dst, lines[i], _rect.x, ystart + i * h, _rect.width, _fontColor, Graphics::kTextAlignCenter);
	}

}

Common::String Dialogue::dump(const Common::String &indent) const {
	Common::String str = Common::String::format(
			"%sDialogue<num %d %s bgcol %d fcol %d unk7 %d unk8 %d fntsz %d flags 0x%02x frame %d delay %d next %d unk15 %d unk18 %d",
			indent.c_str(), _num, _rect.dump("").c_str(), _bgColor, _fontColor, _field7_0xe, _field8_0x10, _fontSize,
			_flags, _frameType, _time, _nextDialogNum, _field15_0x22, _field18_0x28);
	str += _dumpStructList(indent, "subStrings", _subStrings);
	str += "\n";
	str += indent + "  str='" + _str + "'>";
	return str;
}

Common::String DialogueAction::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sDialogueAction<%d span: %d-%d", indent.c_str(), val, strStart, strEnd);
	str += _dumpStructList(indent, "oplist", sceneOpList);
	if (!sceneOpList.empty()) {
		str += "\n";
		str += indent;
	}
	str += ">";
	return str;
}

// //////////////////////////////////// //

Scene::Scene() : _magic(0) {
}

bool Scene::isVersionOver(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) > 0;
}

bool Scene::isVersionUnder(const char *version) const {
	return strncmp(_version.c_str(), version, _version.size()) < 0;
}


bool Scene::readStruct1List(Common::SeekableReadStream *s, Common::Array<SceneStruct1> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct1 &dst : list) {
		dst.val1 = s->readUint16LE();
		dst.flags = s->readUint16LE();
		dst.val3 = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readStruct2(Common::SeekableReadStream *s, SceneStruct2 &dst) const {
	dst.rect.x = s->readUint16LE();
	dst.rect.y = s->readUint16LE();
	dst.rect.width = s->readUint16LE();
	dst.rect.height = s->readUint16LE();
	dst.field1_0x8 = s->readUint16LE();
	dst.field2_0xa = s->readUint16LE();
	readStruct1List(s, dst.struct1List);
	readOpList(s, dst.opList1);
	readOpList(s, dst.opList2);
	readOpList(s, dst.opList3);
	return !s->err();
}


bool Scene::readStruct2List(Common::SeekableReadStream *s, Common::Array<SceneStruct2> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct2 &dst : list) {
		readStruct2(s, dst);
	}
	return !s->err();
}


bool Scene::readStruct2ExtendedList(Common::SeekableReadStream *s, Common::Array<GameItem> &list) const {
	list.resize(s->readUint16LE());
	for (GameItem &dst : list) {
		readStruct2(s, dst);
	}
	for (GameItem &dst : list) {
		dst._iconNum = s->readUint16LE();
		dst.field12_0x28 = s->readUint16LE();
		dst.field14_0x2c = s->readUint16LE();
		if (!isVersionUnder(" 1.211"))
			dst.field13_0x2a = s->readUint16LE() & 0xfffe;
		if (!isVersionUnder(" 1.204")) {
			dst.field10_0x24 = s->readUint16LE();
			readOpList(s, dst.opList5);
			readOpList(s, dst.opList6);
		}
	}
	return !s->err();

}


bool Scene::readMouseHotspotList(Common::SeekableReadStream *s, Common::Array<MouseCursor> &list) const {
	list.resize(s->readUint16LE());
	for (MouseCursor &dst : list) {
		dst._hotX = s->readUint16LE();
		dst._hotY = s->readUint16LE();
	}
	return !s->err();
}


bool Scene::readStruct4List(Common::SeekableReadStream *s, Common::Array<SceneStruct4> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct4 &dst : list) {
		if (!isVersionOver(" 1.205")) {
			dst.val2 = s->readUint16LE();
			dst.val1 = s->readUint16LE();
			dst.val2 += s->readUint16LE();
		} else {
			dst.val1 = s->readUint16LE();
			dst.val2 = s->readUint16LE();
		}
		readOpList(s, dst.opList);
	}
	return !s->err();
}


bool Scene::readOpList(Common::SeekableReadStream *s, Common::Array<SceneOp> &list) const {
	list.resize(s->readUint16LE());
	for (SceneOp &dst : list) {
		readStruct1List(s, dst.struct1List);
		dst._opCode = static_cast<SceneOpCode>(s->readUint16LE());
		int nvals = s->readUint16LE();
		for (int i = 0; i < nvals / 2; i++) {
			dst._args.push_back(s->readUint16LE());
		}
	}

	return !s->err();
}


bool Scene::readDialogueList(Common::SeekableReadStream *s, Common::Array<Dialogue> &list) const {
	// Some data on this format here https://www.oldgamesitalia.net/forum/index.php?showtopic=24055&st=25&p=359214&#entry359214

	list.resize(s->readUint16LE());
	for (Dialogue &dst : list) {
		dst._num = s->readUint16LE();
		dst._rect.x = s->readUint16LE();
		dst._rect.y = s->readUint16LE();
		dst._rect.width = s->readUint16LE();
		dst._rect.height = s->readUint16LE();
		dst._bgColor = s->readUint16LE();
		dst._fontColor = s->readUint16LE(); // 0 = black, 0xf = white
		if (isVersionUnder(" 1.209")) {
			dst._field7_0xe = dst._bgColor;
			dst._field8_0x10 = dst._fontColor;
		} else {
			dst._field7_0xe = s->readUint16LE();
			dst._field8_0x10 = s->readUint16LE();
		}
		dst._fontSize = s->readUint16LE(); // 01 = 8x8, 02 = 6x6, 03 = 4x5
		if (isVersionUnder(" 1.210")) {
			dst._flags = s->readUint16LE();
		} else {
			// Game reads a 32 bit int but then truncates anyway..
			// probably never used the full thing.
			dst._flags = (s->readUint32LE() & 0xffff);
		}

		dst._frameType = s->readUint16LE(); // 01 =simple frame, 02 = with title w/ text before :, 03 = baloon, 04 = eliptical
		dst._time = s->readUint16LE();
		if (isVersionOver(" 1.207")) {
			dst._nextDialogNum = s->readUint16LE();
		}

		uint16 nbytes = s->readUint16LE();
		if (nbytes > 0) {
			dst._str = s->readString('\0', nbytes);
		} else {
			dst._str.clear();
		}
		readDialogSubstringList(s, dst._subStrings);

		if (isVersionUnder(" 1.209") && !dst._subStrings.empty()) {
			if (dst._fontColor == 0)
				dst._field8_0x10 = 4;
			else if (dst._fontColor == 0xff)
				dst._fontColor = 7;
			else
				dst._fontColor = dst._fontColor ^ 8;
		}
	}

	return !s->err();
}


bool Scene::readStruct7List(Common::SeekableReadStream *s, Common::Array<SceneStruct7> &list) const {
	list.resize(s->readUint16LE());
	for (SceneStruct7 &dst : list) {
		dst.val = s->readUint16LE();
		readStruct1List(s, dst.struct1List);
		readOpList(s, dst.sceneOpList);
	}

	return !s->err();
}


bool Scene::readDialogSubstringList(Common::SeekableReadStream *s, Common::Array<DialogueAction> &list) const {
	list.resize(s->readUint16LE());

	if (!list.empty())
		list[0].val = 1;

	for (DialogueAction &dst : list) {
		dst.strStart = s->readUint16LE();
		dst.strEnd = s->readUint16LE();
		readOpList(s, dst.sceneOpList);
	}

	return !s->err();
}


SDSScene::SDSScene() : _num(-1) {
}

bool SDSScene::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *sceneFile = resourceManager->getResource(filename);
	if (!sceneFile)
		error("Scene file %s not found", filename.c_str());

	DgdsChunkReader chunk(sceneFile);

	bool result = false;

	while (chunk.readNextHeader(EX_SDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_SDS)) {
			result = parse(stream);
		}
	}

	delete sceneFile;

	return result;
}

bool SDSScene::parse(Common::SeekableReadStream *stream) {
	_magic = stream->readUint32LE();
	_version = stream->readString();
	if (isVersionOver(" 1.211")) {
	//if (isVersionOver(" 1.216")) { // HoC
	//if (isVersionOver(" 1.224")) { // Beamish
		error("Unsupported scene version '%s'", _version.c_str());
	}
	_num = stream->readUint16LE();
	readOpList(stream, _enterSceneOps);
	readOpList(stream, _leaveSceneOps);
	if (isVersionOver(" 1.206")) {
		readOpList(stream, _opList3);
	}
	readOpList(stream, _opList4);
	_field6_0x14 = stream->readUint16LE();
	_adsFile = stream->readString();
	readStruct2List(stream, _struct2List);
	readStruct4List(stream, _struct4List1);
	if (isVersionOver(" 1.205")) {
		readStruct4List(stream, _struct4List2);
	}
	readDialogueList(stream, _dialogues);
	if (isVersionOver(" 1.203")) {
		readStruct7List(stream, _struct7List);
	}

	return !stream->err();
}

Common::String SDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sSDSScene<num %d %d ads %s", indent.c_str(), _num, _field6_0x14, _adsFile.c_str());
	str += _dumpStructList(indent, "enterSceneOps", _enterSceneOps);
	str += _dumpStructList(indent, "leaveSceneOps", _leaveSceneOps);
	str += _dumpStructList(indent, "opList3", _opList3);
	str += _dumpStructList(indent, "opList4", _opList4);
	str += _dumpStructList(indent, "struct2List", _struct2List);
	str += _dumpStructList(indent, "struct4List1", _struct4List1);
	str += _dumpStructList(indent, "struct4List2", _struct4List2);
	str += _dumpStructList(indent, "dialogues", _dialogues);
	str += _dumpStructList(indent, "struct7List", _struct7List);

	str += "\n";
	str += indent + ">";
	return str;
}



GDSScene::GDSScene() {
}

bool GDSScene::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *sceneFile = resourceManager->getResource(filename);
	if (!sceneFile)
		error("Scene file %s not found", filename.c_str());

	DgdsChunkReader chunk(sceneFile);

	bool result = false;

	while (chunk.readNextHeader(EX_GDS, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_GDS)) {
			// do nothing, this is the container.
			assert(chunk.isContainer());
		} else if (chunk.isSection(ID_INF)) {
			result = parseInf(stream);
		} else if (chunk.isSection(ID_SDS)) {
			result = parse(stream);
		}
	}

	delete sceneFile;

	return result;
}

bool GDSScene::parseInf(Common::SeekableReadStream *s) {
	_magic = s->readUint32LE();
	_version = s->readString();
	return !s->err();
}

bool GDSScene::parse(Common::SeekableReadStream *stream) {
	readOpList(stream, _startGameOps);
	readOpList(stream, _opList2);
	if (isVersionOver(" 1.206"))
		readOpList(stream, _opList3);
	readOpList(stream, _opList4);
	if (isVersionOver(" 1.208"))
		readOpList(stream, _opList5);
	Common::Array<struct SceneStruct1> struct1List;
	readStruct1List(stream, struct1List);
	Common::Array<struct MouseCursor> struct3List;
	_iconFile = stream->readString();
	readMouseHotspotList(stream, struct3List);
	readStruct2ExtendedList(stream, _gameItems);
	readStruct4List(stream, _struct4List2);
	if (isVersionOver(" 1.205"))
		readStruct4List(stream, _struct4List1);

	return !stream->err();
}

Common::String GDSScene::dump(const Common::String &indent) const {
	Common::String str = Common::String::format("%sGDSScene<icons %s", indent.c_str(), _iconFile.c_str());
	str += _dumpStructList(indent, "gameItems", _gameItems);
	str += _dumpStructList(indent, "opList1", _startGameOps);
	str += _dumpStructList(indent, "opList2", _opList2);
	str += _dumpStructList(indent, "opList3", _opList3);
	str += _dumpStructList(indent, "opList4", _opList4);
	str += _dumpStructList(indent, "opList5", _opList5);
	str += _dumpStructList(indent, "struct4List1", _struct4List1);
	str += _dumpStructList(indent, "struct4List2", _struct4List2);

	str += "\n";
	str += indent + ">";
	return str;
}


} // End of namespace Dgds

