#include "create_supernova2.h"
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
void writeDatafile(File& outputFile, int fileNumber, const char* language) {
	File dataFile;
	char fileName[20];
	sprintf(fileName, "ms2_data.%03d-%s", fileNumber, language);
	if (!dataFile.open(fileName, kFileReadMode)) {
		printf("Cannot find dataFile 'ms2_data.%3d' for language '%s'. This file will be skipped.\n", fileNumber, language);
		return;
	}

	// Write block header in output file (4 bytes).
	// M(fileNumber) for example M015
	char number[4];
	sprintf(number, "%03d", fileNumber);
	outputFile.writeByte('M');
	for (int i = 0 ; i < 3 ; ++i) {
			outputFile.writeByte(number[i]);
	}
	// And write the language code on 4 bytes as well (padded with 0 if needed).
	int languageLength = strlen(language);
	for (int i = 0 ; i < 4 ; ++i) {
		if (i < languageLength)
			outputFile.writeByte(language[i]);
		else
			outputFile.writeByte(0);
	}

	// Write block size
	
	dataFile.seek(0, SEEK_END);
	int length = dataFile.pos();
	dataFile.seek(0, SEEK_SET);
	outputFile.writeLong(length);

	// Write all the bytes. We should have w * h / 8 bytes
	// However we need to invert the bits has the engine expects 1 for the background and 0 for the text (black)
	// but pbm uses 0 for white and 1 for black.
	for (int i = 0 ; i < length; ++i) {
		byte b = dataFile.readByte();
		outputFile.writeByte(b);
	}

	dataFile.close();
}

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
	// See http://netpbm.sourceforge.net/doc/pbm.html
	// Header is in the form:
	// - A "magic number" for identifying the file type ("P4" for binary pdm)
	// - Whitespace (blanks, TABs, CRs, LFs).
	// - The width in pixels of the image, formatted as ASCII characters in decimal.
	// - Whitespace.
	// - The height in pixels of the image, again in ASCII decimal.
	// - A single whitespace character (usually a newline).
	// - The raster data.
	// Before the whitespace character that delimits the raster, any characters from a "#"
	// through the next carriage return or newline character, is a comment and is ignored.
	// Note that the comment can starts in the middle of a line. Note also that if you have
	// a comment right before the raster, the newline at the end of the comment is not
	// sufficient to delimit the raster.

	int w = 0, h = 0;
	enum PbmState { PbmMagic, PbmWidth, PbmHeight};
	PbmState state = PbmMagic;
	int i = 0;
	do {
		char c = (char)imgFile.readByte();
		if (c == '#') {
			do {
				c = (char)imgFile.readByte();
			} while (c != '\r' && c != '\n' && !imgFile.eof());
			// If the comment is after the height, we need to read one more character
			// before the raster data begin.
			if (state == PbmHeight && i > 0)
				c = (char)imgFile.readByte();
		}
		if (isspace(c)) {
			if (i > 0) {
				str[i] = 0;
				i = 0;
				if (state == PbmMagic) {
					if (strcmp(str, "P4") != 0) {
						imgFile.close();
						printf("File '%s' doesn't seem to be a binary pbm file! This image will be skipped.\n", fileName);
						return;
					}
				} else {
					int *s = state == PbmWidth ? &w : &h;
					if (sscanf(str, "%d", s) != 1) {
						imgFile.close();
						printf("Failed to read image size in binary pbm file '%s'. This image will be skipped.\n", fileName);
						return;
					}
				}
				if (state == PbmMagic)
					state = PbmWidth;
				else if (state == PbmWidth)
					state = PbmHeight;
				else {
					// We have finished reading the header.
					// Check the size is as expected.
					if ((w != 640 || h != 480) && (w != 320 || h != 200)) {
						imgFile.close();
						printf("Binary pbm file '%s' doesn't have the expected size (expected: 640x480 or 320x200, read: %dx%d). This image will be skipped.\n", fileName, w, h);
						return;
					}
					// And break out of the loop.
					break;
				}
			}
		} else
			str[i++] = c;
		if (imgFile.eof()) {
			printf("Unexpected end of file in '%s' while reading pbm header! This image will be skipped.\n", fileName);
			return;
		}
	} while (1);

	// Write block header in output file (4 bytes).
	// We convert the image name to upper case.
	for (i = 0 ; i < 4 ; ++i) {
		if (name[i] >= 97 && name[i] <= 122)
			outputFile.writeByte(name[i] - 32);
		else
			outputFile.writeByte(name[i]);
	}
	// And write the language code on 4 bytes as well (padded with 0 if needed).
	int languageLength = strlen(language);
	for (i = 0 ; i < 4 ; ++i) {
		if (i < languageLength)
			outputFile.writeByte(language[i]);
		else
			outputFile.writeByte(0);
	}

	// Write block size
	outputFile.writeLong(w * h / 8);

	// Write all the bytes. We should have w * h / 8 bytes
	// However we need to invert the bits has the engine expects 1 for the background and 0 for the text (black)
	// but pbm uses 0 for white and 1 for black.
	for (i = 0 ; i < w * h / 8 ; ++i) {
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
	if (!outputFile.open("supernova2.dat", kFileWriteMode)) {
		printf("Cannot create file 'supernova2.dat' in current directory.\n");
		exit(0);
	}

	// The generated file is of the form:
	// 3 bytes: 'MS2'
	// 1 byte:  version
	// -- data blocks
	// 4 bytes: header  'IMG1' cyphered text image
	//                  'TEXT' for strings
	// 4 bytes: language code ('en\0', 'de\0'- see common/language.cpp)
	// 4 bytes: block size n (uint32)
	// n bytes: data
	// ---

	// Header
	outputFile.write("MS2", 3);
	outputFile.writeByte(VERSION);

	// German strings
	writeGermanStrings(outputFile);

	// TODO make the needed images and reenable writing them to the .dat file
	// Other languages
	const char **l = &lang[0];
	while(*l) {
		writeDatafile(outputFile, 15, *l);
		writeImage(outputFile, "img1", *l);
	//	writeImage(outputFile, "img2", *l);
		writeStrings(outputFile, *l);
		++l;
	}

	outputFile.close();
	return 0;
}
