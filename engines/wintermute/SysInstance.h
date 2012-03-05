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
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SYSINSTANCE_H
#define WINTERMUTE_SYSINSTANCE_H

namespace WinterMute {

class CSysClass;

class CSysInstance {
public:
	CSysInstance(void *Instance, int ID, CSysClass *sysClass);
	virtual ~CSysInstance();

	int GetID() const {
		return m_ID;
	}
	int GetSavedID() const {
		return m_SavedID;
	}
	void *GetInstance() const {
		return m_Instance;
	}
	CSysClass *GetClass() const {
		return m_Class;
	}

	void SetSavedID(int id) {
		m_SavedID = id;
	}

private:
	bool m_Used;
	int m_ID;
	int m_SavedID;
	void *m_Instance;
	CSysClass *m_Class;
};

} // end of namespace WinterMute

#endif
