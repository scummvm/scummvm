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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/games/treasure_loader.h"

#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/misc/util.h"

namespace Ultima {
namespace Ultima8 {

TreasureLoader::TreasureLoader() {
}

TreasureLoader::~TreasureLoader() {
}

void TreasureLoader::loadDefaults() {
	ConfigFileManager *config = ConfigFileManager::get_instance();
	KeyMap lootkeyvals;

	// load default treasure types
	lootkeyvals = config->listKeyValues("game/treasure");
	KeyMap::const_iterator defaultiter;
	for (defaultiter = lootkeyvals.begin();
	        defaultiter != lootkeyvals.end(); ++defaultiter) {
		TreasureInfo ti;
		bool ok = internalParse(defaultiter->_value, ti, true);
		if (ok) {
			_defaultTreasure[defaultiter->_key] = ti;
		} else {
			perr << "Failed to parse treasure type '" << defaultiter->_key
			     << "': " << defaultiter->_value << Std::endl;
		}
	}

}

bool TreasureLoader::parse(const Std::string &desc,
                           Std::vector<TreasureInfo> &treasure) {
	treasure.clear();

	Std::vector<Std::string> tr;
	SplitString(desc, ';', tr);

	TreasureInfo ti;
	for (unsigned int i = 0; i < tr.size(); ++i) {
//		pout << "parse: item=" << tr[i] << Std::endl;
		if (internalParse(tr[i], ti, false)) {
			treasure.push_back(ti);
		} else {
			return false;
		}
	}

	return true;
}

bool TreasureLoader::internalParse(const Std::string &desc, TreasureInfo &ti,
                                   bool loadingDefault) {
	ti._special = "";
	ti._chance = 1;
	ti._map = 0;
	ti._shapes.clear();
	ti._frames.clear();
	ti._minCount = ti._maxCount = 1;

	bool loadedDefault = false;

	Std::vector<Std::pair<Std::string, Std::string> > kv;
	SplitStringKV(desc, ' ', kv);

	for (unsigned int i = 0; i < kv.size(); ++i) {
		Std::string key = kv[i].first;
		Std::string val = kv[i].second;
//		pout << "internalParse: key=" << key << " val=" << val << Std::endl;

		if (key == "shape") {
			if (!parseUInt32Vector(val, ti._shapes))
				return false;
		} else if (key == "frame") {
			if (!parseUInt32Vector(val, ti._frames))
				return false;
		} else if (key == "count") {
			if (!parseUIntRange(val, ti._minCount, ti._maxCount)) {
				int x;
				if (!parseInt(val, x))
					return false;
				ti._minCount = ti._maxCount = x;
			}
		} else if (key == "chance") {
			if (!parseDouble(val, ti._chance))
				return false;
		} else if (key == "map") {
			if (val.size() > 1 && val[0] == '!')
				val[0] = '-'; // HACK: invert map for 'not this map'
			if (!parseInt(val, ti._map))
				return false;
		} else if (key == "special" && loadingDefault) {
			ti._special = val;
		} else if (key == "type" && !loadingDefault) {
			if (loadedDefault)
				return false;
			TreasureMap::const_iterator iter;
			iter = _defaultTreasure.find(val);
			if (iter != _defaultTreasure.end())
				ti = iter->_value;
			else
				return false;
			loadedDefault = true;
		} else if (key == "mult" && !loadingDefault) {
			if (!loadedDefault) return false;
			unsigned int minmult, maxmult;
			if (!parseUIntRange(val, minmult, maxmult)) {
				int x;
				if (!parseInt(val, x))
					return false;
				minmult = maxmult = x;
			}
			ti._minCount *= minmult;
			ti._maxCount *= maxmult;
		} else {
			return false;
		}
	}

	return true;
}

bool TreasureLoader::parseUInt32Vector(const Std::string &val_,
                                       Std::vector<uint32> &vec) {
	Std::string val = val_;
	vec.clear();

	Std::string::size_type pos;
	while (!val.empty()) {
		pos = val.find(',');
		Std::string item = val.substr(0, pos);

		Std::string::size_type itempos = val.find('-');
		if (itempos != Std::string::npos) {
			unsigned int min, max;
			if (!parseUIntRange(item, min, max))
				return false;
			for (unsigned int i = min; i <= max; ++i)
				vec.push_back(i);
		} else {
			int x;
			if (!parseInt(item, x))
				return false;
			vec.push_back(x);
		}

		if (pos != Std::string::npos) pos++;
		val.erase(0, pos);
	}

	return true;
}

bool TreasureLoader::parseUIntRange(const Std::string &val,
                                    unsigned int &min, unsigned int &max) {
	Std::string::size_type pos = val.find('-');
	if (pos == 0 || pos == Std::string::npos || pos + 1 >= val.size())
		return false;
	int t1, t2;
	bool ok = true;
	ok &= parseInt(val.substr(0, pos), t1);
	ok &= parseInt(val.substr(pos + 1), t2);
	if (ok) {
		min = t1;
		max = t2;
	}
	return ok;
}

bool TreasureLoader::parseDouble(const Std::string &val, double &d) {
	// TODO: error checking
	d = Std::atof(val.c_str());
	return true;
}

bool TreasureLoader::parseInt(const Std::string &val, int &i) {
	// TODO: error checking
	i = Std::strtol(val.c_str(), 0, 0);
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
