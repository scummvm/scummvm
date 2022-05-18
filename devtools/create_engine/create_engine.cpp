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

// Specified engine name with different cases
#define MAX_LINE_LENGTH 256
char engineUppercase[MAX_LINE_LENGTH];
char engineCamelcase[MAX_LINE_LENGTH];
char engineLowercase[MAX_LINE_LENGTH];

// List of files to be copied to create engine
static const char *const FILENAMES[] = {
	"configure.engine", "console.cpp", "console.h",
	"credits.pl", "detection.cpp", "detection.h",
	"detection_tables.h", "metaengine.cpp",
	"metaengine.h", "module.mk", "xyzzy.cpp",
	"xyzzy.h", nullptr
};
const char *const ENGINES = "create_project ..\\.. --use-canonical-lib-names --msvc\n";

// Replaces any occurances of the xyzzy placeholder with
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
void process_file(const char *filename) {
	char srcFilename[128], destFilename[128];
	sprintf(srcFilename, "files/%s", filename);
	if (!strncmp(filename, "xyzzy.", 6))
		sprintf(destFilename, "../../engines/%s/%s.%s",
			engineLowercase, engineLowercase, filename + 6);
	else
		sprintf(destFilename, "../../engines/%s/%s",
			engineLowercase, filename);

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

	fclose(in);
	fclose(out);
}

// For Visual Studio convenience, creates a copy of the
// create_msvc.bat to <engine>.bat that allows creating
// the ScummVM solution with just that engine enabled
void create_batch_file() {
	FILE *in, *out;
	char line[MAX_LINE_LENGTH];

	if (!(in = fopen("../../dists/msvc/create_msvc.bat", "r"))) {
		printf("Could not open create_msvc.bat\n");
		exit(0);
	}

	char destFilename[MAX_LINE_LENGTH];
	sprintf(destFilename, "../../dists/msvc/%s.bat", engineLowercase);
	if (!(out = fopen(destFilename, "w"))) {
		printf("Could not create %s.bat\n", engineLowercase);
		exit(0);
	}

	// Get each line until there are none left
	while (fgets(line, MAX_LINE_LENGTH, in)) {
		if (!strcmp(line, ENGINES)) {
			sprintf(line + strlen(line) - 1,
				" --disable-all-engines --enable-engine=%s\n",
				engineLowercase);
		}

		// Write out the line
		fputs(line, out);
	}

	fclose(in);
	fclose(out);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Please specify engine name as a parameter\n");
		return 0;
	}

	// Set up different cased engine names
	for (size_t i = 0; i < strlen(argv[1]) + 1; ++i) {
		engineUppercase[i] = toupper(argv[1][i]);
		engineLowercase[i] = tolower(argv[1][i]);
		engineCamelcase[i] = (i == 0) ?
			engineUppercase[i] : engineLowercase[i];
	}

	// Create a directory for the new engine
	char folder[MAX_LINE_LENGTH];
	sprintf(folder, "../../engines/%s", engineLowercase);
	if (mkdir(folder, 0755)) {
		printf("Could not create engine folder.\n");
		return 0;
	}

	// Process the files
	for (const char *const *filename = FILENAMES; *filename; ++filename)
		process_file(*filename);

	create_batch_file();

	printf("Engine generation complete.\n");
	return 0;
}
