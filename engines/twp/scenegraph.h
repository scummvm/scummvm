
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
#include "twp/font.h"
#include "twp/spritesheet.h"

#define NUMOBJECTS 8

namespace Twp {

// Represents a node in a scene graph.
class Node {
public:
	Node(const Common::String &name, Math::Vector2d scale = Math::Vector2d(1, 1), Color color = Color());
	virtual ~Node();

	void setName(const Common::String &name) { _name = name; }
	const Common::String &getName() const { return _name; }

	void setVisible(bool visible) { _visible = visible; }
	bool isVisible() const { return _visible; }

	// Adds new child in current node.
	//
	// Arguments:
	// - `child`: child node to add.
	void addChild(Node *child);
	void removeChild(Node *node);
	void clear();
	// Removes this node from its parent.
	void remove();
	const Common::Array<Node *> &getChildren() const { return _children; }

	Node *getParent() const { return _parent; }
	const Node *getRoot() const;
	// Finds a node in its children and returns its position.
	int find(Node *other);

	// Gets the local position for this node (relative to its parent)
	void setPos(const Math::Vector2d &pos) { _pos = pos; }
	Math::Vector2d getPos() const { return _pos; }

	void setOffset(const Math::Vector2d &offset) { _offset = offset; }
	Math::Vector2d getOffset() const { return _offset; }

	void setShakeOffset(const Math::Vector2d &offset) { _shakeOffset = offset; }
	Math::Vector2d getShakeOffset() const { return _shakeOffset; }

	void setRenderOffset(const Math::Vector2d &offset) { _renderOffset = offset; }
	Math::Vector2d getRenderOffset() const { return _renderOffset; }

	void setScale(const Math::Vector2d &scale) { _scale = scale; }
	virtual Math::Vector2d getScale() const { return _scale; }

	// Gets the absolute position for this node.
	Math::Vector2d getAbsPos() const;

	// Gets the full transformation for this node.
	virtual Math::Matrix4 getTrsf(Math::Matrix4 parentTrsf);

	void setColor(Color color);
	Color getColor() const { return _color; }
	Color getComputedColor() const { return _computedColor; }

	void setAlpha(float alpha);
	float getAlpha() const { return _color.rgba.a; }

	void setZSort(int zsort) { _zOrder = zsort; }
	virtual int getZSort() const { return _zOrder; }

	void setRotation(float rotation) { _rotation = rotation; }
	virtual float getRotation() const { return _rotation; }

	void setAnchor(Math::Vector2d anchor);
	void setAnchorNorm(Math::Vector2d anchorNorm);
	void setSize(Math::Vector2d size);
	Math::Vector2d getSize() const { return _size; }
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
	int _zOrder = 0;
	Node *_parent = nullptr;
	Common::Array<Node *> _children;
	Math::Vector2d _offset, _shakeOffset, _renderOffset, _anchor, _anchorNorm, _scale, _size;
	Color _color, _computedColor;
	bool _visible = true;
	float _rotation = 0.f;
	float _rotationOffset = 0.f;
};

class ParallaxNode final : public Node {
public:
	ParallaxNode(const Math::Vector2d &parallax, const Common::String &sheet, const Common::StringArray &frames);
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

class Object;
class Anim : public Node {
public:
	Anim(Object *obj);

	void clearFrames();
	void setAnim(const ObjectAnimation *anim, float fps = 0.f, bool loop = false, bool instant = false);
	void update(float elapsed);
	void disable() { _disabled = true; }
	void trigSound();

private:
	virtual void drawCore(Math::Matrix4 trsf) override final;

public:
	const ObjectAnimation *_anim = nullptr;
	bool _disabled = false;

private:
	Common::String _sheet;
	Common::Array<Common::String> _frames;
	int _frameIndex = 0;
	float _elapsed = 0.f;
	float _frameDuration = 0.f;
	bool _loop = false;
	bool _instant;
	Object *_obj = nullptr;
};

class ActorNode final : public Node {
public:
	ActorNode(Object *obj);

	int getZSort() const override final;
	Math::Vector2d getScale() const override final;

private:
	Object *_object = nullptr;
};

class TextNode final : public Node {
public:
	TextNode();
	virtual ~TextNode() final;

	void setText(Text text);
	void updateBounds();
	virtual Rectf getRect() const override final;

private:
	virtual void onColorUpdated(Color color) override final;
	virtual void drawCore(Math::Matrix4 trsf) override final;

private:
	Text _text;
};

class Scene final : public Node {
public:
	Scene();
	virtual ~Scene() final;
};

enum class CursorShape {
    Normal,
    Front,
    Back,
    Left,
    Right,
    Pause
};


enum InputStateFlag {
	II_FLAGS_UI_INPUT_ON = 1,
	II_FLAGS_UI_INPUT_OFF = 2,
	II_FLAGS_UI_VERBS_ON = 4,
	II_FLAGS_UI_VERBS_OFF = 8,
	II_FLAGS_UI_HUDOBJECTS_ON = 0x10,
	II_FLAGS_UI_HUDOBJECTS_OFF = 0x20,
	II_FLAGS_UI_CURSOR_ON = 0x40,
	II_FLAGS_UI_CURSOR_OFF = 0x80
};

class InputState final : public Node {
public:
	InputState();
	virtual ~InputState() final;

	InputStateFlag getState() const;
	void setState(InputStateFlag state);

	void setInputHUD(bool value) { _inputHUD = value; }
	bool getInputHUD() const { return _inputHUD; }

	void setInputActive(bool value) { _inputActive = value; }
	bool getInputActive() const { return _inputActive; }

	void setShowCursor(bool value) { _showCursor = value; }
	bool getShowCursor() const { return _showCursor; }

	void setInputVerbsActive(bool value) { _inputVerbsActive = value; }
	bool getInputVerbsActive() const { return _inputVerbsActive; }

	void setHotspot(bool value) { _hotspot = value; }
	bool getHotspot() const { return _hotspot; }

	void setCursorShape(CursorShape shape);

private:
	virtual void drawCore(Math::Matrix4 trsf) override final;

private:
	bool _inputHUD = false;
    bool _inputActive = false;
    bool _showCursor = false;
    bool _inputVerbsActive = false;
    CursorShape _cursorShape = CursorShape::Normal;
    Common::String _cursorName = "cursor";
    bool _hotspot = false;
};

class OverlayNode final : public Node {
public:
	OverlayNode();

	void setOverlayColor(Color color) { _ovlColor = color; }
	Color getOverlayColor() const { return _ovlColor; }

private:
	virtual void drawCore(Math::Matrix4 trsf) override final;

private:
	Color _ovlColor;
};

class Inventory: public Node {
public:
	Inventory();
	void update(float elapsed, Object* actor = nullptr, Color backColor = Color(0, 0, 0), Color verbNormal = Color(0, 0, 0));

	Object* getObject() const { return _obj; }
	Math::Vector2d getPos(Object* inv) const;

private:
	virtual void drawCore(Math::Matrix4 trsf) override final;
	void drawArrows(Math::Matrix4 trsf);
	void drawBack(Math::Matrix4 trsf);
	void drawItems(Math::Matrix4 trsf);
	void drawSprite(SpriteSheetFrame& sf, Texture* texture, Color color, Math::Matrix4 trsf);

private:
	Object* _actor = nullptr;
    Color _backColor, _verbNormal;
    bool _down = false;
    Object* _obj = nullptr;
	Common::Rect _itemRects[NUMOBJECTS];
	Common::Rect _arrowUpRect;
	Common::Rect _arrowDnRect;
};

class SentenceNode: public Node {
public:
	SentenceNode();
	virtual ~SentenceNode();

	void setText(const Common::String& text);

private:
	void drawCore(Math::Matrix4 trsf) override final;

private:
	Common::String _text;
};

class SpriteNode: public Node {
public:
	SpriteNode();
	virtual ~SpriteNode();

	void setSprite(const Common::String& sheet, const Common::String& frame);

private:
	void drawCore(Math::Matrix4 trsf) override final;

private:
	Common::String _sheet;
	Common::String _frame;
};

class NoOverrideNode: public Node {
public:
	NoOverrideNode();
	virtual ~NoOverrideNode();

	void reset();
	bool update(float elapsed);

private:
	SpriteNode _icon;
	float _elapsed = 0.f;
};

} // End of namespace Twp

#endif
