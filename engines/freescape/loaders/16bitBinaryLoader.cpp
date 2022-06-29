//
//  16bitBinaryLoader.cpp
//  Phantasma
//
//  Created by Thomas Harte on 17/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

//#include "freescape/language/parser.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"

#include "freescape/area.h"
#include "freescape/language/16bitDetokeniser.h"
#include "freescape/language/instruction.h"
#include "freescape/language/parser.h"
#include "freescape/loaders/16bitBinaryLoader.h"
#include "freescape/objects/geometricobject.h"
#include "freescape/objects/object.h"

namespace Freescape {

typedef enum {
		First = 0x0000,
		Border = 0x4524,
} ChunkType;

static Object *load16bitObject(Common::SeekableReadStream *file) {
	// get object flags and type
	uint8 objectFlags = file->readByte();
	Object::Type objectType = (Object::Type)file->readByte();

	/*
		Notes to self:

			0 = no flags
			128 = Movable, Animated
			134 = Movable, Animated, Default invis, invis
			6 = Default invis, Invis
			32 = collided
	*/

	// get unknown value
	uint16 skippedShort = file->readUint16BE();
	debug("skippedShort: %d", skippedShort);

	// grab location, size
	Math::Vector3d position, size;
	position.x() = file->readUint16BE();
	position.y() = file->readUint16BE();
	position.z() = file->readUint16BE();
	size.x() = file->readUint16BE();
	size.y() = file->readUint16BE();
	size.z() = file->readUint16BE();

	// object ID
	uint16 objectID = file->readUint16BE();

	// size of object on disk; we've accounted for 20 bytes
	// already so we can subtract that to get the remaining
	// length beyond here
	uint32 byteSizeOfObject = (uint32)(file->readUint16BE() << 1) - 20;

	debug("Object %d ; type %d ; flags %d ; size %d", (int)objectID, (int)objectType, (int)objectFlags, byteSizeOfObject);
	debug("Location: %f, %f, %f", position.x(), position.y(), position.z());
	debug("Size: %f, %f, %f", size.x(), size.y(), size.z());

	switch (objectType) {
	default: {
		// read the appropriate number of colours
		int numberOfColours = GeometricObject::numberOfColoursForObjectOfType(objectType);
		Common::Array<uint8> *colours = new Common::Array<uint8>;
		for (uint8 colour = 0; colour < numberOfColours/2; colour++) {
			uint8 c1 = file->readByte();
			byteSizeOfObject--;
			uint8 c2 = file->readByte();
			byteSizeOfObject--;
			colours->push_back( (c1 & 0x0f) | ((c2 & 0x0f) << 4));
			debug("color[%d] = %d", 2*colour, (c1 & 0x0f) | ((c2 & 0x0f) << 4));
			colours->push_back(c1 >> 4 | c2 & 0xf0);
			debug("color[%d] = %d", 2*colour+1, c1 >> 4 | c2 & 0xf0);
		}

		// read extra vertices if required...
		int numberOfOrdinates = GeometricObject::numberOfOrdinatesForType(objectType);
		debug("number of ordinates %d", numberOfOrdinates);
		Common::Array<uint16> *ordinates = nullptr;

		if (numberOfOrdinates) {
			assert(byteSizeOfObject > 0);
			ordinates = new Common::Array<uint16>;

			for (int ordinate = 0; ordinate < numberOfOrdinates; ordinate++) {
				ordinates->push_back(file->readUint16BE());
				byteSizeOfObject -= 2;
			}
		}

		// grab the object condition, if there is one
		FCLInstructionVector instructions;
		if (byteSizeOfObject > 0) {
			// get the condition
			byte *conditionData = (byte*)malloc(byteSizeOfObject);
			file->read(conditionData, byteSizeOfObject);
			Common::Array<uint8> conditionArray(conditionData, byteSizeOfObject);
			Common::String *conditionSource = detokenise16bitCondition(conditionArray);
		 	debug("Condition: %s", conditionSource->c_str());
			byteSizeOfObject = 0;
		// 	//instructions = getInstructions(conditionSource);
		}

		debug("Skipping %d bytes", byteSizeOfObject);
		file->seek(byteSizeOfObject, SEEK_CUR);
		debug("End of object at %lx", file->pos());

		// create an object
		return new GeometricObject(
			objectType,
			objectID,
			objectFlags,
			position,
			size,
			colours,
			ordinates,
			instructions);
	} break;

	case Object::Entrance:
		debug("Skipping %d bytes", byteSizeOfObject);
		file->seek(byteSizeOfObject, SEEK_CUR);
		debug("End of object at %lx", file->pos());
		return new Entrance(objectID, position, size); // size will be always 0,0,0?
		break;
	case Object::Sensor:
	case Object::Group:
		break;
	}

	// skip whatever we didn't understand
	//cout << "bytes we don't understand:" << endl;
	//int i = 0;
	//int j = 0;
	//for (i = 0; i < byteSizeOfObject/2; i++)
	//	cout << i << stream.get16() << endl;
	file->seek(byteSizeOfObject, SEEK_CUR);
	debug("End of object at %lx", file->pos());

	return nullptr;
}

/*void load16bitInstrument(StreamLoader &stream) {
	uint16 zero = stream.get16();
	assert( zero == 0);
	uint16 type = stream.get16();
	uint16 x = stream.get16();
	uint16 y = stream.get16();
	uint16 length = stream.get16();
	uint16 height = stream.get16();
	stream.get16();
	stream.get16();
	uint16 lb = stream.get16();
	uint16 rt = stream.get16();
	uint16 v = stream.get16();
	uint16 fgcolor = stream.get16();
	uint16 bgcolor = stream.get16();

	stream.get16();
	stream.get16();
	stream.get16();
	stream.get16();
	debug("type %d ; x %d ; y %d ; length %d ; height %d ; lb %d ; rt %d ; variable: %d", type, x, y, length, height, lb, rt, v);
}*/

Area *load16bitArea(Common::SeekableReadStream *file) {
	// the lowest bit of this value seems to indicate
	// horizon on or off; this is as much as I currently know
	uint16 skippedValue = file->readUint16BE();
	uint16 numberOfObjects = file->readUint16BE();
	uint16 areaNumber = file->readUint16BE();

	debug("Area %d", areaNumber);
	debug("Skipped value %d", skippedValue);
	debug("Objects: %d", numberOfObjects);

	// I've yet to decipher this fully
	file->readUint16BE();
	file->readUint16BE();
	file->readUint16BE();

	uint8 skyColor = file->readByte();
	skyColor = (file->readByte() << 4) | skyColor;

	debug("Sky color %x", skyColor);
	uint8 groundColor = file->readByte();
	groundColor = (file->readByte() << 4) | groundColor;
	debug("Ground color %x", groundColor);
	file->seek(14, SEEK_CUR);

	// this is just a complete guess
	/*Common::Array<uint8> palette;
	uint32 i;
	for (i = 0; i < 7*3; i++) {
		uint8 c = stream.get8();
		palette.push_back(c);
		debug("color %d", c);
	}
	stream.get8(); // ????*/
	//for (int paletteEntry = 0; paletteEntry < 22; paletteEntry++) {
	//	uint8 paletteColour = stream.get8() << 2;
	//	debug("Palette colour (?) %x", paletteColour);
	//}

	// we'll need to collate all objects and entrances; it's likely a
	// plain C array would do but maps are safer and the total application
	// cost is going to be negligible regardless
	ObjectMap *objectsByID = new ObjectMap;
	ObjectMap *entrancesByID = new ObjectMap;

	// get the objects or whatever; entrances use a unique numbering
	// system and have the high bit of their IDs set in the original file
	for (uint16 object = 0; object < numberOfObjects; object++) {
		Object *newObject = load16bitObject(file);

		if (newObject) {
			if (newObject->getType() == Object::Entrance) {
				(*entrancesByID)[newObject->getObjectID() & 0x7fff] = newObject;
			} else {
				(*objectsByID)[newObject->getObjectID()] = newObject;
			}
		}
	}

	uint16 numberOfLocalConditions = file->readUint16BE();
	debug("Number of conditions: %d", numberOfLocalConditions);
	for (uint16 localCondition = 0; localCondition < numberOfLocalConditions; localCondition++) {
		// 12 bytes for the name of the condition;
		// we don't care
		file->seek(12, SEEK_CUR);

		// get the length and the data itself, converting from
		// shorts to bytes
		uint32 lengthOfCondition = (uint32)file->readUint16BE() << 1;

		debug("Length of condition: %d", lengthOfCondition);
		if (lengthOfCondition == 0) {
			break;
		}

		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);

		debug("Local condition %d at %lx", localCondition + 1, file->pos());
		debug("%s", detokenise16bitCondition(conditionArray)->c_str());
	}

	return (new Area(areaNumber, objectsByID, entrancesByID, 1, skyColor, groundColor));
}

void FreescapeEngine::load16bitBinary(Common::SeekableReadStream *file) {
	Common::Array<uint8>::size_type baseOffset = 0;

	// check whether this looks like an Amiga game; if so it'll start with AM
	// XOR'd with the repeating byte pattern 0x71, 0xc1 or with the pattern
	// 0x88 0x2c if it's on the ST (though the signature will still be AM)
	uint16 platformID = file->readUint16BE();
	debug("%d", platformID);

	if (
		//		(platformID != 0x4120) && (platformID != 0x5043)
		(platformID == 12428) || (platformID == 51553)) {
		// TODO: record original platform type, so we can decode the palette
		// and possibly the border properly
		debug("Loading an Amiga game");
		//cout << "AMIGA" << endl;

		// TODO
		//streamLoader.setReadMask((platformID >> 8) ^ 'A', (platformID & 0xff) ^ 'M');
	} else {
		debug("Loading a DOS game");
		// find DOS end of file and consume it
		while (!file->eos()) {
			uint8 b = file->readByte();
			if (b == 0x1a)
				break;
		}
		file->readByte();

		// advance to the next two-byte boundary if necessary
		if (file->pos() % 2 > 0)
			file->readByte();

		// skip bytes with meaning unknown
		file->readUint16BE();

		// this brings us to the beginning of the embedded
		// .KIT file, so we'll grab the base offset for
		// finding areas later
		baseOffset = file->pos(); //streamLoader.getFileOffset();

		// check that the next two bytes are "PC", then
		// skip the number that comes after
		if (file->readByte() != 'C' || file->readByte() != 'P')
			error("invalid header");
	}

	// skip an unknown meaning
	file->readUint16BE();

	// start grabbing some of the basics...

	uint16 numberOfAreas = file->readUint16BE();
	uint16 sm = file->readUint16BE();
	debug("Something??: %d", sm); // meaning unknown
	debug("Number of areas: %d", numberOfAreas);

	uint16 windowCentreX = file->readUint16BE();
	uint16 windowCentreY = file->readUint16BE();
	uint16 windowWidth = file->readUint16BE();
	uint16 windowHeight = file->readUint16BE();

	debug("Window centre: (%d, %d)", windowCentreX, windowCentreY);
	debug("Window size: (%d, %d)", windowWidth, windowHeight);

	uint16 scaleX = file->readUint16BE();
	uint16 scaleY = file->readUint16BE();
	uint16 scaleZ = file->readUint16BE();

	debug("Scale %d, %d, %d", scaleX, scaleY, scaleZ);
	uint16 timerReload = file->readUint16BE();

	debug("Timer: every %d 50Hz frames", timerReload);

	uint16 maximumActivationDistance = file->readUint16BE();
	uint16 maximumFallDistance = file->readUint16BE();
	uint16 maximumClimbDistance = file->readUint16BE();

	debug("Maximum activation distance: %d", maximumActivationDistance);
	debug("Maximum fall distance: %d", maximumFallDistance);
	debug("Maximum climb distance: %d", maximumClimbDistance);

	uint16 startArea = file->readUint16BE();
	uint16 startEntrance = file->readUint16BE();

	debug("Start at entrance %d in area %d", startEntrance, startArea);

	uint16 playerHeight = file->readUint16BE();
	uint16 playerStep = file->readUint16BE();
	uint16 playerAngle = file->readUint16BE();

	debug("Height %d, step %d, angle %d", playerHeight, playerStep, playerAngle);

	uint16 startVehicle = file->readUint16BE();
	uint16 executeGlobalCondition = file->readUint16BE();

	debug("Start vehicle %d, execute global condition %d", startVehicle, executeGlobalCondition);
	// I haven't figured out what the next 106
	// bytes mean, so we'll skip them — global objects
	// maybe? Likely not 106 bytes in every file.
	//
	// ADDED: having rediscovered my source for the 8bit
	// file format, could this be shading information by
	// analogy with that?
	/*cout << "global unknown:";
	int i;
	int j;
	for (i = 0; i < 106/2; i++)
		cout << streamLoader.get16() << endl;*/

	file->seek(106, SEEK_CUR);

	// at this point I should properly load the border/key/mouse
	// bindings, but I'm not worried about it for now.
	//
	// Format is:
	//		(left x, top y, right x, bottom y) - all 16 bit
	//		keyboard key as an ASCII character (or zero for none)
	//		mouse button masl; 00 = both, 01 = left, 02 = right
	//
	// So, 10 bytes per binding. Bindings are listed in the order:
	//
	//	move forwards, move backwards, move right, move left, rise,
	//	fall, turn left, turn right, look up, look down, tilt left,
	//	tilt right, face forward, u-turn, change vehicle type,
	//	select this vehicle, quit game, fire, activate object,
	//	centre cursor on/off, save game position, load game position
	//
	// So 35 total. Which means this area takes up 350 bytes.
	//cout << "more global unknown:";
	//for (i = 0; i < 350/2; i++)
	//	cout << streamLoader.get16() << endl;

	file->seek(350, SEEK_CUR);

	// there are then file pointers for every area — these are
	// the number of shorts from the 'PC' tag, so multiply by
	// two for bytes. Each is four bytes
	uint32 *fileOffsetForArea = new uint32[numberOfAreas];
	for (uint16 area = 0; area < numberOfAreas; area++)
		fileOffsetForArea[area] = file->readUint32BE() << 1;

	// now come the global conditions
	uint16 numberOfGlobalConditions = file->readUint16BE();
	for (uint16 globalCondition = 0; globalCondition < numberOfGlobalConditions; globalCondition++) {
		// 12 bytes for the name of the condition;
		// we don't care
		file->seek(12, SEEK_CUR);

		// get the length and the data itself, converting from
		// shorts to bytes
		uint32 lengthOfCondition = (uint32)file->readUint16BE() << 1;

		// get the condition
		byte *conditionData = (byte*)malloc(lengthOfCondition);
		file->read(conditionData, lengthOfCondition);
		Common::Array<uint8> conditionArray(conditionData, lengthOfCondition);

		debug("Global condition %d at %lx", globalCondition + 1, file->pos());
		debug("%s", detokenise16bitCondition(conditionArray)->c_str());
	}

	// grab the areas
	AreaMap *areaMap = new AreaMap;
	for (uint16 area = 0; area < numberOfAreas; area++) {
		debug("Area offset %d", fileOffsetForArea[area]);

		file->seek(fileOffsetForArea[area] + baseOffset);
		Area *newArea = load16bitArea(file);

		if (newArea) {
			(*areaMap)[newArea->getAreaID()] = newArea;
		}
	}
	// TODO
	//load16bitInstrument(streamLoader);

	Common::Array<uint8>::size_type o;
	Common::Array<uint8> *raw_border = nullptr;
	Common::Array<uint8> *raw_palette = nullptr;
	uint16 chunkType = 0;
	uint16 chunkSize = 0;
	uint16 colorNumber = 0;
	debug("End of areas at %lx", file->pos());
	while (!file->eos()) {
		o = file->pos();
		chunkType = file->readUint16BE();
		if (chunkType == First) {
			chunkSize = file->readUint16LE();
			if (chunkSize != 0xac) {
				debug("skip %x", chunkType);
				file->seek(o+2);
			} else {
				debug("First chunk found at %x with size %x", o, chunkSize);
				file->seek(chunkSize-2, SEEK_CUR);
			}
		}
		else if (chunkType == Border) {
			chunkSize = file->readUint16LE();
			debug("Border found at %x with size %x", o, chunkSize);

			if (chunkSize == 320*200 / 4)
				colorNumber = 4; // CGA
			else if (chunkSize == 320*200 / 2)
				colorNumber = 16; // EGA
			else if (chunkSize == 320*200)
				colorNumber = 256; // VGA
			else
				error("Unexpected size of image %d", chunkSize);

			byte *borderData = (byte*)malloc(chunkSize);
			file->read(borderData, chunkSize);
			raw_border = new Common::Array<uint8>(borderData, chunkSize);

			raw_palette = new Common::Array<uint8>();
			debug("Palete follows at %lx", file->pos());
			for (int i = 0; i < colorNumber*3; i++)
				raw_palette->push_back(file->readByte() << 2);

			debug("Palete finishes at %lx", file->pos());
			chunkSize = file->readUint16LE();
			debug("Something else of size %x at %lx??", chunkSize, file->pos());
			file->seek(chunkSize, SEEK_CUR);
		}
		else {
			debug("skip %x", chunkType);
			//chunkSize = streamLoader.rget16();
			//if (chunkSize > 0 && streamLoader.getFileOffset() + chunkSize-2 == 0x73ea)
			//	error("Found at %x!", o);
			//streamLoader.setFileOffset(o+2);
			//error("Unknown chunk %x find at %x with size %x", chunkType, o, chunkSize);
		}
	}

	if (raw_palette) {
		Graphics::PixelBuffer *palette = new Graphics::PixelBuffer(_gfx->_palettePixelFormat, colorNumber, DisposeAfterUse::NO);
		*palette = raw_palette->data();
		_gfx->_palette = palette;
	}

	if (raw_border) {
		Graphics::PixelBuffer *border = new Graphics::PixelBuffer(_gfx->_originalPixelFormat, 320*200, DisposeAfterUse::NO);
		*border = raw_border->data();
		_border = _gfx->convertFromPalette(border);
	}

	delete[] fileOffsetForArea;
	_playerHeight = playerHeight;
	_playerWidth = 32;
	_playerDepth = 32;

	_startArea = startArea;
	_startEntrance = startEntrance;
	_colorNumber = colorNumber;
	_areasByAreaID = areaMap;
	_scale = Math::Vector3d(1, 1, 1);
	_binaryBits = 16;
	//return Binary{16, startArea, areaMap, raw_border, raw_palette, colorNumber};
}

} // namespace Freescape