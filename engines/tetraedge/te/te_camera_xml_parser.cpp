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

#include "tetraedge/te/te_camera_xml_parser.h"

namespace Tetraedge {

bool TeCameraXmlParser::parserCallback_position(ParserNode *node) {
	_cam->setPosition(parsePoint(node));
	return true;
}

bool TeCameraXmlParser::parserCallback_rotation(ParserNode *node) {
	float x = atof(node->values["x"].c_str());
	float y = atof(node->values["y"].c_str());
	float z = atof(node->values["z"].c_str());
	float w = atof(node->values["w"].c_str());
	_cam->setRotation(TeQuaternion(x, y, z, w));
	return true;

}

bool TeCameraXmlParser::parserCallback_scale(ParserNode *node) {
	_cam->setScale(parsePoint(node));
	return true;
}

bool TeCameraXmlParser::parserCallback_fov(ParserNode *node) {
	float fov = parseDouble(node);
	fov = atanf(1.0f / (1.333333f / tanf(fov / 2)));
	_cam->setFov(fov * 2);
	return true;
}

bool TeCameraXmlParser::parserCallback_aspect(ParserNode *node) {
	_cam->setAspectRatio(parseDouble(node));
	return true;
}

bool TeCameraXmlParser::parserCallback_near(ParserNode *node) {
	_cam->setOrthoNear(parseDouble(node));
	return true;
}

bool TeCameraXmlParser::parserCallback_far(ParserNode *node) {
	_cam->setOrthoFar(parseDouble(node));
	return true;
}

} // end namespace Tetraedge
