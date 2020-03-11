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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_MISC_ARGS_H
#define ULTIMA8_MISC_ARGS_H

// Handy argument processor. I'm certain the implementation could be better
// but it suffices quite well at the moment.

#include "ultima/ultima8/misc/common_types.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class Args {
public:
	Args()  { };
	~Args() { };

	struct Option {
		Option() : option(""), valuetype(no_type) { };
		// bool
		Option(const char *option_cstr, bool *value, const bool defaultvalue = true)
			: option(option_cstr), _bool_val(value), _bool_default(defaultvalue),
			  valuetype(Option::type_bool) {
			/**_bool_val=_bool_default;*/ /* Odd... looks like the 'default' value for bool, isn't. It's the value you set it to, if the flag is found*/
		};
		// string
		Option(const char *option_cstr, Std::string *value, const char *defaultvalue = 0)
			: option(option_cstr), _str_val(value),
			  _str_default(defaultvalue ? defaultvalue : ""), valuetype(Option::type_str) {
			*_str_val = _str_default;
		};
		// sint
		Option(const char *option_cstr, int32 *value, const int32 defaultvalue = true)
			: option(option_cstr), _sint_val(value), _sint_default(defaultvalue),
			  valuetype(Option::type_sint) {
			*_sint_val = _sint_default;
		};
		// uint
		Option(const char *option_cstr, uint32 *value, const uint32 defaultvalue = true)
			: option(option_cstr), _uint_val(value), _uint_default(defaultvalue),
			  valuetype(Option::type_uint) {
			*_uint_val = _uint_default;
		};

		~Option() { };

		Std::string option;

		bool        *_bool_val;
		Std::string *_str_val;
		int32      *_sint_val;
		uint32      *_uint_val;

		bool        _bool_default;
		Std::string _str_default;
		int32      _sint_default;
		uint32      _uint_default;

		enum { no_type = 0, type_bool, type_str, type_sint, type_uint } valuetype;
	};

	Std::vector<Option> _options;

	// bool
	inline void declare(const char *option_cstr, bool *value, const bool defaultvalue = true) {
		_options.push_back(Option(option_cstr, value, defaultvalue));
	};
	// string
	inline void declare(const char *option_cstr, Std::string *value, const char *defaultvalue = 0) {
		_options.push_back(Option(option_cstr, value, defaultvalue));
	};
	// sint
	inline void declare(const char *option_cstr, int32 *value, const int32 defaultvalue = 0) {
		_options.push_back(Option(option_cstr, value, defaultvalue));
	};
	// uint
	inline void declare(const char *option_cstr, uint32 *value, const uint32 defaultvalue = 0) {
		_options.push_back(Option(option_cstr, value, defaultvalue));
	};

	void process(const int32 argc, const char *const *const argv);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif

