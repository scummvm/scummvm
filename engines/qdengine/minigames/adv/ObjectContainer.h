/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose _names
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

#ifndef QDENGINE_MINIGAMES_ADV_OBJECT_CONTAINER_H
#define QDENGINE_MINIGAMES_ADV_OBJECT_CONTAINER_H

namespace QDEngine {

class ObjectContainer {
	QDObjects _objects;
	int _current;
	mgVect3f _coord;
#ifdef _DEBUG
	Common::String _name;
#endif
	const char *name() const;
	void pushObject(QDObject& obj);

public:
	ObjectContainer();
	void release(MinigameManager *runtime);

	bool load(const char *_name, MinigameManager *runtime, bool hide = true);
	void hideAll(MinigameManager *runtime);

	const mgVect3f &coord() const {
		return _coord;
	}

	QDObject getObject();
	void releaseObject(QDObject& obj, MinigameManager *runtime);
};

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_OBJECT_CONTAINER_H
