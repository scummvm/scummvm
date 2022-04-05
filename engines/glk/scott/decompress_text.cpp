#include "decompress_text.h"
#include "common/str.h"

namespace Glk {
namespace Scott {

int rotateLeftWithCarry(uint8_t *byte, int lastCarry) {
	int carry = ((*byte & 0x80) > 0);
	*byte = *byte << 1;
	if (lastCarry)
		*byte = *byte | 0x01;
	return carry;
}

int decompressOne(uint8_t *bytes) {
	uint8_t result = 0;
	int carry;
	for (int i = 0; i < 5; i++) {
		carry = 0;
		for (int j = 0; j < 5; j++) {
			carry = rotateLeftWithCarry(bytes + 4 - j, carry);
		}
		rotateLeftWithCarry(&result, carry);
	}
	return result;
}

char *decompressText(uint8_t *source, int stringIndex) {
	// Lookup table
	Common::String alphabet = " abcdefghijklmnopqrstuvwxyz'\x01,.\x00";

	int pos, c, uppercase, i, j;
	uint8_t decompressed[256];
	uint8_t buffer[5];
	int idx = 0;

	// Find the start of the compressed message
	for (i = 0; i < stringIndex; i++) {
		pos = *source;
		pos = pos & 0x7F;
		source += pos;
	};

	uppercase = ((*source & 0x40) == 0); // Test bit 6

	source++;
	do {
		// Get five compressed bytes
		for (i = 0; i < 5; i++) {
			buffer[i] = *source++;
		}
		for (j = 0; j < 8; j++) {
			// Decompress one character:
			int next = decompressOne(buffer);

			c = alphabet[next];

			if (c == 0x01) {
				uppercase = 1;
				c = ' ';
			}

			if (c >= 'a' && uppercase) {
				c = toupper(c);
				uppercase = 0;
			}
			decompressed[idx++] = c;

			if (idx > 255)
				return nullptr;

			if (idx == 255)
				c = 0; // We've gone too far, return

			if (c == 0) {
				char *result = new char[idx];
				memcpy(result, decompressed, idx);
				return result;
			} else if (c == '.' || c == ',') {
				if (c == '.')
					uppercase = 1;
				decompressed[idx++] = ' ';
			}
		}
	} while (idx < 0xff); // Chosen arbitrarily, might be too small
	return nullptr;
}

} // End of namespace Scott
} // End of namespace Glk
