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

#include "gui/MetadataParser.h"

#include "common/system.h"
#include "common/tokenizer.h"

namespace GUI {

MetadataParser::MetadataParser() : XMLParser() {
}

MetadataParser::~MetadataParser() {
}

void MetadataParser::cleanup() {
}

bool MetadataParser::parserCallback_games(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_game(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_engines(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_engine(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_series(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_serie(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_companies(ParserNode *node) {
	return true;
}

bool MetadataParser::parserCallback_company(ParserNode *node) {
	return true;
}

bool MetadataParser::closedKeyCallback(ParserNode *node) {
	if (node->name == "game")
		_gameInfo[node->values["id"]] = MetadataGame(node->values["id"], node->values["name"], node->values["engine_id"],
										node->values["company_id"],	node->values["moby_id"], node->values["datafiles"],
										node->values["series_id"]);
	if (node->name == "engine")
		_engineInfo[node->values["id"]] = MetadataEngine(node->values["id"], node->values["name"], node->values["alt_name"],
											true);
	if (node->name == "serie")
		_seriesInfo[node->values["id"]] = MetadataSeries(node->values["id"], node->values["name"]);
	if (node->name == "company")
		_companyInfo[node->values["id"]] = MetadataCompany(node->values["id"], node->values["name"], node->values["alt_name"]);
	return true;
}

} // End of namespace GUI
