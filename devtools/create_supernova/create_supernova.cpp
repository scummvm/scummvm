#include "create_supernova.h"
#include "gametext.h"
#include "file.h"
#include "po_parser.h"

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

// List of languages to look for. To add new languages you only need to change the array below
// and add the supporting files:
//   - 640x480 bitmap picture for the newpaper named 'img1-##.pbm' and 'img2-##.pbm'
//     in pbm binary format (you can use gimp to generate those)
//   - strings in a po file named 'strings-##.po' that uses CP850 encoding

const char *lang[] = {
	"en",
	NULL
};

void writeImage(File& outputFile, const char *name, const char* language) {
	File imgFile;
	char fileName[16];
	sprintf(fileName, "%s-%s.pbm", name, language);
	if (!imgFile.open(fileName, kFileReadMode)) {
		printf("Cannot find image '%s' for language '%s'. This image will be skipped.\n", name, language);
		return;
	}

	char str[256];

	// Read header (and check we have a binary PBM file)
	imgFile.readString(str, 256);
	if (strcmp(str, "P4") != 0) {
		imgFile.close();
		printf("File '%s' doesn't seem to be a binary pbm file! This image will be skipped.\n", fileName);
		return;
	}

	// Skip comments and then read and check size
	do {
		imgFile.readString(str, 256);
	} while (str[0] == '#');
	int w = 0, h = 0;
	if (sscanf(str, "%d %d", &w, &h) != 2 || w != 640 || h != 480) {
		imgFile.close();
		printf("Binary pbm file '%s' doesn't have the expected size (expected: 640x480, read: %dx%d). This image will be skipped.\n", fileName, w, h);
		return;
	}

	// Write block header in output file (4 bytes).
	// We convert the image name to upper case.
	for (int i = 0 ; i < 4 ; ++i) {
		if (name[i] >= 97 && name[i] <= 122)
			outputFile.writeByte(name[i] - 32);
		else
			outputFile.writeByte(name[i]);
	}
	// And write the language code on 4 bytes as well (padded with 0 if needed).
	int languageLength = strlen(language);
	for (int i = 0 ; i < 4 ; ++i) {
		if (i < languageLength)
			outputFile.writeByte(language[i]);
		else
			outputFile.writeByte(0);
	}

	// Write block size (640*480 / 8)
	outputFile.writeLong(38400);

	// Write all the bytes. We should have 38400 bytes (640 * 480 / 8)
	// However we need to invert the bits has the engine expects 1 for the background and 0 for the text (black)
	// but pbm uses 0 for white and 1 for black.
	for (int i = 0 ; i < 38400 ; ++i) {
		byte b = imgFile.readByte();
		outputFile.writeByte(~b);
	}

	imgFile.close();
}

void writeGermanStrings(File& outputFile) {
	// Write header and language
	outputFile.write("TEXT", 4);
	outputFile.write("de\0\0", 4);

	// Reserve the size for the block size, but we will write it at the end once we know what it is.
	uint32 blockSizePos = outputFile.pos();
	uint32 blockSize = 0;
	outputFile.writeLong(blockSize);

	// Write all the strings
	const char **s = &gameText[0];
	while (*s) {
		outputFile.writeString(*s);
		blockSize += strlen(*s) + 1;
		++s;
	}

	// Now write the block size and then go back to the end of the file.
	outputFile.seek(blockSizePos, SEEK_SET);
	outputFile.writeLong(blockSize);
	outputFile.seek(0, SEEK_END);
}

void writeStrings(File& outputFile, const char* language) {
	char fileName[16];
	sprintf(fileName, "strings-%s.po", language);
	PoMessageList* poList = parsePoFile(fileName);
	if (!poList) {
		printf("Cannot find strings file for language '%s'.\n", language);
		return;
	}

	// Write block header
	outputFile.write("TEXT", 4);

	// And write the language code on 4 bytes as well (padded with 0 if needed).
	int languageLength = strlen(language);
	for (int i = 0 ; i < 4 ; ++i) {
		if (i < languageLength)
			outputFile.writeByte(language[i]);
		else
			outputFile.writeByte(0);
	}

	// Reserve the size for the block size, but we will write it at the end once we know what it is.
	uint32 blockSizePos = outputFile.pos();
	uint32 blockSize = 0;
	outputFile.writeLong(blockSize);

	// Write all the strings.
	// If a string is not translated we use the German one.
	const char **s = &gameText[0];
	while (*s) {
		const char* translation = poList->findTranslation(*s);
		if (translation) {
			outputFile.writeString(translation);
			blockSize += strlen(translation) + 1;
		} else {
			outputFile.writeString(*s);
			blockSize += strlen(*s) + 1;
		}
		++s;
	}
	delete poList;

	// Now write the block size and then go back to the end of the file.
	outputFile.seek(blockSizePos, SEEK_SET);
	outputFile.writeLong(blockSize);
	outputFile.seek(0, SEEK_END);
}


/**
 * Main method
 */
int main(int argc, char *argv[]) {
	File outputFile;
	if (!outputFile.open("supernova.dat", kFileWriteMode)) {
		printf("Cannot create file 'supernova.dat' in current directory.\n");
		exit(0);
	}

	// The generated file is of the form:
	// 3 bytes: 'MSN'
	// 1 byte:  version
	// -- data blocks
	// 4 bytes: header  'IMG1' and 'IMG2' for newspaper images (for file 1 and file 2 respectively),
	//                  'TEXT' for strings
	// 4 bytes: language code ('en\0', 'de\0'- see common/language.cpp)
	// 4 bytes: block size n (uint32)
	// n bytes: data
	// ---

	// Header
	outputFile.write("MSN", 3);
	outputFile.writeByte(VERSION);

	// German strings
	writeGermanStrings(outputFile);

	// Other languages
	const char **l = &lang[0];
	while(*l) {
		writeImage(outputFile, "img1", *l);
		writeImage(outputFile, "img2", *l);
		writeStrings(outputFile, *l);
		++l;
	}

	outputFile.close();
	return 0;
}
