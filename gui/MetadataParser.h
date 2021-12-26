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

#ifndef GUI_METADATA_PARSER_H
#define GUI_METADATA_PARSER_H

#include "common/xmlparser.h"

namespace GUI {

struct MetadataGame {
	Common::String id;
	Common::String name;
	Common::String engine_id;
	Common::String company_id;
	Common::String moby_id;
	Common::String datafiles;
	Common::String series_id;

	MetadataGame() {}
	MetadataGame(const Common::String i, const Common::String n, const Common::String eid, const Common::String cid, const Common::String mid, const Common::String df, const Common::String sid)
		: id(i), name(n), engine_id(eid), company_id(cid), moby_id(mid), datafiles(df), series_id(sid) {}
};

struct MetadataEngine {
	Common::String id;
	Common::String name;
	Common::String alt_name;
	bool enabled;

	MetadataEngine() : enabled(false) {}
	MetadataEngine(const Common::String i, const Common::String n, const Common::String altn, bool e)
		: id(i), name(n), alt_name(altn), enabled(e) {}
};

struct MetadataSeries {
	Common::String id;
	Common::String name;

	MetadataSeries() {}
	MetadataSeries(const Common::String i, const Common::String n) : id(i), name(n) {}
};

struct MetadataCompany {
	Common::String id;
	Common::String name;
	Common::String alt_name;

	MetadataCompany() {}
	MetadataCompany(const Common::String i, const Common::String n, const Common::String altn)
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
		XML_KEY(games)
			XML_KEY(game)
				XML_PROP(id, true)
				XML_PROP(name, true)
				XML_PROP(engine_id, true)
				XML_PROP(company_id, true)
				XML_PROP(moby_id, true)
				XML_PROP(datafiles, true)
				XML_PROP(wikipedia_page, true)
				XML_PROP(series_id, true)
			KEY_END() // game end
		KEY_END() // games end

		XML_KEY(engines)
			XML_KEY(engine)
				XML_PROP(id, true)
				XML_PROP(name, true)
				XML_PROP(alt_name, true)
				XML_PROP(enabled, true)
			KEY_END() // engine end
		KEY_END() // engines end

		XML_KEY(series)
			XML_KEY(serie)
				XML_PROP(id, true)
				XML_PROP(name, true)
			KEY_END() // serie end
		KEY_END() // series end

		XML_KEY(companies)
			XML_KEY(company)
				XML_PROP(id, true)
				XML_PROP(name, true)
				XML_PROP(alt_name, true)
			KEY_END() // company end
		KEY_END() // companies end
	} PARSER_END()

	/** Render info callbacks */
	bool parserCallback_games(ParserNode *node);
	bool parserCallback_game(ParserNode *node);
	bool parserCallback_engines(ParserNode *node);
	bool parserCallback_engine(ParserNode *node);
	bool parserCallback_series(ParserNode *node);
	bool parserCallback_serie(ParserNode *node);
	bool parserCallback_companies(ParserNode *node);
	bool parserCallback_company(ParserNode *node);

	bool closedKeyCallback(ParserNode *node) override;

	void cleanup() override;
};

} // End of namespace GUI

#endif
