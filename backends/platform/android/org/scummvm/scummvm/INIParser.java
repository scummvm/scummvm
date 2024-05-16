package org.scummvm.scummvm;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
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

	public static File getPath(Map<String, Map<String, String>> ini, String section, String key, File defaultValue) {
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
		// Path components are escaped and puny encoded, undo this
		File path = new File(""); // Create an abstract empty path
		for(String component : value.split("/")) {
			component = decodePathComponent(component);
			path = new File(path, component);
		}
		return path;
	}

	private static String decodePathComponent(String component) {
		if (!component.startsWith("xn--")) {
			return component;
		}

		String decoded = punycodeDecode(component);
		if (component == decoded) {
			return component;
		}

		StringBuilder result = new StringBuilder(decoded);
		int i = result.indexOf("\u0081");
		while(i != -1 && i + 1 < result.length()) {
			char c = decoded.charAt(i + 1);
			if (c != 0x79) {
				result.setCharAt(i, (char)(c - 0x80));
			}
			result.deleteCharAt(i + 1);
			i = result.indexOf("\u0081", i + 1);
		}
		return result.toString();
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

	// punycode parameters, see https://datatracker.ietf.org/doc/html/rfc3492#section-5
	private static final int BASE         = 36;
	private static final int TMIN         = 1;
	private static final int TMAX         = 26;
	private static final int SKEW         = 38;
	private static final int DAMP         = 700;
	private static final int INITIAL_N    = 0x80;
	private static final int INITIAL_BIAS = 72;
	private static final int SMAX         = 2147483647; // maximum Unicode code point

	private static String punycodeDecode(String src) {
		// Check for prefix
		if (!src.startsWith("xn--")) {
			return src;
		}

		// Check if it is ASCII
		for (int i = 0; i < src.length(); i++) {
			int c = src.charAt(i);
			if (c > 0x7F) {
				return src;
			}
		}

		src = src.substring(4);

		int tail = src.length();
		int startInsert = src.lastIndexOf('-', tail) + 1;
		while(true) {
			// Check the insertions string and chop off invalid characters
			int i;
			for(i = startInsert; i < tail; i++) {
				char c = src.charAt(i);
				if (!((c >= '0' && c <= '9') ||
					(c >= 'a' && c <= 'z') ||
					(c >= 'A' && c <= 'Z'))) {
					break;
				}
			}
			if (i == tail) {
				// All good
				break;
			}
			if (src.charAt(i) == '.') {
				// Assume it's an extension, stop there
				tail = i;
				break;
			}

			if (startInsert == 0) {
				// That was all invalid
				return src;
			}

			// Look for previous dash
			tail = startInsert;
			startInsert = src.lastIndexOf('-', tail) + 1;
			// Check again
		}

		// Do punycode work
		StringBuilder dest = new StringBuilder(src.substring(0, startInsert > 0 ? startInsert - 1 : 0));

		int di = dest.length();
		int i = 0;
		int n = INITIAL_N;
		int bias = INITIAL_BIAS;

		for (int si = startInsert; si < tail; di++) {
			int org_i = i;

			for (int w = 1, k = BASE; true; k += BASE) {
				if (si >= tail) {
					Log.w(LOG_TAG, "punycode_decode: incorrect digit for string: " + src);
					return src;
				}

				int digit = decodeDigit(src.charAt(si));
				si++;

				if (digit == SMAX) {
					Log.w(LOG_TAG, "punycode_decode: incorrect digit2 for string: " + src);
					return src;
				}

				if (digit > (SMAX - i) / w) {
					// OVERFLOW
					Log.w(LOG_TAG, "punycode_decode: overflow1 for string: " + src);
					return src;
				}

				i += digit * w;
				int t;

				if (k <= bias) {
					t = TMIN;
				} else if (k >= bias + TMAX) {
					t = TMAX;
				} else {
					t = k - bias;
				}

				if (digit < t) {
					break;
				}

				if (w > SMAX / (BASE - t)) {
					// OVERFLOW
					Log.w(LOG_TAG, "punycode_decode: overflow2 for string: "+ src);
					return src;
				}

				w *= BASE - t;
			}

			bias = adaptBias(i - org_i, di + 1, org_i == 0);

			if (i / (di + 1) > SMAX - n) {
				// OVERFLOW
				Log.w(LOG_TAG, "punycode_decode: overflow3 for string: " + src);
				return src;
			}

			n += i / (di + 1);
			i %= (di + 1);

			dest.insert(i, Character.toChars(n));
			i++;
		}

		// Re-add tail
		dest.append(src.substring(tail));
		return dest.toString();
	}

	private static int decodeDigit(char c) {
		if (c >= '0' && c <= '9') {
			return 26 + c - '0';
		} else if (c >= 'a' && c <= 'z') {
			return c - 'a';
		} else if (c >= 'A' && c <= 'Z') {
			return c - 'A';
		} else {
			return SMAX;
		}
	}

	private static int adaptBias(int delta, int nPoints, boolean isFirst) {
		int k;

		delta /= isFirst ? DAMP : 2;
		delta += delta / nPoints;

		// while delta > 455: delta /= 35
		for (k = 0; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE) {
			delta /= (BASE - TMIN);
		}

		return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
	}
}
