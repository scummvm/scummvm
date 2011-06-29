/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SOURCE_LINE_LENGTH	256
#define MAX_TOKEN_STRING_LENGTH MAX_SOURCE_LINE_LENGTH
#define MAX_DIGIT_COUNT			20
#define MAX_SYMBOLS				1024
#define MAX_SUBROUTINES			1024
#define MAX_SUBROUTINE_SIZE		4096
#define MAX_SUBROUTINE_JUMPS	256

#define	OPSIZE8		0x40	///< when this bit is set - the operand size is 8 bits
#define	OPSIZE16	0x80	///< when this bit is set - the operand size is 16 bits
#define	OPSIZE32	0x00	///< when no bits are set - the operand size is 32 bits

#define VERSION					1

enum CharCode {
	LETTER, DIGIT, SPECIAL, EOF_CODE, EOL_CODE
};

enum TokenCode {
	NO_TOKEN, WORD, NUMBER, IDENTIFIER, END_OF_FILE, END_OF_LINE,
	RW_DEFINE, RW_COLON, RW_SUB, RW_END, RW_OPCODE,
	ERROR
};

enum LiteralType {
	INTEGER_LIT
};

struct Literal {
	LiteralType type;
	union {
		int integer;
	} value;
};

struct SymbolEntry {
	char symbol[MAX_TOKEN_STRING_LENGTH];
	char value[MAX_TOKEN_STRING_LENGTH];
};

struct SubEntry {
	char name[MAX_TOKEN_STRING_LENGTH];
	int fileOffset;
};

struct JumpSource {
	char name[MAX_TOKEN_STRING_LENGTH];
	int line_number;
	int offset;
};

struct JumpDest {
	char name[MAX_TOKEN_STRING_LENGTH];
	int offset;
};

enum Opcodes {
	OP_HALT	= 0, OP_IMM	= 1, OP_ZERO =  2, OP_ONE = 3, OP_MINUSONE = 4, OP_STR = 5, OP_DLOAD = 6,
	OP_DSTORE = 7, OP_PAL = 8, OP_LOAD = 9, OP_GLOAD = 10, OP_STORE = 11, OP_GSTORE = 12,
	OP_CALL = 13, OP_LIBCALL = 14, OP_RET = 15, OP_ALLOC = 16, OP_JUMP = 17, OP_JMPFALSE = 18,
	OP_JMPTRUE = 19, OP_EQUAL = 20, OP_LESS = 21, OP_LEQUAL = 22, OP_NEQUAL = 23, OP_GEQUAL = 24,
	OP_GREAT = 25, OP_PLUS = 26, OP_MINUS = 27, OP_LOR = 28, OP_MULT = 29, OP_DIV = 30,
	OP_MOD = 31, OP_AND = 32, OP_OR = 33, OP_EOR = 34, OP_LAND = 35, OP_NOT = 36, OP_COMP = 37,
	OP_NEG = 38, OP_DUP = 39,
	TOTAL_OPCODES = 40
};

typedef unsigned char byte;

const unsigned char EOF_CHAR = (unsigned char)255;
const unsigned char EOL_CHAR = (unsigned char)254;

/*----------------------------------------------------------------------*/
/*  Reserved words tables												*/
/*----------------------------------------------------------------------*/

enum OpcodeParamType {OP_NO_PARAM, OP_IMM_PARAM, OP_TRANSFER_PARAM};

struct OpcodeEntry {
	const char *str;
	OpcodeParamType paramType;
};

OpcodeEntry OpcodeList[OP_DUP + 1] = {
	{"HALT", OP_NO_PARAM}, {"IMM", OP_IMM_PARAM}, {"ZERO", OP_NO_PARAM}, {"ONE", OP_NO_PARAM},
	{"MINUSONE", OP_NO_PARAM}, {"STR", OP_IMM_PARAM}, {"DLOAD", OP_IMM_PARAM}, {"DSTORE", OP_IMM_PARAM},
	{"PAL", OP_IMM_PARAM}, {"LOAD", OP_IMM_PARAM}, {"GLOAD", OP_IMM_PARAM}, {"STORE", OP_IMM_PARAM},
	{"GSTORE", OP_IMM_PARAM}, {"CALL", OP_IMM_PARAM}, {"LIBCALL", OP_IMM_PARAM}, {"RET", OP_NO_PARAM},
	{"ALLOC", OP_IMM_PARAM}, {"JUMP", OP_TRANSFER_PARAM}, {"JMPFALSE", OP_TRANSFER_PARAM},
	{"JMPTRUE", OP_TRANSFER_PARAM}, {"EQUAL", OP_NO_PARAM}, {"LESS", OP_NO_PARAM},
	{"LEQUAL", OP_NO_PARAM}, {"NEQUAL", OP_NO_PARAM}, {"GEQUAL", OP_NO_PARAM},
	{"GREAT", OP_NO_PARAM}, {"PLUS", OP_NO_PARAM}, {"MINUS", OP_NO_PARAM},
	{"LOR", OP_NO_PARAM}, {"MULT", OP_NO_PARAM}, {"DIV", OP_IMM_PARAM}, {"MOD", OP_NO_PARAM},
	{"AND", OP_NO_PARAM}, {"OR", OP_NO_PARAM}, {"EOR", OP_NO_PARAM}, {"LAND", OP_NO_PARAM},
	{"NOT", OP_NO_PARAM}, {"COMP", OP_NO_PARAM}, {"NEG", OP_NO_PARAM}, {"DUP", OP_NO_PARAM}
};


const char *symbol_strings[] = {"#DEFINE", ":", "SUB", "END"};

/*----------------------------------------------------------------------*/
/*  Globals																*/
/*----------------------------------------------------------------------*/

unsigned char	ch;					// Current input character
TokenCode		token;				// code of current token
Opcodes			opcode;				// Current instruction opcode
OpcodeParamType	paramType;			// Parameter type opcode expects
Literal			literal;			// Value of literal
int				buffer_offset;		// Char offset into source buffer
int				level = 0;			// current nesting level
int				line_number = 0;	// current line number

char source_buffer[MAX_SOURCE_LINE_LENGTH];		// Source file buffer
char token_string[MAX_TOKEN_STRING_LENGTH];		// Token string
const char *bufferp = source_buffer;					// Source buffer ptr
char *tokenp = token_string;					// Token string ptr

int		digit_count;				// Total no. of digits in number
bool	count_error;				// Too many digits in number?

FILE *source_file;
FILE *dest_file;
CharCode char_table[256];

SymbolEntry		symbolTable[MAX_SYMBOLS];
int symbolCount = 0;

int			game_number = 0;
int			language = 0;

int			indexSize = 0;
int			fileOffset = 0;
SubEntry	subroutinesTable[MAX_SUBROUTINES];
int			subroutinesCount = 0;

byte		subroutineData[MAX_SUBROUTINE_SIZE];
int			subroutineSize = 0;

JumpSource	jumpSources[MAX_SUBROUTINE_JUMPS];
int			jumpSourceCount = 0;
JumpDest	jumpDests[MAX_SUBROUTINE_JUMPS];
int			jumpDestCount = 0;

#define char_code(ch)	char_table[ch]

void get_char();
void get_token();

/*----------------------------------------------------------------------*/
/*  Miscellaneous support functions										*/
/*----------------------------------------------------------------------*/

void strToUpper(char *string) {
	while (*string) {
		*string = toupper(*string);
		++string;
	}
}

void strToLower(char *string) {
	while (*string) {
		*string = tolower(*string);
		++string;
	}
}

int strToInt(const char *s) {
	unsigned int tmp;

	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) == 'H')
		// Hexadecimal string with trailing 'h'
		sscanf(s, "%xh", &tmp);
	else if (*s == '$')
		// Hexadecimal string starting with '$'
		sscanf(s + 1, "%x", &tmp);
	else
		// Standard decimal string
		return atoi(s);

	return (int)tmp;
}

/*----------------------------------------------------------------------*/
/*  Initialisation / De-initialisation code								*/
/*----------------------------------------------------------------------*/

/**
 * Open the input file for parsing
 */
void open_source_file(const char *name) {
	if ((source_file = fopen(name, "r")) == NULL) {
		printf("*** Error:  Failed to open source file.\n");
		exit(0);
	}

	// Fetch the first character
	bufferp = "";
	get_char();
}

/**
 * Close the source file
 */
void close_source_file() {
	fclose(source_file);
}

/**
 * Initializes the scanner
 */
void init_scanner(const char *name) {
	// Initialize character table
	for (int i = 0;   i < 256;  ++i) char_table[i] = SPECIAL;
	for (int i = '0'; i <= '9'; ++i) char_table[i] = DIGIT;
	for (int i = 'A'; i <= 'Z'; ++i) char_table[i] = LETTER;
	for (int i = 'a'; i <= 'z'; ++i) char_table[i] = LETTER;
	char_table[EOF_CHAR] = EOF_CODE;
	char_table[EOL_CHAR] = EOL_CODE;
	char_table[(int)'$'] = DIGIT;	// Needed for hexadecimal number handling

	open_source_file(name);
}

/**
 * Shuts down the scanner
 */
void quit_scanner() {
	close_source_file();
}

/*----------------------------------------------------------------------*/
/*  Output routines														*/
/*----------------------------------------------------------------------*/


/**
 * Initializes the output
 */
void init_output(const char *destFilename) {
	dest_file = fopen(destFilename, "wb");
	if (dest_file == NULL) {
		printf("Could not open file for writing\n");
		exit(0);
	}
}

/**
 * Closes the output file
 */
void close_output() {
	fclose(dest_file);
}

/**
 * Writes a single byte to the output
 */
void write_byte(byte v) {
	fwrite(&v, 1, 1, dest_file);
	++fileOffset;
}

/**
 * Writes a word to the output
 */
void write_word(int v) {
	write_byte(v & 0xff);
	write_byte((v >> 8) & 0xff);
}

/**
 * Writes a 32-bit value to the output
 */
void write_long(int v) {
	write_byte(v & 0xff);
	write_byte((v >> 8) & 0xff);
	write_byte((v >> 16) & 0xff);
	write_byte((v >> 24) & 0xff);
}

/**
 * Writes a sequence of bytes to the output
 */
void write_bytes(byte *v, int len) {
	fwrite(v, 1, len, dest_file);
	fileOffset += len;
}

/**
 * Writes a repeat sequence of a value to the output
 */
void write_byte_seq(byte v, int len) {
	byte *tempData = (byte *)malloc(len);
	memset(tempData, v, len);
	write_bytes(tempData, len);
	free(tempData);
}

/**
 * Writes out the header and allocates space for the symbol table
 */
void write_header() {
	// Write out three bytes - game Id, language Id, and version number
	if (game_number == 0) {
		game_number = 1;
		printf("No game specified, defaulting to Rex Nebular\n");
	}
	write_byte(game_number);

	if (language == 0) {
		language = 1;
		printf("No language specified, defaulting to English\n");
	}
	write_byte(language);

	write_byte(VERSION);

	// Write out space to later come back and store the list of subroutine names and offsets
	if (indexSize == 0) {
		indexSize = 4096;
		printf("No index size specified, defaulting to %d bytes\n", indexSize);
	}
	write_byte_seq(0, indexSize - 3);

	fileOffset = indexSize;
}

/**
 * Goes back and writes out the subroutine list
 */
void write_index() {
	fseek(dest_file, 3, SEEK_SET);

	int bytesRemaining = indexSize - 3;
	for (int i = 0; i < subroutinesCount; ++i) {
		int entrySize = strlen(subroutinesTable[i].name) + 5;

		// Ensure there is enough remaining space
		if ((bytesRemaining - entrySize) < 0) {
			printf("Index has exceeded allowable size.\n");
			token = ERROR;
		}

		// Write out the name and the file offset
		write_bytes((byte *)&subroutinesTable[i].name, strlen(subroutinesTable[i].name) + 1);
		write_long(subroutinesTable[i].fileOffset);
	}
}

/*----------------------------------------------------------------------*/
/*  Processing routines													*/
/*----------------------------------------------------------------------*/

int symbolFind() {
	for (int i = 0; i < symbolCount; ++i) {
		if (!strcmp(symbolTable[i].symbol, token_string))
			return i;
	}
	return -1;
}

int subIndexOf() {
	for (int i = 0; i < subroutinesCount; ++i) {
		if (!strcmp(subroutinesTable[i].name, token_string))
			return i;
	}
	return -1;
}

int jumpIndexOf(const char *name) {
	for (int i = 0; i < jumpDestCount; ++i) {
		if (!strcmp(jumpDests[i].name, name))
			return i;
	}
	return -1;
}

void handle_define() {
	// Read the variable name
	get_token();
	if (token != IDENTIFIER) {
		token = ERROR;
		return;
	}

	// Make sure it doesn't already exist
	if (symbolFind() != -1) {
		printf("Duplicate symbol encountered.\n");
		token = ERROR;
		return;
	}

	// Store the new symbol name
	strcpy(symbolTable[symbolCount].symbol, token_string);

	// Get the value
	get_token();
	if (token == END_OF_LINE) {
		printf("Unexpected end of line.\n");
		token = ERROR;
	}
	if ((token != NUMBER) && (token != IDENTIFIER)) {
		printf("Invalid define value.\n");
		token = ERROR;
	}
	if (token == ERROR)
		return;

	// Handle special symbols
	if (!strcmp(symbolTable[symbolCount].symbol, "GAME_ID")) {
		// Specify game number
		if (!strcmp(token_string, "REX"))
			game_number = 1;
		else
			token = ERROR;
	} else if (!strcmp(symbolTable[symbolCount].symbol, "LANGUAGE")) {
		// Specify the language
		if (!strcmp(token_string, "ENGLISH"))
			language = 1;
		else
			token = ERROR;
	} else if (!strcmp(symbolTable[symbolCount].symbol, "INDEX_BLOCK_SIZE")) {
		// Specifying the size of the index
		indexSize = strToInt(token_string);
	} else {
		// Standard symbol - save it's value
		strcpy(symbolTable[symbolCount].value, token_string);
		++symbolCount;
	}

	if (token == ERROR)
		return;

	// Ensure the next symbol is the end of line
	get_token();
	if (token != END_OF_LINE) {
		printf("Extraneous information on line.\n");
		token = ERROR;
	}
}

/**
 * Handles getting a parameter for an opcode
 */
void get_parameter() {
	int nvalue;

	if (token == NUMBER) {
		literal.value.integer	= strToInt(token_string);
		return;
	}

	if (token != IDENTIFIER)
		return;

	nvalue = symbolFind();
	if (nvalue != -1) {
		// Found symbol, so get it's numeric value and return
		token = NUMBER;
		literal.value.integer	= strToInt(symbolTable[nvalue].value);
		return;
	}

	// Check if the parameter is the name of an already processed subroutine
	strToLower(token_string);
	nvalue = subIndexOf();
	if (nvalue == -1) {
		token = ERROR;
		return;
	}

	// Store the index (not the offset) of the subroutine to call
	token = NUMBER;
	literal.value.integer = nvalue;
}

#define INC_SUB_PTR if (++subroutineSize == MAX_SUBROUTINE_SIZE) { \
		printf("Maximum allowable subroutine size exceeded\n"); \
		token = ERROR; \
		return; \
	}

#define WRITE_SUB_BYTE(v) subroutineData[subroutineSize] = (byte)(v)

/**
 * Handles a single instruction within the sub-routine
 */
void handle_instruction() {
	// Write out the opcode
	WRITE_SUB_BYTE(opcode);
	INC_SUB_PTR;

	get_token();

	if (OpcodeList[opcode].paramType == OP_IMM_PARAM) {
		get_parameter();

		if (token != NUMBER) {
			printf("Incorrect opcode parameter encountered\n");
			token = ERROR;
			return;
		}

		// Apply the correct opcode size to the previously stored opcode and save the byte(s)
		if (literal.value.integer <= 0xff) {
			subroutineData[subroutineSize - 1] |= OPSIZE8;
			WRITE_SUB_BYTE(literal.value.integer);
			INC_SUB_PTR;
		} else if (literal.value.integer <= 0xffff) {
			subroutineData[subroutineSize - 1] |= OPSIZE16;
			WRITE_SUB_BYTE(literal.value.integer);
			INC_SUB_PTR;
			WRITE_SUB_BYTE(literal.value.integer >> 8);
			INC_SUB_PTR;

		} else {
			subroutineData[subroutineSize - 1] |= OPSIZE32;
			int v = literal.value.integer;
			for (int i = 0; i < 4; ++i, v >>= 8) {
				WRITE_SUB_BYTE(v);
				INC_SUB_PTR;
			}
		}

		get_token();
	} else if (OpcodeList[opcode].paramType == OP_TRANSFER_PARAM) {

		if (token != IDENTIFIER) {
			printf("Incorrect opcode parameter encountered\n");
			token = ERROR;
			return;
		}

		// Check to see if it's a backward jump to an existing label
		int idx = jumpIndexOf(token_string);
		if (idx != -1) {
			// It's a backwards jump whose destination is already known
			if (jumpDests[idx].offset < 256) {
				// 8-bit destination
				subroutineData[subroutineSize - 1] |= OPSIZE8;
				subroutineData[subroutineSize] = jumpDests[idx].offset;
				INC_SUB_PTR;
			} else {
				// 16-bit destination
				subroutineData[subroutineSize - 1] |= OPSIZE16;
				INC_SUB_PTR;
				subroutineData[subroutineSize] = jumpDests[idx].offset & 0xff;
				INC_SUB_PTR;
				subroutineData[subroutineSize] = (jumpDests[idx].offset >> 8) & 0xff;
			}
		} else {
			// Unknown destination, so save it for later resolving
			strcpy(jumpSources[jumpSourceCount].name, token_string);
			jumpSources[jumpSourceCount].line_number = line_number;
			jumpSources[jumpSourceCount].offset = subroutineSize;
			if (++jumpSourceCount == MAX_SUBROUTINE_JUMPS) {
				printf("Maximum allowable jumps size exceeded\n");
				token = ERROR;
				return;
			}

			// Store a 16-bit placeholder
			subroutineData[subroutineSize - 1] |= OPSIZE16;
			WRITE_SUB_BYTE(0);
			INC_SUB_PTR;
			WRITE_SUB_BYTE(0);
			INC_SUB_PTR;
		}

		get_token();
	}

	if (token != END_OF_LINE)
		token = ERROR;
}

/**
 * Called at the end of the sub-routine, fixes the destination of any forward jump references
 */
void fix_subroutine_jumps() {
	for (int i = 0; i < jumpSourceCount; ++i) {
		// Scan through the list of transfer destinations within the script
		int idx = jumpIndexOf(jumpSources[i].name);
		if (idx == -1) {
			token = ERROR;
			line_number = jumpSources[i].line_number;
			return;
		}

		// Replace the placeholder bytes with the new destination
		subroutineData[jumpSources[i].offset] = jumpDests[idx].offset & 0xff;
		subroutineData[jumpSources[i].offset + 1] = (jumpDests[idx].offset >> 8) & 0xff;
	}
}

/**
 * Handles parsing a sub-routine
 */
void handle_sub() {
	// Get the subroutine name
	get_token();
	if (token != IDENTIFIER) {
		printf("Missing subroutine name.\n");
		token = ERROR;
		return;
	}

	strToLower(token_string);
	if (subIndexOf() != -1) {
		printf("Duplicate sub-routine encountered\n");
		token = ERROR;
		return;
	}

	// If this is the first subroutine, start writing out the data
	if (subroutinesCount == 0)
		write_header();

	// Save the sub-routine details
	strcpy(subroutinesTable[subroutinesCount].name, token_string);
	subroutinesTable[subroutinesCount].fileOffset = fileOffset;
	if (++subroutinesCount == MAX_SUBROUTINES) {
		printf("Exceeded maximum allowed subroutine count\n");
		token = ERROR;
		return;
	}

	// Ensure the line end
	get_token();
	if (token != END_OF_LINE) {
		token = ERROR;
		return;
	}

	// Initial processing arrays
	memset(subroutineData, 0, MAX_SUBROUTINE_SIZE);
	subroutineSize = 0;
	jumpSourceCount = 0;
	jumpDestCount = 0;

	// Loop through the lines of the sub-routine
	while (token != ERROR) {
		get_token();

		if (token == END_OF_LINE) continue;
		if (token == RW_OPCODE) {
			// Handle instructions
			handle_instruction();

		} else if (token == IDENTIFIER) {
			// Save identifier, it's hopefully a jump symbol
			strcpy(jumpDests[jumpDestCount].name, token_string);
			get_token();
			if (token != RW_COLON)
				token = ERROR;
			else {
				// Save the jump point
				jumpDests[jumpDestCount].offset = subroutineSize;

				if (++jumpDestCount == MAX_SUBROUTINE_JUMPS) {
					printf("Subroutine exceeded maximum allowable jump points\n");
					token = ERROR;
					return;
				}

				// Ensure it's the last value on the line
				get_token();
				if (token != END_OF_LINE)
					token = ERROR;
			}
		} else if (token == RW_END) {
			// End of subroutine reached
			get_token();
			if (token != ERROR)
				fix_subroutine_jumps();
			write_bytes(&subroutineData[0], subroutineSize);
			break;

		} else {
			token = ERROR;
			printf("Unexpected error\n");
		}
	}
}

/*----------------------------------------------------------------------*/
/*  Character routines													*/
/*----------------------------------------------------------------------*/

/**
 * Read the next line from the source file.
 */
bool get_source_line() {
	if ((fgets(source_buffer, MAX_SOURCE_LINE_LENGTH, source_file)) != NULL) {
		return true;
	}

	return false;
}

/**
 * Set ch to the next character from the source buffer
 */
void get_char() {
	// If at the end of current source line, read another line.
	// If at end of file, set ch to the EOF character and return
	if (*bufferp == '\0') {
		if (!get_source_line()) {
			ch = EOF_CHAR;
			return;
		}
		bufferp = source_buffer;
		buffer_offset = 0;
		++line_number;
		ch = EOL_CHAR;
		return;
	}

	ch = *bufferp++;		// Next character in the buffer

	if ((ch == '\n') || (ch == '\t')) ch = ' ';
}

/**
 * Skip past any blanks in the current location in the source buffer.
 * Set ch to the next nonblank character
 */
void skip_blanks() {
	while (ch == ' ') get_char();
}

/*----------------------------------------------------------------------*/
/*  Token routines														*/
/*----------------------------------------------------------------------*/

bool is_reserved_word() {
	for (int i = 0; i < 4; ++i) {
		if (!strcmp(symbol_strings[i], token_string)) {
			token = (TokenCode)(RW_DEFINE + i);
			return true;
		}
	}
	return false;
}

bool is_opcode() {
	for (int i = 0; i < TOTAL_OPCODES; ++i) {
		if (!strcmp(OpcodeList[i].str, token_string)) {
			token = RW_OPCODE;
			opcode = (Opcodes)i;
			paramType = OpcodeList[i].paramType;
			return true;
		}
	}
	return false;
}

/**
 * Extract a word token and set token to IDENTIFIER
 */
void get_word() {
	// Extract the word
	while ((char_code(ch) == LETTER) || (char_code(ch) == DIGIT) || (ch == '_')) {
		*tokenp++ = ch;
		get_char();
	}

	*tokenp = '\0';

	strToUpper(token_string);
	token	= WORD;
	if (!is_reserved_word() && !is_opcode()) token = IDENTIFIER;
}

/**
 * Extract a number token and set literal to it's value. Set token to NUMBER
 */
void get_number() {
	digit_count	= 0;		// Total no. of digits in number */
	count_error	= false;	// Too many digits in number?

	do {
		*tokenp++ = ch;

		if (++digit_count > MAX_DIGIT_COUNT) {
			count_error = true;
			break;
		}

		get_char();
	} while ((char_code(ch) == DIGIT) || (toupper(ch) == 'X') || ((toupper(ch) >= 'A') && (toupper(ch) <= 'F')));

	if (count_error) {
		token = ERROR;
		return;
	}

	literal.type			= INTEGER_LIT;
	literal.value.integer	= strToInt(token_string);
	*tokenp = '\0';
	token	= NUMBER;
}

/**
 * Extract a special token
 */
void get_special() {
	*tokenp++ = ch;
	if (ch == ':') {
		token = RW_COLON;
		get_char();
		return;
	} else if (ch == '/') {
		*tokenp++ = ch;
		get_char();
		if (ch == '/') {
			// Comment, so read until end of line
			while ((ch != EOL_CHAR) && (ch != EOF_CHAR))
				get_char();
			token = END_OF_LINE;
			return;
		}
	}

	// Extract the rest of the word
	get_char();
	while ((char_code(ch) == LETTER) || (char_code(ch) == DIGIT)) {
		*tokenp++ = ch;
		get_char();
	}
	*tokenp = '\0';

	strToUpper(token_string);
	if (token_string[0] == '@')
		token = IDENTIFIER;
	else if (!is_reserved_word())
		token = ERROR;
}

/**
 * Extract the next token from the source buffer
 */
void get_token() {
	skip_blanks();
	tokenp = token_string;

	switch (char_code(ch)) {
	case LETTER:	get_word();			break;
	case DIGIT:		get_number();		break;
	case EOL_CODE:	{ token = END_OF_LINE; get_char(); break; }
	case EOF_CODE:	token = END_OF_FILE; break;
	default:		get_special();		break;
	}
}

/**
 * Handles processing a line outside of subroutines
 */
void process_line() {
	if ((token == ERROR) || (token == END_OF_FILE)) return;

	switch (token) {
	case RW_DEFINE:
		handle_define();
		break;
	case RW_SUB:
		handle_sub();
		break;
	case END_OF_LINE:
		break;
	default:
		token = ERROR;
		break;
	}

	if (token == END_OF_LINE) {
		get_token();
	}
}

/*----------------------------------------------------------------------*/
/*  Interface methods													*/
/*----------------------------------------------------------------------*/

/**
 * Main compiler method
 */
bool Compile(const char *srcFilename, const char *destFilename) {
	init_scanner(srcFilename);
	init_output(destFilename);

	get_token();
	while ((token != END_OF_FILE) && (token != ERROR))
		process_line();

	if (token != ERROR) {
		write_index();
	}

	quit_scanner();

	if (token == ERROR)
		printf("Error encountered on line %d\n", line_number);
	else
		printf("Compilation complete\n");
	return token != ERROR;
}
