
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

#ifndef TWP_SCENEGRAPH_H
#define TWP_SCENEGRAPH_H

#include "math/vector2d.h"
#include "math/matrix4.h"
#include "common/array.h"
#include "common/str.h"
#include "twp/gfx.h"
#include "twp/rectf.h"
#include "twp/room.h"
#include "twp/spritesheet.h"

namespace Twp {

// Represents a node in a scene graph.
class Node {
public:
	Node(const Common::String& name, bool visible = true, Math::Vector2d scale = Math::Vector2d(1, 1), Color color = Color());
	virtual ~Node();

	void setName(const Common::String& name) { _name = name; }
	const Common::String& getName() const { return _name; }

	void setVisible(bool visible) { _visible = visible; }
	bool isVisible() const { return _visible; }

	// Adds new child in current node.
	//
	// Arguments:
	// - `child`: child node to add.
	void addChild(Node *child);
	void removeChild(Node* node);
	void clear();
	// Removes this node from its parent.
	void remove();
	const Common::Array<Node *>& getChildren() const { return _children; }

	Node* getParent() const { return _parent; }
	const Node* getRoot() const;
	// Finds a node in its children and returns its position.
	int find(Node* other);

	// Gets the local position for this node (relative to its parent)
	void setPos(const Math::Vector2d& pos) { _pos = pos; }
	Math::Vector2d getPos() const { return _pos; }
	Math::Vector2d getOffset() const { return _offset; }
	// Gets the absolute position for this node.
	Math::Vector2d getAbsPos() const;

	// Gets the full transformation for this node.
	virtual Math::Matrix4 getTrsf(Math::Matrix4 parentTrsf);

	void setColor(Color color);
	Color getColor() const { return _color; }
	Color getComputedColor() const { return _computedColor; }

	void setAlpha(float alpha);
	Color getAlpha() const { return _color.rgba.a; }

	void setZSort(int zsort) { _zOrder = zsort; }
	virtual int getZSort() const { return _zOrder; }
	virtual Math::Vector2d getScale() const { return _scale; }

	void setAnchor(Math::Vector2d anchor);
	void setSize(Math::Vector2d size);
	virtual Rectf getRect() const;

	void draw(Math::Matrix4 parent = Math::Matrix4());

protected:
	virtual void onColorUpdated(Color c) {}
	virtual void drawCore(Math::Matrix4 trsf) {}

private:
	// Gets the location transformation = translation * rotation * scale.
	Math::Matrix4 getLocalTrsf();
	void updateColor();
	void updateColor(Color parentColor);
	void updateAlpha();
	void updateAlpha(float parentAlpha);

protected:
	Common::String _name;
	Math::Vector2d _pos;
	float _zOrder = 0.f;
	Node *_parent = nullptr;
	Common::Array<Node *> _children;
	Math::Vector2d _offset, _shakeOffset, _renderOffset, _anchor, _anchorNorm, _scale, _size;
	Color _color, _computedColor;
	bool _visible = false;
	float _rotation = 0.f, _rotationOffset = 0.f;
};

class OverlayNode final: public Node {
public:
	OverlayNode();
	virtual ~OverlayNode();

protected:
	void drawCore(Math::Matrix4 trsf) override final;

private:
	Color _ovlColor;
};

class ParallaxNode final: public Node {
public:
	ParallaxNode(const Math::Vector2d& parallax, const Common::String& sheet, const Common::StringArray& frames);
	virtual ~ParallaxNode();

	Math::Matrix4 getTrsf(Math::Matrix4 parentTrsf) override final;

protected:
	void drawCore(Math::Matrix4 trsf) override final;

private:
	Math::Vector2d _parallax;
	Common::String _sheet;
	Common::StringArray _frames;
};

struct ObjectAnimation;

class Anim : public Node {
public:
	Anim(Object* obj);

	void clearFrames();
	void setAnim(const ObjectAnimation* anim, float fps = 0.f, bool loop = false, bool instant = false);
	void update(float elapsed);
	void disable() { _disabled = true; }
	void trigSound();

private:
	virtual void drawCore(Math::Matrix4 trsf) override final;

private:
	Common::String _sheet;
    const ObjectAnimation* _anim;
    bool _disabled;
    Common::Array<Common::String> _frames;
    int _frameIndex;
    float _elapsed;
    float _frameDuration;
    bool _loop, _instant;
    Object* _obj;
};

class Scene: public Node {
public:
	Scene();
	virtual ~Scene() final;
};

} // End of namespace Twp

#endif
