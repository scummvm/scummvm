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

#ifndef TETRAEDGE_TE_TE_CAMERA_XML_PARSER_H
#define TETRAEDGE_TE_TE_CAMERA_XML_PARSER_H

#include "tetraedge/te/te_xml_parser.h"
#include "tetraedge/te/te_camera.h"

namespace Tetraedge {

class TeCameraXmlParser : public TeXmlParser {
public:
	// Parser
	CUSTOM_XML_PARSER(TeCameraXmlParser) {
		XML_KEY(camera)
			XML_KEY(position)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(rotation)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
				XML_PROP(w, true)
			KEY_END()
			XML_KEY(scale)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(fov)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(aspect)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(near)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(far)
				XML_PROP(value, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

public:
	bool parserCallback_camera(ParserNode *node) { return true; }
	bool parserCallback_position(ParserNode *node);
	bool parserCallback_rotation(ParserNode *node);
	bool parserCallback_scale(ParserNode *node);
	bool parserCallback_fov(ParserNode *node);
	bool parserCallback_aspect(ParserNode *node);
	bool parserCallback_near(ParserNode *node);
	bool parserCallback_far(ParserNode *node);

	TeCamera *_cam;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_CAMERA_XML_PARSER_H
