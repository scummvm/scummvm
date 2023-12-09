#include "twp/twp.h"
#include "twp/room.h"
#include "twp/ggpack.h"

namespace Twp {

static Math::Vector2d parseVec2(const Common::String &s) {
	float x, y;
	sscanf(s.c_str(), "{%f,%f}", &x, &y);
	return {x, y};
}

static Math::Vector2d parseParallax(const Common::JSONValue &v) {
	if (v.isIntegerNumber()) {
		return {(float)v.asIntegerNumber(), 1};
	}
	if (v.isNumber()) {
		return {(float)v.asNumber(), 1};
	}
	if (v.isString()) {
		return parseVec2(v.asString());
	}
	error("parseParallax expected a float, int or string, not this: %s", v.stringify().c_str());
}

void Room::load(Common::SeekableReadStream &s) {
	GGHashMapDecoder d;
	Common::JSONValue *value = d.open(&s);
	const Common::JSONObject &jRoom = value->asObject();

	name = jRoom["name"]->asString();
	sheet = jRoom["sheet"]->asString();

	roomSize = parseVec2(jRoom["roomsize"]->asString());
	height = jRoom.contains("height") ? jRoom["height"]->asIntegerNumber() : roomSize.getY();
	fullscreen = jRoom.contains("fullscreen") ? jRoom["fullscreen"]->asIntegerNumber() : 0;

	// backgrounds
	Common::StringArray backNames;
	if (jRoom["background"]->isString()) {
		backNames.push_back(jRoom["background"]->asString());
	} else {
		const Common::JSONArray &jBacks = jRoom["background"]->asArray();
		for (int i = 0; i < jBacks.size(); i++) {
			backNames.push_back(jBacks[i]->asString());
		}
	}

	{
		Layer layer;
		layer.names.push_back(backNames);
		layer.zsort = 0;
		layer.parallax = Math::Vector2d(1, 1);
		layers.push_back(layer);
	}

	// layers
	if (jRoom.contains("layers")) {
		const Common::JSONArray &jLayers = jRoom["layers"]->asArray();
		for (int i = 0; i < jLayers.size(); i++) {
			Layer layer;
			const Common::JSONObject &jLayer = jLayers[i]->asObject();
			if (jLayer["name"]->isArray()) {
				const Common::JSONArray &jNames = jLayer["name"]->asArray();
				for (int j = 0; j < jNames.size(); j++) {
					layer.names.push_back(jNames[j]->asString());
				}
			} else if (jLayer["name"]->isString()) {
				layer.names.push_back(jLayer["name"]->asString());
			}
			layer.parallax = parseParallax(*jLayer["parallax"]);
			layer.zsort = jLayer["zsort"]->asIntegerNumber();
			layers.push_back(layer);
		}
	}

	// TODO: walkboxes, objects, scalings
	delete value;
}

} // namespace Twp
