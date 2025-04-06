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

#include "ultima/ultima8/misc/common_types.h"
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
	lootkeyvals = config->listKeyValues("game", "treasure");
	for (const auto &i : lootkeyvals) {
		TreasureInfo ti;
		bool ok = internalParse(i._value, ti, true);
		if (ok) {
			_defaultTreasure[i._key] = ti;
		} else {
			warning("Failed to parse treasure type '%s': %s", i._key.c_str(), i._value.c_str());
		}
	}
}

bool TreasureLoader::parse(const Std::string &desc,
						   Std::vector<TreasureInfo> &treasure) const {
	treasure.clear();

	Std::vector<Std::string> tr;
	SplitString(desc, ';', tr);

	TreasureInfo ti;
	for (unsigned int i = 0; i < tr.size(); ++i) {
		if (internalParse(tr[i], ti, false)) {
			treasure.push_back(ti);
		} else {
			return false;
		}
	}

	return true;
}

bool TreasureLoader::internalParse(const Std::string &desc, TreasureInfo &ti,
								   bool loadingDefault) const {
	ti.clear();
	bool loadedDefault = false;

	Std::vector<Common::Pair<Std::string, Std::string> > kv;
	SplitStringKV(desc, ' ', kv);

	for (unsigned int i = 0; i < kv.size(); ++i) {
		const Std::string &key = kv[i].first;
		Std::string val = kv[i].second;

		if (key == "shape") {
			if (!parseUInt32Vector(val, ti._shapes)) {
				warning("Failed to parse treasure shape list '%s'", val.c_str());
				return false;
			}
			// validate the shapes are > 0 and < max shape
			for (unsigned int j = 0; j < ti._shapes.size(); j++) {
				if (ti._shapes[j] <= 0 || ti._shapes[j] > 65535) {
					warning("Invalid treasure shape in list '%s'", val.c_str());
					return false;
				}
			}
		} else if (key == "frame") {
			if (!parseUInt32Vector(val, ti._frames)) {
				warning("Failed to parse treasure frame list '%s'", val.c_str());
				return false;
			}
			// validate the frames are < max frame (0 frame is valid)
			for (unsigned int j = 0; j < ti._frames.size(); j++) {
				if (ti._frames[j] > 65535) {
					warning("Invalid treasure frame in list '%s'", val.c_str());
					return false;
				}
			}
		} else if (key == "count") {
			if (!parseUIntRange(val, ti._minCount, ti._maxCount)) {
				int x;
				if (!parseInt(val, x) || x <= 0) {
					warning("Invalid treasure count '%s'", val.c_str());
					return false;
				}
				ti._minCount = ti._maxCount = x;
			}
		} else if (key == "chance") {
			if (!parseDouble(val, ti._chance) || ti._chance <= 0) {
				warning("Invalid treasure chance '%s'", val.c_str());
				return false;
			}
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
			if (!loadedDefault) {
				warning("Need defaults before applying multiplier in treasure data '%s'", val.c_str());
				return false;
			}
			unsigned int minmult, maxmult;
			if (!parseUIntRange(val, minmult, maxmult)) {
				int x;
				if (!parseInt(val, x) || x <= 0) {
					warning("Invalid treasure multiplier '%s'", val.c_str());
					return false;
				}
				minmult = maxmult = x;
			}
			ti._minCount *= minmult;
			ti._maxCount *= maxmult;
		} else {
			warning("Unknown key parsing treasure '%s'", key.c_str());
			return false;
		}
	}

	return true;
}

bool TreasureLoader::parseUInt32Vector(const Std::string &val_,
									   Std::vector<uint32> &vec) const {
	Std::string val = val_;
	vec.clear();

	if (val.empty())
		return false;

	while (!val.empty()) {
		Std::string::size_type pos = val.find(',');
		const Std::string item = val.substr(0, pos);

		Std::string::size_type itempos = val.find('-');
		if (itempos != Std::string::npos) {
			unsigned int min, max;
			if (!parseUIntRange(item, min, max))
				return false;
			for (unsigned int i = min; i <= max; ++i)
				vec.push_back(i);
		} else {
			int x;
			if (!parseInt(item, x) || x < 0)
				return false;
			vec.push_back(x);
		}

		if (pos != Std::string::npos) pos++;
		val.erase(0, pos);
	}

	return true;
}

bool TreasureLoader::parseUIntRange(const Std::string &val,
									unsigned int &min, unsigned int &max) const {
	Std::string::size_type pos = val.find('-');
	if (pos == 0 || pos == Std::string::npos || pos + 1 >= val.size())
		return false;
	int t1 = 0;
	int t2 = 0;
	bool ok = true;
	ok = ok && parseInt(val.substr(0, pos), t1);
	ok = ok && parseInt(val.substr(pos + 1), t2);
	ok = ok && (t1 <= t2 && t1 >= 0 && t2 >= 0);
	if (ok) {
		min = t1;
		max = t2;
	}
	return ok;
}

bool TreasureLoader::parseDouble(const Std::string &val, double &d) const {
	if (val.empty())
		return false;
	// TODO: error checking
	d = atof(val.c_str());
	return true;
}

bool TreasureLoader::parseInt(const Std::string &val, int &i) const {
	if (val.empty())
		return false;
	// TODO: error checking
	i = strtol(val.c_str(), 0, 0);
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
