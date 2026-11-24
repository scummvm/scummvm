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

#include "ngi/ngi.h"
#include "ngi/constants.h"
#include "ngi/font.h"
#include "ngi/gameloader.h"
#include "ngi/modal.h"
#include "ngi/anihandler.h"
#include "backends/keymapper/keymapper.h"
#include "ngi/scenes.h"
#include "common/file.h"
#include "common/formats/xmlparser.h"

namespace NGI {

namespace {

class ChapterSettings : public Common::XMLParser {
public:
	explicit ChapterSettings(const char *id) : _id(id) {}

	bool load(const Common::Path &directory) {
		if (!loadFile("data/chapter.xml") || !parse() || !_number || _textId.empty())
			return false;
		close();

		Common::File encoding, texts;
		if (!encoding.open(directory.join("enc.dat")) || encoding.size() < 2 || (encoding.size() & 1) ||
				!texts.open(directory.join("texts.xml")) || texts.readUint16LE() != 0xfeff)
			return false;

		Common::HashMap<uint16, byte> codes;
		for (int i = 128; encoding.pos() + 2 < encoding.size(); ++i) {
			const uint16 c = encoding.readUint16LE();
			if (!codes.contains(c))
				codes[c] = i;
		}
		const uint16 first = encoding.readUint16LE();
		const int base = codes.size() + 1 - 128 - first;
		// The UTF-16 resource starts with a code page line and ends with </TEXTCONTAINER>.
		const int64 size = texts.size();
		while (texts.pos() + 2 <= size) {
			if (texts.readUint16LE() == '\n')
				break;
		}
		Common::String xml("<?xml version=\"1.0\"?><TEXTCONTAINER>");
		while (texts.pos() + 2 <= size) {
			const uint16 c = texts.readUint16LE();
			xml += c < 128 ? (byte)c : codes.getValOrDefault(c, (byte)(base + c));
		}
		if (!loadBuffer((const byte *)xml.c_str(), xml.size()) || !parse())
			return false;
		close();
		if (_title.empty() || _text.empty())
			return false;
		_title += Common::String::format("%d", _number);
		return true;
	}

	Common::String _title, _text;
	int _fadeIn = 1025, _fadeOut = 1025, _soundDelay = 4500, _charDelay = 123;
	bool _textAppearance = false;
	int _countShow = 0;

private:
	Common::String _id, _textId;
	int _chapterIndex = 0, _number = 0;

	CUSTOM_XML_PARSER(ChapterSettings) {
		XML_KEY(CHAPTERS)
			XML_KEY(CHAPTER)
				XML_PROP(id, true)
				XML_PROP(countShow, false)
				XML_KEY(START)
					XML_PROP(text_id, true)
				KEY_END()
			KEY_END()
		KEY_END()
		XML_KEY(CHAPTER_CONTROLLER)
			XML_PROP(text_appearance, false)
			XML_PROP(char_delay, false)
			XML_PROP(fadein_delay, false)
			XML_PROP(fadeout_delay, false)
			XML_PROP(sound_delay, false)
		KEY_END()
		XML_KEY(TEXTCONTAINER)
			XML_KEY(TEXT)
				XML_PROP(id, true)
				XML_PROP(text, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_CHAPTERS(ParserNode *) { return true; }
	bool parserCallback_TEXTCONTAINER(ParserNode *) { return true; }

	bool parserCallback_CHAPTER(ParserNode *node) {
		++_chapterIndex;
		if (node->values["id"] == _id) {
			_number = _chapterIndex;
			_countShow = atoi(node->values.getValOrDefault("countShow").c_str());
		}
		return true;
	}

	bool parserCallback_START(ParserNode *node) {
		if (_number == _chapterIndex)
			_textId = node->values["text_id"];
		return true;
	}

	static int delay(ParserNode *node, const char *key, int defaultValue) {
		if (!node->values.contains(key))
			return defaultValue;
		return MAX(0, atoi(node->values[key].c_str()));
	}

	bool parserCallback_CHAPTER_CONTROLLER(ParserNode *node) {
		const Common::String &appearance = node->values.getValOrDefault("text_appearance");
		_textAppearance = appearance.equalsIgnoreCase("true") || appearance == "1";
		_charDelay = delay(node, "char_delay", _charDelay);
		_soundDelay = delay(node, "sound_delay", _soundDelay);
		_fadeIn = delay(node, "fadein_delay", _fadeIn);
		_fadeOut = delay(node, "fadeout_delay", _fadeOut);
		return true;
	}

	bool parserCallback_TEXT(ParserNode *node) {
		const Common::String &id = node->values["id"];
		if (id == "STR_CHAPTER")
			_title = node->values["text"];
		if (id == _textId)
			_text = node->values["text"];
		return true;
	}

	bool handleUnknownKey(ParserNode *node) override {
		if (!getParentNode(node))
			return false;
		node->layout = _XMLkeys;
		node->ignore = true;
		return true;
	}
};

class ModalChapter : public BaseModalObject {
public:
	ModalChapter(Scene &scene, const char *id) : _scene(scene), _settings(id) {}

	~ModalChapter() override {
		if (_shown) {
			if (_scene._soundList)
				_scene.stopAllSounds();
			setModal(false);
		}
	}

	bool open() {
		const Common::String locale = g_nmi->getAndroidResourceLocale();
		const Common::Path directory("data/res_" + locale);
		const Common::Path fonts = directory.join("Fonts");
		if (!_settings.load(directory)) {
			warning("Could not read Android chapter settings or text for %s", locale.c_str());
			return false;
		}
		if (_settings._countShow == 1)
			return false;
		if (!_font.load(fonts.join("task_dlg_30.fnt")))
			return false;
		if (locale == "ru") {
			_titleFont.reset(new BitmapFont());
			if (!_titleFont->load(fonts.join("task_dlg_38.fnt")))
				return false;
		}

		const uint length = _settings._title.size() + _settings._text.size();
		_visibleChars = _settings._textAppearance ? 0 : length;
		const int64 soundTime = (int64)_settings._soundDelay - _settings._fadeIn - _settings._fadeOut;
		const int64 textTime = (int64)length * _settings._charDelay;
		const uint32 hold = MIN<int64>(UINT32_MAX, MAX(soundTime, textTime));
		_shown = true;
		setModal(true);
		_fader.start(_settings._fadeIn, hold, _settings._fadeOut);
		if (_scene._soundList) {
			Sound *sound = _scene._soundList->getSound(SND_CHAPTER_DLG);
			if (sound)
				sound->play(0);
		}
		debugC(2, kDebugSceneLogic, "Opened Android chapter dialog (%s, %u ms hold)", locale.c_str(), hold);
		return true;
	}

	bool pollEvent() override { return true; }
	void saveload() override {}

	bool handleMessage(Command *message) override {
		if (message->_cmd != 17 || (message->_messageNum != 30 &&
				!(message->_messageNum == 37 && message->_param == kActionCloseDialog)))
			return false;
		_visibleChars = _settings._title.size() + _settings._text.size();
		_fader.fadeOut();
		return true;
	}

	bool init(int elapsed) override {
		_fader.update(elapsed);
		if (!_fader.active()) {
			debugC(2, kDebugSceneLogic, "Closed Android chapter dialog");
			return false;
		}
		if (_visibleChars < _settings._title.size() + _settings._text.size()) {
			_charElapsed += elapsed;
			if (_charElapsed >= (uint)_settings._charDelay) {
				++_visibleChars;
				_charElapsed = 0;
			}
		}
		return true;
	}

	void update() override {
		Graphics::ManagedSurface &frame = g_nmi->_backgroundSurface;
		_scene.drawAndroid(frame);
		if (_fader.brightness() < 255)
			g_nmi->drawAlphaRectangle(0, 0, frame.w, frame.h, _fader.brightness());
		const BitmapFont &titleFont = _titleFont ? *_titleFont : _font;
		titleFont.drawCentered(frame, Common::Rect(130, 300, 360, _titleFont ? 385 : 405),
			_settings._title.substr(0, _visibleChars));
		if (_visibleChars > _settings._title.size())
			_font.drawCentered(frame, Common::Rect(440, 180, 685, _titleFont ? 495 : 505),
				_settings._text.substr(0, _visibleChars - _settings._title.size()));
	}

private:
	void setModal(bool active) {
		Common::Keymapper *mapper = g_system->getEventManager()->getKeymapper();
		mapper->setGameKeymapState("ngi-default", !active);
		mapper->setGameKeymapState("dialog", active);
		for (int i = 0; i < g_nmi->_currSoundListCount; ++i) {
			SoundList &sounds = *g_nmi->_currSoundList1[i];
			for (int j = 0; j < sounds.getCount(); ++j)
				sounds.getSoundByIndex(j).pause(active);
		}
	}

	bool _shown = false;
	Scene &_scene;
	ChapterSettings _settings;
	BitmapFont _font;
	Common::ScopedPtr<BitmapFont> _titleFont;
	Fader _fader;
	uint _visibleChars = 0;
	uint32 _charElapsed = 0;
};

} // End of anonymous namespace

void sceneChapter_show(const char *chapterId) {
	Scene *scene = g_nmi->findScene(SC_CHAPTER);
	if (!scene)
		return;
	Common::ScopedPtr<ModalChapter> dialog(new ModalChapter(*scene, chapterId));
	if (dialog->open()) {
		dialog->_parentObj = g_nmi->_modalObject;
		g_nmi->_modalObject = dialog.release();
	}
}

} // End of namespace NGI
