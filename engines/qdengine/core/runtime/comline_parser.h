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

#ifndef QDENGINE_CORE_RUNTIME_COMLINE_PARSER_H
#define QDENGINE_CORE_RUNTIME_COMLINE_PARSER_H


namespace QDEngine {

class comlineParser {
public:
	comlineParser();
	~comlineParser();

	bool register_option(const char *name, int id);

	void parse_comline(int argc, char **argv);

	static bool is_option(const char *arg_str) {
		if (arg_str[0] == '-' || arg_str[0] == '/') return true;
		return false;
	}

	bool has_argument(int id) const;
	const char *argument_string(int id) const;

private:

	struct comlineArgument {
		const char *data_;
		int optionID_;

		comlineArgument(const char *data, int id) : data_(data), optionID_(id) { }
		comlineArgument() : data_(NULL), optionID_(-1) { }

		bool operator == (int opt_id) const {
			return (optionID_ == opt_id);
		}
	};

	struct comlineOption {
		std::string name_;
		int ID_;

		comlineOption(const char *name, int id) : name_(name), ID_(id) { }

		bool operator == (const char *str) const {
			if (!str) return false;
			return strcasecmp(name_.c_str(), str);
		}
	};

	typedef std::vector<comlineArgument> arguments_container_t;
	arguments_container_t arguments_;

	typedef std::list<comlineOption> options_container_t;
	options_container_t options_;
};

} // namespace QDEngine

#endif // QDENGINE_CORE_RUNTIME_COMLINE_PARSER_H
