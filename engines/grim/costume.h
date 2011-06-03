/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
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

#include "engines/grim/object.h"
#include "engines/grim/model.h"

namespace Grim {

#define DEFAULT_COLORMAP "item.cmp"

typedef uint32 tag32;

class TextSplitter;

class Costume : public Object {
public:
	Costume(const Common::String &filename, const char *data, int len, Costume *prevCost);
    Costume() : Object() { _chores = 0; }

	void loadGRIM(TextSplitter &ts, Costume *prevCost);
	void loadEMI(Common::MemoryReadStream &ms, Costume *prevCost);

	virtual ~Costume();

	const Common::String &getFilename() const { return _fname; }
	void playChore(const char *name);
	void playChore(int num);
	void playChoreLooping(int num);
	void setChoreLastFrame(int num) { _chores[num].setLastFrame(); }
	void setChoreLooping(int num, bool val) { _chores[num].setLooping(val); }
	void stopChore(int num);
	void fadeChoreIn(int chore, int msecs);
	void fadeChoreOut(int chore, int msecs);
	Model::HierNode *getModelNodes();
	Model *getModel();
	void setColormap(const Common::String &map);
	void stopChores();
	int isChoring(const char *name, bool excludeLooping);
	int isChoring(int num, bool excludeLooping);
	int isChoring(bool excludeLooping);
	int getNumChores() const { return _numChores; }

	void setHead(int joint1, int joint2, int joint3, float maxRoll, float maxPitch, float maxYaw);
	void moveHead(bool lookingMode, const Graphics::Vector3d &lookAt, float rate);

	void update();
	void animate();
	void setupTextures();
	void draw();
	void setPosRotate(Graphics::Vector3d pos, float pitch, float yaw, float roll);

	Costume *getPreviousCostume() const;

	void saveState(SaveGame *state) const;
	bool restoreState(SaveGame *state);

	class Component {
	public:
		Component(Component *parent, int parentID, tag32 tag);

		tag32 getTag() { return _tag; }
		CMap *getCMap();
		void setColormap(CMap *c);
		void setFade(float fade);
		bool isVisible();
		Component *getParent() { return _parent; }
		virtual void setMatrix(Graphics::Matrix4) { };
		virtual void init() { }
		virtual void setKey(int) { }
		virtual void setMapName(char *) { }
		virtual void update() { }
		virtual void animate() { }
		virtual void setupTexture() { }
		virtual void draw() { }
		virtual void reset() { }
		virtual void resetColormap() { }
		virtual void saveState(SaveGame *) { }
		virtual void restoreState(SaveGame *) { }
		virtual ~Component();

	protected:
		ObjectPtr<CMap> _cmap, _previousCmap;
		tag32 _tag;
		int _parentID;
		bool _visible;
		float _fade;
		Component *_parent, *_child, *_sibling;
		Graphics::Matrix4 _matrix;
		Costume *_cost;
		void setCostume(Costume *cost) { _cost = cost; }
		void setParent(Component *newParent);
		void removeChild(Component *child);

		friend class Costume;
	};

private:
	Component *loadComponent(tag32 tag, Component *parent, int parentID, const char *name, Component *prevComponent);
	Component *loadComponentEMI(Costume::Component *parent, int parentID, const char *name, Costume::Component *prevComponent);

	Common::String _fname;
	Costume *_prevCostume;

	int _numComponents;
	Component **_components;

	struct TrackKey {
		int time, value;
	};

	struct ChoreTrack {
		int compID;
		int numKeys;
		TrackKey *keys;
	};

	struct Head {
		int joint1;
		int joint2;
		int joint3;
		float maxRoll;
		float maxPitch;
		float maxYaw;
	} _head;

	class Chore {
	public:
		enum FadeMode {
			None,
			FadeIn,
			FadeOut
		};

		Chore();
		~Chore();
		void load(int id, Costume *owner, TextSplitter &ts);
		void play();
		void playLooping();
		void setLooping(bool val) { _looping = val; }
		void stop();
		void update();
		void setLastFrame();
		void fade(FadeMode mode, int msecs);

	private:
		Costume *_owner;

		int _id;
		int _length;
		int _numTracks;
		ChoreTrack *_tracks;
		char _name[32];

		bool _hasPlayed, _playing, _looping;
		int _currTime;

		int _fadeLength;
		float _fade;
		FadeMode _fadeMode;

		void setKeys(int startTime, int stopTime);

		friend class Costume;
	};

	ObjectPtr<CMap> _cmap;
	int _numChores;
	Chore *_chores;
	Common::List<Chore*> _playingChores;
	Graphics::Matrix4 _matrix;
	Model::HierNode *_joint1Node;
	Model::HierNode *_joint2Node;
	Model::HierNode *_joint3Node;

	float _headPitch;
	float _headYaw;
};

} // end of namespace Grim

#endif
