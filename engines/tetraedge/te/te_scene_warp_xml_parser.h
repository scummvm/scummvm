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

#ifndef TETRAEDGE_TE_TE_SCENE_WARP_XML_PARSER_H
#define TETRAEDGE_TE_TE_SCENE_WARP_XML_PARSER_H

#include "tetraedge/te/te_xml_parser.h"
#include "tetraedge/te/te_scene_warp.h"

namespace Tetraedge {

class TeSceneWarpXmlParser : public TeXmlParser {
public:
	enum ObjType {
		kObjNone,
		kObjExit,
		kObjObject
	};

	TeSceneWarpXmlParser(TeSceneWarp *sceneWarp, bool flag) : _sceneWarp(sceneWarp),
		_objType(kObjNone), _flag(flag) {}

	// Parser
	CUSTOM_XML_PARSER(TeSceneWarpXmlParser) {
		XML_KEY(FileFormatVersion)
		KEY_END()
		XML_KEY(exit)
			XML_PROP(linkedWarp, true)
			XML_PROP(nbWarpBlock, true)
			XML_KEY(camera)
				XML_PROP(angleX, true)
				XML_PROP(angleY, true)
			KEY_END()
			XML_KEY(marker)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(block)
				XML_PROP(face, true)
				XML_PROP(offsetX, true)
				XML_PROP(offsetY, true)
			KEY_END()
		KEY_END()
		XML_KEY(animation)
			XML_PROP(name, true)
			XML_PROP(fps, true)
		KEY_END()
		XML_KEY(object)
			XML_PROP(name, true)
			XML_KEY(marker)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

public:
	bool parserCallback_FileFormatVersion(ParserNode *node) { return true; }
	bool parserCallback_exit(ParserNode *node);
	bool parserCallback_camera(ParserNode *node);
	bool parserCallback_marker(ParserNode *node);
	bool parserCallback_block(ParserNode *node);
	bool parserCallback_object(ParserNode *node);
	bool parserCallback_animation(ParserNode *node);

	TeSceneWarp *_sceneWarp;
	ObjType _objType;
	bool _flag;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SCENE_WARP_XML_PARSER_H
