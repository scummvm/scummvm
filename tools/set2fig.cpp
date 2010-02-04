/* Residual - A 3D game interpreter
*
* Residual is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the AUTHORS
* file distributed with this source distribution.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.

* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*
* $URL$
* $Id$
*
*/

// Utility to dump drawing of the sectors in a set file to an xfig file

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: set2fig mo.set >mo.fig\n");
		exit(1);
	}
	printf("#FIG 3.2\n"
		"Landscape\n"
		"Center\n"
		"Metric\n"
		"A4\n"
		"100.00\n"
		"Single\n"
		"-2\n"
		"1200 2\n");

	FILE *in = fopen(argv[1], "r");
	if (in == NULL) {
		perror(argv[1]);
		exit(1);
	}

	char line[1024];
	do {
		fgets(line, 1024, in);
	} while (strcmp(line, "section: sectors\r\n") != 0);

	int numSectors = -1, numRead = 0;
	do {
		char sectorName[256];
		fscanf(in, " sector %256s", sectorName);

		int id;
		fscanf(in, " ID %d", &id);
		if (numSectors < 0)
			numSectors = id + 1;

		char buf[256];
		int color = 0;
		fscanf(in, " type %256s", buf);
		if (strcmp(buf, "walk") == 0)
			color = 0;
		else if (strcmp(buf, "funnel") == 0)
			color = 0;
		else if (strcmp(buf, "camera") == 0)
			color = 1;
		else if (strcmp(buf, "special") == 0)
			color = 5;
		else if (strcmp(buf, "chernobyl") == 0)
			color = 4;

		int linetype = 0;
		fscanf(in, " default visibility %256s", buf);
		if (strcmp(buf, "visible") == 0)
			linetype = 0;
		else if (strcmp(buf, "invisible") == 0)
			linetype = 1;

		float dummy;
		fscanf(in, " height %f", &dummy);

		int numVertices;
		fscanf(in, " numvertices %d", &numVertices);

		printf("# %s\n2 3 %d 1 %d 7 %d -1 -1 0.000 0 0 -1 0 0 %d\n\t", sectorName, linetype, color, color * 10, numVertices + 1);

		float vertices[300];
		fscanf(in, " vertices:");
		for (int i = 0; i < numVertices; i++) {
			fscanf(in, " %f %f %f", vertices + 3*i, vertices + (3 * i + 1), vertices + (3 * i + 2));
			printf(" %d %d", (int) round(vertices[3 * i] * 1000), -(int)round(vertices[3 * i + 1] * 1000));
		}
		printf(" %d %d\n", (int) round(vertices[0] * 1000), -(int)round(vertices[1] * 1000));

		numRead++;
	} while (numRead < numSectors);
	fclose(in);
}
