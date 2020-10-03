/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NGI_BEHAVIOR_H
#define NGI_BEHAVIOR_H

namespace NGI {

struct BehaviorMove {
	MessageQueue *_messageQueue;
	int _delay;
	uint32 _percent;
	int _flags;

	BehaviorMove(GameVar *subvar, Scene *sc, int *delay);
};

struct BehaviorAnim {
	int _staticsId;
	int _flags;
	Common::Array<BehaviorMove> _behaviorMoves;

	BehaviorAnim();
	BehaviorAnim(GameVar *var, Scene *sc, StaticANIObject *ani, int *minDelay);
};

struct BehaviorInfo {
	StaticANIObject *_ani;
	int _staticsId;
	int _counter;
	int _counterMax;
	int _flags;
	int _subIndex;
	int _animsCount;
	Common::Array<BehaviorAnim> _behaviorAnims;

	BehaviorInfo() { clear(); }

	void clear();
	void initAmbientBehavior(GameVar *var, Scene *sc);
	void initObjectBehavior(GameVar *var, Scene *sc, StaticANIObject *ani);
};

class BehaviorManager : public CObject {
	Common::Array<BehaviorInfo> _behaviors;
	Scene *_scene;
	bool _isActive;

  public:
	BehaviorManager();
	~BehaviorManager() override;

	void clear();

	void initBehavior(Scene *scene, GameVar *var);

	void updateBehaviors();
	void updateBehavior(BehaviorInfo &behaviorInfo, BehaviorAnim &entry);
	void updateStaticAniBehavior(StaticANIObject &ani, int delay, const BehaviorAnim &beh);

	bool setBehaviorEnabled(StaticANIObject *obj, int aniId, int quId, int flag);

	void setFlagByStaticAniObject(StaticANIObject *ani, int flag);

	BehaviorMove *getBehaviorMoveByMessageQueueDataId(StaticANIObject *ani, int id1, int id2);
};

} // End of namespace NGI

#endif /* NGI_BEHAVIOR_H */
