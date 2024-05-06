package org.scummvm.scummvm;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.Reader;
import java.util.HashMap;
import java.util.Map;

/**
 * INI file parser modeled after config manager one in C++ side
 */
public class INIParser {
	private final static String LOG_TAG = "INIParser";

	// This class can not be instantiated
	private INIParser() {
	}

	public static Map<String, Map<String, String>> parse(Reader reader) throws IOException {
		Map<String, Map<String, String>> ret = new HashMap<>();
		BufferedReader lineReader = new BufferedReader(reader);
		Map<String, String> domain = null;
		int lineno = 0;
		String line;

		while ((line = lineReader.readLine()) != null) {
			lineno++;

			if (lineno == 1 && line.startsWith("\357\273\277")) {
				line = line.substring(3);
			}

			if (line.isEmpty()) {
				continue;
			}

			final char firstChar = line.charAt(0);

			/* Unlike C++ parser, we ignore comments for simplicity */
			if (firstChar == '#') {
				continue;
			}

			if (firstChar == '[') {
				int i;
				for(i = 1; i < line.length(); i++) {
					final char c = line.charAt(i);
					if (c > 127) {
						break;
					}
					if (!Character.isLetterOrDigit(c) && c != '-' && c != '_') {
						break;
					}
				}

				if (i == line.length()) {
					return null;
				}
				if (line.charAt(i) != ']') {
					return null;
				}

				String domainName = line.substring(1, i);
				domain = new HashMap<>();
				ret.put(domainName, domain);

				continue;
			}

			int i;
			for (i = 0; i < line.length(); i++) {
				final char c = line.charAt(i);
				if (!isSpace(c)) {
					break;
				}
			}

			if (i == line.length()) {
				continue;
			}

			if (domain == null) {
				return null;
			}

			int equal = line.indexOf('=');
			if (equal == -1) {
				return null;
			}

			String key = line.substring(i, equal);
			String value = line.substring(equal + 1);

			key = trim(key);
			value = trim(value);

			domain.put(key, value);
		}

		return ret;
	}

	public static String get(Map<String, Map<String, String>> ini, String section, String key, String defaultValue) {
		if (ini == null) {
			return defaultValue;
		}
		Map<String, String> s = ini.get(section);
		if (s == null) {
			return defaultValue;
		}
		String value = s.get(key);
		if (value == null) {
			return defaultValue;
		}
		return value;
	}

	/* Java isWhitespace is more inclusive than C one */
	private static boolean isSpace(char c) {
		return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\013');
	}

	/* Java trim is more strict than C one */
	private static String trim(String s) {
		int begin, end;
		for(begin = 0; begin < s.length(); begin++) {
			if (!isSpace(s.charAt(begin))) {
				break;
			}
		}
		for(end = s.length() - 1; end > begin; end--) {
			if (!isSpace(s.charAt(end))) {
				break;
			}
		}
		return s.substring(begin, end + 1);
	}
}
