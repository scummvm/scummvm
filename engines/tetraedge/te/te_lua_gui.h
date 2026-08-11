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

#ifndef TETRAEDGE_TE_TE_LUA_GUI_H
#define TETRAEDGE_TE_TE_LUA_GUI_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "tetraedge/te/te_button_layout.h"
#include "tetraedge/te/te_checkbox_layout.h"
#include "tetraedge/te/te_clip_layout.h"
#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_curve_anim2.h"
#include "tetraedge/te/te_extended_text_layout.h"
#include "tetraedge/te/te_i_3d_object2.h"
#include "tetraedge/te/te_i_layout.h"
#include "tetraedge/te/te_i_text_layout.h"
#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_list_layout.h"
#include "tetraedge/te/te_lua_context.h"
#include "tetraedge/te/te_lua_script.h"
#include "tetraedge/te/te_object.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_scrolling_layout.h"
#include "tetraedge/te/te_sprite_layout.h"
#include "tetraedge/te/te_text_layout.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_variant.h"

namespace Tetraedge {

class TeLuaGUI : public TeObject {
public:
	TeLuaGUI();
	virtual ~TeLuaGUI() {
		unload();
	};

	virtual void enter() {};
	virtual void leave() {};

	TeLayout *layout(const Common::String &name);
	TeButtonLayout *buttonLayout(const Common::String &name);
	TeCheckboxLayout *checkboxLayout(const Common::String &name);
	TeClipLayout *clipLayout(const Common::String &name);
	TeCurveAnim2<Te3DObject2, TeColor> *colorLinearAnimation(const Common::String &name);
	TeExtendedTextLayout *extendedTextLayout(const Common::String &name);
	TeCurveAnim2<TeLayout, TeVector3f32> *layoutAnchorLinearAnimation(const Common::String &name);
	TeCurveAnim2<TeLayout, TeVector3f32> *layoutPositionLinearAnimation(const Common::String &name);
	TeListLayout *listLayout(const Common::String &name);
	TeCurveAnim2<TeI3DObject2, TeQuaternion> *rotationLinearAnimation(const Common::String &name);
	TeScrollingLayout *scrollingLayout(const Common::String &name);
	TeSpriteLayout *spriteLayout(const Common::String &name);
	TeITextLayout *textLayout(const Common::String &name);

	// Same as the above functions, but call error() if the result is null.
	TeLayout *layoutChecked(const Common::String &name);
	TeButtonLayout *buttonLayoutChecked(const Common::String &name);
	TeSpriteLayout *spriteLayoutChecked(const Common::String &name);

	bool load(const Common::Path &subPath);
	bool load(const TetraedgeFSNode &node);
	void unload();

	TeVariant value(const Common::String &key);

	template <typename T> using StringMap = Common::HashMap<Common::String, T>;

	StringMap<TeLayout *> &layouts() { return _layouts; }
	StringMap<TeButtonLayout *> &buttonLayouts() { return _buttonLayouts; }
	StringMap<TeCheckboxLayout *> &checkboxLayouts() { return _checkboxLayouts; }
	StringMap<TeListLayout *> &listLayouts() { return _listLayouts; }
	StringMap<TeSpriteLayout *> &spriteLayouts() { return _spriteLayouts; }
	StringMap<TeTextLayout *> &textLayouts() { return _textLayouts; }
	StringMap<TeScrollingLayout *> &scrollingLayouts() { return _scrollingLayouts; }
	StringMap<TeClipLayout *> &clipLayouts() { return _clipLayouts; }
	StringMap<TeExtendedTextLayout *> &extendedTextLayouts() { return _extendedTextLayouts; }
	StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> &layoutAnchorLinearAnimations() { return _layoutAnchorLinearAnimations; }
	StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> &layoutPositionLinearAnimations() { return _layoutPositionLinearAnimations; }
	StringMap<TeCurveAnim2<Te3DObject2, TeColor> *> &colorLinearAnimations() { return _colorLinearAnimations; }

	bool loaded() const { return _loaded; }
	const Common::Path &scriptPath() const { return _scriptPath; }

protected:
	bool _loaded;

private:
	Common::Path _scriptPath;

	TeLuaContext _luaContext;
	TeLuaScript _luaScript;

	StringMap<TeLayout *> _layouts;
	StringMap<TeButtonLayout *> _buttonLayouts;
	StringMap<TeCheckboxLayout *> _checkboxLayouts;
	StringMap<TeListLayout *> _listLayouts;
	StringMap<TeSpriteLayout *> _spriteLayouts;
	StringMap<TeTextLayout *> _textLayouts;
	StringMap<TeScrollingLayout *> _scrollingLayouts;
	StringMap<TeClipLayout *> _clipLayouts;
	StringMap<TeExtendedTextLayout *> _extendedTextLayouts;
	StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> _layoutAnchorLinearAnimations;
	StringMap<TeCurveAnim2<TeLayout, TeVector3f32> *> _layoutPositionLinearAnimations;
	StringMap<TeCurveAnim2<Te3DObject2, TeColor> *> _colorLinearAnimations;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LUA_GUI_H
