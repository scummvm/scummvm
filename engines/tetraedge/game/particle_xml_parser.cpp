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

#include "tetraedge/game/particle_xml_parser.h"

namespace Tetraedge {

bool ParticleXmlParser::parserCallback_particle(ParserNode *node) {
	_scene->particles().push_back(new TeParticle(_scene));
	return true;
}

bool ParticleXmlParser::parserCallback_name(ParserNode *node) {
	_scene->particles().back()->setName(node->values["value"]);
	return true;
}

bool ParticleXmlParser::parserCallback_texture(ParserNode *node) {
	_scene->particles().back()->loadTexture(node->values["value"]);
	return true;
}

bool ParticleXmlParser::parserCallback_position(ParserNode *node) {
	_scene->particles().back()->setPosition(parsePoint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_direction(ParserNode *node) {
	_scene->particles().back()->setDirection(parsePoint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_size(ParserNode *node) {
	_scene->particles().back()->setSize(parseDouble(node));
	return true;
}

bool ParticleXmlParser::parserCallback_volumesize(ParserNode *node) {
	_scene->particles().back()->setVolumeSize(parsePoint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_startcolor(ParserNode *node) {
	TeColor col;
	if (parseCol(node, col)) {
		_scene->particles().back()->setStartColor(col);
		return true;
	}
	return false;
}

bool ParticleXmlParser::parserCallback_endcolor(ParserNode *node) {
	TeColor col;
	if (parseCol(node, col)) {
		_scene->particles().back()->setEndColor(col);
		return true;
	}
	return false;
}

bool ParticleXmlParser::parserCallback_colortime(ParserNode *node) {
	_scene->particles().back()->setColorTime(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_time(ParserNode *node) {
	_scene->particles().back()->setTime(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_period(ParserNode *node) {
	_scene->particles().back()->setPeriod(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_particleperperiod(ParserNode *node) {
	_scene->particles().back()->setParticlePerPeriod(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_startloop(ParserNode *node) {
	_scene->particles().back()->setStartLoop(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_enabled(ParserNode *node) {
	_scene->particles().back()->setEnabled(parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_gravity(ParserNode *node) {
	_scene->particles().back()->setGravity(parseDouble(node));
	return true;
}

bool ParticleXmlParser::parserCallback_randomdirection(ParserNode *node) {
	_scene->particles().back()->setRandomDir((bool)parseUint(node));
	return true;
}

bool ParticleXmlParser::parserCallback_orientation(ParserNode *node) {
	_scene->particles().back()->setOrientation(parsePoint(node));
	return true;
}


} // end namespace Tetraedge
