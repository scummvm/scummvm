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

#include "common/file.h"
#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_lua_gui_lua_callbacks.h"

namespace Tetraedge {

TeLuaGUI::TeLuaGUI() : _loaded(false) {
}

TeButtonLayout *TeLuaGUI::buttonLayout(const Common::String &name) {
	StringMap<TeButtonLayout *>::iterator iter = _buttonLayouts.find(name);
	if (iter != _buttonLayouts.end())
		return iter->_value;
	return nullptr;
}

TeCheckboxLayout *TeLuaGUI::checkboxLayout(const Common::String &name) {
	StringMap<TeCheckboxLayout *>::iterator iter = _checkboxLayouts.find(name);
	if (iter != _checkboxLayouts.end())
		return iter->_value;
	return nullptr;
}

TeClipLayout *TeLuaGUI::clipLayout(const Common::String &name) {
	StringMap<TeClipLayout *>::iterator iter = _clipLayouts.find(name);
	if (iter != _clipLayouts.end())
		return iter->_value;
	return nullptr;
}

TeCurveAnim2<Te3DObject2, TeColor> *TeLuaGUI::colorLinearAnimation(const Common::String &name) {
	StringMap<TeCurveAnim2<Te3DObject2, TeColor> *>::iterator iter = _colorLinearAnimations.find(name);
	if (iter != _colorLinearAnimations.end())
		return iter->_value;
	return nullptr;
}

TeExtendedTextLayout *TeLuaGUI::extendedTextLayout(const Common::String &name) {
	StringMap<TeExtendedTextLayout *>::iterator iter = _extendedTextLayouts.find(name);
	if (iter != _extendedTextLayouts.end())
		return iter->_value;
	return nullptr;
}

TeLayout *TeLuaGUI::layout(const Common::String &name) {
	StringMap<TeLayout *>::iterator iter = _layouts.find(name);
	if (iter != _layouts.end())
		return iter->_value;

	TeLayout *layout;
	layout = buttonLayout(name);
	if (layout)
		return layout;

	layout = spriteLayout(name);
	if (layout)
		return layout;

	layout = dynamic_cast<TeLayout *>(textLayout(name));
	if (layout)
		return layout;

	layout = checkboxLayout(name);
	if (layout)
		return layout;

	layout = listLayout(name);
	if (layout)
		return layout;

	layout = scrollingLayout(name);
	if (layout)
		return layout;

	layout = clipLayout(name);
	if (layout)
		return layout;

	layout = extendedTextLayout(name);
	if (layout)
		return layout;

	return nullptr;
}

TeCurveAnim2<TeLayout, TeVector3f32> *TeLuaGUI::layoutAnchorLinearAnimation(const Common::String &name) {
	return _layoutAnchorLinearAnimations.getVal(name);
}

TeCurveAnim2<TeLayout, TeVector3f32> *TeLuaGUI::layoutPositionLinearAnimation(const Common::String &name) {
	return _layoutPositionLinearAnimations.getVal(name);
}

TeListLayout *TeLuaGUI::listLayout(const Common::String &name) {
	StringMap<TeListLayout *>::iterator iter = _listLayouts.find(name);
	if (iter != _listLayouts.end())
		return iter->_value;
	return nullptr;
}

TeCurveAnim2<TeI3DObject2, TeQuaternion> *TeLuaGUI::rotationLinearAnimation(const Common::String &name) {
	error("TODO: Implement TeLuaGUI::rotationLinearAnimation.");
	return nullptr;
}

TeScrollingLayout *TeLuaGUI::scrollingLayout(const Common::String &name) {
	StringMap<TeScrollingLayout *>::iterator iter = _scrollingLayouts.find(name);
	if (iter != _scrollingLayouts.end())
		return iter->_value;
	return nullptr;
}

TeSpriteLayout *TeLuaGUI::spriteLayout(const Common::String &name) {
	StringMap<TeSpriteLayout *>::iterator iter = _spriteLayouts.find(name);
	if (iter != _spriteLayouts.end())
		return iter->_value;
	return nullptr;
}

TeITextLayout *TeLuaGUI::textLayout(const Common::String &name) {
	StringMap<TeTextLayout *>::iterator iter = _textLayouts.find(name);
	if (iter != _textLayouts.end())
		return iter->_value;
	StringMap<TeExtendedTextLayout *>::iterator iter2 = _extendedTextLayouts.find(name);
	if (iter2 != _extendedTextLayouts.end())
		return iter2->_value;
	return nullptr;
}

TeButtonLayout *TeLuaGUI::buttonLayoutChecked(const Common::String &name) {
	TeButtonLayout *l = buttonLayout(name);
	if (!l) {
		error("No button '%s' in gui data '%s'", name.c_str(),
			  _scriptPath.toString(Common::Path::kNativeSeparator).c_str());
	}
	return l;
}

TeLayout *TeLuaGUI::layoutChecked(const Common::String &name) {
	TeLayout *l = layout(name);
	if (!l) {
		error("No layout '%s' in gui data '%s'", name.c_str(),
			  _scriptPath.toString(Common::Path::kNativeSeparator).c_str());
	}
	return l;
}

TeSpriteLayout *TeLuaGUI::spriteLayoutChecked(const Common::String &name) {
	TeSpriteLayout *l = spriteLayout(name);
	if (!l) {
		error("No sprite layout '%s' in gui data '%s'", name.c_str(),
			  _scriptPath.toString(Common::Path::kNativeSeparator).c_str());
	}
	return l;
}

bool TeLuaGUI::load(const Common::Path &subPath) {
	TeCore *core = g_engine->getCore();
	return load(core->findFile(subPath));
}

bool TeLuaGUI::load(const TetraedgeFSNode &node) {
	unload();
	_scriptPath = node.getPath();
	// Not the same as original, we abstract the search logic a bit.
	_luaContext.setGlobal("Pixel", 0);
	_luaContext.setGlobal("Percent", 1);
	_luaContext.setGlobal("None", 0);
	_luaContext.setGlobal("LetterBox", 1);
	_luaContext.setGlobal("PanScan", 2);
	_luaContext.setGlobal("MultiLine", 0);
	_luaContext.setGlobal("SingleLine", 1);
	_luaContext.setGlobal("Fixed", 0);
	_luaContext.setGlobal("Proportional", 1);
	_luaContext.registerCFunction("TeLayout", layoutBindings);
	_luaContext.registerCFunction("TeListLayout", listLayoutBindings);
	_luaContext.registerCFunction("TeSpriteLayout", spriteLayoutBindings);
	_luaContext.registerCFunction("TeButtonLayout", buttonLayoutBindings);
	_luaContext.registerCFunction("TeCheckboxLayout", checkboxLayoutBindings);
	_luaContext.registerCFunction("TeLayoutPositionLinearAnimation", layoutPositionLinearAnimationBindings);
	_luaContext.registerCFunction("TeLayoutAnchorLinearAnimation", layoutAnchorLinearAnimationBindings);
	_luaContext.registerCFunction("TeTextLayout", textLayoutBindings);
	_luaContext.registerCFunction("TeClipLayout", clipLayoutBindings);
	_luaContext.registerCFunction("TeColorLinearAnimation", colorLinearAnimationBindings);
	_luaContext.registerCFunction("TeRotationLinearAnimation", rotationLinearAnimationBindings);
	_luaContext.registerCFunction("TeScrollingLayout", scrollingLayoutBindings);
	_luaContext.registerCFunction("TeExtendedTextLayout", extendedTextLayoutBindings);
	// TODO: We replaced the video layout from Amerzone with a sprite layout.  Probably
	// works ok?
	_luaContext.registerCFunction("TeVideoLayout", spriteLayoutBindings);
	_luaContext.setInRegistry("__TeLuaGUIThis", this);
	_luaScript.attachToContext(&_luaContext);
	_luaScript.load(node);
	_luaScript.execute();
	_luaScript.unload();
	_loaded = true;
	return true;
}

void TeLuaGUI::unload() {
	for (auto &iter : _layouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_layouts.clear();

	for (auto &iter : _buttonLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_buttonLayouts.clear();

	for (auto &iter : _checkboxLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_checkboxLayouts.clear();

	for (auto &iter : _listLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_listLayouts.clear();

	for (auto &iter : _spriteLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_spriteLayouts.clear();

	for (auto &iter : _textLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_textLayouts.clear();

	for (auto &iter : _scrollingLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_scrollingLayouts.clear();

	for (auto &iter : _clipLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_clipLayouts.clear();

	for (auto &iter : _extendedTextLayouts) {
		iter._value->setVisible(false);
		iter._value->deleteLater();
	}
	_extendedTextLayouts.clear();

	for (auto &iter : _layoutAnchorLinearAnimations) {
		iter._value->stop();
		delete iter._value;
	}
	_layoutAnchorLinearAnimations.clear();

	for (auto &iter : _layoutPositionLinearAnimations) {
		iter._value->stop();
		delete iter._value;
	}
	_layoutPositionLinearAnimations.clear();

	for (auto &iter : _colorLinearAnimations) {
		iter._value->stop();
		delete iter._value;
	}
	_colorLinearAnimations.clear();
	_loaded = false;
}

TeVariant TeLuaGUI::value(const Common::String &globalName) {
	return _luaContext.global(globalName);
}

} // end namespace Tetraedge
