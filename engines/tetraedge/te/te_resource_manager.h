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

#ifndef TETRAEDGE_TE_TE_RESOURCE_MANAGER_H
#define TETRAEDGE_TE_TE_RESOURCE_MANAGER_H

#include "common/array.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_intrusive_ptr.h"

namespace Tetraedge {

class TeResource;

class TeResourceManager {
public:
	TeResourceManager();

	~TeResourceManager();

	void addResource(const TeIntrusivePtr<TeResource> &resource);
	void addResource(TeResource *resource);
	bool exists(const Common::Path &path);
	void removeResource(const TeIntrusivePtr<TeResource> &resource);
	void removeResource(const TeResource *resource);

	template<class T> TeIntrusivePtr<T> getResourceByName(const Common::Path &path) {
		for (TeIntrusivePtr<TeResource> &resource : this->_resources) {
			if (resource->getAccessName() == path) {
				return TeIntrusivePtr<T>(dynamic_cast<T *>(resource.get()));
			}
		}
		debug("getResourceByName: didn't find resource %s", path.toString(Common::Path::kNativeSeparator).c_str());
		return TeIntrusivePtr<T>();
	}

	template<class T>
	TeIntrusivePtr<T> getResource(const TetraedgeFSNode &node) {
		Common::Path path = node.getPath();
		for (TeIntrusivePtr<TeResource> &resource : this->_resources) {
			if (resource->getAccessName() == path) {
				return TeIntrusivePtr<T>(dynamic_cast<T *>(resource.get()));
			}
		}

		TeIntrusivePtr<T> retval = new T();

		if (retval.get()) {
			if (!node.isReadable())
				warning("getResource: asked to fetch unreadable resource %s", node.toString().c_str());
			retval->load(node);
			addResource(retval.get());
		}
		return retval;
	}

private:
	Common::Array<TeIntrusivePtr<TeResource>> _resources;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RESOURCE_MANAGER_H
