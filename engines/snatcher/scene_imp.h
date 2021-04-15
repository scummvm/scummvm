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

#ifndef SNATCHER_SCENE_IMP_H
#define SNATCHER_SCENE_IMP_H

namespace Snatcher {

#define SH_HEAD_BEGIN(id) \
	class Scene_##id : public SceneHandler { \
	public: \
		Scene_##id(SnatcherEngine *vm, SceneResource *scn, FIO *fio); \
		~Scene_##id() override; \
		void operator()() override; \
	private:

#define SH_HEAD_END(id) }; \
	SceneHandler *createSceneHandler_##id(SnatcherEngine *vm, SceneResource *scn, FIO *fio) { return new Scene_##id(vm, scn, fio); }
#define SH_IMP_CTOR(id) \
	Scene_##id::Scene_##id(SnatcherEngine *vm, SceneResource *scn, FIO *fio) : SceneHandler(vm, scn, fio)

} // End of namespace Snatcher

#endif // SNATCHER_SCENE_IMP_H
