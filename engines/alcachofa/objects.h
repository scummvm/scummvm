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

#ifndef OBJECTS_H
#define OBJECTS_H

#include "shape.h"
#include "graphics.h"

#include "common/serializer.h"

namespace Alcachofa {

class Room;
class Process;
struct Task;

class ObjectBase {
public:
	static constexpr const char *kClassName = "CObjetoBase";
	ObjectBase(Room *room, const char *name);
	ObjectBase(Room *room, Common::ReadStream &stream);
	virtual ~ObjectBase() = default;

	inline const Common::String &name() const { return _name; }
	inline Room *&room() { return _room; }
	inline Room *room() const { return _room; }
	inline bool isEnabled() const { return _isEnabled; }

	virtual void toggle(bool isEnabled);
	virtual void draw();
	virtual void drawDebug();
	virtual void update();
	virtual void loadResources();
	virtual void freeResources();
	virtual void serializeSave(Common::Serializer &serializer);
	virtual Graphic *graphic();
	virtual Shape *shape();
	virtual const char *typeName() const;

private:
	Common::String _name;
	bool _isEnabled = true;
	Room *_room = nullptr;
};

class PointObject : public ObjectBase {
public:
	static constexpr const char *kClassName = "CObjetoPunto";
	PointObject(Room *room, Common::ReadStream &stream);

	inline Common::Point &position() { return _pos; }
	inline Common::Point position() const { return _pos; }
	virtual const char *typeName() const;

private:
	Common::Point _pos;
};

enum class GraphicObjectType : byte
{
	Normal,
	NormalPosterize, // the posterization is not actually applied in the original engine
	Effect
};

class GraphicObject : public ObjectBase {
public:
	static constexpr const char *kClassName = "CObjetoGrafico";
	GraphicObject(Room *room, Common::ReadStream &stream);
	virtual ~GraphicObject() override = default;

	virtual void draw() override;
	virtual void loadResources() override;
	virtual void freeResources() override;
	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual Graphic *graphic() override;
	virtual const char *typeName() const;

	Task *animate(Process &process);

protected:
	GraphicObject(Room *room, const char *name);

	Graphic _graphic;
	GraphicObjectType _type;
	int32 _posterizeAlpha;
};

class SpecialEffectObject final : public GraphicObject {
public:
	static constexpr const char *kClassName = "CObjetoGraficoMuare";
	SpecialEffectObject(Room *room, Common::ReadStream &stream);

	virtual void draw() override;
	virtual const char *typeName() const;

private:
	static constexpr const float kShiftSpeed = 1 / 256.0f;
	Common::Point _topLeft, _bottomRight;
	Math::Vector2d _texShift;
};

class ShapeObject : public ObjectBase {
public:
	ShapeObject(Room *room, Common::ReadStream &stream);
	virtual ~ShapeObject() override = default;

	inline int8 order() const { return _order; }

	virtual void update() override;
	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual Shape *shape() override;
	virtual CursorType cursorType() const;
	virtual void onHoverStart();
	virtual void onHoverEnd();
	virtual void onHoverUpdate();
	virtual void onClick();
	virtual const char *typeName() const;
	void markSelected();

protected:
	void updateSelection();

	// original inconsistency: base class has member that is read by the sub classes
	int8 _order = 0;
private:
	Shape _shape;
	CursorType _cursorType;
	bool _isSelected = false,
		_wasSelected = false;
};

class PhysicalObject : public ShapeObject {
public:
	PhysicalObject(Room *room, Common::ReadStream &stream);
	virtual const char *typeName() const;
};

class MenuButton : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CBotonMenu";
	MenuButton(Room *room, Common::ReadStream &stream);
	virtual ~MenuButton() override = default;

	inline int32 actionId() const { return _actionId; }
	virtual const char *typeName() const;

private:
	int32 _actionId;
	Graphic
		_graphicNormal,
		_graphicHovered,
		_graphicClicked,
		_graphicDisabled;
};

class InternetMenuButton final : public MenuButton {
public:
	static constexpr const char *kClassName = "CBotonMenuInternet";
	InternetMenuButton(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;
};

class OptionsMenuButton final : public MenuButton {
public:
	static constexpr const char *kClassName = "CBotonMenuOpciones";
	OptionsMenuButton(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;
};

class MainMenuButton final : public MenuButton {
public:
	static constexpr const char *kClassName = "CBotonMenuPrincipal";
	MainMenuButton(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;
};

class PushButton final : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CPushButton";
	PushButton(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;

private:
	// TODO: Reverse engineer PushButton
	bool _alwaysVisible;
	Graphic _graphic1, _graphic2;
	int32 _actionId;
};

class EditBox final : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CEditBox";
	EditBox(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;

private:
	// TODO: Reverse engineer EditBox
	int32 i1;
	Common::Point p1;
	Common::String _labelId;
	bool b1;
	int32 i3, i4, i5,
		_fontId;
};

class CheckBox : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CCheckBox";
	CheckBox(Room *room, Common::ReadStream &stream);
	virtual ~CheckBox() override = default;

	virtual const char *typeName() const;

private:
	// TODO: Reverse engineer CheckBox
	bool b1;
	Graphic
		_graph1,
		_graph2,
		_graph3,
		_graph4;
	int32 _valueId;
};

class CheckBoxAutoAdjustNoise final : public CheckBox {
public:
	static constexpr const char *kClassName = "CCheckBoxAutoAjustarRuido";
	CheckBoxAutoAdjustNoise(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;
};

class SlideButton final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CSlideButton";
	SlideButton(Room *room, Common::ReadStream &stream);
	virtual ~SlideButton() override = default;

	virtual const char *typeName() const;

private:
	// TODO: Reverse engineer SlideButton
	int32 i1;
	Common::Point p1, p2;
	Graphic
		_graph1,
		_graph2,
		_graph3;
};

class IRCWindow final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CVentanaIRC";
	IRCWindow(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;

private:
	Common::Point _p1, _p2;
};

class MessageBox final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CMessageBox";
	MessageBox(Room *room, Common::ReadStream &stream);
	virtual ~MessageBox() override = default;

	virtual const char *typeName() const;

private:
	// TODO: Reverse engineer MessageBox
	Graphic
		_graph1,
		_graph2,
		_graph3,
		_graph4,
		_graph5;
};

class VoiceMeter final : public GraphicObject {
public:
	static constexpr const char *kClassName = "CVuMeter";
	VoiceMeter(Room *room, Common::ReadStream &stream);

	virtual const char *typeName() const;
};

class Item : public GraphicObject {
public:
	static constexpr const char *kClassName = "CObjetoInventario";
	Item(Room *room, Common::ReadStream &stream);
	Item(const Item &other);

	virtual const char *typeName() const;
	void trigger();
};

class ITriggerableObject {
public:
	ITriggerableObject(Common::ReadStream &stream);

	inline Direction interactionDirection() const { return _interactionDirection; }
	inline const Common::Point &interactionPoint() const { return _interactionPoint; }

	virtual void trigger(const char *action) = 0;

protected:
	void onClick();

	Common::Point _interactionPoint;
	Direction _interactionDirection = Direction::Right;
};

class InteractableObject : public PhysicalObject, public ITriggerableObject {
public:
	static constexpr const char *kClassName = "CObjetoTipico";
	InteractableObject(Room *room, Common::ReadStream &stream);
	virtual ~InteractableObject() override = default;

	virtual void drawDebug() override;
	virtual void onClick() override;
	virtual void trigger(const char *action) override;
	virtual void toggle(bool isEnabled) override;
	virtual const char *typeName() const;

private:
	Common::String _relatedObject;
};

class Door final : public InteractableObject {
public:
	static constexpr const char *kClassName = "CPuerta";
	Door(Room *room, Common::ReadStream &stream);

	inline const Common::String &targetRoom() const { return _targetRoom; }
	inline const Common::String &targetObject() const { return _targetObject; }
	inline Direction characterDirection() const { return _characterDirection; }

	virtual CursorType cursorType() const override;
	virtual void onClick() override;
	virtual void trigger(const char *action) override;
	virtual const char *typeName() const;

private:
	Common::String _targetRoom, _targetObject;
	Direction _characterDirection;
	uint32 _lastClickTime = 0;
};

class Character : public ShapeObject, public ITriggerableObject {
public:
	static constexpr const char *kClassName = "CPersonaje";
	Character(Room *room, Common::ReadStream &stream);
	virtual ~Character() override = default;

	virtual void update() override;
	virtual void draw() override;
	virtual void drawDebug() override;
	virtual void loadResources() override;
	virtual void freeResources() override;
	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual Graphic *graphic() override;
	virtual void onClick() override;
	virtual void trigger(const char *action) override;
	virtual const char *typeName() const;

	Task *sayText(Process &process, int32 dialogId);

protected:
	friend struct SayTextTask;
	void syncObjectAsString(Common::Serializer &serializer, ObjectBase *&object);
	void updateTalkingAnimation();

	Direction _direction = Direction::Right;
	Graphic _graphicNormal, _graphicTalking;

	bool _isTalking = false;
	int _curDialogId = -1;
	float _lodBias = 0.0f;
	ObjectBase
		*_curAnimateObject = nullptr,
		*_curTalkingObject = nullptr;
};

class WalkingCharacter : public Character {
public:
	static constexpr const char *kClassName = "CPersonajeAnda";
	WalkingCharacter(Room *room, Common::ReadStream &stream);
	virtual ~WalkingCharacter() override = default;

	inline bool isWalking() const { return _isWalking; }
	inline const Common::Point &position() const { return _currentPos; }
	inline float stepSizeFactor() const { return _stepSizeFactor; }

	virtual void update() override;
	virtual void draw() override;
	virtual void drawDebug() override;
	virtual void loadResources() override;
	virtual void freeResources() override;
	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual void walkTo(
		const Common::Point &target,
		Direction endDirection = Direction::Invalid,
		ITriggerableObject *activateObject = nullptr,
		const char *activateAction = nullptr);
	void stopWalking(Direction direction = Direction::Invalid);
	void setPosition(const Common::Point &target);
	virtual const char *typeName() const;

	Task *waitForArrival(Process &process);

protected:
	virtual void onArrived();
	void updateWalking();
	void updateWalkingAnimation();

	inline Animation *currentAnimationOf(Common::ScopedPtr<Animation> *const animations) {
		Animation *animation = animations[(int)_direction].get();
		if (animation == nullptr)
			animation = animations[0].get();
		assert(animation != nullptr);
		return animation;
	}
	inline Animation *walkingAnimation() { return currentAnimationOf(_walkingAnimations); }
	inline Animation *talkingAnimation() { return currentAnimationOf(_talkingAnimations); }

	Common::ScopedPtr<Animation>
		_walkingAnimations[kDirectionCount],
		_talkingAnimations[kDirectionCount];

	int32
		_lastWalkAnimFrame = -1,
		_walkedDistance = 0,
		_curPathPointI = -1;
	float _stepSizeFactor = 0.0f;
	Common::Point
		_sourcePos,
		_currentPos;
	bool _isWalking = false;
	Direction
		_direction = Direction::Right,
		_endWalkingDirection = Direction::Invalid;
	Common::Stack<Common::Point> _pathPoints;
};

struct DialogMenuLine {
	int32 _dialogId;
	int32 _yPosition = 0;
	int32 _returnValue = 0;
};

class MainCharacter final : public WalkingCharacter {
public:
	static constexpr const char *kClassName = "CPersonajePrincipal";
	MainCharacter(Room *room, Common::ReadStream &stream);
	virtual ~MainCharacter() override;

	inline MainCharacterKind kind() const { return _kind; }
	inline ObjectBase *&currentlyUsing() { return _currentlyUsingObject; }
	inline ObjectBase *currentlyUsing() const { return _currentlyUsingObject; }
	inline FakeSemaphore &semaphore() { return _semaphore; }
	bool isBusy() const;

	virtual void update() override;
	virtual void draw() override;
	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual const char *typeName() const;
	virtual void walkTo(
		const Common::Point &target,
		Direction endDirection = Direction::Invalid,
		ITriggerableObject *activateObject = nullptr,
		const char *activateAction = nullptr) override;
	void walkToMouse();
	bool clearTargetIf(const ITriggerableObject *target);
	void clearInventory();
	bool hasItem(const Common::String &name) const;
	void pickup(const Common::String &name, bool putInHand);
	void drop(const Common::String &name);
	void addDialogLine(int32 dialogId);
	void setLastDialogReturnValue(int32 returnValue);
	Task *dialogMenu(Process &process);

protected:
	virtual void onArrived() override;

private:
	friend class Inventory;
	friend struct DialogMenuTask;
	Item *getItemByName(const Common::String &name) const;
	void drawInner();

	Common::Array<Item *> _items;
	Common::Array<DialogMenuLine> _dialogLines;
	ObjectBase *_currentlyUsingObject = nullptr;
	MainCharacterKind _kind;
	FakeSemaphore _semaphore;
	ITriggerableObject *_activateObject = nullptr;
	const char *_activateAction = nullptr;
};

class Background final : public GraphicObject {
public:
	Background(Room *room, const Common::String &animationFileName, int16 scale);
	virtual const char *typeName() const;
};

class FloorColor final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CSueloColor";
	FloorColor(Room *room, Common::ReadStream &stream);
	virtual ~FloorColor() override = default;

	virtual void drawDebug() override;
	virtual Shape *shape() override;
	virtual const char *typeName() const;

private:
	FloorColorShape _shape;
};

}

#endif // OBJECTS_H
