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

#ifndef MOHAWK_LIVINGBOOKS_PAGE_H
#define MOHAWK_LIVINGBOOKS_PAGE_H

#include "common/array.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBItem;
class LBCode;
class Archive;

class LBPage {
public:
	LBPage(MohawkEngine_LivingBooks *vm);
	~LBPage();

	void open(Archive *mhk, uint16 baseId);
	uint16 getResourceVersion();

	void addClonedItem(LBItem *item);
	void itemDestroyed(LBItem *item);

	LBCode *_code;

protected:
	MohawkEngine_LivingBooks *_vm;

	Archive *_mhk;
	Common::Array<LBItem *> _items;

	uint16 _baseId;
	bool _cascade;

	void loadBITL(uint16 resourceId);
};

} // End of namespace Mohawk

#endif
