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

#ifndef NGI_BEHAVIOR_H
#define NGI_BEHAVIOR_H

#include "ngi/utils.h"

namespace NGI {

class AniObject;
class GameVar;
class MessageQueue;
class Scene;

struct BehaviorMove {
	MessageQueue *_messageQueue = nullptr;
	int _delay = 0;
	uint32 _percent = 0;
	int _flags = 0;
	int16 _movementId = 0;
	int16 _queueId = 0;
	bool _mirror = false;

	BehaviorMove() = default;
	BehaviorMove(GameVar *subvar, Scene *sc, int *delay);
};

struct BehaviorAnim {
	int _staticsId;
	int _flags;
	Common::Array<BehaviorMove> _behaviorMoves;

	BehaviorAnim();
	BehaviorAnim(GameVar *var, Scene *sc, AniObject *ani, int *minDelay);
};

struct BehaviorAni {
	AniObject *_ani;
	int _staticsId;
	int _counter;
	int _counterMax;
	int _flags;
	int _subIndex;
	int _animsCount;
	Common::Array<BehaviorAnim> _behaviorAnims;
	int16 _objectId = 0;
	int16 _sourceSceneId = 0;
	uint32 _elapsed = 0;
	const BehaviorMove *_activeMove = nullptr;

	BehaviorAni() { clear(); }

	void clear();
	BehaviorMove &addMove(int16 staticsId);
	bool updateIdle(uint32 elapsed);
	void initAmbientBehavior(GameVar *var, Scene *sc);
	void initObjectBehavior(GameVar *var, Scene *sc, AniObject *ani);
};

// A scene can schedule the same shared sound with different delays and chances.
struct BehaviorSound {
	uint16 _soundId;
	uint32 _delay;
	uint32 _percent;
	uint32 _elapsed;
	bool _loop;
	bool _enabled;

	bool update(uint32 elapsed);
};

class BehaviorController : public CObject {
	Common::Array<BehaviorAni> _behaviors;
	Common::Array<BehaviorSound> _sounds;
	Common::Array<BehaviorSound> _music;
	Scene *_scene;
	bool _isActive;
	void updateSounds(uint32 elapsed);

  public:
	BehaviorController();
	~BehaviorController() override;

	void clear();

	void initBehavior(Scene *scene, GameVar *var);

	void update();
	void addSound(const BehaviorSound &sound, bool music);
	void enableAllMusic();
	BehaviorAni &addAni(int16 objectId, int16 sceneId, int flags);
	void onShow(Scene &scene);
	bool update(uint32 elapsed);
	void updateBehavior(BehaviorAni &behaviorInfo, BehaviorAnim &entry);
	void updateStaticAniBehavior(AniObject &ani, int delay, const BehaviorAnim &beh);

	bool enableAni(AniObject *obj, int aniId, int quId, int flag);

	void enableAllQueuesForAni(AniObject *ani, int flag);

	BehaviorMove *getAniQueue(AniObject *ani, int id1, int id2);
};

} // End of namespace NGI

#endif /* NGI_BEHAVIOR_H */
