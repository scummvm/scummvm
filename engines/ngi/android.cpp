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

#include "backends/keymapper/keymapper.h"
#include "engines/util.h"
#include "ngi/ngi.h"
#include "ngi/behavior.h"
#include "ngi/constants.h"
#include "ngi/detection.h"
#include "ngi/messages.h"
#include "ngi/modal.h"
#include "ngi/motion.h"
#include "ngi/ngiarchive.h"
#include "ngi/objects.h"
#include "ngi/utils.h"
#include "ngi/scene.h"
#include "ngi/statics.h"
#include "ngi/anihandler.h"
#include "ngi/scenes.h"

namespace NGI {

bool ObjectIdList::load(const Common::Path &path) {
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

bool ObjectIdList::findId(const Common::String &symbol, uint32 &id) const {
	const auto entry = _ids.find(symbol);
	if (entry == _ids.end())
		return false;
	id = entry->_value;
	return true;
}

namespace {

class XMLProjectParser : public Common::XMLParser {
public:
	explicit XMLProjectParser(GameProject &project) : _project(project) {
		_project._sceneTagList.reset(new SceneTagList());
	}

	const SceneTag *startScene() const { return _startScene; }
	int width() const { return _width; }
	int height() const { return _height; }

private:
	GameProject &_project;
	Common::String _startName;
	SceneTag *_startScene = nullptr;
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
		_startName = node->values["start"];
		if (node->values.contains("width"))
			_width = atoi(node->values["width"].c_str());
		if (node->values.contains("height"))
			_height = atoi(node->values["height"].c_str());
		if (_width <= 0 || _width > 32767 || _height <= 0 || _height > 32767)
			return parserError("Invalid project dimensions");
		return true;
	}

	bool parserCallback_SCENE(ParserNode *node) {
		const Common::String &name = node->values["id"];
		uint32 id;
		if (!_project._objectIds.findId(name, id) || id > 0xffff)
			return parserError("Unknown project scene ID '" + name + "'");
		_project._sceneTagList->emplace_back();
		SceneTag &tag = _project._sceneTagList->back();
		tag._sceneId = static_cast<int16>(id);
		tag._tag = node->values["file"];
		if (name == _startName)
			_startScene = &tag;
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

class XMLTextureSizeParser : public Common::XMLParser {
public:
	explicit XMLTextureSizeParser(GameProject &project) : _project(project) {}

	void loadSizes() {
		if (!read("tex_data_main.xml"))
			return;
		Common::Array<Common::Path> files;
		files.swap(_files);
		for (const Common::Path &path : files)
			read(path);
	}

private:
	GameProject &_project;
	Common::Array<Common::Path> _files;

	bool read(const Common::Path &name) {
		const Common::Path path = Common::Path("data").join(name);
		close();
		return loadFile(path) && parse();
	}

	CUSTOM_XML_PARSER(XMLTextureSizeParser) {
		XML_KEY(FILES)
			XML_KEY(FLIE)
				XML_PROP(path, true)
			KEY_END()
		KEY_END()
		XML_KEY(TEX_DATA)
			XML_KEY(DATA)
				XML_PROP(path, true)
				XML_PROP(w, true)
				XML_PROP(h, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_FILES(ParserNode *) { return true; }
	bool parserCallback_TEX_DATA(ParserNode *) { return true; }

	bool parserCallback_FLIE(ParserNode *node) {
		_files.push_back(Common::Path(node->values["path"], '\\'));
		return true;
	}

	bool parserCallback_DATA(ParserNode *node) {
		const Common::Path path = Common::Path("data").join(Common::Path(node->values["path"], '\\'));
		if (!_project._textureSizes.contains(path))
			_project._textureSizes[path] = Dims(atoi(node->values["w"].c_str()), atoi(node->values["h"].c_str()));
		return true;
	}
};

class XMLSceneParser : public Common::XMLParser {
public:
	XMLSceneParser(const GameProject &project, Scene &scene, const char *locale)
		: _project(project), _scene(scene), _locale(locale) {
		setAllowText();
	}

private:
	const GameProject &_project;
	Scene &_scene;
	const char *_locale;
	uint _objectOrder = 0;
	bool _hasInitialStatics = false;
	int16 _initialStaticsId = 0;
	BehaviorAni *_behaviorAni = nullptr;
	MctlItem *_motionItem = nullptr;
	MctlGraph *_graph = nullptr;
	Common::Array<Common::Pair<int, int> > _linkEndpoints;

	Common::Path texturePath(const Common::String &resource, bool localized) const {
		const Common::String directory = localized ? Common::String::format("data/res_%s", _locale) : "data/res";
		return Common::Path(directory).join(resource + ".png");
	}

	void loadTexture(Texture &texture, const Common::String &resource, bool localized) {
		const Common::Path path = texturePath(resource, localized);
		texture.loadPNG(path, _project._textureSizes.getValOrDefault(path));
	}

	CUSTOM_XML_PARSER(XMLSceneParser) {
		XMLKeyLayout *commandLayout;
		XMLKeyLayout *polygonLayout, *parallelLayout;
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
			XML_KEY(MCTL_COMPOUND)
				XML_KEY(MCTL_ITEM)
					XML_PROP(ofsX, false)
					XML_PROP(ofsY, false)
					XML_PROP(ofsZ, false)
					XML_KEY(REACT_POLYGONAL)
						polygonLayout = layout.top();
						XML_PROP(centerX, false)
						XML_PROP(centerY, false)
						XML_KEY(REACT_POINT)
							XML_PROP(x, false)
							XML_PROP(y, false)
						KEY_END()
					KEY_END()
					XML_KEY(REACT_PARALLEL)
						parallelLayout = layout.top();
						XML_PROP(x1, false)
						XML_PROP(y1, false)
						XML_PROP(x2, false)
						XML_PROP(y2, false)
						XML_PROP(left, false)
						XML_PROP(right, false)
					KEY_END()
					XML_KEY(MCTL_GRAPH)
						XML_KEY(GRAPH_NODE)
							XML_PROP(id, true)
							XML_PROP(x, false)
							XML_PROP(y, false)
							XML_PROP(z, false)
							XML_PROP(flags, false)
						KEY_END()
						XMLKeyLayout *nodeLayout = layout.top()->children["GRAPH_NODE"];
						XML_KEY(GRAPH_LINK)
							XML_PROP(flags, false)
							XML_PROP(length, false)
							XML_PROP(angle, false)
							XML_PROP(title, false)
							XML_KEY(MOVE)
								XML_PROP(id, true)
								XML_PROP(mirror, false)
							KEY_END()
							layout.top()->children["STAT"] = layout.top()->children["MOVE"];
							layout.top()->children["GRAPH_SRC"] = nodeLayout;
							layout.top()->children["GRAPH_DST"] = nodeLayout;
							layout.top()->children["REACT_PARALLEL"] = parallelLayout;
							layout.top()->children["REACT_POLYGONAL"] = polygonLayout;
						KEY_END()
					KEY_END()
				KEY_END()
			KEY_END()
			XML_KEY(ADDANI)
				XML_PROP(id, true)
				XML_PROP(scene, true)
				XML_PROP(copy, false)
			KEY_END()
			XML_KEY(ENTRANCE)
				XML_PROP(id, true)
				XML_PROP(scene, true)
				XML_PROP(queue, true)
			KEY_END()
			XML_KEY(SOUND)
				XML_PROP(id, true)
				XML_PROP(file, true)
				XML_PROP(title, false)
				XML_PROP(flags, false)
			KEY_END()
			XML_KEY(BEHAVIORCONTROLLER)
				XML_KEY(BHVR_ANI)
					XML_PROP(id, true)
					XML_PROP(scene, false)
					XML_PROP(flags, false)
					XML_KEY(BHVR_MOVE)
						XML_PROP(static, true)
						XML_PROP(id, false)
						XML_PROP(queue, false)
						XML_PROP(delay, false)
						XML_PROP(percent, false)
						XML_PROP(mirror, false)
						XML_PROP(enable, false)
						XML_PROP(reverse, false)
					KEY_END()
				KEY_END()
				XML_KEY(BHVR_SOUND)
					XML_PROP(id, true)
					XML_PROP(delay, false)
					XML_PROP(percent, false)
					XML_PROP(flags, false)
					XML_PROP(enable, false)
				KEY_END()
				layout.top()->children["BHVR_MUSIC"] = layout.top()->children["BHVR_SOUND"];
			KEY_END()
			XML_KEY(QUEUE)
				XML_PROP(id, true)
				XML_PROP(title, false)
				XML_KEY(COMMAND)
					commandLayout = layout.top();
					XML_PROP(cmd, true)
					XML_PROP(wait, false)
					XML_PROP(id, false)
					XML_PROP(ani, false)
					XML_PROP(mov, false)
					XML_PROP(vis, false)
					XML_PROP(copy, false)
					XML_PROP(msg, false)
					XML_PROP(cycled, false)
					XML_PROP(stop, false)
					XML_PROP(delay, false)
					XML_PROP(x, false)
					XML_PROP(y, false)
					XML_PROP(z, false)
					XML_PROP(mirror, false)
					XML_PROP(static, false)
					XML_PROP(stat, false)
					XML_PROP(type, false)
					XML_PROP(state, false)
					XML_PROP(picId, false)
					XML_PROP(title, false)
				KEY_END()
			KEY_END()
			XML_KEY(PICTURE)
				XML_PROP(filter, false)
				XML_PROP(serializable, false)
				XML_PROP(id, true)
				XML_PROP(title, false)
				XML_PROP(x, false)
				XML_PROP(y, false)
				XML_PROP(z, false)
				XML_PROP(flags, false)
				XML_PROP(locale, false)
			KEY_END()
			XML_KEY(OLDANI)
				XML_PROP(mirror, false)
				XML_PROP(serializable, false)
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
					XML_PROP(mirrorPrev, false)
					XML_PROP(mirrorNext, false)
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
						layout.top()->children["COMMAND"] = commandLayout;
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
		if (node->values.contains("hero") && !node->values["hero"].empty() && !readId(node->values["hero"], _scene._heroId))
			return parserError("Unknown scene hero");
		_scene._sceneName = node->values.getValOrDefault("title");
		_scene._x = strtod(node->values.getValOrDefault("scrollX").c_str(), nullptr);
		_scene._y = strtod(node->values.getValOrDefault("scrollY").c_str(), nullptr);
		return true;
	}

	bool parserCallback_ADDANI(ParserNode *node) {
		PicAniInfo ref;
		if (!readId(node->values["id"], ref.objectId) || !readId(node->values["scene"], ref.sceneId))
			return parserError("Unknown roaming animation or scene");
		ref.copyId = atoi(node->values.getValOrDefault("copy").c_str());
		_scene._roamingObjects.push_back(ref);
		return true;
	}

	bool parserCallback_ENTRANCE(ParserNode *node) {
		uint32 id = 0, scene = 0, queue = 0;
		if (!_project._objectIds.findId(node->values["id"], id) ||
			!_project._objectIds.findId(node->values["scene"], scene) ||
			!_project._objectIds.findId(node->values["queue"], queue))
			return parserError("Unknown ID in scene entrance");
		_scene._entrances.push_back(Entrance{(int32)scene, (int32)id, (int32)queue, 0});
		return true;
	}

	static int intProperty(const ParserNode &node, const char *key) {
		return atoi(node.values.getValOrDefault(key).c_str());
	}

	bool parserCallback_MCTL_COMPOUND(ParserNode *) {
		_scene._motionController.reset(new MctlCompound());
		return true;
	}

	bool parserCallback_MCTL_ITEM(ParserNode *node) {
		_motionItem = new MctlItem();
		_scene._motionController->_motionControllers.push_back(_motionItem);
		_motionItem->_ofsX = intProperty(*node, "ofsX");
		_motionItem->_ofsY = intProperty(*node, "ofsY");
		_motionItem->_ofsZ = intProperty(*node, "ofsZ");
		return true;
	}

	bool parserCallback_MCTL_GRAPH(ParserNode *) {
		_graph = new MctlGraph();
		_motionItem->_motionControllerObj.reset(_graph);
		_linkEndpoints.clear();
		return true;
	}

	bool parserCallback_GRAPH_NODE(ParserNode *node) {
		const int id = intProperty(*node, "id");
		if (node->name == "GRAPH_SRC") {
			_linkEndpoints.back().first = id;
		} else if (node->name == "GRAPH_DST") {
			_linkEndpoints.back().second = id;
		} else {
			MovGraphNode *point = new MovGraphNode();
			_graph->_nodes.push_back(point);
			point->_id = id;
			point->_x = intProperty(*node, "x");
			point->_y = intProperty(*node, "y");
			point->_z = intProperty(*node, "z");
			point->_flags = intProperty(*node, "flags");
		}
		return true;
	}

	bool parserCallback_GRAPH_LINK(ParserNode *node) {
		MovGraphLink *link = new MovGraphLink();
		_graph->_links.push_back(link);
		_linkEndpoints.emplace_back(-1, -1);
		if (node->values.contains("flags"))
			link->_flags = intProperty(*node, "flags");
		link->_length = strtof(node->values.getValOrDefault("length").c_str(), nullptr);
		link->_angle = strtof(node->values.getValOrDefault("angle").c_str(), nullptr);
		link->_title = node->values.getValOrDefault("title");
		return true;
	}

	bool parserCallback_MOVE(ParserNode *node) {
		uint32 id;
		if (!readId(node->values["id"], id))
			return parserError("Unknown ID on motion graph link: " + node->values["id"]);
		MovGraphLink *link = _graph->_links.back();
		const bool move = node->name == "MOVE";
		(move ? link->_movementIds : link->_staticsIds).push_back(id);
		(move ? link->_movementMirrors : link->_staticsMirrors).push_back(boolProperty(*node, "mirror"));
		return true;
	}

	bool parserCallback_REACT_PARALLEL(ParserNode *node) {
		ReactParallel *region = new ReactParallel(intProperty(*node, "left"), intProperty(*node, "right"));
		region->setAnchorPoints(intProperty(*node, "x1"), intProperty(*node, "y1"), intProperty(*node, "x2"), intProperty(*node, "y2"));
		region->create();
		return setGraphRegion(node, region);
	}

	bool parserCallback_REACT_POLYGONAL(ParserNode *node) {
		return setGraphRegion(node, new ReactPolygonal(intProperty(*node, "centerX"), intProperty(*node, "centerY")));
	}

	bool setGraphRegion(ParserNode *node, MovGraphReact *region) {
		if (getParentNode(node)->name == "GRAPH_LINK") {
			MovGraphLink *link = _graph->_links.back();
			delete link->_movGraphReact;
			link->_movGraphReact = region;
		} else {
			_motionItem->_movGraphReactObj.reset(region);
		}
		return true;
	}

	bool parserCallback_REACT_POINT(ParserNode *node) {
		MovGraphReact *region = getParentNode(getParentNode(node))->name == "GRAPH_LINK" ?
				_graph->_links.back()->_movGraphReact : _motionItem->_movGraphReactObj.get();
		region->_points.push_back(Common::Point(intProperty(*node, "x"), intProperty(*node, "y")));
		return true;
	}

	bool parserCallback_PICTURE(ParserNode *node) {
		Common::ScopedPtr<PictureObject> picture(new PictureObject());
		if (!parseObject(*picture, *node))
			return false;

		picture->_picture = new Texture();
		const Common::String resource = Common::String::format("%08hu/%08hu",
			static_cast<uint16>(_scene._sceneId), static_cast<uint16>(picture->_id));
		loadTexture(*picture->_picture, resource, isLocalized(*node));
		picture->setOXY2();
		_scene.addPicture(picture.release());
		return true;
	}

	bool parserCallback_SOUND(ParserNode *node) {
		uint16 id;
		if (!readId(node->values["id"], id))
			return parserError("Unknown sound ID " + node->values["id"]);
		const int flags = atoi(node->values.getValOrDefault("flags").c_str());
		const bool speech = flags & Sound::kSpeech;
		Common::Path path(speech ? Common::String::format("data/res_%s", _locale) : "data/res");
		path = path.join(Common::String::format("Sounds/%08hu", static_cast<uint16>(_scene._sceneId)));
		if (speech)
			path = path.join("Talks");
		path = path.join(node->values["file"]);
		if (!SearchMan.hasFile(path))
			path = Common::Path("assets").join(path);
		if (!_scene._soundList)
			_scene._soundList.reset(new SoundList());
		_scene._soundList->addSound(id, path, flags);
		return true;
	}

	bool parserCallback_QUEUE(ParserNode *node) {
		Common::ScopedPtr<MessageQueue> queue(new MessageQueue());
		if (!readId(node->values["id"], queue->_queueId))
			return parserError("Unknown queue ID " + node->values["id"]);
		queue->_title = node->values.getValOrDefault("title");
		_scene._messageQueues.push_back(queue.release());
		return true;
	}

	bool parserCallback_BEHAVIORCONTROLLER(ParserNode *) {
		_scene._behaviorController.reset(new BehaviorController());
		return true;
	}

	bool parserCallback_BHVR_SOUND(ParserNode *node) {
		BehaviorSound sound = {};
		if (!readId(node->values["id"], sound._soundId))
			return parserError("Unknown behavior sound " + node->values["id"]);
		sound._delay = strtoul(node->values.getValOrDefault("delay").c_str(), nullptr, 10);
		sound._percent = strtoul(node->values.getValOrDefault("percent").c_str(), nullptr, 10);
		sound._loop = atoi(node->values.getValOrDefault("flags").c_str()) != 0;
		sound._enabled = !node->values.contains("enable") || boolProperty(*node, "enable");
		_scene._behaviorController->addSound(sound, node->name == "BHVR_MUSIC");
		return true;
	}

	bool parserCallback_BHVR_ANI(ParserNode *node) {
		int16 actorId;
		int16 sceneId = _scene._sceneId;
		if (!readId(node->values["id"], actorId) ||
				(node->values.contains("scene") && !readId(node->values["scene"], sceneId)))
			return parserError("Unknown actor or scene in BHVR_ANI");
		_behaviorAni = &_scene._behaviorController->addAni(actorId, sceneId, atoi(node->values.getValOrDefault("flags").c_str()));
		return true;
	}

	bool parserCallback_BHVR_MOVE(ParserNode *node) {
		int16 staticsId;
		if (!readId(node->values["static"], staticsId))
			return parserError("Unknown statics in BHVR_MOVE");
		BehaviorMove &move = _behaviorAni->addMove(staticsId);
		if ((node->values.contains("id") && !readId(node->values["id"], move._movementId)) ||
				(node->values.contains("queue") && !readId(node->values["queue"], move._queueId)))
			return parserError("Unknown movement or queue in BHVR_MOVE");
		move._delay = atoi(node->values.getValOrDefault("delay").c_str());
		move._percent = atoi(node->values.getValOrDefault("percent").c_str());
		move._mirror = boolProperty(*node, "mirror");
		if (node->values.contains("enable") && !boolProperty(*node, "enable"))
			move._flags |= 1;
		return true;
	}

	static bool boolProperty(const ParserNode &node, const char *name) {
		const Common::String &value = node.values.getValOrDefault(name);
		return value == "1" || value == "true" || value == "yes";
	}

	bool parserCallback_COMMAND(ParserNode *node) {
		Common::ScopedPtr<Command> cmd(new Command());
		Common::String kind = node->values["cmd"];
		kind.toUppercase();
		cmd->_excFlags = 2;
		cmd->_wait = boolProperty(*node, "wait");
		cmd->_param = atoi(node->values.getValOrDefault("copy").c_str());

		const char *idProperty = nullptr;
		if (kind == "SHOW" || kind == "1") {
			cmd->_cmd = 5;
			cmd->_wait = 0;
			if (!readId(node->values.getValOrDefault("ani"), cmd->_parentId))
				return parserError("Unknown animation in SHOW");
			cmd->_messageNum = -1;
			if (!node->values.getValOrDefault("mov").empty())
				idProperty = "mov";
			cmd->_x = atoi(node->values.getValOrDefault("x", "-1").c_str());
			cmd->_y = atoi(node->values.getValOrDefault("y", "-1").c_str());
			cmd->_z = atoi(node->values.getValOrDefault("z", "-1").c_str());
			cmd->_auxParam = node->values.contains("mirror") ? (int)boolProperty(*node, "mirror") : -1;
		} else if (kind == "USER" || kind == "12" || kind == "SET_STATIC" || kind == "25") {
			cmd->_cmd = 63;
			cmd->_messageName = node->values.getValOrDefault("type", kind);
			cmd->_wait = 0;
		} else if (kind == "START_MOVEMENT" || kind == "3") {
			cmd->_cmd = 1;
			cmd->_mirror = boolProperty(*node, "mirror");
			if (!readId(node->values.getValOrDefault("ani"), cmd->_parentId))
				return parserError("Unknown animation in START_MOVEMENT");
			idProperty = "mov";
		} else if (kind == "HIDE" || kind == "2") {
			cmd->_cmd = 6;
			cmd->_wait = 0;
			if (!readId(node->values.getValOrDefault("vis"), cmd->_parentId))
				return parserError("Unknown object in HIDE");
		} else if (kind == "QUEUE" || kind == "24") {
			cmd->_cmd = 18;
			idProperty = "id";
		} else if (kind == "SOUND" || kind == "7") {
			if (cmd->_wait)
				return parserError("Waiting for an XML sound is not implemented");
			cmd->_cmd = 35;
			cmd->_param = boolProperty(*node, "cycled");
			cmd->_z = boolProperty(*node, "stop") ? 2 : 1;
			if (!readId(node->values.getValOrDefault("id"), cmd->_messageNum))
				return parserError("Unknown sound in SOUND");
		} else if (kind == "MESSAGE" || kind == "5") {
			cmd->_cmd = 17;
			cmd->_messageName = node->values.getValOrDefault("msg");
			uint32 id;
			if (_project._objectIds.findId(cmd->_messageName, id))
				cmd->_messageNum = id;
		} else if (kind == "PAUSE" || kind == "22") {
			cmd->_cmd = Command::kCmdPause;
			cmd->_messageNum = MAX(0, atoi(node->values.getValOrDefault("delay").c_str()));
		} else {
			return parserError("Unsupported XML command " + kind);
		}
		if (idProperty) {
			int16 id;
			if (!readId(node->values.getValOrDefault(idProperty), id))
				return parserError("Unknown " + Common::String(idProperty) + " in " + kind);
			cmd->_messageNum = id;
		}
		if (getParentNode(node)->name == "QUEUE") {
			_scene._messageQueues.back()->addCommandToEnd(cmd.release());
		} else {
			Movement &movement = *_scene._aniObjects.back()->_movements.back();
			movement._phases[movement._phases.size() - 2]->_command.reset(cmd.release());
		}
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

		if (boolProperty(*node, "mirror"))
			animation->_flags |= 8;
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
		loadTexture(*pose, resource, isLocalized(*node));

		if (_hasInitialStatics ? pose->_staticsId == _initialStaticsId :
				(!object._statics || static_cast<uint16>(pose->_staticsId) < static_cast<uint16>(object._statics->_staticsId)))
			object._statics = pose.get();
		object._staticsList.push_back(pose.release());
		return true;
	}

	bool parserCallback_MOVEMENT(ParserNode *node) {
		AniObject &object = *_scene._aniObjects.back();
		int16 prevId = 0;
		int16 nextId = 0;
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
		movement->_mirrorPrev = boolProperty(*node, "mirrorPrev");
		movement->_mirrorNext = boolProperty(*node, "mirrorNext");
		movement->_prevStepX = atoi(node->values.getValOrDefault("nPrevStepX").c_str());
		movement->_prevStepY = atoi(node->values.getValOrDefault("nPrevStepY").c_str());
		movement->_nextStepX = atoi(node->values.getValOrDefault("nNextStepX").c_str());
		movement->_nextStepY = atoi(node->values.getValOrDefault("nNextStepY").c_str());
		movement->_loopDelay = atoi(node->values.getValOrDefault("dwLoopDelay", "42").c_str());
		movement->_link = !node->values.contains("link") || boolProperty(*node, "link");

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
		if (node->values.contains("sound") && !readId(node->values["sound"], phase->_soundId))
			return parserError("Unknown phase sound " + node->values["sound"]);
		phase->_loopSound = boolProperty(*node, "cycled");
		phase->_flags = atoi(node->values.getValOrDefault("flags").c_str());
		phase->_delayMs = atoi(node->values.getValOrDefault("delay").c_str());
		phase->_delayRelative = atoi(node->values.getValOrDefault("delayRelative").c_str());
		phase->_x = atoi(node->values.getValOrDefault("x").c_str());
		phase->_y = atoi(node->values.getValOrDefault("y").c_str());
		const Common::Path path = texturePath(resource, isLocalized(*getParentNode(node)));
		phase->setPNG(path, _project._textureSizes.getValOrDefault(path));

		movement._framePosOffsets.insert_at(index, Common::Point(phase->_x, phase->_y));
		movement._phases.insert_at(index, phase.release());
		return true;
	}

	bool closedKeyCallback(ParserNode *node) override {
		if (node->name == "MCTL_GRAPH") {
			// Links can precede their nodes. Only GRAPH_NODE defines the coordinates.
			uint i = 0;
			for (MovGraphLink *link : _graph->_links) {
				const auto &ids = _linkEndpoints[i++];
				link->_graphSrc = _graph->getNodeById(ids.first);
				link->_graphDst = _graph->getNodeById(ids.second);
				if (!link->_graphSrc || !link->_graphDst || !link->_movGraphReact)
					return parserError("Incomplete Android motion graph link");
			}
		}
		if (node->name == "MCTL_ITEM" && !_motionItem->_motionControllerObj)
			return parserError("Missing Android motion graph");
		if (node->name == "MOVEMENT")
			_scene._aniObjects.back()->_movements.back()->updateCurrDynamicPhase();
		if (node->name == "OLDANI" && !_scene._aniObjects.back()->_statics)
			return parserError("Missing initial statics for Android animation");
		return true;
	}

	template<typename T>
	bool readId(const Common::String &name, T &id) const {
		uint32 value;
		if (!_project._objectIds.findId(name, value) || value > 0xffff)
			return false;
		id = static_cast<T>(value);
		return true;
	}

	static bool isLocalized(const ParserNode &node) {
		return boolProperty(node, "locale");
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

} // End of anonymous namespace

void Scene::drawAndroid(Graphics::ManagedSurface &target) const {
	Common::Array<const GameObject *> visibleObjects;
	for (const PictureObject *picture : _pictureObjects) {
		if (picture->_flags & 4)
			visibleObjects.push_back(picture);
	}
	for (const AniObject *animation : _renderableAnis) {
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
		Common::Point position(object->_x - _x, object->_y - _y);
		if (object->_objtype == kObjTypePictureObject) {
			static_cast<const PictureObject *>(object)->_picture->blitTo(target, position);
		} else {
			const AniObject &animation = *static_cast<const AniObject *>(object);
			const Phase *phase = animation._movement ? animation._movement->_curPhase : animation._statics;
			Common::Point anchor = animation.getSomeXY();
			const bool mirror = (animation._flags & 8) != 0;
			if (mirror)
				anchor.x = phase->getDimensions().x - anchor.x;
			phase->blitTo(target, position - anchor, mirror);
		}
	}
}

const char *NGIEngine::getAndroidResourceLocale() const {
	Common::Language language = Common::parseLanguage(ConfMan.get("language"));
	if (language == Common::UNK_LANG)
		language = Common::parseLanguageFromLocale(g_system->getSystemLanguage().c_str());

	switch (language) {
	case Common::DE_DEU:
		return "de";
	case Common::RU_RUS:
		return "ru";
	default:
		return "en";
	}
}

void SceneTag::loadSceneXML(const GameProject &project) {
	Common::ScopedPtr<Scene> scene(new Scene());
	scene->_sceneId = _sceneId;
	const Common::Path path = Common::Path("data/res").join(_tag);
	XMLSceneParser parser(project, *scene, g_nmi->getAndroidResourceLocale());
	if (!parser.loadStream(SearchMan.createReadStreamForMember(path), path.toString()) || !parser.parse()) {
		warning("Could not load Full Pipe Android scene %s", _tag.c_str());
		return;
	}

	debugC(1, kDebugXML, "Loaded Full Pipe Android scene %s with %u pictures and %u animations",
		_tag.c_str(), scene->_pictureObjects.size(), scene->_aniObjects.size());
	delete _scene;
	_scene = scene.release();
}

Common::Error NGIEngine::enterAndroidScene1() {
	Scene *room = findScene(SC_1);
	Scene *common = findScene(SC_COMMON);
	if (!room || !common)
		return Common::Error(Common::kNoGameDataFoundError, "Could not load the first Full Pipe Android room");
	room->_commonQueueScene = common;
	for (const PicAniInfo &ref : room->_roamingObjects) {
		Scene *source = findScene(ref.sceneId);
		AniObject *actor = source ? source->findAniObjectById(ref.objectId, ref.copyId) : nullptr;
		if (!actor)
			return Common::Error(Common::kNoGameDataFoundError, "Missing roaming animation for the first Android room");
		if (!room->findAniObjectById(ref.objectId, ref.copyId))
			room->_renderableAnis.push_back(actor);
	}
	_aniMan = room->findAniObjectById(room->_heroId, 0);
	_aniMan2 = _aniMan;
	if (!_aniMan)
		return Common::Error(Common::kNoGameDataFoundError, "Missing hero for the first Android room");

	clearGlobalMessageQueueList1();
	clearMessageHandlers();
	stopAllSounds(Sound::kSpeech);
	stopAllSounds(Sound::kEffect);
	if (room->_behaviorController)
		room->_behaviorController->onShow(*room);
	_currSoundListCount = 0;
	if (room->_soundList)
		_currSoundList1[_currSoundListCount++] = room->_soundList.get();
	if (common->_soundList)
		_currSoundList1[_currSoundListCount++] = common->_soundList.get();

	_currentScene = room;
	addMessageHandlerByIndex(global_messageHandler4, 0, 4);
	addMessageHandler(sceneHandler01, 2);
	// CStateManager::SetStartStates starts a new game with the egg uncracked.
	scene01_initObjects(room, TrubaLeft, true);
	for (const Entrance &entrance : room->_entrances) {
		if (entrance._id == TrubaLeft && entrance._sceneId == room->_sceneId && runQueue(entrance._queueId, 0)) {
			debugC(1, kDebugSceneLogic, "Entered Android scene %d through entrance %d, queue %d", room->_sceneId, entrance._id, entrance._queueId);
			return Common::kNoError;
		}
	}
	return Common::Error(Common::kNoGameDataFoundError, "Could not start the TrubaLeft entrance queue");
}

Common::Error NGIEngine::runAndroid() {
	const auto &files = _gameDescription->desc.filesDescriptions;
	const Common::FSNode gameDirectory(ConfMan.getPath("path"));
	Common::ScopedPtr<Common::Archive> apk(Common::makeZipArchive(gameDirectory.getChild(files[0].fileName)));
	Common::ScopedPtr<Common::Archive> obb(Common::makeZipArchive(gameDirectory.getChild(files[1].fileName)));
	if (!apk || !obb)
		return Common::Error(Common::kNoGameDataFoundError, "Could not open the Full Pipe APK and OBB archives");

	SearchMan.add("ngi-android-obb", obb.release());
	SearchMan.add("ngi-android-apk", apk.release());

	GameProject project;
	XMLTextureSizeParser(project).loadSizes();
	if (!project._objectIds.load("data/obj.h"))
		return Common::Error(Common::kNoGameDataFoundError, "Could not read the Full Pipe Android object IDs");

	const Common::Path projectPath("data/FullPipe.xml");
	XMLProjectParser parser(project);
	if (!parser.loadStream(SearchMan.createReadStreamForMember(projectPath), projectPath.toString()) || !parser.parse())
		return Common::Error(Common::kNoGameDataFoundError, "Could not read the Full Pipe Android project");

	const SceneTag *start = parser.startScene();
	if (!start)
		return Common::Error(Common::kNoGameDataFoundError, "Could not find the Full Pipe Android start scene");
	debugC(1, kDebugXML, "Loaded Full Pipe Android project, start scene %s (%hu)",
		start->_tag.c_str(), static_cast<uint16>(start->_sceneId));
	Scene *startScene = project.findScene(start->_sceneId);
	if (!startScene)
		return Common::Error(Common::kNoGameDataFoundError, "Could not load the Full Pipe Android start scene");

	const bool intro = start->_sceneId == SC_INTRO1;
	Scene *intro2 = intro ? project.findScene(SC_INTRO2) : nullptr;
	if (intro && !intro2)
		return Common::Error(Common::kNoGameDataFoundError, "Could not load the second Full Pipe Android intro scene");
	Scene *common = intro ? project.findScene(SC_COMMON) : nullptr;
	if (intro && !common)
		return Common::Error(Common::kNoGameDataFoundError, "Could not load the shared Full Pipe Android resources");

	_isSaveAllowed = false;
	initGraphics(parser.width(), parser.height(), nullptr);
	const Graphics::PixelFormat format = _system->getScreenFormat();
	if (format.isCLUT8())
		return Common::kUnsupportedColorMode;

	Common::Keymapper *mapper = _system->getEventManager()->getKeymapper();
	mapper->disableAllGameKeymaps();
	mapper->setGameKeymapState("ngi-default", true);
	_gameProject = &project;
	_currentScene = startScene;
	if (startScene->_behaviorController)
		startScene->_behaviorController->onShow(*startScene);
	addMessageHandlerByIndex(global_messageHandler4, 0, 4);
	if (intro) {
		sceneIntro_initMan(*startScene);
		addMessageHandler(sceneHandlerIntro, 2);
	}
	if (startScene->_soundList)
		_currSoundList1[_currSoundListCount++] = startScene->_soundList.get();
	if (intro2 && intro2->_soundList)
		_currSoundList1[_currSoundListCount++] = intro2->_soundList.get();
	if (common && common->_soundList)
		_currSoundList1[_currSoundListCount++] = common->_soundList.get();

	Graphics::ManagedSurface &frame = _backgroundSurface;
	frame.create(parser.width(), parser.height(), format);
	byte brightness = intro ? 0 : 255;
	frame.clear(format.RGBToColor(0, 0, 0));
	if (brightness)
		_currentScene->drawAndroid(frame);
	_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
	_system->updateScreen();
	debugC(1, kDebugDrawing, "Presented first Full Pipe Android frame for %s", start->_tag.c_str());

	Common::Event event;
	Common::EventManager *eventManager = _system->getEventManager();
	Common::Error result = Common::kNoError;
	uint32 lastUpdate = _system->getMillis();
	while (!shouldQuit()) {
		bool screenChanged = false;
		while (eventManager->pollEvent(event)) {
			if (event.type == Common::EVENT_SCREEN_CHANGED) {
				screenChanged = true;
			} else if (_modalObject) {
				const bool back = (event.type == Common::EVENT_KEYUP &&
						(event.kbd.keycode == Common::KEYCODE_ESCAPE || event.kbd.keycode == Common::KEYCODE_AC_BACK)) ||
					(event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == kActionCloseDialog);
				if (event.type == Common::EVENT_LBUTTONUP || back) {
					Command cmd;
					cmd._cmd = 17;
					cmd._messageNum = back ? 37 : 30;
					cmd._param = back ? kActionCloseDialog : 0;
					_modalObject->handleMessage(&cmd);
				}
			}
		}
		if (shouldQuit())
			break;

		const uint32 now = _system->getMillis();
		const uint32 elapsed = now - lastUpdate;
		lastUpdate = now;
		bool redraw = false;
		Scene *scene = _currentScene;
		if (_modalObject) {
			if (!_modalObject->init(elapsed))
				deleteModalObject();
			redraw = true;
		} else {
			if (scene->_behaviorController)
				redraw |= scene->_behaviorController->update(elapsed);
			for (MessageQueue *queue : *_globalMessageQueueList)
				redraw |= queue->updatePause(elapsed);
			processMessages();
			if (!_modalObject && scene == _currentScene)
				redraw |= scene->update(elapsed);
			if (!_modalObject && scene == _currentScene && scene->_sceneId == SC_INTRO1)
				redraw |= sceneIntro_updateAndroid(elapsed, brightness);
			processMessages();
			redraw |= _modalObject != nullptr;
		}

		if (scene != _currentScene) {
			stopAllSounds(Sound::kSpeech);
			stopAllSounds(Sound::kEffect);
			if (_currentScene->_behaviorController)
				_currentScene->_behaviorController->onShow(*_currentScene);
			debugC(2, kDebugDrawing, "Switched Android scene from %d to %d", scene->_sceneId, _currentScene->_sceneId);
			redraw = true;
		}
		if (!_modalObject && _currentScene->_sceneId == SC_INTRO1 && !g_vars->sceneIntro_playing) {
			result = enterAndroidScene1();
			if (result.getCode() != Common::kNoError)
				break;
			lastUpdate = _system->getMillis();
			brightness = 255;
			redraw = true;
		}
		if (redraw) {
			frame.clear(format.RGBToColor(0, 0, 0));
			if (_modalObject) {
				_modalObject->update();
			} else {
				_currentScene->drawAndroid(frame);
				if (brightness < 255)
					drawAlphaRectangle(0, 0, frame.w, frame.h, brightness);
			}
		}
		if (redraw || screenChanged)
			_system->copyRectToScreen(frame.getPixels(), frame.pitch, 0, 0, frame.w, frame.h);
		_system->updateScreen();
		_system->delayMillis(10);
	}
	deleteModalObject();
	clearMessages();
	clearGlobalMessageQueueList();
	clearMessageHandlers();
	while (_currSoundListCount)
		_currSoundList1[--_currSoundListCount] = nullptr;
	g_vars->sceneIntro_aniin1man = nullptr;
	g_vars->scene01_picSc01Osk = nullptr;
	g_vars->scene01_picSc01Osk2 = nullptr;
	_aniMan = _aniMan2 = nullptr;
	_currentScene = nullptr;
	_gameProject = nullptr;
	return result;
}

} // End of namespace NGI
