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
		Scene_##id(SnatcherEngine *vm, SceneModule *scn, FIO *fio); \
		~Scene_##id() override; \
		void operator()(GameState &state) override; \
	private: \
		typedef Common::Functor1Mem<GameState&, void, Scene_##id> FrameProc; \
		typedef void(Scene_##id::*FrameProcTblEntry)(GameState&); \
		Common::Array<FrameProc*> _frameProcs; \
		static const FrameProcTblEntry frameProcTable[];		

#define SH_HEAD_END(id) }; \
	SceneHandler *createSceneHandler_##id(SnatcherEngine *vm, SceneModule *scn, FIO *fio) { return new Scene_##id(vm, scn, fio); }
#define SH_IMP_FRMTBL(id) \
	const Scene_##id::FrameProcTblEntry Scene_##id::frameProcTable[] =
#define SH_FRM(id, no) \
	&Scene_##id::frameUpdate##no
#define SH_IMP_CTOR(id) \
	Scene_##id::Scene_##id(SnatcherEngine *vm, SceneModule *scn, FIO *fio) : SceneHandler(vm, scn, fio)
#define SH_CTOR_MAKEPROCS(id) \
	for (uint i = 0; i < ARRAYSIZE(frameProcTable); ++i) \
		_frameProcs.push_back(new FrameProc(this, frameProcTable[i]));
#define SH_IMP_DTOR(id) \
	Scene_##id::~Scene_##id()
#define SH_DTOR_DELPROCS(id) \
	for (uint i = 0; i < _frameProcs.size(); ++i) \
		delete _frameProcs[i];
#define SH_IMPL_UPDT(id) \
	void Scene_##id::operator()(GameState &state)
#define SH_DCL_FRM(no) \
	void frameUpdate##no(GameState &state);
#define SH_IMPL_FRM(id, no) \
	void Scene_##id::frameUpdate##no(GameState &state)
} // End of namespace Snatcher

#endif // SNATCHER_SCENE_IMP_H
