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

#include "glk/quest/geas_util.h"
#include "glk/quest/geas_file.h"
#include "glk/quest/streams.h"
#include "glk/quest/string.h"

namespace Glk {
namespace Quest {

int eval_int(String s) {
	cerr << "eval_int (" << s << ")" << endl;

	uint index = 0, index2;
	String tmp;
	while (index < s.length() && Common::isSpace(s[index])) {
		cerr << "  index == " << index << endl;
		index ++;
	}
	if (index == s.length() || !Common::isDigit(s[index])) {
		cerr << "Failed to match, returning 0" << endl;
		return 0;
	}
	for (index2 = index; index2 < s.length() && Common::isDigit(s[index2]); index2 ++) {
		cerr << "  index2 == " << index2 << endl;
	}
	//;
	tmp = s.substr(index, index2 - index);
	cerr << "tmp == < " << tmp << ">" << endl;

	//cerr << "index == " << index << ", index2 == " << index2
	//     << ", tmp == " << tmp << endl;

	int arg1 = atoi(tmp.c_str());
	cerr << "arg1 == " << arg1 << endl;
	index = index2;
	while (index < s.length() && Common::isSpace(s[index]))
		++ index;
	if (index == s.length())
		return arg1;

	//cerr << "index == " << index << ", s.length() == " << s.length() << endl;

	char symbol = s[index];

	//cerr << "symbol == " << symbol << "; find --> "
	//     << String("+-*/").find (symbol) << endl;

	if (String("+-*/").find(symbol) == (int)String::npos)
		return arg1;

	++ index;
	while (index < s.length() && Common::isSpace(s[index]))
		++ index;
	if (index == s.length() || ! Common::isDigit(s[index])) {
		if (symbol == '*')
			return 0;
		return arg1;
	}
	index2 = index + 1;
	while (index2 < s.length() && Common::isDigit(s[index2]))
		++ index2;
	tmp = s.substr(index, index2 - index);
	int arg2 = atoi(tmp.c_str());

	switch (symbol) {
	case '+':
		return arg1 + arg2;
	case '-':
		return arg1 - arg2;
	case '*':
		return arg1 * arg2;
	case '/':
		return arg1 / arg2;
		// TODO: division should use accountant's round
	default:
		break;
	}
	return 0;
}

String trim_braces(String s) {
	if (s.length() > 1 && s[0] == '[' && s[s.length() - 1] == ']')
		return s.substr(1, s.length() - 2);
	else
		return s;
}

bool is_param(String s) {
	return s.length() > 1 && s[0] == '<' && s[s.length() - 1] == '>';
}

String param_contents(String s) {
	//cerr << "param_contents (" << s << ")" << endl;
	assert(is_param(s));
	return s.substr(1, s.length() - 2);
}

String nonparam(String type, String var) {
	return "Non-parameter for " + type + " in \"" + var + "\"";
}

//ostream &operator << (ostream &o, const GeasBlock &gb) { return o; }
//String trim (String s, trim_modes) { return s; }

String string_geas_block(const GeasBlock &gb) {
	ostringstream oss;
	oss << gb;  // temporary removed TODO
	return oss.str();
}


bool starts_with(String a, String b) {
	return (a.length() >= b.length()) && (a.substr(0, b.length()) == b);
}
bool ends_with(String a, String b) {
	return (a.length() >= b.length()) &&
	       (a.substr(a.length() - b.length(), b.length()) == b);
}

bool starts_with_i(String a, String b) {
	return (a.length() >= b.length()) && ci_equal(a.substr(0, b.length()), b);
	//  return starts_with (lcase(a), lcase(b));
}
bool ends_with_i(String a, String b) {
	return (a.length() >= b.length()) &&
	       ci_equal(a.substr(a.length() - b.length(), b.length()), b);
	//return ends_with (lcase(a), lcase(b));
}

String pcase(String s) {
	if (s.length() == 0)
		return s;
	if (Common::isLower(s[0]))
		s[0] = toupper(s[0]);
	return s;
}

String ucase(String s) {
	for (uint i = 0; i < s.length(); i ++)
		s[i] = toupper(s[i]);
	return s;
}

// There's a good chance s is already all-lowercase, in which case
// the test will avoid making a copy
String lcase(String s) {
	for (uint i = 0; i < s.length(); i ++)
		if (Common::isUpper(s[i]))
			s[i] = tolower(s[i]);
	return s;
}

Common::Array<String> split_param(String s) {
	Common::Array<String> rv;
	int c1 = 0, c2;

	for (;;) {
		c2 = s.find(';', c1);
		if (c2 == -1) {
			rv.push_back(s.substr(c1).trim());
			return rv;
		}
		rv.push_back(s.substr(c1, c2 - c1).trim());
		c1 = c2 + 1;
	}
}

Common::Array<String> split_f_args(String s) {
	Common::Array<String> rv = split_param(s);
	for (uint i = 0; i < rv.size(); i ++) {
		String tmp = rv[i];
		if (tmp[0] == '_')
			rv[i][0] = ' ';
		if (tmp[tmp.length() - 1] == '_')
			rv[i][tmp.length() - 1] = ' ';
	}
	return rv;
}

void show_split(String s) {
	Common::Array<String> tmp = split_param(s);
	cerr << "Splitting <" << s << ">: ";
	for (uint i = 0; i < tmp.size(); i ++)
		cerr << "<" << tmp[i] << ">, ";
	cerr << "\n";
}

Logger::Nullstreambuf Logger::cnull;

Logger::Logger() : logfilestr_(NULL) { //, cerrbuf_(NULL) {
/*
	cerr.flush();

	const char *const logfile = getenv("GEAS_LOGFILE");
	if (logfile) {
		ofstream *filestr = new ofstream(logfile);
		if (filestr->fail())
			delete filestr;
		else {
			logfilestr_ = filestr;
			cerrbuf_ = cerr.rdbuf(filestr->rdbuf());
		}
	}

	if (!cerrbuf_)
		cerrbuf_ = cerr.rdbuf(&cnull);
		*/
}

Logger::~Logger() {
	/*
	cerr.flush();

	cerr.rdbuf(cerrbuf_);
	cerrbuf_ = NULL;

	if (logfilestr_) {
		logfilestr_->close();
		delete logfilestr_;
		logfilestr_ = NULL;
	}
	*/
}

} // End of namespace Quest
} // End of namespace Glk
