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

#include "qdengine/xlibs/util/serialization/Serialization.h"
#include "qdengine/xlibs/util/serialization/Handle.h"


namespace QDEngine {


template<class Type, class BaseType>
bool ShareHandle<Type, BaseType>::serialize(Archive &ar, const char *name, const char *nameAlt) {
	if (ar.isInput() && ptr_) {
		ptr_->decrRef();
		xassert("БАГ: Возможно удаление с созданием висячей ссылки (после Ignore упадет непредсказуемо)!" && ptr_->numRef() == 0);
	}

	bool log;
	if (!ar.inPlace()) {
		Type *ptr = get();
		log = ar.serializePolymorphic(ptr, name, nameAlt);
		ptr_ = ptr;
	} else
		log = ar.serializePolymorphic(reinterpret_cast<Type *&>(ptr_), name, nameAlt);

	if (ar.isInput() && ptr_)
		ptr_->addRef();
	return log;
}
} // namespace QDEngine
