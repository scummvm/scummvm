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

#ifndef TETRAEDGE_GAME_PARTICLE_XML_PARSER_H
#define TETRAEDGE_GAME_PARTICLE_XML_PARSER_H

#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/te/te_xml_parser.h"

namespace Tetraedge {

class ParticleXmlParser : public TeXmlParser {
public:
	CUSTOM_XML_PARSER(ParticleXmlParser) {
		XML_KEY(particle)
			XML_KEY(name)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(texture)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(position)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(direction)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(size)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(volumesize)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
			XML_KEY(startcolor)
				XML_PROP(r, true)
				XML_PROP(g, true)
				XML_PROP(b, true)
				XML_PROP(a, true)
			KEY_END()
			XML_KEY(endcolor)
				XML_PROP(r, true)
				XML_PROP(g, true)
				XML_PROP(b, true)
				XML_PROP(a, true)
			KEY_END()
			XML_KEY(colortime)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(time)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(period)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(particleperperiod)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(startloop)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(enabled)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(gravity)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(randomdirection)
				XML_PROP(value, true)
			KEY_END()
			XML_KEY(orientation)
				XML_PROP(x, true)
				XML_PROP(y, true)
				XML_PROP(z, true)
			KEY_END()
		KEY_END()
	} PARSER_END()

	bool parserCallback_particle(ParserNode *node);
	bool parserCallback_name(ParserNode *node);
	bool parserCallback_texture(ParserNode *node);
	bool parserCallback_position(ParserNode *node);
	bool parserCallback_direction(ParserNode *node);
	bool parserCallback_size(ParserNode *node);
	bool parserCallback_volumesize(ParserNode *node);
	bool parserCallback_startcolor(ParserNode *node);
	bool parserCallback_endcolor(ParserNode *node);
	bool parserCallback_colortime(ParserNode *node);
	bool parserCallback_time(ParserNode *node);
	bool parserCallback_period(ParserNode *node);
	bool parserCallback_particleperperiod(ParserNode *node);
	bool parserCallback_startloop(ParserNode *node);
	bool parserCallback_enabled(ParserNode *node);
	bool parserCallback_gravity(ParserNode *node);
	bool parserCallback_randomdirection(ParserNode *node);
	bool parserCallback_orientation(ParserNode *node);

public:
	InGameScene *_scene;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_PARTICLE_XML_PARSER_H
