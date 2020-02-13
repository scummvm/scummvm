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

#ifndef GLK_QUEST_GEAS_STATE
#define GLK_QUEST_GEAS_STATE

#include "glk/quest/string.h"
#include "common/array.h"
#include "common/stream.h"
#include "common/serializer.h"

namespace Glk {
namespace Quest {

struct GeasFile;
struct GeasState;
class Serializer;
class GeasInterface;

struct PropertyRecord {
	String name, data;

	PropertyRecord() {}
	PropertyRecord(const String &in_name, const String &in_data) : name(in_name), data(in_data) {}
};

struct ObjectRecord {
	String name, parent;
	bool hidden, invisible;

	//ObjectRecord (String in_name, String in_parent) : name (in_name), parent (in_parent), hidden (false), concealed (false) {}

};

struct ExitRecord {
	String src, dest;

	ExitRecord() {}
	ExitRecord(const String &in_src, const String &in_dest) : src(in_src), dest(in_dest) {}
};

struct TimerRecord {
	String name;
	bool is_running;
	uint interval, timeleft;
};

struct SVarRecord {
private:
	Common::Array<String> data;
public:
	String name;

	SVarRecord() {}
	SVarRecord(String in_name) : name(in_name) {
		set(0, "");
	}
	uint size() const {
		return data.size();
	}
	uint max() const {
		return size() - 1;
	}
	void set(uint i, String val) {
		if (i >= size()) data.resize(i + 1);
		data[i] = val;
	}
	String get(uint i) const {
		if (i < size()) return data[i];
		return "!";
	}
	void set(String val) {
		data[0] = val;
	}
	String get() const {
		return data[0];
	}

	void sync(Serializer &s);
};

struct IVarRecord {
private:
	Common::Array<int> data;
public:
	String name;

	IVarRecord() {}
	IVarRecord(String in_name) : name(in_name) {
		set(0, 0);
	}
	uint size() const {
		return data.size();
	}
	uint max() const {
		return size() - 1;
	}
	void set(uint i, int val) {
		if (i >= size()) data.resize(i + 1);
		data[i] = val;
	}
	int get(uint i) const {
		if (i < size()) return data[i];
		else return -32767;
	}
	void set(int val) {
		data[0] = val;
	}
	int get() const {
		return data[0];
	}

	void sync(Serializer &s);
};

class Serializer : public Common::Serializer {
public:
	Serializer(Common::SeekableReadStream *in, Common::WriteStream *out) : Common::Serializer(in, out) {}

	void sync(bool &b);
	void sync(String &s);
	void sync(PropertyRecord &pr);
	void sync(ObjectRecord &pr);
	void sync(ExitRecord &er);
	void sync(TimerRecord &tr);
	void sync(SVarRecord &svr);
	void sync(IVarRecord &ivr);
	void sync(GeasState &gs);

	template <class T> void sync(Common::Array<T> &v) {
		uint count = v.size();
		syncAsUint32LE(count);
		if (isLoading())
			v.resize(count);

		for (uint idx = 0; idx < count; ++idx)
			sync(v[idx]);
	}
};

struct GeasState {
	//private:
	//std::auto_ptr<GeasFile> gf;

public:
	bool running;
	String location;
	Common::Array<PropertyRecord> props;
	Common::Array<ObjectRecord> objs;
	Common::Array<ExitRecord> exits;
	Common::Array<TimerRecord> timers;
	Common::Array<SVarRecord> svars;
	Common::Array<IVarRecord> ivars;
	//std::map <String, String> obj_types;

	//void register_block (String blockname, String blocktype);

	GeasState() : running(false) {}
	GeasState(GeasInterface &, const GeasFile &);

	/**
	 * Save the state
	 */
	void load(Common::SeekableReadStream *rs);

	/**
	 * Save the state
	 */
	void save(Common::WriteStream *ws);
};

Common::WriteStream &operator<<(Common::WriteStream &o, const StringMap &m);
Common::WriteStream &operator<<(Common::WriteStream &o, const PropertyRecord &pr);
Common::WriteStream &operator<<(Common::WriteStream &o, const ObjectRecord &objr);
Common::WriteStream &operator<<(Common::WriteStream &o, const ExitRecord er);
Common::WriteStream &operator<<(Common::WriteStream &o, const TimerRecord &tr);
Common::WriteStream &operator<<(Common::WriteStream &o, const SVarRecord &sr);
Common::WriteStream &operator<<(Common::WriteStream &o, const IVarRecord &ir);
Common::WriteStream &operator<<(Common::WriteStream &o, const GeasState &gs);

} // End of namespace Quest
} // End of namespace Glk

#endif
