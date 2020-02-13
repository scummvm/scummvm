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
#ifndef SLUDGE_NEWFATAL_H
#define SLUDGE_NEWFATAL_H

#include "common/str.h"
#include "common/singleton.h"

#include "sludge/errors.h"

namespace Sludge {

class FatalMsgManager : public Common::Singleton<Sludge::FatalMsgManager>{
public:
	FatalMsgManager();
	~FatalMsgManager() override;

	void reset();

	bool hasFatal();
	int fatal(const Common::String &str);
	void setFatalInfo(const Common::String &userFunc, const Common::String &BIF);
	void setResourceForFatal(int n);

private:
	Common::String _fatalMessage;
	Common::String _fatalInfo;

	int _resourceForFatal;
};

inline bool hasFatal() {
	return FatalMsgManager::instance().hasFatal();
}

inline int fatal(const Common::String &str) {
	return FatalMsgManager::instance().fatal(str);
}

inline void setFatalInfo(const Common::String &userFunc, const Common::String &BIF) {
	FatalMsgManager::instance().setFatalInfo(userFunc, BIF);
}

inline void setResourceForFatal(int n) {
	FatalMsgManager::instance().setResourceForFatal(n);
}

int checkNew(const void *mem);
int fatal(const Common::String &str1, const Common::String &str2);

} // End of namespace Sludge

#endif
