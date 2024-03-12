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

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define MAX_LINE_LENGTH 256

// Specified engine name with different cases
#define MAX_ENGINE_NAME_LENGTH 64
char engineUppercase[MAX_ENGINE_NAME_LENGTH];
char engineCamelcase[MAX_ENGINE_NAME_LENGTH];
char engineLowercase[MAX_ENGINE_NAME_LENGTH];

// List of files to be copied to create engine
static const char *const FILENAMES[] = {
	"files/configure.engine", "files/console.cpp", "files/console.h",
	"files/credits.pl", "files/detection.cpp", "files/detection.h",
	"files/detection_tables.h", "files/metaengine.cpp",
	"files/metaengine.h", "files/module.mk", "files/xyzzy.cpp",
	"files/xyzzy.h", "files/POTFILES", nullptr
};

static const char *const FILENAMES_EVENTS[] = {
	"files/configure.engine", "files/console.cpp", "files/console.h",
	"files/credits.pl", "files/detection.cpp", "files/detection.h",
	"files/detection_tables.h",
	"files_events/events.cpp", "files_events/events.h",
	"files_events/messages.cpp", "files_events/messages.h",
	"files/metaengine.cpp", "files/metaengine.h",
	"files_events/module.mk", "files_events/xyzzy.cpp",
	"files_events/xyzzy.h", "files/POTFILES",
	"files_events/views.h", "files_events/view1.cpp",
	"files_events/view1.h", nullptr
};

const char *const ENGINES = "create_project ..\\.. --msvc\n";

bool fileExists(const char *name) {
#ifdef _WIN32
	return (GetFileAttributesA(name) != INVALID_FILE_ATTRIBUTES);
#else
	return (!access(name, F_OK));
#endif
}

bool createDirectory(const char *name) {
#ifdef _WIN32
	return CreateDirectoryA(name, nullptr);
#else
	return (!mkdir(name, 0755));
#endif
}

// Replaces any occurrences of the xyzzy placeholder with
// whatever engine name was specified
void replace_placeholders(char line[MAX_LINE_LENGTH]) {
	char buf[MAX_LINE_LENGTH];
	char *pos;

	while ((pos = strstr(line, "XYZZY"))) {
		*pos = '\0';
		snprintf(buf, MAX_LINE_LENGTH, "%s%s%s", line,
			engineUppercase, pos + 5);
		strncpy(line, buf, MAX_LINE_LENGTH);
	}

	while ((pos = strstr(line, "Xyzzy"))) {
		*pos = '\0';
		snprintf(buf, MAX_LINE_LENGTH, "%s%s%s", line,
			engineCamelcase, pos + 5);
		strncpy(line, buf, MAX_LINE_LENGTH);
	}

	while ((pos = strstr(line, "xyzzy"))) {
		*pos = '\0';
		snprintf(buf, MAX_LINE_LENGTH, "%s%s%s", line,
			engineLowercase, pos + 5);
		strncpy(line, buf, MAX_LINE_LENGTH);
	}
}

// Loops through copying and processing a single file
void process_file(FILE *in, FILE *out) {
	char line[MAX_LINE_LENGTH] = { 0 };

	// Get each line until there are none left
	while (fgets(line, MAX_LINE_LENGTH, in)) {
		// Do any replacements of engine name
		replace_placeholders(line);

		// Write out the line
		fputs(line, out);
	}
}

// Copies and processes the specified file
void process_file(const char *filePath, const char *prefix, const char *prefix2) {
	char srcFilename[MAX_LINE_LENGTH], destFilename[MAX_LINE_LENGTH];
	const char *filename = strchr(filePath, '/') + 1;

	snprintf(srcFilename, MAX_LINE_LENGTH, "%s/%s", prefix2, filePath);
	if (!strncmp(filename, "xyzzy.", 6))
		snprintf(destFilename, MAX_LINE_LENGTH, "%s/engines/%s/%s.%s",
			prefix, engineLowercase, engineLowercase, filename + 6);
	else
		snprintf(destFilename, MAX_LINE_LENGTH, "%s/engines/%s/%s",
			prefix, engineLowercase, filename);

	printf("Creating file %s...", destFilename);
	fflush(stdout);

	FILE *in, *out;
	if (!(in = fopen(srcFilename, "r"))) {
		printf("Could not locate file - %s\n", srcFilename);
		exit(0);
	}

	if (!(out = fopen(destFilename, "w"))) {
		printf("Could not create file - %s\n", destFilename);
		exit(0);
	}

	process_file(in, out);

	printf("done\n");

	fclose(in);
	fclose(out);
}

// For Visual Studio convenience, creates a copy of the
// create_msvc.bat to <engine>.bat that allows creating
// the ScummVM solution with just that engine enabled
void create_batch_file(const char *prefix) {
	FILE *in, *out;
	char line[MAX_LINE_LENGTH];
	char destFilename[MAX_LINE_LENGTH];

	snprintf(destFilename, MAX_LINE_LENGTH, "%s/dists/msvc/create_msvc.bat", prefix);
	if (!(in = fopen(destFilename, "r"))) {
		printf("Could not open create_msvc.bat\n");
		exit(0);
	}

	snprintf(destFilename, MAX_LINE_LENGTH, "%s/dists/msvc/%s.bat", prefix, engineLowercase);

	printf("Creating file %s...", destFilename);
	fflush(stdout);

	if (!(out = fopen(destFilename, "w"))) {
		printf("Could not create %s.bat\n", engineLowercase);
		exit(0);
	}

	// Get each line until there are none left
	while (fgets(line, MAX_LINE_LENGTH, in)) {
		if (!strcmp(line, ENGINES)) {
			snprintf(line + strlen(line) - 1, MAX_LINE_LENGTH - strlen(line) + 1,
				" --disable-all-engines --disable-detection-full --enable-engine=%s\n",
				engineLowercase);
		}

		// Write out the line
		fputs(line, out);
	}

	printf("done\n");

	fclose(in);
	fclose(out);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Please specify engine name as a parameter\n");
		printf("With an optional -events to create an events based engine.\n");
		return 0;
	}

	bool isEvents = (argc == 3) && !strcmp(argv[2], "-events");

	// Set up different cased engine names
	for (size_t i = 0; i < strlen(argv[1]) + 1; ++i) {
		engineUppercase[i] = toupper(argv[1][i]);
		engineLowercase[i] = tolower(argv[1][i]);
		engineCamelcase[i] = (i == 0) ?
			engineUppercase[i] : engineLowercase[i];
	}

	char prefix[100];
	char prefix2[100];
	if (fileExists("../../engines")) {
		strcpy(prefix, "../..");
		strcpy(prefix2, ".");
	} else if (fileExists("engines")) {
		strcpy(prefix, ".");
		strcpy(prefix2, "devtools/create_engine");
	} else {
		printf("Cound not locate engines directory. Run from the scummvm source root directory\n");
		return 0;
	}


	// Create a directory for the new engine
	char folder[MAX_LINE_LENGTH];
	snprintf(folder, MAX_LINE_LENGTH, "%s/engines/%s", prefix, engineLowercase);

	printf("Creating directory %s...", folder);
	fflush(stdout);

	if (!createDirectory(folder)) {
		printf("Could not create engine folder.\n");
		perror(folder);
		return 0;
	}
	printf("done\n");

	// Process the files
	const char *const *filenames = isEvents ? FILENAMES_EVENTS : FILENAMES;
	for (const char *const *filename = filenames; *filename; ++filename)
		process_file(*filename, prefix, prefix2);

	create_batch_file(prefix);

	printf("Engine generation complete.\n");
	return 0;
}
