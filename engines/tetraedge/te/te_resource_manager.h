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

	template<class T> TeIntrusivePtr<T> getResourceNoSearch(const Common::Path &path) {
		for (TeIntrusivePtr<TeResource> &resource : this->_resources) {
			if (resource->getAccessName() == path) {
				return TeIntrusivePtr<T>(dynamic_cast<T *>(resource.get()));
			}
		}
		TeIntrusivePtr<T> retval = new T();
		addResource(retval.get());
		return retval;
	}

	template<class T> TeIntrusivePtr<T> getResource(Common::Path &path) {
		for (TeIntrusivePtr<TeResource> &resource : this->_resources) {
			if (resource->getAccessName() == path) {
				return TeIntrusivePtr<T>(dynamic_cast<T *>(resource.get()));
			}
		}

		TeIntrusivePtr<T> retval;
		// Note: original search logic here abstracted away in our version..
		TeCore *core = g_engine->getCore();
		path = core->findFile(path);
		retval = new T();

		if (retval.get()) {
			retval->load(path);
			addResource(retval.get());
		}
		return retval;
	}

	template<class T> TeIntrusivePtr<T> getResourceOrMakeInstance(Common::Path &path) {
		for (TeIntrusivePtr<TeResource> &resource : this->_resources) {
			if (resource->getAccessName() == path) {
				return TeIntrusivePtr<T>(dynamic_cast<T *>(resource.get()));
			}
		}

		TeIntrusivePtr<T> retval;
		// Note: original search logic here abstracted away in our version..
		TeCore *core = g_engine->getCore();
		path = core->findFile(path);
		retval = T::makeInstance();

		if (retval.get()) {
			retval->load(path);
			addResource(retval.get());
		}
		return retval;
	}

private:
	Common::Array<TeIntrusivePtr<TeResource>> _resources;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RESOURCE_MANAGER_H
