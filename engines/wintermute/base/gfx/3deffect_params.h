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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#ifndef WINTERMUTE_3D_EFFECT_PARAMS_H
#define WINTERMUTE_3D_EFFECT_PARAMS_H

#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
class Effect3DParams {
public:

	//////////////////////////////////////////////////////////////////////////
	class Effect3DParam {
	public:

		enum ParamType {
			EP_UNKNOWN,
			EP_STRING,
			EP_FLOAT,
			EP_INT,
			EP_BOOL,
			EP_VECTOR
		};

		Effect3DParam();
		Effect3DParam(const char *paramName);
		~Effect3DParam();

		void setValue(char *val);
		void setValue(int val);
		void setValue(float val);
		void setValue(bool val);
		void setValue(DXVector4 val);

		const char *getParamName() const { return _paramName.c_str(); }

		bool persist(BasePersistenceManager *persistMgr);

	private:
		void setDefaultValues();
		ParamType _type;
		Common::String _paramName;
		bool _initialized;

		Common::String _valString;
		int32 _valInt;
		float _valFloat;
		DXVector4 _valVector;
		bool _valBool;
	};


	//////////////////////////////////////////////////////////////////////////
	Effect3DParams();
	~Effect3DParams();

	bool persist(BasePersistenceManager *persistMgr);
	void clear();
	void setParam(const char *paramName, ScValue *val);
	void setParam(const char *paramName, DXVector4 Val);

private:
	Effect3DParam *getParamByName(const char *paramName);
	BaseArray<Effect3DParam *> _params;
};

} // namespace Wintermute

#endif
