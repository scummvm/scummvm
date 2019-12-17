/*
Copyright (C) 2005 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ULTIMA8_GAMES_STARTU8PROCESS_H
#define ULTIMA8_GAMES_STARTU8PROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class Item;

class StartU8Process : public Process {
protected:
	bool _init;
	bool _skipStart;
	std::string _saveName;

	virtual void saveData(ODataSource *ods);
public:
	StartU8Process(const std::string &saveName);

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	virtual void run();

	bool loadData(IDataSource *ids, uint32 version);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
