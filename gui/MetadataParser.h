/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef METADATA_PARSER_H
#define METADATA_PARSER_H

#include "common/scummsys.h"
#include "common/xmlparser.h"

namespace GUI {

struct MetadataGame {
	typedef Common::String String;

	String id;
	String name;
	String engine_id;
	String company_id;
	String moby_id;
	String datafiles;
	String series_id;

	MetadataGame() : id(nullptr), name(nullptr), engine_id(nullptr), company_id(nullptr), moby_id(nullptr), datafiles(nullptr), series_id(nullptr) {}
	MetadataGame(const String i, const String n, const String eid, const String cid, const String mid, const String df, const String sid)
	: id(i), name(n), engine_id(eid), company_id(cid), moby_id(mid), datafiles(df), series_id(sid) {}
};

struct MetadataEngine {
	typedef Common::String String;

	String id;
	String name;
	String alt_name;
	bool enabled;

	MetadataEngine() : id(nullptr), name(nullptr), alt_name(nullptr), enabled(false) {}
	MetadataEngine(const String i, const String n, const String altn, bool e)
	: id(i), name(n), alt_name(altn), enabled(e) {}
};

struct MetadataSeries {
	typedef Common::String String;

	String id;
	String name;

	MetadataSeries() : id(nullptr), name(nullptr) {}
	MetadataSeries(const String i, const String n) : id(i), name(n) {}
};

struct MetadataCompany {
	typedef Common::String String;

	String id;
	String name;
	String alt_name;

	MetadataCompany() : id(nullptr), name(nullptr), alt_name(nullptr) {}
	MetadataCompany(const String i, const String n, const String altn)
	: id(i), name(n), alt_name(altn) {}
};

class MetadataParser : public Common::XMLParser {
public:
	MetadataParser();

	~MetadataParser() override;

	Common::HashMap<Common::String, MetadataGame, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _gameInfo;
	Common::HashMap<Common::String, MetadataEngine, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _engineInfo;
	Common::HashMap<Common::String, MetadataSeries, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _seriesInfo;
	Common::HashMap<Common::String, MetadataCompany, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _companyInfo;

protected:

	CUSTOM_XML_PARSER(MetadataParser) {
		XML_KEY(game)
			XML_PROP(id, true)
			XML_PROP(name, true)
			XML_PROP(engine_id, true)
			XML_PROP(company_id, true)
			XML_PROP(moby_id, true)
			XML_PROP(datafiles, true)
			XML_PROP(series_id, true)

		KEY_END() // game end

		XML_KEY(engine)
			XML_PROP(id, true)
			XML_PROP(name, true)
			XML_PROP(alt_name, true)
			XML_PROP(enabled, true)

		KEY_END() // engine end

		XML_KEY(series)
			XML_PROP(id, true)
			XML_PROP(name, true)

		KEY_END() // series end

		XML_KEY(company)
			XML_PROP(id, true)
			XML_PROP(name, true)
			XML_PROP(alt_name, true)

		KEY_END() // company end

	} PARSER_END()

	/** Render info callbacks */
	bool parserCallback_game(ParserNode *node);
	bool parserCallback_engine(ParserNode *node);
	bool parserCallback_series(ParserNode *node);
	bool parserCallback_company(ParserNode *node);

	bool closedKeyCallback(ParserNode *node) override;

	void cleanup() override;

};

} // End of namespace GUI

#endif
