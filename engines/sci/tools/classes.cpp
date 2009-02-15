#include <stdio.h>
#include <stdlib.h>

#include <engine.h>
#include <vocabulary.h>

int main(int argc, char** argv) {
	int res;
	int sizes[1000];
	int altsizes[1000];
	int count, *classes;
	loadResources(SCI_VERSION_AUTODETECT, 1);

	for (res = 0; res < 1000; res++) {
		resource_t* r;
		int i = 0;

		sizes[res] = -1;

		if ((r = findResource(sci_script, res)) == 0) continue;
		sizes[res] = 0;
		altsizes[res] = 0;
		i += 2;
		i = getInt16(r->data + i);

		while (i < r->length - 2) {
			switch (getInt16(r->data + i)) {
			case 1:
			case 6:
				sizes[res]++;
				break;
			default:
				altsizes[res]++;
			}
			i += getInt16(r->data + i + 2);
		}
		fflush(stdout);
	}

	for (res = 0; res < 1000; res++) if (sizes[res] != -1) printf("%03d %d\n", res, sizes[res]);
	printf("\n");

	classes = vocabulary_get_classes(&count);
	for (res = 0; res < count; res++) {
		printf("%03d %d (%d)\n", classes[res],
		       sizes[classes[res]]--, altsizes[classes[res]]);
	}

	freeResources();
	return 0;
}
