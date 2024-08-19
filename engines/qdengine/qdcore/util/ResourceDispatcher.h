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

#ifndef QDENGINE_QDCORE_UTIL_RESOURCE_DISPATCHER_H
#define QDENGINE_QDCORE_UTIL_RESOURCE_DISPATCHER_H

#include "qdengine/qdcore/util/Handle.h"
#include "qdengine/qdcore/util/SynchroTimer.h"


namespace QDEngine {

/////////////////////////////////////////////////////////////////////////////////////////
//	General Time Resourcing
/////////////////////////////////////////////////////////////////////////////////////////
class ResourceUser {
	int ID;
	static int IDs;
public:
	ResourceUser(time_type period) {
		dtime = period;
		time = 0;
		ID = ++IDs;
	}
	virtual ~ResourceUser() {}
	virtual int quant() {
		return 1;
	}

protected:
	time_type time;
	time_type dtime;

	virtual void init_time(time_type time_) {
		time = time_ + time_step();
	}
	virtual time_type time_step() {
		return dtime;
	}

	friend class ResourceDispatcher;
};

class VoidFunctionCallResourceUser : public ResourceUser {
public:
	typedef void (*type)();

	VoidFunctionCallResourceUser(type func_, time_type dtime) :
		ResourceUser(dtime), func(func_) {}
	virtual ~VoidFunctionCallResourceUser() {}
	int quant() {
		func();
		return 1;
	}

protected:
	type func;
};

template<class T>
class MemberFunctionCallResourceUser : public ResourceUser {
public:
	typedef void (T::*type)();

	MemberFunctionCallResourceUser(T &object_, type func_, time_type dtime) :
		ResourceUser(dtime), object(object_), func(func_) {}
	virtual ~MemberFunctionCallResourceUser() {}
	int quant() {
		(object.*func)();
		return 1;
	}

protected:
	T &object;
	type func;
};


/////////////////////////////////////////////////////////////////////////////////////////
//		Resource Dispatcher
/////////////////////////////////////////////////////////////////////////////////////////
class ResourceDispatcher {
public:
	ResourceDispatcher() : _start_log(false) { }
	void setTimer(int syncro_by_clock, time_type time_per_frame, time_type max_time_interval_) {
		_syncro_timer.set(syncro_by_clock, time_per_frame, _max_time_interval = max_time_interval_);
	}

	void attach(ResourceUser *user) {
		PtrHandle<ResourceUser> p(0);
		users.push_back(p);
		users.back() = user;
		user->init_time(_syncro_timer());
	}
	void attach(void (*func)(), time_type dtime) {
		attach(new VoidFunctionCallResourceUser(func, dtime));
	}
	template<class T>
	void attach(T &obj, void (T::*func)(), time_type dtime) {
		attach(new MemberFunctionCallResourceUser<T>(obj, func, dtime));
	}
	void detach(ResourceUser *user) {
		PtrHandle<ResourceUser> p(user);
		users.remove(p);
		p.set(0);
	}

	void start() {
		_start_log = true;
	}
	void reset();
	void skip_time() {
		_syncro_timer.skip();
	}
	void quant();
	void set_speed(float speed) {
		_syncro_timer.setSpeed(speed);
	}

private:

	typedef Std::list<PtrHandle<ResourceUser> > UserList;
	UserList users;

	SyncroTimer _syncro_timer;
	time_type _max_time_interval;
	bool _start_log;

	void do_start();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_UTIL_RESOURCE_DISPATCHER_H
