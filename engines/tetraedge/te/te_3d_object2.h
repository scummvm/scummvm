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

#ifndef TETRAEDGE_TE_TE_3D_OBJECT2_H
#define TETRAEDGE_TE_TE_3D_OBJECT2_H

#include "common/array.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_i_3d_object2.h"
#include "tetraedge/te/te_matrix4x4.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_object.h"
#include "tetraedge/te/te_signal.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class Te3DObject2 : public TeI3DObject2, public TeObject {
public:
	Te3DObject2();
	virtual ~Te3DObject2();

	// note, probably should be Te*I*3DObject2 args here
	virtual void addChild(Te3DObject2 *newChild);
	virtual void addChildBefore(Te3DObject2 *newChild, const Te3DObject2 *ref);
	virtual Te3DObject2 *child(int offset);
	int childCount() {
		return _children.size();
	}

	int childIndex(Te3DObject2 *childToFind) const;

	const Common::Array<Te3DObject2 *> &childList() const {
		return _children;
	}

	bool childListChanged() const {
		return _childListChanged;
	}

	const TeColor &color() const {
		return _color;
	}

	bool colorInheritance() const {
		return _colorInheritance;
	}

	/* Note: Added control for includesName not in original as Syberia 2 data format uses
	   the file name as the model name. */
	static void deserialize(Common::ReadStream &stream, Te3DObject2 &dest, bool includesName = true);
	static void serialize(Common::WriteStream &stream, Te3DObject2 &src);

	virtual void draw() {}
	const Common::String &name() const {
		return _name;
	}

	virtual bool onParentWorldColorChanged();
	bool onParentWorldTransformationMatrixChanged();
	bool onWorldVisibleChangedSlot();
	TeSignal0Param &onPositionChanged() {
		return _onPositionChangedSignal;
	}
	TeSignal0Param &onSizeChanged() {
		return _onSizeChangedSignal;
	}
	TeSignal0Param &onWorldColorChanged() {
		return _onParentWorldColorChangedSignal;
	}
	TeSignal0Param &onWorldTransformationMatrixChanged() {
		return _onParentWorldTransformationMatrixChangedSignal;
	}
	TeSignal0Param &onWorldVisibleChanged() {
		return _onWorldVisibleChangedSlotSignal;
	}

	Te3DObject2 *parent() {
		return _parent;
	}
	virtual TeVector3f32 position() {
		return _position;
	}
	virtual void removeChild(Te3DObject2 *toRemove);
	virtual void removeChildren();
	void rotate(const TeQuaternion &rot);
	const TeQuaternion &rotation() {
		return _rotation;
	}
	const TeVector3f32 &scale() const {
		return _scale;
	}
	virtual void setColor(const TeColor &col);
	virtual void setColorInheritance(bool val) {
		_colorInheritance = val;
	}
	virtual bool setName(const Common::String &newName) {
		_name = newName;
		return true;
	}
	virtual void setParent(Te3DObject2 *newparent);  // note, probably should be Te*I*3DObject2 arg
	virtual void setPosition(const TeVector3f32 &pos);
	virtual void setPositionFast(const TeVector3f32 &pos);
	virtual void setRotation(const TeQuaternion &rot);
	virtual void setScale(const TeVector3f32 &newScale);
	virtual void setSize(const TeVector3f32 &newSize);
	void setVisible(bool visible);
	virtual void setZPosition(float zpos);
	virtual TeVector3f32 size() {
		return _size;
	}
	TeMatrix4x4 transformationMatrix();
	virtual void translate(const TeVector3f32 &vec);
	virtual void updateZ() {};
	virtual bool visible() const {
		return _visible;
	}

	TeColor worldColor();
	virtual TeVector3f32 worldPosition();
	TeQuaternion worldRotation();
	TeVector3f32 worldScale();
	virtual TeMatrix4x4 worldTransformationMatrix();
	virtual bool worldVisible();
	virtual float xSize() { return _size.x(); };
	virtual float ySize() { return _size.y(); };
	virtual float zSize() { return _size.z(); };

	static bool loadAndCheckFourCC(Common::ReadStream &stream, const char *str);
	static Common::String deserializeString(Common::ReadStream &stream);
	static void deserializeVectorArray(Common::ReadStream &stream, Common::Array<TeVector3f32> &dest);
	static void deserializeUintArray(Common::ReadStream &stream, Common::Array<uint> &dest);

protected:
	TeVector3f32 _size;
	TeVector3f32 _position;
	TeQuaternion _rotation;
	TeVector3f32 _scale;

private:
	Common::Array<Te3DObject2 *> _children;
	bool _childListChanged;
	TeColor _color;
	bool _colorInheritance;
	Common::String _name;
	Te3DObject2 *_parent;
	bool _visible;

	TeSignal0Param _childListChangedSignal;
	TeSignal0Param _onWorldVisibleChangedSlotSignal;
	TeSignal0Param _onPositionChangedSignal;
	TeSignal0Param _onSizeChangedSignal;
	TeSignal0Param _onParentWorldColorChangedSignal;
	TeSignal0Param _onParentWorldTransformationMatrixChangedSignal;

	TeICallback0ParamPtr _onWorldVisibleChangedParentCallback;
	TeICallback0ParamPtr _onWorldTransformationMatrixChangedParentCallback;
	TeICallback0ParamPtr _onWorldColorChangedParentCallback;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_3D_OBJECT2_H
