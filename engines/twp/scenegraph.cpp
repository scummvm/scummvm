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

#include "math/matrix3.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/lighting.h"
#include "twp/object.h"
#include "twp/resmanager.h"
#include "twp/room.h"
#include "twp/scenegraph.h"

namespace Twp {

#define DEFAULT_FPS 10.f

#define NUMOBJECTSBYROW 4
#define MARGIN 8.f
#define MARGINBOTTOM 10.f
#define BACKOFFSET 7.f
#define ARROWWIDTH 56.f
#define ARROWHEIGHT 86.f
#define BACKWIDTH 137.f
#define BACKHEIGHT 75.f

class ShakeInventory : public Motor {
public:
	ShakeInventory(Math::Vector2d &shakeOffset, float amount) : _shakeOffset(shakeOffset), _amount(amount) {}

protected:
	void onUpdate(float elapsed) override {
		_shakeTime += 40.f * elapsed;
		_elapsed += elapsed;
		_shakeOffset = Math::Vector2d(_amount * cos(_shakeTime + 0.3f), _amount * sin(_shakeTime));
	}

private:
	Math::Vector2d &_shakeOffset;
	const float _amount;
	float _shakeTime = 0.f;
	float _elapsed = 0.f;
};

static float _getFps(float fps, float animFps) {
	if (fps != 0.f)
		return fps;
	return (animFps < 1e-3) ? DEFAULT_FPS : animFps;
}

Node::Node(const Common::String &name, const Math::Vector2d &scale, const Color &color)
	: _name(name),
	  _color(color),
	  _computedColor(color),
	  _scale(scale) {
}

Node::~Node() {
	remove();
	if (_children.empty())
		return;

	for (size_t i = 0; i < _children.size(); i++) {
		_children[i]->_parent = nullptr;
	}
}

void Node::addChild(Node *child) {
	if (child->_parent == this)
		return;
	if (child->_parent) {
		child->_pos -= getAbsPos();
		child->remove();
	}
	_children.push_back(child);
	child->_parent = this;
	child->updateColor();
	child->updateAlpha();
}

const Node *Node::getRoot() const {
	const Node *result = this;
	while (result->_parent != NULL) {
		result = result->_parent;
	}
	return result;
}

int Node::find(Node *other) {
	for (size_t i = 0; i < _children.size(); i++) {
		if (_children[i] == other) {
			return i;
		}
	}
	return -1;
}

void Node::removeChild(Node *child) {
	int i = find(child);
	if (i != -1) {
		_children.remove_at(i);
	}
	child->_parent = nullptr;
}

void Node::clear() {
	if (_children.empty())
		return;

	Common::Array<Node *> children(_children);
	for (size_t i = 0; i < children.size(); i++) {
		children[i]->remove();
	}
	_children.clear();
}

void Node::remove() {
	if (_parent)
		_parent->removeChild(this);
}

void Node::setColor(const Color &c) {
	_color.rgba.r = c.rgba.r;
	_color.rgba.g = c.rgba.g;
	_color.rgba.b = c.rgba.b;
	_computedColor.rgba.r = c.rgba.r;
	_computedColor.rgba.g = c.rgba.g;
	_computedColor.rgba.b = c.rgba.b;
	updateColor();
}

void Node::setAlpha(float alpha) {
	_color.rgba.a = alpha;
	_computedColor.rgba.a = alpha;
	updateAlpha();
}

void Node::updateColor() {
	Color parentColor = !_parent ? Color(1.f, 1.f, 1.f) : _parent->_computedColor;
	updateColor(parentColor);
}

void Node::updateAlpha() {
	float parentOpacity = !_parent ? 1.f : _parent->_computedColor.rgba.a;
	updateAlpha(parentOpacity);
}

void Node::updateColor(const Color &parentColor) {
	_computedColor.rgba.r = _color.rgba.r * parentColor.rgba.r;
	_computedColor.rgba.g = _color.rgba.g * parentColor.rgba.g;
	_computedColor.rgba.b = _color.rgba.b * parentColor.rgba.b;
	onColorUpdated(_computedColor);
	for (size_t i = 0; i < _children.size(); i++) {
		Node *child = _children[i];
		child->updateColor(_computedColor);
	}
}

void Node::updateAlpha(float parentAlpha) {
	_computedColor.rgba.a = _color.rgba.a * parentAlpha;
	onColorUpdated(_computedColor);
	for (size_t i = 0; i < _children.size(); i++) {
		Node *child = _children[i];
		child->updateAlpha(_computedColor.rgba.a);
	}
}

void Node::setAnchor(const Math::Vector2d &anchor) {
	if (_anchor != anchor) {
		_anchorNorm = anchor / _size;
		_anchor = anchor;
	}
}

void Node::setAnchorNorm(const Math::Vector2d &anchorNorm) {
	if (_anchorNorm != anchorNorm) {
		_anchorNorm = anchorNorm;
		_anchor = _size * _anchorNorm;
	}
}

void Node::setSize(const Math::Vector2d &size) {
	if (_size != size) {
		_size = size;
		_anchor = size * _anchorNorm;
	}
}

// this structure is used to have a stable sort
typedef struct {
	size_t index;
	Node *node;
} NodeSort;

static int cmpNodes(const NodeSort &x, const NodeSort &y) {
	if (y.node->getZSort() == x.node->getZSort()) {
		return x.index < y.index;
	}
	return x.node->getZSort() > y.node->getZSort();
}

void Node::onDrawChildren(const Math::Matrix4 &trsf) {
	// use this "stable sort" until there is something better available
	Common::Array<NodeSort> children;
	for (size_t i = 0; i < _children.size(); i++) {
		children.push_back({i, _children[i]});
	}
	Common::sort(children.begin(), children.end(), cmpNodes);
	_children.clear();
	_children.reserve(children.size());
	for (size_t i = 0; i < children.size(); i++) {
		_children.push_back(children[i].node);
	}
	for (auto &child : _children) {
		child->draw(trsf);
	}
}

void Node::draw(const Math::Matrix4 &parent) {
	if (_visible) {
		Math::Matrix4 trsf = getTrsf(parent);
		Math::Matrix4 myTrsf(trsf);
		myTrsf.translate(Math::Vector3d(-_anchor.getX(), _anchor.getY(), 0.f));
		drawCore(myTrsf);
		onDrawChildren(trsf);
	}
}

Math::Vector2d Node::getAbsPos() const {
	return !_parent ? _pos : _parent->getAbsPos() + _pos + _offset;
}

Math::Matrix4 Node::getTrsf(const Math::Matrix4 &parentTrsf) {
	return parentTrsf * getLocalTrsf();
}

Math::Matrix4 Node::getLocalTrsf() {
	Math::Vector2d p = _pos + _offset + _shakeOffset;
	Math::Matrix4 m1;
	m1.translate(Math::Vector3d(p.getX(), p.getY(), 0.f));
	Math::Matrix3 mRot;
	mRot.buildAroundZ(Math::Angle(-_rotation + _rotationOffset));
	Math::Matrix4 m2;
	m2.setRotation(mRot);
	scale(m2, getScale());
	Math::Matrix4 m3;
	m3.translate(Math::Vector3d(_renderOffset.getX(), _renderOffset.getY(), 0.f));
	return m1 * m2 * m3;
}

Rectf Node::getRect() const {
	Math::Vector2d size = _size * getScale();
	return Rectf::fromPosAndSize(getAbsPos(), Math::Vector2d(-size.getX(), size.getY()) * _anchorNorm * _size);
}

ParallaxNode::ParallaxNode(const Math::Vector2d &parallax, const Common::String &sheet, const Common::StringArray &frames)
	: Node("parallax"),
	  _parallax(parallax),
	  _sheet(sheet),
	  _frames(frames) {
}

ParallaxNode::~ParallaxNode() {}

Math::Matrix4 ParallaxNode::getTrsf(const Math::Matrix4 &parentTrsf) {
	Gfx &gfx = g_twp->getGfx();
	Math::Matrix4 trsf = Node::getTrsf(parentTrsf);
	Math::Vector2d camPos = gfx.cameraPos();
	Math::Vector2d p = Math::Vector2d(-camPos.getX() * _parallax.getX(), -camPos.getY() * _parallax.getY());
	trsf.translate(Math::Vector3d(p.getX(), p.getY(), 0.0f));
	return trsf;
}

void ParallaxNode::onDrawChildren(const Math::Matrix4 &trsf) {
	Node::onDrawChildren(trsf);
}

void ParallaxNode::drawCore(const Math::Matrix4 &trsf) {
	Gfx &gfx = g_twp->getGfx();
	SpriteSheet *sheet = g_twp->_resManager->spriteSheet(_sheet);
	Texture *texture = g_twp->_resManager->texture(sheet->meta.image);

	// enable debug lighting ?
	if (_zOrder == 0 && g_twp->_lighting->_debug) {
		g_twp->getGfx().use(g_twp->_lighting.get());
	} else {
		g_twp->getGfx().use(nullptr);
	}

	Math::Matrix4 t = trsf;
	float x = 0.f;
	for (size_t i = 0; i < _frames.size(); i++) {
		const SpriteSheetFrame &frame = sheet->getFrame(_frames[i]);
		g_twp->_lighting->setSpriteOffset({0.f, static_cast<float>(-frame.frame.height())});
		g_twp->_lighting->setSpriteSheetFrame(frame, *texture, false);

		Math::Matrix4 myTrsf = t;
		myTrsf.translate(Math::Vector3d(x + frame.spriteSourceSize.left, frame.sourceSize.getY() - frame.spriteSourceSize.height() - frame.spriteSourceSize.top, 0.0f));
		gfx.drawSprite(frame.frame, *texture, getColor(), myTrsf);
		t = trsf;
		x += frame.frame.width();
	}

	g_twp->getGfx().use(nullptr);
}

Anim::Anim(Object *obj)
	: Node("anim") {
	_obj = obj;
	_zOrder = 1000;
}

void Anim::clearFrames() {
	_frames.clear();
}

void Anim::setAnim(const ObjectAnimation *anim, float fps, bool loop, bool instant) {
	_anim = anim;
	_disabled = false;
	setName(anim->name);
	_sheet = anim->sheet;
	_frames = anim->frames;
	_frameIndex = instant && _frames.size() > 0 ? _frames.size() - 1 : 0;
	_frameDuration = 1.0 / _getFps(fps, anim->fps);
	_loop = loop || anim->loop;
	_instant = instant;
	if (_obj)
		setVisible(Twp::find(_obj->_hiddenLayers, _anim->name) == (size_t)-1);

	clear();
	for (size_t i = 0; i < _anim->layers.size(); i++) {
		const ObjectAnimation &layer = _anim->layers[i];
		Common::SharedPtr<Anim> node(new Anim(_obj));
		_anims.push_back(node);
		node->setAnim(&layer, fps, loop, instant);
		addChild(node.get());
	}
}

void Anim::trigSound() {
	if (_anim && (_anim->triggers.size() > 0) && (_frameIndex < _anim->triggers.size())) {
		const Common::String &trigger = _anim->triggers[_frameIndex];
		if ((trigger.size() > 0) && trigger != "null") {
			_obj->trig(trigger);
		}
	}
}

void Anim::update(float elapsed) {
	if (_anim)
		setVisible(Twp::find(_obj->_hiddenLayers, _anim->name) == (size_t)-1);
	if (_instant)
		disable();
	else if (_frames.size() != 0) {
		_elapsed += elapsed;
		if (_elapsed > _frameDuration) {
			_elapsed = 0;
			if (_frameIndex < _frames.size() - 1) {
				_frameIndex++;
				trigSound();
			} else if (_loop) {
				_frameIndex = 0;
				trigSound();
			} else {
				disable();
			}
		}
		if (_anim && _anim->offsets.size() > 0) {
			Math::Vector2d off = _frameIndex < _anim->offsets.size() ? _anim->offsets[_frameIndex] : Math::Vector2d();
			if (_obj->getFacing() == Facing::FACE_LEFT) {
				off.setX(-off.getX());
			}
			_offset = Common::move(off);
		}
	} else if (_children.size() != 0) {
		bool disabled = true;
		for (size_t i = 0; i < _children.size(); i++) {
			Anim *layer = static_cast<Anim *>(_children[i]);
			layer->update(elapsed);
			disabled = disabled && layer->_disabled;
		}
		if (disabled) {
			disable();
		}
	} else {
		disable();
	}
}

void Anim::drawCore(const Math::Matrix4 &t) {
	Math::Matrix4 trsf(t);
	if (_frameIndex < _frames.size()) {
		const Common::String &frame = _frames[_frameIndex];
		if (frame == "null")
			return;

		bool flipX = _obj->getFacing() == Facing::FACE_LEFT;
		if (_sheet.size() == 0) {
			_sheet = _obj->_sheet;
			if (_sheet.size() == 0 && _obj->_room) {
				_sheet = _obj->_room->_sheet;
			}
		}
		if (_sheet == "raw") {
			Texture *texture = g_twp->_resManager->texture(frame);
			Math::Vector3d pos(-texture->width / 2.f, -texture->height / 2.f, 0.f);
			trsf.translate(pos);
			g_twp->getGfx().drawSprite(Common::Rect(texture->width, texture->height), *texture, getComputedColor(), trsf, flipX);
		} else {
			const SpriteSheet *sheet = g_twp->_resManager->spriteSheet(_sheet);
			const SpriteSheetFrame *sf = sheet->frame(frame);
			if (!sf)
				return;
			Texture *texture = g_twp->_resManager->texture(sheet->meta.image);
			if (_obj->_lit) {
				g_twp->getGfx().use(g_twp->_lighting.get());
				Math::Vector2d p = getAbsPos() + _obj->_node->getRenderOffset();
				const float left = flipX ? (-1.f + sf->sourceSize.getX()) / 2.f - sf->spriteSourceSize.left : sf->spriteSourceSize.left - sf->sourceSize.getX() / 2.f;
				const float top = -sf->sourceSize.getY() / 2.f + sf->spriteSourceSize.top;

				g_twp->_lighting->setSpriteOffset({p.getX() + left, -p.getY() + top});
				g_twp->_lighting->setSpriteSheetFrame(*sf, *texture, flipX);
			} else {
				g_twp->getGfx().use(nullptr);
			}
			const float x = flipX ? (1.f - sf->sourceSize.getX()) / 2.f + sf->frame.width() + sf->spriteSourceSize.left : sf->sourceSize.getX() / 2.f - sf->spriteSourceSize.left;
			const float y = sf->sourceSize.getY() / 2.f - sf->spriteSourceSize.height() - sf->spriteSourceSize.top;
			Math::Vector3d pos(int(-x), int(y), 0.f);
			trsf.translate(pos);
			g_twp->getGfx().drawSprite(sf->frame, *texture, getComputedColor(), trsf, flipX);
			g_twp->getGfx().use(nullptr);
		}
	}
}

ActorNode::ActorNode(Common::SharedPtr<Object> obj)
	: Node(obj->_key), _object(obj) {
}

int ActorNode::getZSort() const { return getPos().getY(); }

Math::Vector2d ActorNode::getScale() const {
	float y = _object->_room->getScaling(_object->_node->getPos().getY());
	return Math::Vector2d(y, y);
}

TextNode::TextNode() : Node("text") {
}

TextNode::~TextNode() {}

void TextNode::setText(const Text &text) {
	_text = text;
	updateBounds();
}

void TextNode::updateBounds() {
	setSize(_text.getBounds());
}

Rectf TextNode::getRect() const {
	Math::Vector2d size = _size * getScale();
	return Rectf::fromPosAndSize(getAbsPos() + Math::Vector2d(0, -size.getY()) + Math::Vector2d(-size.getX(), size.getY()) * _anchorNorm, size);
}

void TextNode::onColorUpdated(const Color &color) {
	_text.setColor(color);
}

void TextNode::drawCore(const Math::Matrix4 &trsf) {
	_text.draw(g_twp->getGfx(), trsf);
}

Scene::Scene() : Node("Scene") {
	_zOrder = -100;
}
Scene::~Scene() {}

InputState::InputState() : Node("InputState") {
	_zOrder = -100;
}

InputState::~InputState() {}

Common::String InputState::getCursorName() const {
	switch (_cursorShape) {
	case CursorShape::Left:
		return "cursor_left";
	case CursorShape::Right:
		return "cursor_right";
	case CursorShape::Front:
		return "cursor_front";
	case CursorShape::Back:
		return "cursor_back";
	case CursorShape::Pause:
		return "cursor_pause";
	case CursorShape::Normal:
		return "cursor";
	}
	return "cursor";
}

void InputState::drawCore(const Math::Matrix4 &t) {
	Math::Matrix4 trsf(t);
	Common::String cursorName = getCursorName();
	// draw cursor
	SpriteSheet *gameSheet = g_twp->_resManager->spriteSheet("GameSheet");
	Texture *texture = g_twp->_resManager->texture(gameSheet->meta.image);
	if (ConfMan.getBool("hudSentence") && _hotspot) {
		cursorName = "hotspot_" + cursorName;
	}
	const SpriteSheetFrame &sf = gameSheet->getFrame(cursorName);
	Math::Vector3d pos(sf.spriteSourceSize.left - sf.sourceSize.getX() / 2.f, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY() / 2.f, 0.f);
	trsf.translate(pos * 2.f);
	scale(trsf, Math::Vector2d(2.f, 2.f));
	g_twp->getGfx().drawSprite(sf.frame, *texture, getComputedColor(), trsf);
}

InputStateFlag InputState::getState() const {
	int tmp = 0;
	tmp |= (_inputActive ? UI_INPUT_ON : UI_INPUT_OFF);
	tmp |= (_inputVerbsActive ? UI_VERBS_ON : UI_VERBS_OFF);
	tmp |= (_showCursor ? UI_CURSOR_ON : UI_CURSOR_OFF);
	tmp |= (_inputHUD ? UI_HUDOBJECTS_ON : UI_HUDOBJECTS_OFF);
	return (InputStateFlag)tmp;
}

void InputState::setState(InputStateFlag state) {
	if ((UI_INPUT_ON & state) == UI_INPUT_ON)
		_inputActive = true;
	if ((UI_INPUT_OFF & state) == UI_INPUT_OFF)
		_inputActive = false;
	if ((UI_VERBS_ON & state) == UI_VERBS_ON)
		_inputVerbsActive = true;
	if ((UI_VERBS_OFF & state) == UI_VERBS_OFF)
		_inputVerbsActive = false;
	if ((UI_CURSOR_ON & state) == UI_CURSOR_ON) {
		_showCursor = true;
		_visible = true;
	}
	if ((UI_CURSOR_OFF & state) == UI_CURSOR_OFF) {
		_showCursor = false;
		_visible = false;
	}
	if ((UI_HUDOBJECTS_ON & state) == UI_HUDOBJECTS_ON)
		_inputHUD = true;
	if ((UI_HUDOBJECTS_OFF & state) == UI_HUDOBJECTS_OFF)
		_inputHUD = false;
}

void InputState::setCursorShape(CursorShape shape) {
	_cursorShape = shape;
}

OverlayNode::OverlayNode() : Node("overlay") {
	_ovlColor = Color(0.f, 0.f, 0.f, 0.f); // transparent
	_zOrder = INT_MIN;
}

void OverlayNode::drawCore(const Math::Matrix4 &trsf) {
	Math::Vector2d size = g_twp->getGfx().camera();
	g_twp->getGfx().drawQuad(size, _ovlColor);
}

static bool hasUpArrow(Common::SharedPtr<Object> actor) {
	return actor->_inventoryOffset != 0;
}

static bool hasDownArrow(Common::SharedPtr<Object> actor) {
	return actor->_inventory.size() > (size_t)(actor->_inventoryOffset * NUMOBJECTSBYROW + NUMOBJECTS);
}

Inventory::Inventory() : Node("Inventory") {
	_visible = false;
	for (int i = 0; i < NUMOBJECTS; i++) {
		float x = SCREEN_WIDTH / 2.f + ARROWWIDTH + MARGIN + ((i % NUMOBJECTSBYROW) * (BACKWIDTH + BACKOFFSET));
		float y = MARGINBOTTOM + BACKHEIGHT + BACKOFFSET - ((float)(i / NUMOBJECTSBYROW) * (BACKHEIGHT + BACKOFFSET));
		_itemRects[i] = Common::Rect(x, y, x + BACKWIDTH, y + BACKHEIGHT);
	}
	_arrowUpRect = Common::Rect(SCREEN_WIDTH / 2.f, ARROWHEIGHT + MARGINBOTTOM + BACKOFFSET, SCREEN_WIDTH / 2.f + ARROWWIDTH, ARROWHEIGHT + MARGINBOTTOM + BACKOFFSET + ARROWHEIGHT);
	_arrowDnRect = Common::Rect(SCREEN_WIDTH / 2.f, MARGINBOTTOM, SCREEN_WIDTH / 2.f + ARROWWIDTH, MARGINBOTTOM + ARROWHEIGHT);
	for (int i = 0; i < NUMOBJECTS; i++) {
		_shakeTime[i] = 0.f;
		_inventoryOver[i] = false;
	}
}

Math::Vector2d Inventory::getPos(Common::SharedPtr<Object> inv) const {
	if (_actor) {
		int i = Twp::find(_actor->_inventory, inv) - _actor->_inventoryOffset * NUMOBJECTSBYROW;
		return Math::Vector2d(_itemRects[i].left + _itemRects[i].width() / 2.f, _itemRects[i].bottom + _itemRects[i].height() / 2.f);
	}
	return {};
}

Math::Vector2d Inventory::getPos(int i) const {
	assert((i >= 0) && (i < 8));
	return Math::Vector2d(_itemRects[i].left + _itemRects[i].width() / 2.f, _itemRects[i].top + _itemRects[i].height() / 2.f);
}

int Inventory::getOverIndex() const {
	for (int i = 0; i < NUMOBJECTS; i++) {
		if (_inventoryOver[i])
			return i;
	}
	return -1;
}

void Inventory::drawSprite(const SpriteSheetFrame &sf, Texture *texture, const Color &color, const Math::Matrix4 &t) {
	Math::Matrix4 trsf(t);
	Math::Vector3d pos(sf.spriteSourceSize.left - sf.sourceSize.getX() / 2.f, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY() / 2.f, 0.f);
	trsf.translate(pos);
	g_twp->getGfx().drawSprite(sf.frame, *texture, color, trsf);
}

void Inventory::drawArrows(const Math::Matrix4 &trsf) {
	bool isRetro = ConfMan.getBool("retroVerbs");
	SpriteSheet *gameSheet = g_twp->_resManager->spriteSheet("GameSheet");
	Texture *texture = g_twp->_resManager->texture(gameSheet->meta.image);
	const SpriteSheetFrame *arrowUp = &gameSheet->getFrame(isRetro ? "scroll_up_retro" : "scroll_up");
	const SpriteSheetFrame *arrowDn = &gameSheet->getFrame(isRetro ? "scroll_down_retro" : "scroll_down");
	float alphaUp = hasUpArrow(_actor) ? 1.f : 0.f;
	float alphaDn = hasDownArrow(_actor) ? 1.f : 0.f;
	Math::Matrix4 tUp(trsf);
	tUp.translate(Math::Vector3d(SCREEN_WIDTH / 2.f + ARROWWIDTH / 2.f + MARGIN, 1.5f * ARROWHEIGHT + BACKOFFSET, 0.f));
	Math::Matrix4 tDn(trsf);
	tDn.translate(Math::Vector3d(SCREEN_WIDTH / 2.f + ARROWWIDTH / 2.f + MARGIN, 0.5f * ARROWHEIGHT, 0.f));

	drawSprite(*arrowUp, texture, Color::withAlpha(_verbNormal, alphaUp * getAlpha()), tUp);
	drawSprite(*arrowDn, texture, Color::withAlpha(_verbNormal, alphaDn * getAlpha()), tDn);
}

void Inventory::drawBack(const Math::Matrix4 &trsf) {
	SpriteSheet *gameSheet = g_twp->_resManager->spriteSheet("GameSheet");
	Texture *texture = g_twp->_resManager->texture(gameSheet->meta.image);
	const SpriteSheetFrame *back = &gameSheet->getFrame("inventory_background");

	float startOffsetX = SCREEN_WIDTH / 2.f + ARROWWIDTH + MARGIN + back->sourceSize.getX() / 2.f;
	float offsetX = startOffsetX;
	float offsetY = 3.f * back->sourceSize.getY() / 2.f + MARGINBOTTOM + BACKOFFSET;

	for (int i = 0; i < 4; i++) {
		Math::Matrix4 t(trsf);
		t.translate(Math::Vector3d(offsetX, offsetY, 0.f));
		drawSprite(*back, texture, Color::withAlpha(_backColor, getAlpha()), t);
		offsetX += back->sourceSize.getX() + BACKOFFSET;
	}

	offsetX = startOffsetX;
	offsetY = back->sourceSize.getY() / 2.f + MARGINBOTTOM;
	for (int i = 0; i < 4; i++) {
		Math::Matrix4 t(trsf);
		t.translate(Math::Vector3d(offsetX, offsetY, 0.f));
		drawSprite(*back, texture, Color::withAlpha(_backColor, getAlpha()), t);
		offsetX += back->sourceSize.getX() + BACKOFFSET;
	}
}

void Inventory::drawItems(const Math::Matrix4 &trsf) {
	float startOffsetX = SCREEN_WIDTH / 2.f + ARROWWIDTH + MARGIN + BACKWIDTH / 2.f;
	float startOffsetY = MARGINBOTTOM + 1.5f * BACKHEIGHT + BACKOFFSET;
	SpriteSheet *itemsSheet = g_twp->_resManager->spriteSheet("InventoryItems");
	Texture *texture = g_twp->_resManager->texture(itemsSheet->meta.image);
	int count = MIN(NUMOBJECTS, (int)(_actor->_inventory.size() - _actor->_inventoryOffset * NUMOBJECTSBYROW));

	for (int i = 0; i < count; i++) {
		Common::SharedPtr<Object> obj = _actor->_inventory[_actor->_inventoryOffset * NUMOBJECTSBYROW + i];
		Common::String icon = obj->getIcon();
		if (itemsSheet->_frameTable.contains(icon)) {
			SpriteSheetFrame *itemFrame = &itemsSheet->_frameTable[icon];
			Math::Vector2d pos(startOffsetX + ((float)(i % NUMOBJECTSBYROW) * (BACKWIDTH + BACKOFFSET)), startOffsetY - ((float)(i / NUMOBJECTSBYROW) * (BACKHEIGHT + BACKOFFSET)));
			Math::Matrix4 t(trsf);
			t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
			if (obj->_jiggle) {
				Math::Matrix3 rot;
				rot.buildAroundZ(18.f * sin(_jiggleTime));
				t.setRotation(rot);
			}
			float s = obj->getScale();
			Twp::scale(t, Math::Vector2d(s, s));
			Math::Vector2d shakeOffset = _shakeOffset[i];
			t.translate(Math::Vector3d(shakeOffset.getX(), shakeOffset.getY(), 0.f));
			drawSprite(*itemFrame, texture, Color::withAlpha(Color(), getAlpha()), t);
		}
	}
}

void Inventory::drawCore(const Math::Matrix4 &trsf) {
	if (_actor) {
		drawArrows(trsf);
		drawBack(trsf);
		drawItems(trsf);
	}
}

void Inventory::update(float elapsed, Common::SharedPtr<Object> actor, const Color &backColor, const Color &verbNormal) {
	_jiggleTime += 10.f * elapsed;
	_fadeTime += elapsed;

	if (_fadeTime > 2.f) {
		_fadeTime = 2.f;
	}

	if (_fadeIn) {
		float alpha = MIN(_fadeTime, 2.0f) / 2.0f;
		setAlpha(alpha);
	}

	// udate colors
	_actor = actor;
	_backColor = backColor;
	_verbNormal = verbNormal;

	_obj = nullptr;
	if (_actor) {
		Math::Vector2d scrPos = g_twp->winToScreen(g_twp->_cursor.pos);
		_over = scrPos.getY() < 180.f && scrPos.getX() > 704.f;

		// update mouse click
		bool down = g_twp->_cursor.leftDown;
		if (!_down && down) {
			_down = true;
			if (_arrowUpRect.contains(scrPos.getX(), scrPos.getY())) {
				_actor->inventoryScrollUp();
			} else if (_arrowDnRect.contains(scrPos.getX(), scrPos.getY())) {
				_actor->inventoryScrollDown();
			}
		} else if (!down) {
			_down = false;
		}

		for (int i = 0; i < NUMOBJECTS; i++) {
			const Common::Rect &item = _itemRects[i];
			if (item.contains(scrPos.getX(), scrPos.getY())) {
				size_t index = _actor->_inventoryOffset * NUMOBJECTSBYROW + i;
				if (index < _actor->_inventory.size()) {
					_obj = _actor->_inventory[index];

					if (!_inventoryOver[i] && (_shakeTime[i] < 0.1f)) {
						_shakeTime[i] = 0.25f;
						_shake[i] = Common::ScopedPtr<Motor>(new ShakeInventory(_shakeOffset[i], 0.4f));
						_inventoryOver[i] = true;
					}
				} else {
					_inventoryOver[i] = true;
				}
			} else {
				_inventoryOver[i] = false;
			}

			// shake choice when cursor is over
			if ((_shakeTime[i] > 0.0f) && _shake[i]) {
				_shake[i]->update(elapsed);
				_shakeTime[i] -= elapsed;
				if (_shakeTime[i] < 0.f) {
					_shakeTime[i] = 0.f;
				}
			}
		}

		for (size_t i = 0; i < _actor->_inventory.size(); i++) {
			Common::SharedPtr<Object> obj = _actor->_inventory[i];
			obj->update(elapsed);
		}
	}
}

void Inventory::setVisible(bool visible) {
	if (_fadeIn != visible) {
		_fadeIn = visible;
		Node::setVisible(visible);
		_fadeTime = 0;
	}
}

SentenceNode::SentenceNode() : Node("Sentence") {
	_zOrder = -100;
}

SentenceNode::~SentenceNode() {
}

void SentenceNode::setText(const Common::String &text) {
	_text = text;
}

void SentenceNode::drawCore(const Math::Matrix4 &trsf) {
	Text text("sayline", _text);
	float x, y;
	if (ConfMan.getBool("hudSentence")) {
		x = (SCREEN_WIDTH - text.getBounds().getX()) / 2.f;
		y = 152.f;
	} else {
		x = MAX(_pos.getX() - text.getBounds().getX() / 2.f, MARGIN);
		x = MIN(x, SCREEN_WIDTH - text.getBounds().getX() - MARGIN);
		y = _pos.getY() + 16.f;
		if (y >= (SCREEN_HEIGHT - 32.f))
			y = _pos.getY() - 92.f;
	}
	Math::Matrix4 t;
	t.translate(Math::Vector3d(x, y, 0.f));
	text.draw(g_twp->getGfx(), t);
}

SpriteNode::SpriteNode() : Node("Sprite") {}
SpriteNode::~SpriteNode() {}

void SpriteNode::setSprite(const Common::String &sheet, const Common::String &frame) {
	_sheet = sheet;
	_frame = frame;
}

void SpriteNode::drawCore(const Math::Matrix4 &trsf) {
	SpriteSheet *sheet = g_twp->_resManager->spriteSheet(_sheet);
	const SpriteSheetFrame *frame = &sheet->getFrame(_frame);

	Common::Rect rect = frame->frame;
	setSize(Math::Vector2d(frame->frame.width(), frame->frame.height()));
	float x = frame->sourceSize.getX() / 2.f - frame->spriteSourceSize.left;
	float y = (frame->sourceSize.getY() + 1.f) / 2.f - frame->spriteSourceSize.height() - frame->spriteSourceSize.top;
	Math::Vector2d anchor((int)(x), (int)(y));
	setAnchor(anchor);

	Texture *texture = g_twp->_resManager->texture(sheet->meta.image);
	g_twp->getGfx().drawSprite(rect, *texture, getComputedColor(), trsf);
}

NoOverrideNode::NoOverrideNode() : Node("NoOverride") {
	_zOrder = -1000;
	_elapsed = 42.f;

	_icon.setSprite("GameSheet", "icon_no");
	_icon.setScale(Math::Vector2d(2.f, 2.f));
	_icon.setPos(Math::Vector2d(32.f, SCREEN_HEIGHT - 32.f));
	addChild(&_icon);
}

NoOverrideNode::~NoOverrideNode() {
}

void NoOverrideNode::reset() {
	_elapsed = 0.f;
	setVisible(true);
}

bool NoOverrideNode::update(float elapsed) {
	if (_elapsed > 2.f) {
		setVisible(false);
		return false;
	}
	_elapsed += elapsed;
	setAlpha(CLIP((2.f - _elapsed) / 2.f, 0.f, 1.f));
	debugC(kDebugGame, "no override: %.2f, %.2f", _elapsed, getAlpha());
	return true;
}

HotspotMarkerNode::HotspotMarkerNode() : Node("HotspotMarker") {
	_zOrder = -1000;
	_visible = false;
}

HotspotMarkerNode::~HotspotMarkerNode() {}

void HotspotMarkerNode::drawSprite(const SpriteSheetFrame &sf, Texture *texture, const Color &color, const Math::Matrix4 &t) {
	Math::Matrix4 trsf(t);
	Math::Vector3d pos(sf.spriteSourceSize.left - sf.sourceSize.getX() / 2.f, -sf.spriteSourceSize.height() - sf.spriteSourceSize.top + sf.sourceSize.getY() / 2.f, 0.f);
	trsf.translate(pos);
	g_twp->getGfx().drawSprite(sf.frame, *texture, color, trsf);
}

void HotspotMarkerNode::drawCore(const Math::Matrix4 &trsf) {
	SpriteSheet *gameSheet = g_twp->_resManager->spriteSheet("GameSheet");
	Texture *texture = g_twp->_resManager->texture(gameSheet->meta.image);
	const SpriteSheetFrame *frame = &gameSheet->getFrame("hotspot_marker");
	Color color = Color::create(255, 165, 0);
	for (size_t i = 0; i < g_twp->_room->_layers.size(); i++) {
		Common::SharedPtr<Layer> layer = g_twp->_room->_layers[i];
		for (size_t j = 0; j < layer->_objects.size(); j++) {
			Common::SharedPtr<Object> obj = layer->_objects[j];
			if (g_twp->_resManager->isObject(obj->getId()) && (obj->_objType == otNone) && obj->isTouchable()) {
				Math::Vector2d pos = g_twp->roomToScreen(obj->_node->getAbsPos());
				Math::Matrix4 t;
				t.translate(Math::Vector3d(pos.getX(), pos.getY(), 0.f));
				drawSprite(*frame, texture, color, t);
			}
		}
	}
}

} // namespace Twp
