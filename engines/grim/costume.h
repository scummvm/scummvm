/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_COSTUME_H
#define GRIM_COSTUME_H

#include "common/memstream.h"

#include "math/matrix4.h"

#include "engines/grim/object.h"

namespace Grim {

typedef uint32 tag32;

class CMap;
class Model;
class ModelNode;
class TextSplitter;
class ModelComponent;
class Component;
class Chore;
class Head;

class Costume : public Object {
public:
	Costume(const Common::String &filename, Costume *prevCost);

	virtual ~Costume();
	virtual void load(Common::SeekableReadStream *data);

	const Common::String &getFilename() const { return _fname; }
	void playChore(const char *name);
	void playChore(int num);
	void playChoreLooping(int num);
	void setChoreLastFrame(int num);
	void setChoreLooping(int num, bool val);
	void stopChore(int num);
	void fadeChoreIn(int chore, int msecs);
	void fadeChoreOut(int chore, int msecs);
	ModelNode *getModelNodes();
	Model *getModel();
	void setColormap(const Common::String &map);
	void stopChores();
	int isChoring(const char *name, bool excludeLooping);
	int isChoring(int num, bool excludeLooping);
	int isChoring(bool excludeLooping);
	int getNumChores() const { return _numChores; }
	Chore *getChore(const char *name);
	Chore *getChore(int i) { return _chores[i]; }
	int getChoreId(const char *name);

	void setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);
	void setLookAtRate(float rate);
	float getLookAtRate() const;
	void moveHead(bool entering, const Math::Vector3d &lookAt);

	CMap *getCMap() { return _cmap; }

	virtual int update(uint frameTime);
	void animate();
	void setupTextures();
	virtual void draw();
	void getBoundingBox(int *x1, int *y1, int *x2, int *y2);
	void setPosRotate(const Math::Vector3d &pos, const Math::Angle &pitch,
					  const Math::Angle &yaw, const Math::Angle &roll);
	Math::Matrix4 getMatrix() const;

	Costume *getPreviousCostume() const;

	virtual void saveState(SaveGame *state) const;
	virtual bool restoreState(SaveGame *state);

	Component *getComponent(int num) { return _components[num]; }
protected:
	virtual Component *loadComponent(tag32 tag, Component *parent, int parentID, const char *name, Component *prevComponent);

	void load(TextSplitter &ts, Costume *prevCost);

	ModelComponent *getMainModelComponent() const;

	Common::String _fname;
	Costume *_prevCostume;

	int _numComponents;
	Component **_components;

	Head *_head;

	ObjectPtr<CMap> _cmap;
	int _numChores;
	Chore **_chores;
	Common::List<Chore*> _playingChores;
	Math::Matrix4 _matrix;

	float _lookAtRate;

	friend class Chore;
};

} // end of namespace Grim

#endif
