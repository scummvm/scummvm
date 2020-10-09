// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cstdlib>

std::string filenumber;

int writePalette(std::ofstream &output) {
	std::string filename = filenumber + "/palette";
	std::ifstream palette(filename.c_str());
	if (!palette.is_open()) {
		std::cerr << "Couldn't open palette file" << std::endl;
		return 1;
	}
	int input;
	do {
		palette >> input;
		if (palette.eof())
			break;
		output << (char) input;
	} while(true);
	palette.close();
	return 0;
}

int writeSections(std::ofstream &output) {
	std::string filename = filenumber + "/section_info";
	std::ifstream sectionInfo(filename.c_str());
	if (!sectionInfo.is_open()) {
		std::cerr << "Couldn't open section_info file" << std::endl;
		return 0;
	}
	int numSections;
	sectionInfo >> numSections;
	output << (char) numSections;

	int input;
	char *input_byte = (char *) &input;
	for(int i = 0; i < numSections; i++) {
		//x1
		sectionInfo >> input;
		output << (char)input_byte[0];
		output << (char)input_byte[1];
		//x2
		sectionInfo >> input;
		output << (char)input_byte[0];
		output << (char)input_byte[1];
		//y1
		sectionInfo >> input;
		output << (char)input;
		//y2
		sectionInfo >> input;
		output << (char)input;
		//next
		sectionInfo >> input;
		output << (char)input;
		//addressLow
		sectionInfo >> input;
		output << (char)input_byte[0];
		output << (char)input_byte[1];
		//addressHigh
		sectionInfo >> input;
		output << (char)input;
	}
	sectionInfo.close();
	return numSections;
}

int writeClickFields(std::ofstream &output) {
	std::string filename = filenumber + "/clickfield_info";
	std::ifstream clickFieldInfo(filename.c_str());
	if (!clickFieldInfo.is_open()) {
		std::cerr << "Couldn't open clickfield_info file" << std::endl;
		return 1;
	}
	int numClickFields;
	clickFieldInfo >> numClickFields;
	output << (char) numClickFields;

	int input;
	char *input_byte = (char *) &input;
	for(int i = 0; i < numClickFields; i++) {
		//x1
		clickFieldInfo >> input;
		output << (char)input_byte[0];
		output << (char)input_byte[1];
		//x2
		clickFieldInfo >> input;
		output << (char)input_byte[0];
		output << (char)input_byte[1];
		//y1
		clickFieldInfo >> input;
		output << (char)input;
		//y2
		clickFieldInfo >> input;
		output << (char)input;
		//next
		clickFieldInfo >> input;
		output << (char)input;
	}
	clickFieldInfo.close();
	return 0;
}

int writePixelData(std::ofstream &output, int imageNum, int skip) {
	std::ostringstream fileName;
	fileName << filenumber << "/image" << imageNum << ".bmp";
	std::ifstream image(fileName.str().c_str(), std::ios::binary);
	if (!image.is_open()) {
		std::cerr << "Couldn't open " << fileName.str() << " file" << std::endl;
		return 0;
	}

	image.seekg(0, image.end);
	int length = image.tellg();
	length -= skip;
	image.seekg(skip, image.beg);
	char *buf = new char[length];
	image.read(buf, length);
	output.write(buf, length);
	delete[] buf;
	return length;
}

void printHelp() {
	std::cout << "create_image output-prefix file-number bytes-skip" << std::endl << std::endl;
	std::cout << "	output-prefix: String the created file should begin with" << std::endl;
	std::cout << "	file-number: Number of the created file and also name of source folder" << std::endl;
	std::cout << "	bytes-skip: How many bytes to to skip in each .bmp file" << std::endl;
	std::cout << std::endl << "EXAMPLE:" << std::endl
		<< "create_image ms2_data 15 1146" << std::endl
		<< "	creates ms2_data.015 assuming there are the right files inside 015 folder" << std::endl;
}

int main(int argc, char *argv[]) {
	if (argc == 2 && !strcmp(argv[1], "--help")) {
		printHelp();
		return 0;
	}
	if (argc != 4 ) {
		printHelp();
		return 1;
	}
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(3) << argv[2];
	filenumber = oss.str();
	char *ptr;
	int bytesSkip = strtol(argv[3], &ptr, 10);
	std::string oFileName = argv[1];
	oFileName += "." + filenumber;
	std::ofstream output(oFileName.c_str(), std::ios::binary);
	if (!output.is_open()) {
		std::cerr << "Couldn't open output file" << std::endl;
		return 1;
	}
	// size
	// just a place holder, will be filled later
	output << (char) 0x40;
	output << (char) 0x70;
	output << (char) 0x01;
	output << (char) 0x00;
	if (writePalette(output))
		return 1;
	int numImages = writeSections(output);
	if (numImages == 0)
		return 1;
	if (writeClickFields(output))
		return 1;
	// we don't compress the images, so set to 0
	output << (char) 0; //numRepeat
	output << (char) 0; //numZw
	int totalLength = 0;
	int oldLength = 0;
	for (int i = 0; i < numImages; i++) {
		totalLength += writePixelData(output, i, bytesSkip);
		if (oldLength == totalLength) {
			std::cerr << "Error while reading the image number: " << i << std::endl; 
			return 1;
		}
		oldLength = totalLength;
	}

	//reversing the size computation inside the engine
	totalLength += 15;
	totalLength /= 16;
	totalLength -= 0x70;
	int i = (totalLength & 0xf000) >> 12;
	int j = totalLength << 4;
	char *i_p = (char *) &i;
	char *j_p = (char *) &j;

	//writing the size
	output.seekp(0);
	output << j_p[0];
	output << j_p[1];
	output << i_p[0];
	output << i_p[1];
	output.close();
	return 0;
}
