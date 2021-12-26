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

#ifndef AGS_ENGINE_PLATFORM_THREAD_STD_H
#define AGS_ENGINE_PLATFORM_THREAD_STD_H

//include <system_error>
//include <thread>

namespace AGS3 {
namespace AGS {
namespace Engine {

class StdThread : public BaseThread {
public:
	StdThread() : thread_(), entry_(nullptr), looping_(false) {
	}

	~StdThread() override {
		Stop();
	}

	StdThread &operator=(const StdThread &) = delete;
	StdThread(const StdThread &) = delete;

	bool Create(AGSThreadEntry entryPoint, bool looping) override {
		if (!entryPoint) {
			return false;
		}

		entry_ = entryPoint;
		looping_ = looping;
		return true;
	}

	bool Start() override {
		if (thread_.joinable()) {
			return true;
		}
		if (!entry_) {
			return false;
		}

		//try {
		thread_ = std::thread(thread_start_, this);
		/*} catch (std::system_error) {
		    return false;
		}*/
		return thread_.joinable();
	}

	bool Stop() override {
		if (!thread_.joinable()) {
			return true;
		}

		looping_ = false; // signal thread to stop
		thread_.join();
		return true;
	}

private:
	std::thread thread_;
	AGSThreadEntry entry_;
	bool looping_;

	static void thread_start_(StdThread *self) {
		auto entry = self->entry_;
		for (;;) {
			entry();
			if (!self->looping_) {
				break;
			}
			std::this_thread::yield();
		}
	}
};

typedef StdThread Thread;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
