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

#include "common/algorithm.h"
#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/formats/xmlparser.h"

#include "engines/util.h"
#include "ngi/ngi.h"
#include "ngi/constants.h"
#include "ngi/detection.h"
#include "ngi/messages.h"
#include "ngi/utils.h"
#include "ngi/scene.h"
#include "ngi/statics.h"
#include "ngi/anihandler.h"
#include "ngi/scenes.h"

namespace NGI {

namespace {

class XMLProjectParser : public Common::XMLParser {
public:
	const Common::String &startScene() const { return _startScene; }
	int width() const { return _width; }
	int height() const { return _height; }

	Common::String startSceneFile() const {
		return _scenes.getValOrDefault(_startScene);
	}

private:
	Common::String _startScene;
	Common::StringMap _scenes;
	int _width = 800;
	int _height = 600;

	CUSTOM_XML_PARSER(XMLProjectParser) {
		XML_KEY(PROJECT)
			XML_PROP(title, false)
			XML_PROP(width, false)
			XML_PROP(height, false)
			XML_PROP(screenWidth, false)
			XML_PROP(screenHeight, false)
			XML_PROP(dbg_font, false)
			XML_PROP(start, true)
			XML_PROP(fullscreen, false)
			XML_PROP(adjust, false)
			XML_PROP(create_inv, false)
			XML_KEY(SCENE)
				XML_PROP(id, true)
				XML_PROP(file, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_PROJECT(ParserNode *node) {
		_startScene = node->values["start"];
		if (node->values.contains("width"))
			_width = atoi(node->values["width"].c_str());
		if (node->values.contains("height"))
			_height = atoi(node->values["height"].c_str());
		if (_width <= 0 || _width > 32767 || _height <= 0 || _height > 32767)
			return parserError("Invalid project dimensions");
		return true;
	}

	bool parserCallback_SCENE(ParserNode *node) {
		_scenes[node->values["id"]] = node->values["file"];
		return true;
	}

	bool handleUnknownKey(ParserNode *node) override {
		const auto *parent = getParentNode(node);
		if (!parent)
			return false;
		if (!parent->ignore && (parent->name != "PROJECT" ||
				(node->name != "DIALOGCONTROLLER" && node->name != "CURSORCONTROLLER")))
			return false;
		node->layout = _XMLkeys;
		node->ignore = true;
		return true;
	}
};

class ObjectIdList {
public:
	bool load(const Common::Path &path) {
		Common::File file;
		if (!file.open(path))
			return false;

		while (!file.eos() && !file.err()) {
			const Common::String line = file.readLine();
			if (!line.hasPrefix("#define"))
				continue;

			const char *name = line.c_str() + 7;
			while (*name == ' ' || *name == '\t')
				++name;
			const char *value = name;
			while (*value && *value != ' ' && *value != '\t')
				++value;
			if (name == value || !*value)
				continue;

			const Common::String symbol(name, value);
			if (!_ids.contains(symbol))
				_ids[symbol] = strtoul(value, nullptr, 10);
		}
		return !file.err();
	}

	bool findId(const Common::String &symbol, uint32 &id) const {
		const auto entry = _ids.find(symbol);
		if (entry == _ids.end())
			return false;
		id = entry->_value;
		return true;
	}

private:
	Common::HashMap<Common::String, uint32> _ids;
};

static const char *getResourceLocale(Common::Language language) {
	switch (language) {
	case Common::DE_DEU:
		return "de";
	case Common::RU_RUS:
		return "ru";
	default:
		return "en";
	}
}

class XMLSceneParser : public Common::XMLParser {
public:
	XMLSceneParser(const ObjectIdList &ids, Scene &scene, const char *locale)
		: _ids(ids), _scene(scene), _locale(locale) {
		setAllowText();
	}

private:
	const ObjectIdList &_ids;
	Scene &_scene;
	const char *_locale;
	uint _objectOrder = 0;
	bool _hasInitialStatics = false;
	int16 _initialStaticsId = 0;

	bool loadTexture(Texture &texture, const Common::String &resource, bool localized) {
		const Common::String directory = localized ? Common::String::format("data/res_%s", _locale) : "data/res";
		const Common::Path path = Common::Path(directory).join(resource + ".png");
		Common::File file;
		if (!file.open(path) || !texture.loadPNG(file))
			return parserError("Could not load scene texture " + path.toString());

		const Dims dimensions = texture.getDimensions();
		debugC(2, kDebugLoading, "Loaded scene texture %s (%d x %d)", path.toString().c_str(), dimensions.x, dimensions.y);
		return true;
	}

	CUSTOM_XML_PARSER(XMLSceneParser) {
		XML_KEY(SCENE)
			XML_PROP(id, true)
			XML_PROP(title, false)
			XML_PROP(width, false)
			XML_PROP(height, false)
			XML_PROP(hero, false)
			XML_PROP(drawOffset, false)
			XML_PROP(serializable, false)
			XML_PROP(waitscreen, false)
			XML_PROP(putPrev, false)
			XML_PROP(scrollX, false)
			XML_PROP(scrollY, false)
			XML_KEY(PICTURE)
				XML_PROP(id, true)
				XML_PROP(title, false)
				XML_PROP(x, false)
				XML_PROP(y, false)
				XML_PROP(z, false)
				XML_PROP(flags, false)
				XML_PROP(locale, false)
			KEY_END()
			XML_KEY(OLDANI)
				XML_PROP(id, true)
				XML_PROP(title, false)
				XML_PROP(x, false)
				XML_PROP(y, false)
				XML_PROP(z, false)
				XML_PROP(flags, false)
				XML_PROP(locale, false)
				XML_PROP(static, false)
				XML_KEY(STATICS)
					XML_PROP(id, true)
					XML_PROP(title, false)
					XML_PROP(x, false)
					XML_PROP(y, false)
					XML_PROP(locale, false)
				KEY_END()
				XML_KEY(MOVEMENT)
					XML_PROP(id, true)
					XML_PROP(title, false)
					XML_PROP(x, false)
					XML_PROP(y, false)
					XML_PROP(z, false)
					XML_PROP(flags, false)
					XML_PROP(locale, false)
					XML_PROP(nIdPrev, true)
					XML_PROP(nIdNext, true)
					XML_PROP(nPrevStepX, false)
					XML_PROP(nPrevStepY, false)
					XML_PROP(nNextStepX, false)
					XML_PROP(nNextStepY, false)
					XML_PROP(dwLoopDelay, false)
					XML_PROP(link, false)
					XML_KEY(PHASE)
						XML_PROP(x, false)
						XML_PROP(y, false)
						XML_PROP(flags, false)
						XML_PROP(delay, false)
						XML_PROP(delayRelative, false)
						XML_PROP(sound, false)
						XML_PROP(cycled, false)
					KEY_END()
				KEY_END()
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_SCENE(ParserNode *node) {
		int16 id;
		if (!readId(node->values["id"], id) || id != _scene._sceneId)
			return parserError("Scene ID does not match the project");
		_scene._sceneName = node->values.getValOrDefault("title");
		_scene._x = strtod(node->values.getValOrDefault("scrollX").c_str(), nullptr);
		_scene._y = strtod(node->values.getValOrDefault("scrollY").c_str(), nullptr);
		return true;
	}

	bool parserCallback_PICTURE(ParserNode *node) {
		Common::ScopedPtr<PictureObject> picture(new PictureObject());
		if (!parseObject(*picture, *node))
			return false;

		picture->_picture = new Texture();
		const Common::String resource = Common::String::format("%08hu/%08hu",
			static_cast<uint16>(_scene._sceneId), static_cast<uint16>(picture->_id));
		if (!loadTexture(*picture->_picture, resource, isLocalized(*node)))
			return false;
		picture->setOXY2();
		_scene.addPicture(picture.release());
		return true;
	}

	bool parserCallback_OLDANI(ParserNode *node) {
		AniObject *animation = new AniObject();
		// addAni resets visibility and draw order, so read the XML fields afterwards.
		_scene.addAni(animation, true);
		if (!parseObject(*animation, *node))
			return false;

		const Common::String &statics = node->values.getValOrDefault("static");
		_hasInitialStatics = !statics.empty();
		if (_hasInitialStatics && !readId(statics, _initialStaticsId))
			return parserError("Unknown initial statics ID '" + statics + "'");

		animation->_sceneId = _scene._sceneId;
		return true;
	}

	bool parseObject(GameObject &object, ParserNode &node) {
		if (!readId(node.values["id"], object._id))
			return parserError("Unknown scene object ID '" + node.values["id"] + "'");

		object._title = node.values.getValOrDefault("title");
		object._x = strtod(node.values.getValOrDefault("x").c_str(), nullptr);
		object._y = strtod(node.values.getValOrDefault("y").c_str(), nullptr);
		object._z = strtod(node.values.getValOrDefault("z").c_str(), nullptr);
		object._objectFlags = strtoul(node.values.getValOrDefault("flags").c_str(), nullptr, 10);
		// XML bit 0 and NGI runtime bit 2 both mean visible.
		object._flags = (object._objectFlags & 1) ? 4 : 0;
		object._cnum = _objectOrder++;
		return true;
	}

	bool parserCallback_STATICS(ParserNode *node) {
		Common::ScopedPtr<Statics> pose(new Statics());
		if (!readId(node->values["id"], pose->_staticsId))
			return parserError("Unknown statics ID '" + node->values["id"] + "'");

		pose->_staticsName = node->values.getValOrDefault("title");
		// AniObject subtracts the anchor when drawing a pose. XML stores the texture offset.
		pose->_someX = -strtod(node->values.getValOrDefault("x").c_str(), nullptr);
		pose->_someY = -strtod(node->values.getValOrDefault("y").c_str(), nullptr);
		AniObject &object = *_scene._aniObjects.back();
		const Common::String resource = Common::String::format("%08hu/%08hu/%08hu",
			static_cast<uint16>(_scene._sceneId), static_cast<uint16>(object._id), static_cast<uint16>(pose->_staticsId));
		if (!loadTexture(*pose, resource, isLocalized(*node)))
			return false;

		if (_hasInitialStatics ? pose->_staticsId == _initialStaticsId :
				(!object._statics || static_cast<uint16>(pose->_staticsId) < static_cast<uint16>(object._statics->_staticsId)))
			object._statics = pose.get();
		object._staticsList.push_back(pose.release());
		return true;
	}

	bool parserCallback_MOVEMENT(ParserNode *node) {
		AniObject &object = *_scene._aniObjects.back();
		int16 prevId;
		int16 nextId;
		if (!readId(node->values["nIdPrev"], prevId) || !readId(node->values["nIdNext"], nextId))
			return parserError("Unknown statics ID in movement '" + node->values["id"] + "'");

		Statics *previous = object.getStatic(prevId);
		Statics *next = object.getStatic(nextId);
		if (!previous || !next)
			return parserError("Missing statics for movement '" + node->values["id"] + "'");

		Common::ScopedPtr<Movement> movement(new Movement());
		if (!parseObject(*movement, *node))
			return false;

		movement->_prevStatic = previous;
		movement->_nextStatic = next;
		movement->_prevStepX = atoi(node->values.getValOrDefault("nPrevStepX").c_str());
		movement->_prevStepY = atoi(node->values.getValOrDefault("nPrevStepY").c_str());
		movement->_nextStepX = atoi(node->values.getValOrDefault("nNextStepX").c_str());
		movement->_nextStepY = atoi(node->values.getValOrDefault("nNextStepY").c_str());
		movement->_loopDelay = atoi(node->values.getValOrDefault("dwLoopDelay", "42").c_str());
		const Common::String &link = node->values.getValOrDefault("link");
		movement->_link = link.empty() || link == "true" || link == "1";

		// Movement owns its phases except for the terminal statics shared with the animation.
		movement->_phases.push_back(next);
		movement->_framePosOffsets.push_back(Common::Point(movement->_nextStepX, movement->_nextStepY));
		object._movements.push_back(movement.release());
		return true;
	}

	bool parserCallback_PHASE(ParserNode *node) {
		AniObject &animation = *_scene._aniObjects.back();
		Movement &movement = *animation._movements.back();
		const uint index = movement._phases.size() - 1;
		const uint16 movementId = static_cast<uint16>(movement._id);
		const Common::String resource = Common::String::format("%08hu/%08hu/%08hu/%08hu.%03u",
			static_cast<uint16>(_scene._sceneId), static_cast<uint16>(animation._id), movementId, movementId, index);
		Common::ScopedPtr<Phase> phase(new Phase());
		phase->_flags = atoi(node->values.getValOrDefault("flags").c_str());
		phase->_delayMs = atoi(node->values.getValOrDefault("delay").c_str());
		phase->_delayRelative = atoi(node->values.getValOrDefault("delayRelative").c_str());
		phase->_x = atoi(node->values.getValOrDefault("x").c_str());
		phase->_y = atoi(node->values.getValOrDefault("y").c_str());
		if (!loadTexture(*phase, resource, isLocalized(*getParentNode(node))))
			return false;

		movement._framePosOffsets.insert_at(index, Common::Point(phase->_x, phase->_y));
		movement._phases.insert_at(index, phase.release());
		return true;
	}

	bool closedKeyCallback(ParserNode *node) override {
		if (node->name == "MOVEMENT")
			_scene._aniObjects.back()->_movements.back()->updateCurrDynamicPhase();
		if (node->name == "OLDANI" && !_scene._aniObjects.back()->_statics)
			return parserError("Missing initial statics for Android animation");
		return true;
	}

	bool readId(const Common::String &name, int16 &id) const {
		uint32 value;
		if (!_ids.findId(name, value) || value > 0xffff)
			return false;
		id = static_cast<int16>(value);
		return true;
	}

	static bool isLocalized(const ParserNode &node) {
		const Common::String &locale = node.values.getValOrDefault("locale");
		return locale == "true" || locale == "1";
	}

	bool handleUnknownKey(ParserNode *node) override {
		if (!getParentNode(node))
			return false;
		// Scene nodes outside the supported object types are ignored for now.
		node->ignore = true;
		node->layout = _XMLkeys;
		return true;
	}

	bool textCallback(const Common::String &text) override { return true; }
};

static void drawAndroidScene(const Scene &scene, Graphics::ManagedSurface &target) {
	Common::Array<const GameObject *> visibleObjects;
	for (const PictureObject *picture : scene._pictureObjects) {
		if (picture->_flags & 4)
			visibleObjects.push_back(picture);
	}
	for (const AniObject *animation : scene._renderableAnis) {
		if (animation->_flags & 4)
			visibleObjects.push_back(animation);
	}

	// The Android renderer draws higher z values first and preserves XML order at equal z.
	Common::sort(visibleObjects.begin(), visibleObjects.end(), [](const GameObject *left, const GameObject *right) {
		if (left->_z == right->_z)
			return left->_cnum < right->_cnum;
		return left->_z > right->_z;
	});

	for (const GameObject *object : visibleObjects) {
		Common::Point position(object->_x - scene._x, object->_y - scene._y);
		if (object->_objtype == kObjTypePictureObject) {
			static_cast<const PictureObject *>(object)->_picture->blitTo(target, position);
		} else {
			const AniObject &animation = *static_cast<const AniObject *>(object);
			const Phase *phase = animation._movement ? animation._movement->_curPhase : animation._statics;
			phase->blitTo(target, position - animation.getSomeXY());
		}
	}
}

} // End of anonymous namespace

Common::Error NGIEngine::runAndroid() {
	const auto &files = _gameDescription->desc.filesDescriptions;
	const Common::FSNode gameDirectory(ConfMan.getPath("path"));
	Common::ScopedPtr<Common::Archive> apk(Common::makeZipArchive(gameDirectory.getChild(files[0].fileName)));
	Common::ScopedPtr<Common::Archive> obb(Common::makeZipArchive(gameDirectory.getChild(files[1].fileName)));
	if (!apk || !obb)
		return Common::Error(Common::kNoGameDataFoundError, "Could not open the Full Pipe APK and OBB archives");

	SearchMan.add("ngi-android-obb", obb.release());
	SearchMan.add("ngi-android-apk", apk.release());

	const Common::Path projectPath("data/FullPipe.xml");
	XMLProjectParser project;
	if (!project.loadStream(SearchMan.createReadStreamForMember(projectPath), projectPath.toString()) || !project.parse())
		return Common::Error(Common::kNoGameDataFoundError, "Could not read the Full Pipe Android project");

	ObjectIdList ids;
	if (!ids.load("data/obj.h"))
		return Common::Error(Common::kNoGameDataFoundError, "Could not read the Full Pipe Android object IDs");

	const Common::String startSceneFile = project.startSceneFile();
	uint32 startSceneId;
	if (!ids.findId(project.startScene(), startSceneId) || startSceneId > 0xffff || startSceneFile.empty())
		return Common::Error(Common::kNoGameDataFoundError, "Could not find the Full Pipe Android start scene");

	debugC(1, kDebugXML, "Loaded Full Pipe Android project, start scene %s (%u)", startSceneFile.c_str(), startSceneId);

	Common::Language language = Common::parseLanguage(ConfMan.get("language"));
	if (language == Common::UNK_LANG)
		language = Common::parseLanguageFromLocale(_system->getSystemLanguage().c_str());

	Scene scene;
	scene._sceneId = static_cast<int16>(startSceneId);
	const Common::Path scenePath = Common::Path("data/res").join(startSceneFile);
	XMLSceneParser sceneParser(ids, scene, getResourceLocale(language));
	if (!sceneParser.loadStream(SearchMan.createReadStreamForMember(scenePath), scenePath.toString()) || !sceneParser.parse())
		return Common::Error(Common::kNoGameDataFoundError, "Could not load the Full Pipe Android start scene");

	debugC(1, kDebugXML, "Loaded Full Pipe Android scene %s with %u pictures and %u animations",
		startSceneFile.c_str(), scene._pictureObjects.size(), scene._aniObjects.size());

	_isSaveAllowed = false;
	initGraphics(project.width(), project.height(), nullptr);
	const Graphics::PixelFormat format = _system->getScreenFormat();
	if (format.isCLUT8())
		return Common::kUnsupportedColorMode;

	Graphics::ManagedSurface &frame = _backgroundSurface;
	frame.create(project.width(), project.height(), format);
	byte brightness = scene._sceneId == SC_INTRO1 ? 0 : 255;
	frame.clear(format.RGBToColor(0, 0, 0));
	if (brightness)
		drawAndroidScene(scene, frame);
	_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
	_system->updateScreen();
	debugC(1, kDebugDrawing, "Presented first Full Pipe Android frame for %s", startSceneFile.c_str());

	Common::Event event;
	Common::EventManager *eventManager = _system->getEventManager();
	uint32 lastUpdate = _system->getMillis();
	while (!shouldQuit()) {
		bool screenChanged = false;
		while (eventManager->pollEvent(event)) {
			if (event.type == Common::EVENT_SCREEN_CHANGED)
				screenChanged = true;
		}
		if (shouldQuit())
			break;

		const uint32 now = _system->getMillis();
		const uint32 elapsed = now - lastUpdate;
		lastUpdate = now;
		bool redraw = scene.update(elapsed);
		if (scene._sceneId == SC_INTRO1)
			redraw |= sceneIntro_updateAndroid(scene, elapsed, brightness);
		processMessages();

		if (redraw) {
			frame.clear(format.RGBToColor(0, 0, 0));
			drawAndroidScene(scene, frame);
			if (brightness < 255)
				drawAlphaRectangle(0, 0, frame.w, frame.h, brightness);
		}
		if (redraw || screenChanged)
			_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
		_system->updateScreen();
		_system->delayMillis(10);
	}
	return Common::kNoError;
}

} // End of namespace NGI
