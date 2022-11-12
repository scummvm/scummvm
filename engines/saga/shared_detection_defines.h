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

// Default scenes
#define ITE_DEFAULT_SCENE 32
#define IHNM_DEFAULT_SCENE 151
#define ITEDEMO_DEFAULT_SCENE 68
#define IHNMDEMO_DEFAULT_SCENE 144

namespace Saga {
typedef int (SceneProc) (int, void *);

class SceneHandlers {
public:
	static int SC_ITEIntroAnimProc(int param, void *refCon);
	static int SC_ITEIntroCave1Proc(int param, void *refCon);
	static int SC_ITEIntroCave2Proc(int param, void *refCon);
	static int SC_ITEIntroCave3Proc(int param, void *refCon);
	static int SC_ITEIntroCave4Proc(int param, void *refCon);
	static int SC_ITEIntroValleyProc(int param, void *refCon);
	static int SC_ITEIntroTreeHouseProc(int param, void *refCon);
	static int SC_ITEIntroFairePathProc(int param, void *refCon);
	static int SC_ITEIntroFaireTentProc(int param, void *refCon);
	static int SC_ITEIntroCaveDemoProc(int param, void *refCon);
};

enum SceneTransitionType {
	kTransitionNoFade,
	kTransitionFade
};

enum SceneLoadFlags {
	kLoadByResourceId,
	kLoadBySceneNumber
};

struct LoadSceneParams {
	int32 sceneDescriptor;
	SceneLoadFlags loadFlag;
	SceneProc *sceneProc;
	bool sceneSkipTarget;
	SceneTransitionType transitionType;
	int actorsEntrance;
	int chapter;
};

#define NO_CHAPTER_CHANGE -2
}
