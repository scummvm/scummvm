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

#include "engines/wintermute/ad/ad_response_context.h"
#include "engines/wintermute/base/base_persistence_manager.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdResponseContext, false)

//////////////////////////////////////////////////////////////////////////
AdResponseContext::AdResponseContext(BaseGame *inGame) : BaseClass(inGame) {
	_id = 0;
	_context = nullptr;
}


//////////////////////////////////////////////////////////////////////////
AdResponseContext::~AdResponseContext() {
	delete[] _context;
	_context = nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseContext::persist(BasePersistenceManager *persistMgr) {
	persistMgr->transferPtr(TMEMBER_PTR(_gameRef));
	persistMgr->transferCharPtr(TMEMBER(_context));
	persistMgr->transferSint32(TMEMBER(_id));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void AdResponseContext::setContext(const char *context) {
	delete[] _context;
	_context = nullptr;
	if (context) {
		size_t contextSize = strlen(context) + 1;
		_context = new char [contextSize];
		Common::strcpy_s(_context, contextSize, context);
	}
}

} // End of namespace Wintermute
