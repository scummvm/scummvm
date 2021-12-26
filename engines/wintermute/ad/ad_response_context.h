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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_ADRESPONSECONTEXT_H
#define WINTERMUTE_ADRESPONSECONTEXT_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/base/base.h"

namespace Wintermute {

class AdResponseContext : public BaseClass {
public:
	void setContext(const char *context);
	const char *getContext() const { return _context; }
	int32 _id;

	DECLARE_PERSISTENT(AdResponseContext, BaseClass)
	AdResponseContext(BaseGame *inGame);
	~AdResponseContext() override;
private:
	char *_context;
};

} // End of namespace Wintermute

#endif
