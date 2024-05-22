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

#include "Shape.h"
#include "Graphics.h"

#include "common/serializer.h"

namespace Alcachofa {

class Room;

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

private:
	static constexpr const float kShiftSpeed = 1 / 256.0f;
	Common::Point _topLeft, _bottomRight;
	Math::Vector2d _texShift;
};

class ShapeObject : public ObjectBase {
public:
	ShapeObject(Room *room, Common::ReadStream &stream);
	virtual ~ShapeObject() override = default;

	virtual void serializeSave(Common::Serializer &serializer) override;
	virtual Shape *shape() override;
	virtual CursorType cursorType() const;

private:
	Shape _shape;
	CursorType _cursorType;

protected:
	// original inconsistency: base class has member that is read by the sub classes
	int8 _order = 0;
};

class PhysicalObject : public ShapeObject {
public:
	PhysicalObject(Room *room, Common::ReadStream &stream);
};

class MenuButton : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CBotonMenu";
	MenuButton(Room *room, Common::ReadStream &stream);
	virtual ~MenuButton() override = default;

	inline int32 actionId() const { return _actionId; }

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
};

class OptionsMenuButton final : public MenuButton {
public:
	static constexpr const char *kClassName = "CBotonMenuOpciones";
	OptionsMenuButton(Room *room, Common::ReadStream &stream);
};

class MainMenuButton final : public MenuButton {
public:
	static constexpr const char *kClassName = "CBotonMenuPrincipal";
	MainMenuButton(Room *room, Common::ReadStream &stream);
};

class PushButton final : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CPushButton";
	PushButton(Room *room, Common::ReadStream &stream);

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
};

class SlideButton final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CSlideButton";
	SlideButton(Room *room, Common::ReadStream &stream);
	virtual ~SlideButton() override = default;

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

private:
	Common::Point _p1, _p2;
};

class MessageBox final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CMessageBox";
	MessageBox(Room *room, Common::ReadStream &stream);
	virtual ~MessageBox() override = default;

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
};

class Item : public GraphicObject {
public:
	static constexpr const char *kClassName = "CObjetoInventario";
	Item(Room *room, Common::ReadStream &stream);
};

class InteractableObject : public PhysicalObject {
public:
	static constexpr const char *kClassName = "CObjetoTipico";
	InteractableObject(Room *room, Common::ReadStream &stream);
	virtual ~InteractableObject() override = default;

	virtual void drawDebug() override;

private:
	Common::Point _interactionPoint;
	CursorType _cursorType;
	Common::String _relatedObject;
};

class Door final : public InteractableObject {
public:
	static constexpr const char *kClassName = "CPuerta";
	Door(Room *room, Common::ReadStream &stream);

private:
	Common::String _targetRoom, _targetObject;
	Direction _characterDirection;
};

class Character : public ShapeObject {
public:
	static constexpr const char *kClassName = "CPersonaje";
	Character(Room *room, Common::ReadStream &stream);
	virtual ~Character() override = default;

	virtual void serializeSave(Common::Serializer &serializer) override;

protected:
	void syncObjectAsString(Common::Serializer &serializer, ObjectBase *&object);

private:
	Common::Point _interactionPoint;
	Direction _direction;
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

	virtual void serializeSave(Common::Serializer &serializer) override;

private:
	Graphic _graphicWalking;
	Common::SharedPtr<Animation>
		_walkingAnimations[kDirectionCount],
		_standingAnimations[kDirectionCount];

	int32
		_lastWalkAnimFrame = -1,
		_walkSpeed = 0,
		_curPathPointI = -1;
	Common::Point
		_sourcePos,
		_targetPos;
	bool _isWalking = false;
	Direction _direction = Direction::Up;
	Common::Array<Common::Point> _pathPoints;
};

enum class MainCharacterKind {
	None,
	Mortadelo,
	Filemon
};

struct DialogMenuLine {
	int32 _dialogId;
	int32 _yPosition;
	int32 _returnId;
};

class MainCharacter final : public WalkingCharacter {
public:
	static constexpr const char *kClassName = "CPersonajePrincipal";
	MainCharacter(Room *room, Common::ReadStream &stream);
	virtual ~MainCharacter() override;

	inline MainCharacterKind kind() const { return _kind; }

	virtual void serializeSave(Common::Serializer &serializer) override;

private:
	Common::Array<Item *> _items;
	Common::Array<DialogMenuLine> _dialogMenuLines;
	ObjectBase *_currentlyUsingObject = nullptr;
	MainCharacterKind _kind;
	int32_t _relatedProcessCounter = 0;
};

class Background final : public GraphicObject {
public:
	Background(Room *room, const Common::String &animationFileName, int16 scale);
};

class FloorColor final : public ObjectBase {
public:
	static constexpr const char *kClassName = "CSueloColor";
	FloorColor(Room *room, Common::ReadStream &stream);
	virtual ~FloorColor() override = default;

	virtual void drawDebug() override;
	virtual Shape *shape() override;

private:
	FloorColorShape _shape;
};

}

#endif // OBJECTS_H
