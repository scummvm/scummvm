/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "gui/eval.h"
#include "gui/widget.h"
#include "gui/newgui.h"

#include "graphics/scaler.h"

namespace GUI {

static bool isdelim(char c) {
	return strchr(" ;,+-<>/*%^=()", c) != 0 || c == 9 || c == '\n' || !c;
}

Eval::Eval() {
	loadConstants();
}

Eval::~Eval() {
	_vars.clear();
	_aliases.clear();
}

int Eval::eval(const String &input, const String &section, const String &name, int startpos) {
	int result;

	debug(5, "%s=%s", name.c_str(), input.c_str());

	strncpy(_input, input.c_str(), 256);
	_section = section;
	_name = name;
	_startpos = startpos;

	_pos = 0;

	getToken();

	if (_tokenType == tString)
		return EVAL_STRING_VAR;

	if (!*_token)
		exprError(eBadExpr);

	level2(&result);

	debug(5, "Result: %d", result);

	return result;
}

void Eval::level2(int *result) {
	char op;
	int hold;

	level3(result);

	while ((op = *_token) == '+' || op == '-') {
		getToken();
		level3(&hold);
		arith(op, result, &hold);
	}
}

void Eval::level3(int *result) {
	char op;
	int hold;

	level4(result);

	while ((op = *_token) == '*' || op == '/' || op == '%') {
		getToken();
		level4(&hold);
		arith(op, result, &hold);
	}
}

void Eval::level4(int *result) {
	char op;

	op = 0;
	if ((_tokenType == tDelimiter) && (*_token == '+' || *_token == '-')) {
		op = *_token;
		getToken();
	}

	level5(result);

	if (op)
		unary(op, result);
}

void Eval::level5(int *result) {
	if ((*_token == '(') && (_tokenType == tDelimiter)) {
		getToken();

		level2(result);

		if (*_token != ')')
			exprError(eUnclosedBracket);
		getToken();
	} else {
		primitive(result);
	}
}

void Eval::primitive(int *result) {
	if (*_token == ')')
		exprError(eExtraBracket);

	switch (_tokenType) {
	case tVariable:
		*result = getVar_(_token);
		if (*result == EVAL_UNDEF_VAR)
			exprError(eUndefVar);
		getToken();
		return;
	case tNumber:
		*result = atoi(_token);
		getToken();
		return;
	default:
		exprError(eSyntaxError);
	}
}

void Eval::arith(char op, int *r, int *h) {
	switch (op) {
	case '-':
		*r = *r - *h;
		break;
	case '+':
		*r = *r + *h;
		break;
	case '*':
		*r = *r * *h;
		break;
	case '/':
		*r = (*r) / (*h);
		break;
	case '%':
		*r = (*r) % (*h);
		break;
	}
}

void Eval::unary(char op, int *r) {
	if (op == '-')
		*r = -(*r);
}

void Eval::getToken() {
	char *temp;

	_tokenType = tNone;
	temp = _token;

	if (_input[_pos] == 0) {
		*_token = 0;
		_tokenType = tDelimiter;
		return;
	}
	while (isspace(_input[_pos]))
		_pos++;

	if (_input[_pos] == '"') {
		_pos++;
		while (_input[_pos] != '"' && _input[_pos] != '\n')
			*temp++ = _input[_pos++];

		if (_input[_pos] == '\n')
			exprError(eMissingQuote);

		_pos++;
		*temp = 0;

		_tokenType = tString;
		return;
	}

	if (isdigit(_input[_pos])) {
		while (!isdelim(_input[_pos]))
			*temp++ = _input[_pos++];
		*temp = 0;

		_tokenType = tNumber;
		return;
	}

	if (isalpha(_input[_pos])) {
		while (!isdelim(_input[_pos]))
			*temp++ = _input[_pos++];
		*temp = 0;
		_tokenType = tVariable;
		return;
	}

	if (!_tokenType && isdelim(_input[_pos])) {
		*temp++ = _input[_pos++];
		*temp = 0;
		_tokenType = tDelimiter;
	}
}

void Eval::exprError(EvalErrors err) {
	static const char *errors[] = {
		"Syntax error",
		"Extra ')'",
		"Missing ')'",
		"Bad expression",
		"Undefined variable",
		"Missing '\"'"
	};

	error("%s in section [%s] expression: \"%s\" start is at: %d near token '%s'",
		  errors[err], _section.c_str(), _name.c_str(), _pos + _startpos, _token);
}

struct BuiltinConsts {
	const char *name;
	int value;
};

static const BuiltinConsts builtinConsts[] = {
	{"kButtonWidth", GUI::kButtonWidth},
	{"kButtonHeight", GUI::kButtonHeight},
	{"kSliderWidth", GUI::kSliderWidth},
	{"kSliderHeight", GUI::kSliderHeight},

	{"kBigButtonWidth", GUI::kBigButtonWidth},
	{"kBigButtonHeight", GUI::kBigButtonHeight},
	{"kBigSliderWidth", GUI::kBigSliderWidth},
	{"kBigSliderHeight", GUI::kBigSliderHeight},

	{"kNormalWidgetSize", GUI::kNormalWidgetSize},
	{"kBigWidgetSize", GUI::kBigWidgetSize},

	{"kThumbnailWidth", kThumbnailWidth},

	{"kTextAlignLeft", kTextAlignLeft},
	{"kTextAlignRight", kTextAlignRight},
	{"kTextAlignCenter", kTextAlignCenter},

	{"kFontStyleBold", Theme::kFontStyleBold},
	{"kFontStyleNormal", Theme::kFontStyleNormal},
	{"kFontStyleItalic", Theme::kFontStyleItalic},

	{"kFontStyleFixedBold", Theme::kFontStyleFixedBold},
	{"kFontStyleFixedNormal", Theme::kFontStyleFixedNormal},
	{"kFontStyleFixedItalic", Theme::kFontStyleFixedItalic},

	{"kShadingNone", Theme::kShadingNone},
	{"kShadingDim", Theme::kShadingDim},
	{"kShadingLuminance", Theme::kShadingLuminance},

	{"false", 0},
	{"true", 1},
	{NULL, 0}
};

void Eval::loadConstants() {
	int i;

	for (i = 0; builtinConsts[i].name; i++)
		_vars[builtinConsts[i].name] = builtinConsts[i].value;
}

int Eval::getBuiltinVar(const char *s) {
	if (!strcmp(s, "w"))
		return g_system->getOverlayWidth();

	if (!strcmp(s, "h"))
		return g_system->getOverlayHeight();

	return EVAL_UNDEF_VAR;
}

int Eval::getVar_(const Common::String &s, bool includeAliases) {
	int val;

	val = getBuiltinVar(s.c_str());

	if (val != EVAL_UNDEF_VAR)
		return val;

	const Common::String *var = &s;
	if (includeAliases) {
		AliasesMap::const_iterator itera = _aliases.find(s);
		if (itera != _aliases.end())
			var = &(itera->_value);
	}

	VariablesMap::const_iterator iterv = _vars.find(*var);
	if (iterv != _vars.end())
		return iterv->_value;

	return EVAL_UNDEF_VAR;
}

void Eval::setVar(const String &section, const String &name, const String &value) {
	_vars[name.c_str() + 4] = eval(value, section, name, 0);
}

void Eval::reset() {
	_vars.clear();
	_aliases.clear();
	loadConstants();
}

} // end of namespace GUI
