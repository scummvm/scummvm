#include <ddb.h>
#include <os_char.h>
#include <os_bito.h>
#include <os_lib.h>

static uint8_t buffer[2048];

static uint8_t DDB_GetNullWordChar(DDB* ddb)
{
	return ddb->nullWordChar == 0 ? '_' : ddb->nullWordChar;
}

static DDB_DumpOptions DDB_GetEffectiveDumpOptions(const DDB_DumpOptions* options)
{
	DDB_DumpOptions effectiveOptions;
	effectiveOptions.includeMessageSamples = true;
	effectiveOptions.strictPAWCompatibility = false;
	effectiveOptions.rawTokens = false;
	if (options != 0)
		effectiveOptions = *options;
	return effectiveOptions;
}

static void DDB_DumpTextChar(uint8_t ch, DDB_PrintFunc print)
{
	if (ch == 0x7F)
		print("\\f");
	else
	{
		const char* utf8 = DDB_CharToUTF8(ch);
		if (utf8 != 0)
			print("%s", utf8);
	}
}

const char* DDB_GetCondactName(DDB_Condact condact)
{
	static char error[32];

	switch (condact)
	{
		case CONDACT_ABILITY: return "ABILITY";
		case CONDACT_ABSENT:  return "ABSENT";
		case CONDACT_ADD:     return "ADD";
		case CONDACT_ADJECT1: return "ADJECT1";
		case CONDACT_ADJECT2: return "ADJECT2";
		case CONDACT_ADVERB:  return "ADVERB";
		case CONDACT_ANYKEY:  return "ANYKEY";
		case CONDACT_AT:      return "AT";
		case CONDACT_ATGT:    return "ATGT";
		case CONDACT_ATLT:    return "ATLT";
		case CONDACT_AUTOD:   return "AUTOD";
		case CONDACT_AUTOG:   return "AUTOG";
		case CONDACT_AUTOP:   return "AUTOP";
		case CONDACT_AUTOR:   return "AUTOR";
		case CONDACT_AUTOT:   return "AUTOT";
		case CONDACT_AUTOW:   return "AUTOW";
		case CONDACT_BACKAT:  return "BACKAT";
		case CONDACT_BEEP:    return "BEEP";
		case CONDACT_BIGGER:  return "BIGGER";
		case CONDACT_BORDER:  return "BORDER";
		case CONDACT_CALL:    return "CALL";
		case CONDACT_CARRIED: return "CARRIED";
		case CONDACT_CENTRE:  return "CENTRE";
		case CONDACT_CHANCE:  return "CHANCE";
		case CONDACT_CLEAR:   return "CLEAR";
		case CONDACT_CLS:     return "CLS";
		case CONDACT_COPYBF:  return "COPYBF";
		case CONDACT_COPYFF:  return "COPYFF";
		case CONDACT_COPYFO:  return "COPYFO";
		case CONDACT_COPYOF:  return "COPYOF";
		case CONDACT_COPYOO:  return "COPYOO";
		case CONDACT_CREATE:  return "CREATE";
		case CONDACT_DESC:    return "DESC";
		case CONDACT_DESTROY: return "DESTROY";
		case CONDACT_DISPLAY: return "DISPLAY";
		case CONDACT_DOALL:   return "DOALL";
		case CONDACT_DONE:    return "DONE";
		case CONDACT_DPRINT:  return "DPRINT";
		case CONDACT_DROP:    return "DROP";
		case CONDACT_DROPALL: return "DROPALL";
		case CONDACT_END:     return "END";
		case CONDACT_EQ:      return "EQ";
		case CONDACT_EXIT:    return "EXIT";
		case CONDACT_EXTERN:  return "EXTERN";
		case CONDACT_GET:     return "GET";
		case CONDACT_GFX:     return "GFX";
		case CONDACT_GOTO:    return "GOTO";
		case CONDACT_GRAPHIC: return "GRAPHIC";
		case CONDACT_GT:      return "GT";
		case CONDACT_HASAT:   return "HASAT";
		case CONDACT_HASNAT:  return "HASNAT";
		case CONDACT_INK:     return "INK";
		case CONDACT_INKEY:   return "INKEY";
		case CONDACT_INPUT:   return "INPUT";
		case CONDACT_ISAT:    return "ISAT";
		case CONDACT_ISDONE:  return "ISDONE";
		case CONDACT_ISNDONE: return "ISNDONE";
		case CONDACT_ISNOTAT: return "ISNOTAT";
		case CONDACT_LET:     return "LET";
		case CONDACT_LISTAT:  return "LISTAT";
		case CONDACT_LISTOBJ: return "LISTOBJ";
		case CONDACT_LOAD:    return "LOAD";
		case CONDACT_LT:      return "LT";
		case CONDACT_MES:     return "MES";
		case CONDACT_MESSAGE: return "MESSAGE";
		case CONDACT_MINUS:   return "MINUS";
		case CONDACT_MODE:    return "MODE";
		case CONDACT_MOUSE:   return "MOUSE";
		case CONDACT_MOVE:    return "MOVE";
		case CONDACT_NEWLINE: return "NEWLINE";
		case CONDACT_NEWTEXT: return "NEWTEXT";
		case CONDACT_NOTAT:   return "NOTAT";
		case CONDACT_NOTCARR: return "NOTCARR";
		case CONDACT_NOTDONE: return "NOTDONE";
		case CONDACT_NOTEQ:   return "NOTEQ";
		case CONDACT_NOTSAME: return "NOTSAME";
		case CONDACT_NOTWORN: return "NOTWORN";
		case CONDACT_NOTZERO: return "NOTZERO";
		case CONDACT_NOUN2:   return "NOUN2";
		case CONDACT_OK:      return "OK";
		case CONDACT_PAPER:   return "PAPER";
		case CONDACT_PARSE:   return "PARSE";
		case CONDACT_PAUSE:   return "PAUSE";
		case CONDACT_PICTURE: return "PICTURE";
		case CONDACT_PLACE:   return "PLACE";
		case CONDACT_PLUS:    return "PLUS";
		case CONDACT_PREP:    return "PREP";
		case CONDACT_PRESENT: return "PRESENT";
		case CONDACT_PRINT:   return "PRINT";
		case CONDACT_PRINTAT: return "PRINTAT";
		case CONDACT_PROCESS: return "PROCESS";
		case CONDACT_PROMPT:  return "PROMPT";
		case CONDACT_PUTIN:   return "PUTIN";
		case CONDACT_PUTO:    return "PUTO";
		case CONDACT_QUIT:    return "QUIT";
		case CONDACT_RAMLOAD: return "RAMLOAD";
		case CONDACT_RAMSAVE: return "RAMSAVE";
		case CONDACT_RANDOM:  return "RANDOM";
		case CONDACT_REDO:    return "REDO";
		case CONDACT_REMOVE:  return "REMOVE";
		case CONDACT_RESET:   return "RESET";
		case CONDACT_RESTART: return "RESTART";
		case CONDACT_SAME:    return "SAME";
		case CONDACT_SAVE:    return "SAVE";
		case CONDACT_SAVEAT:  return "SAVEAT";
		case CONDACT_SET:     return "SET";
		case CONDACT_SETCO:   return "SETCO";
		case CONDACT_SFX:     return "SFX";
		case CONDACT_SKIP:    return "SKIP";
		case CONDACT_SMALLER: return "SMALLER";
		case CONDACT_SPACE:   return "SPACE";
		case CONDACT_SUB:     return "SUB";
		case CONDACT_SWAP:    return "SWAP";
		case CONDACT_SYNONYM: return "SYNONYM";
		case CONDACT_SYSMESS: return "SYSMESS";
		case CONDACT_TAB:     return "TAB";
		case CONDACT_TAKEOUT: return "TAKEOUT";
		case CONDACT_TIME:    return "TIME";
		case CONDACT_TIMEOUT: return "TIMEOUT";
		case CONDACT_TURNS:   return "TURNS";
		case CONDACT_WEAR:    return "WEAR";
		case CONDACT_WEIGH:   return "WEIGH";
		case CONDACT_WEIGHT:  return "WEIGHT";
		case CONDACT_WHATO:   return "WHATO";
		case CONDACT_WINAT:   return "WINAT";
		case CONDACT_WINDOW:  return "WINDOW";
		case CONDACT_WINSIZE: return "WINSIZE";
		case CONDACT_WORN:    return "WORN";
		case CONDACT_ZERO:    return "ZERO";
		
		case CONDACT_INVEN:   return "INVEN";
		case CONDACT_SCORE:   return "SCORE";
		case CONDACT_CHARSET: return "CHARSET";
		case CONDACT_LINE:    return "LINE";
		case CONDACT_PROTECT: return "PROTECT";

		case CONDACT_INDIR:   return "INDIR";
		case CONDACT_SETAT:   return "SETAT";
		case CONDACT_XMESSAGE:return "XMESSAGE";

		case CONDACT_INVALID: 
			StrCopy(error, 32, "[Invalid condact 0x00]");
			IntToHex2(condact, error + 19);
			return error;
		default:
			StrCopy(error, 32, "[Unknown condact 0x00]");
			IntToHex2(condact, error + 19);
			return error;
	}
}

void DDB_DumpVocabularyWord (DDB* ddb, uint8_t type, uint8_t index, DDB_PrintFunc print)
{
	uint8_t* ptr = ddb->vocabulary;

	while (*ptr != 0)
	{
		if (ptr[5] == index && ptr[6] == type)
		{
			for (int n = 0; n < 5; n++)
				DDB_DumpTextChar((ptr[n] ^ 0xFF) & 0x7F, print);
			return;
		}
		ptr += 7;
	}
	if (type == WordType_Verb)
	{
		const int convertibleNoun = ddb->version < 2 ? 20 : 40;
		if (index < convertibleNoun)
		{
			DDB_DumpVocabularyWord(ddb, WordType_Noun, index, print);
			return;
		}
	}
	print("%-5d", index);
}

static void DDB_DumpMessageChar(uint8_t ch, DDB_PrintFunc print, bool preserveLayout)
{
	if (ch == 7 && preserveLayout)
		print("\n");
	else if (ch == ' ' && preserveLayout)
		print(" ");
	else if (ch == 0x0B)
		print("\\b");
	else if (ch == 0x0C)
		print("\\k");
	else if (ch == '\r')
	{
		if (preserveLayout)
			print("\n\n");
		else
			print("\\n");
	}
	else if (ch == 0x0E)
		print("\\g");
	else if (ch == 0x0F)
		print("\\t");
	else if (ch == 0x7F)
		print("\\f");
	else if (ch == '\\')
		print("\\\\");
	else if (ch == '{' || ch == '}')
		print("%c", ch);
	else if (ch >= 16)
	{
		const char* utf8 = DDB_CharToUTF8(ch);
		if (utf8 != 0)
			print("%s", utf8);
		else
			print("{%d}", ch);
	}
	else
		print("{%d}", ch);
}

static int DDB_GetPAWSControlParameterCount(uint8_t ch)
{
	if (ch >= 0x10 && ch <= 0x15)
		return 1;
	if (ch == 0x16 || ch == 0x17)
		return 2;
	return 0;
}

static void DDB_DumpPAWSMessageByte(uint8_t* bytes, size_t size, size_t* index,
	DDB_PrintFunc print, int* maxLength, bool atLineStart, bool atLineEnd, bool strictCompatibility)
{
	uint8_t ch = bytes[*index];
	bool preserveLayout = *maxLength == 0;
	if (*maxLength > 0)
	{
		if (--(*maxLength) == 0)
		{
			print("...");
			*index = size;
			return;
		}
	}

	if (ch <= 0x17)
	{
		int parameters = DDB_GetPAWSControlParameterCount(ch);
		if (strictCompatibility)
		{
			if ((ch == 0x07 || ch == 0x0D) && preserveLayout)
				print(*index + 1 < size ? "\n\n" : "\n");
			else if (ch == 0x06)
				print(" ");
			else if (ch == 0x07 || ch == 0x0D)
				print(" ");
			while (parameters-- > 0 && *index + 1 < size)
				++(*index);
			return;
		}
		if (ch == 0x07 && preserveLayout)
		{
			// One newline ends the current source line (none when already at a
			// line start), one more produces the blank line the compiler reads
			// back as a break; the caller prints a final newline after the
			// message, which stands in for the last one of a trailing run.
			int newlines = (*index == 0 || bytes[*index - 1] == 0x07 ? 1 : 2) -
				(*index + 1 < size ? 0 : 1);
			while (newlines-- > 0)
				print("\n");
		}
		else if (ch == 0x0D)
			print("\\n");
		else
			print("{%u}", (unsigned)ch);

		while (parameters-- > 0 && *index + 1 < size)
		{
			ch = bytes[++(*index)];
			print("{%u}", (unsigned)ch);
		}
		return;
	}

	if (ch == ' ')
	{
		if (strictCompatibility)
			print(" ");
		else if (atLineStart || atLineEnd)
			print("\\s");
		else
			print(" ");
	}
	else if (strictCompatibility && ch >= 0x7F)
		print("?");
	else if (strictCompatibility)
	{
		if (ch == '/' || ch == ';')
			print(" ");
		print("%c", ch);
	}
	else if (ch == '\\')
		print("\\\\");
	else if (ch == '{')
		print("\\{");
	else if (ch == '}')
		print("\\}");
	else if (ch >= 0x7F)
		print("{%u}", (unsigned)ch);
	else
		DDB_DumpTextChar(ch, print);
}

static uint8_t* DDB_DumpGetMessagePtr(DDB* ddb, DDB_MsgType type, uint8_t n)
{
	switch (type)
	{
		case DDB_MSG:
			if (n >= ddb->numMessages)
				return 0;
			return ddb->messages[n];
		case DDB_SYSMSG:
			if (n >= ddb->numSystemMessages)
				return 0;
			return ddb->data + ddb->sysMsgTable[n];
		case DDB_OBJNAME:
			if (n >= ddb->numObjects)
				return 0;
			return ddb->data + ddb->objNamTable[n];
		case DDB_LOCDESC:
			if (n >= ddb->numLocations)
				return 0;
			return ddb->locDescriptions[n];
		default:
			return 0;
	}
}

static bool DDB_DumpMessageByte(uint8_t ch, DDB_PrintFunc print, int* maxLength, bool preserveLayout, bool atLineStart, bool atLineEnd)
{
	if (*maxLength > 0)
	{
		if (--(*maxLength) == 0)
		{
			print("...");
			return false;
		}
		preserveLayout = false;
	}

	if (ch == ' ' && preserveLayout)
	{
		if (atLineStart || atLineEnd)
			print("\\s");
		else
			print(" ");
	}
	else
		DDB_DumpMessageChar(ch, print, preserveLayout);
	return true;
}

static bool DDB_DumpMessage(DDB* ddb, DDB_MsgType type, uint8_t n, DDB_PrintFunc print, int maxLength,
	bool strictPAWCompatibility, bool rawTokens)
{
	uint8_t* ptr = DDB_DumpGetMessagePtr(ddb, type, n);
	if (ptr <= ddb->data || ptr >= ddb->data + ddb->dataSize)
		return false;

	uint8_t endMarker = ddb->version == DDB_VERSION_PAWS ? 0x1F : 0x0A;
	uint8_t* out = buffer;
	uint8_t* outEnd = buffer + sizeof(buffer);

	while (ptr < ddb->data + ddb->dataSize)
	{
		uint8_t c = *ptr++ ^ 0xFF;
		if (c == endMarker)
			break;

		if (c >= ddb->firstToken)
		{
			if (rawTokens && ddb->version == DDB_VERSION_PAWS)
			{
				if (out < outEnd)
					*out++ = c;
				continue;
			}
			if (!ddb->hasTokens)
			{
				if (ddb->version == DDB_VERSION_PAWS)
				{
					if (out < outEnd)
						*out++ = c;
					continue;
				}
				DDB_Warning("Message contains token 0x%02X but DDB has no tokens!", c);
				continue;
			}
			uint8_t* token = ddb->tokensPtr[c - ddb->firstToken];
			if (token == 0)
			{
				if (ddb->version == DDB_VERSION_PAWS)
				{
					if (out < outEnd)
						*out++ = c;
					continue;
				}
				DDB_Warning("Message contains token 0x%02X but it's not defined in the DDB!", c);
				continue;
			}
			while (token < ddb->data + ddb->dataSize && out < outEnd)
			{
				uint8_t tokenByte = *token++;
				*out++ = tokenByte & 0x7F;
				if ((tokenByte & 0x80) != 0)
					break;
			}
			continue;
		}

		if (out < outEnd)
			*out++ = c;
	}

	if (out == buffer)
		return false;

	if (ddb->version == DDB_VERSION_PAWS)
	{
		size_t size = (size_t)(out - buffer);
		for (size_t n = 0; n < size; n++)
		{
			bool atLineStart = n == 0 || buffer[n - 1] == 0x07 || buffer[n - 1] == 0x0D;
			bool atLineEnd = n + 1 == size || buffer[n + 1] == 0x07 || buffer[n + 1] == 0x0D;
			DDB_DumpPAWSMessageByte(buffer, size, &n, print, &maxLength, atLineStart, atLineEnd,
				strictPAWCompatibility);
		}
	}
	else
	{
		for (uint8_t* ch = buffer; ch < out; ch++)
		{
			bool atLineStart = ch == buffer || ch[-1] == '\r';
			bool atLineEnd = ch + 1 == out || ch[1] == '\r';
			if (!DDB_DumpMessageByte(*ch, print, &maxLength, maxLength == 0, atLineStart, atLineEnd))
				break;
		}
	}
	return true;
}

static void DDB_DumpMessageTable(DDB* ddb, DDB_MsgType type, DDB_PrintFunc print,
	bool strictPAWCompatibility, bool rawTokens)
{
	int count;

	switch (type)
	{
		case DDB_LOCDESC: count = ddb->numLocations; break;
		case DDB_OBJNAME: count = ddb->numObjects; break;
		case DDB_MSG:     count = ddb->numMessages; break;
		case DDB_SYSMSG:  count = ddb->numSystemMessages; break;
		default:          return;
	}
	
	for (int n = 0 ; n < count; n++)
	{
		print("/%d\n", n);
		if (DDB_DumpMessage(ddb, type, n, print, 0, strictPAWCompatibility, rawTokens))
			print("\n");
	}
	if (strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS && type == DDB_SYSMSG)
	{
		for (int n = count; n < 61; n++)
			print("/%d\n", n);
	}
}

static bool DDB_IsClassicPAWUnsupportedCondact(DDB_Condact condact)
{
	switch (condact)
	{
		case CONDACT_PAPER:
		case CONDACT_INK:
		case CONDACT_BORDER:
		case CONDACT_CHARSET:
		case CONDACT_LINE:
		case CONDACT_PICTURE:
		case CONDACT_GRAPHIC:
		case CONDACT_INPUT:
		case CONDACT_SAVEAT:
		case CONDACT_BACKAT:
		case CONDACT_PRINTAT:
		case CONDACT_PROTECT:
			return true;
		default:
			return false;
	}
}

static bool DDB_ClassicPAWEntryHasInstructions(DDB* ddb, uint8_t* code)
{
	while (code < ddb->data + ddb->dataSize && *code != 0xFF)
	{
		uint8_t condactIndex = *code++ & 0x7F;
		DDB_Condact condact = (DDB_Condact)ddb->condactMap[condactIndex].condact;
		int parameters = ddb->condactMap[condactIndex].parameters;
		if (code + parameters > ddb->data + ddb->dataSize)
			return false;
		if (!DDB_IsClassicPAWUnsupportedCondact(condact))
			return true;
		code += parameters;
	}
	return false;
}

void DDB_DumpProcessWithOptions (DDB* ddb, uint8_t index, DDB_PrintFunc print, const DDB_DumpOptions* options)
{
	DDB_DumpOptions effectiveOptions = DDB_GetEffectiveDumpOptions(options);

	if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS)
		print("/PRO %d\n\n", index);
	else
		print("\n/PRO %d\n\n", index);

	if (index >= ddb->numProcesses || ddb->processTable[index] == 0)
		return;

	uint8_t* entry = ddb->data + ddb->processTable[index];
	while (entry < ddb->data + ddb->dataSize)
	{
		if (*entry == 0)
			break;

		if (entry + 4 > ddb->data + ddb->dataSize)
			break;

		uint8_t  verb   = entry[0];
		uint8_t  noun   = entry[1];
		uint16_t offset = *(uint16_t*)(entry + 2);
		uint8_t* code   = ddb->data + offset;
		if (code >= ddb->data + ddb->dataSize || code == ddb->data)
			break;
		entry += 4;
		if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
			!DDB_ClassicPAWEntryHasInstructions(ddb, code))
			continue;

		if (ddb->version == DDB_VERSION_PAWS && verb == 1)
			print("*    ");
		else if (verb == 255)
			print("%c    ", DDB_GetNullWordChar(ddb));
		else
			DDB_DumpVocabularyWord(ddb, WordType_Verb, verb, print);
		print("       ");

		if (ddb->version == DDB_VERSION_PAWS && noun == 1)
			print("*    ");
		else if (noun == 255)
			print("%c    ", DDB_GetNullWordChar(ddb));
		else
			DDB_DumpVocabularyWord(ddb, WordType_Noun, noun, print);
		print("       ");

		bool first = true;
		while (code < ddb->data + ddb->dataSize && *code != 0xFF)
		{
			uint8_t condactIndex = *code & 0x7F;
			bool indirection = (*code & 0x80) != 0;
			int parameters = ddb->condactMap[condactIndex].parameters;
			uint8_t condact = ddb->condactMap[condactIndex].condact;
			code++;
			if (code + parameters > ddb->data + ddb->dataSize)
				break;

			if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
				DDB_IsClassicPAWUnsupportedCondact((DDB_Condact)condact))
			{
				code += parameters;
				continue;
			}

			if (!first)
				print("                        ");
			if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
				condact == CONDACT_BEEP)
				print("%-12s", "BELL");
			else
				print("%-12s", DDB_GetCondactName((DDB_Condact)condact));
			if (condact == CONDACT_CALL && parameters == 2)
			{
				// The two parameter bytes form a 16 bit address
				print("%u", code[0] | (code[1] << 8));
				code += 2;
			}
			else if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
				condact == CONDACT_BEEP)
			{
				code += parameters;
			}
			else if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
				!indirection && (condact == CONDACT_ADJECT1 || condact == CONDACT_ADJECT2 ||
				condact == CONDACT_ADVERB || condact == CONDACT_NOUN2 || condact == CONDACT_PREP))
			{
				uint8_t wordType =
					(condact == CONDACT_ADJECT1 || condact == CONDACT_ADJECT2) ? WordType_Adjective :
					condact == CONDACT_ADVERB ? WordType_Adverb :
					condact == CONDACT_PREP ? WordType_Preposition : WordType_Noun;
				DDB_DumpVocabularyWord(ddb, wordType, *code++, print);
			}
			else if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS &&
				condact == CONDACT_MODE && parameters == 2)
			{
				print("%d", code[0]);
				code += 2;
			}
			else if (parameters > 0)
			{
				if (indirection)
					print("[%d]", *code++);
				else
					print("%d", *code++);
				if (parameters > 1)
					print(" %d", *code++);
				if (parameters > 2)
					print(" %d", *code++);
			}
			if (!indirection && effectiveOptions.includeMessageSamples)
			{
				if (condact == CONDACT_MES || condact == CONDACT_MESSAGE)
				{
					print("\t\t; ");
					DDB_DumpMessage(ddb, DDB_MSG, code[-1], print, 47,
						effectiveOptions.strictPAWCompatibility, false);
				}
				else if (condact == CONDACT_SYSMESS)
				{
					print("\t\t; ");
					DDB_DumpMessage(ddb, DDB_SYSMSG, code[-1], print, 47,
						effectiveOptions.strictPAWCompatibility, false);
				}
			}
			print("\n");
			first = false;

			if (condact == CONDACT_DONE || 
				condact == CONDACT_NOTDONE ||
				condact == CONDACT_OK ||
				condact == CONDACT_SKIP ||
				condact == CONDACT_RESTART ||
				condact == CONDACT_REDO)
				break;
		}
		print("\n");
	}
	if (!(effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS))
		print("\n");
}

void DDB_DumpProcess (DDB* ddb, uint8_t index, DDB_PrintFunc print)
{
	DDB_DumpProcessWithOptions(ddb, index, print, 0);
}

void DDB_DumpWithOptions (DDB* ddb, DDB_PrintFunc print, const DDB_DumpOptions* options)
{
	DDB_DumpOptions effectiveOptions = DDB_GetEffectiveDumpOptions(options);

	if (effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS)
		print("/CTL\n%c\n", DDB_GetNullWordChar(ddb));
	else
		print("\n/CTL\n%c\n\n", DDB_GetNullWordChar(ddb));

	if (ddb->hasTokens && ddb->version != DDB_VERSION_PAWS)
	{
		print("/TOK\n");
		for (int n = 0; n < 128; n++)
		{
			uint8_t* ptr = ddb->tokensPtr[n];
			if (ptr != 0)
			{
				for (; ptr < ddb->data + ddb->dataSize; ptr++)
				{
					if ((*ptr & 0x7F) == ' ')
						print("%c", DDB_GetNullWordChar(ddb));
					else
						DDB_DumpTextChar(*ptr & 0x7F, print);
					if (*ptr & 0x80)
						break;
				}
			}
			print("\n");
		}
		print("\n");
	}

	print("/VOC\n");
	for (uint8_t* ptr = ddb->vocabulary; *ptr; ptr += 7)
	{
		if (ddb->version == DDB_VERSION_PAWS && ptr[6] == 0xFF)
			continue;
		for (int n = 0; n < 5; n++)
			DDB_DumpTextChar((ptr[n] ^ 0xFF) & 0x7F, print);
		print("       %3d    ", ptr[5]);
		switch (ptr[6])
		{
			case WordType_Verb:        print("Verb\n"); break;
			case WordType_Noun:        print("Noun\n"); break;
			case WordType_Adjective:   print("Adjective\n"); break;
			case WordType_Adverb:      print("Adverb\n"); break;
			case WordType_Preposition: print("Preposition\n"); break;
			case WordType_Conjunction: print("Conjunction\n"); break;
			case WordType_Pronoun:     print("Pronoun\n"); break;
			default:              print("%d\n", ptr[6]); break;
		}
	}
	if (!(effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS))
		print("\n");

	print("/STX\n");
	DDB_DumpMessageTable(ddb, DDB_SYSMSG, print, effectiveOptions.strictPAWCompatibility, effectiveOptions.rawTokens);
	print("/MTX\n");
	DDB_DumpMessageTable(ddb, DDB_MSG, print, effectiveOptions.strictPAWCompatibility, effectiveOptions.rawTokens);
	print("/OTX\n");
	DDB_DumpMessageTable(ddb, DDB_OBJNAME, print, effectiveOptions.strictPAWCompatibility, effectiveOptions.rawTokens);
	print("/LTX\n");
	DDB_DumpMessageTable(ddb, DDB_LOCDESC, print, effectiveOptions.strictPAWCompatibility, effectiveOptions.rawTokens);

	print("/CON\n");
	for (int n = 0; n < ddb->numLocations; n++)
	{
		uint8_t* ptr = ddb->locConnections[n];
		if (ptr == 0)
			continue;

		print("/%d\n", n);
		while (ptr < ddb->data + ddb->dataSize && *ptr != 0xFF)
		{
			print("    ");
			DDB_DumpVocabularyWord(ddb, WordType_Verb, *ptr++, print);
			print(" %d\n", *ptr++);
		}
	}
	if (!(effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS))
		print("\n");

	if (ddb->objExAttrTable == 0)
		print("/OBJ\n;       LOC     WEIGHT  CONT?   WEAR?   NOUN    ADJECTIVE\n");
	else
		print("/OBJ\n;       LOC     WEIGHT  CONT?   WEAR?   -------------------------------   NOUN    ADJECTIVE\n");

	for (int n = 0; n < ddb->numObjects; n++)
	{
		print("/%-7d", n);

		uint8_t loc = ddb->objLocTable[n];
		switch (loc)
		{
			case Loc_Destroyed: print("%c       ", DDB_GetNullWordChar(ddb)); break;
			case Loc_Worn:      print("WORN    "); break;
			case Loc_Carried:   print("CARRIED "); break;
			case Loc_Here:      print("HERE    "); break;
			default:            print("%-8d", loc); break;
		}

		uint8_t flags = ddb->objAttrTable[n];
		print("%-8d", flags & Obj_Weight);
		print("%-8c", (flags & Obj_Container) ? 'Y' : DDB_GetNullWordChar(ddb));
		print("%-8c", (flags & Obj_Wearable) ? 'Y' : DDB_GetNullWordChar(ddb));

		if (ddb->objExAttrTable != 0)
		{
			uint16_t flags = ddb->objExAttrTable[n];
			flags = (flags >> 8) | (flags << 8);
			for (int i = 15; i >= 0; i--)
				print("%c ", (flags & (0x1 << i)) ? 'Y' : DDB_GetNullWordChar(ddb));
			print("  ");
		}

		uint8_t noun = ddb->objWordsTable[2*n];
		uint8_t adj  = ddb->objWordsTable[2*n+1];
		if (noun == 255)
			print("%c    ", DDB_GetNullWordChar(ddb));
		else
			DDB_DumpVocabularyWord(ddb, WordType_Noun, noun, print);
		print("   ");
		if (adj == 255)
			print("%c    ", DDB_GetNullWordChar(ddb));
		else
			DDB_DumpVocabularyWord(ddb, WordType_Adjective, adj, print);

		print("\n");
	}
	if (!(effectiveOptions.strictPAWCompatibility && ddb->version == DDB_VERSION_PAWS))
		print("\n");

	for (int n = 0; n < ddb->numProcesses; n++)
		DDB_DumpProcessWithOptions(ddb, n, print, &effectiveOptions);
}

void DDB_Dump (DDB* ddb, DDB_PrintFunc print)
{
	DDB_DumpWithOptions(ddb, print, 0);
}

int DDB_DumpMessageMetrics (DDB* ddb, uint8_t** pointers, const char* name, DDB_PrintFunc print)
{
	int count = 0;
	int total = 0;

	uint8_t endMarker = (ddb->version == DDB_VERSION_PAWS ? 0x1F : 0x0A) ^ 0xFF;

	for (int n = 0; n < 256; n++)
	{
		uint8_t* ptr = pointers[n];
		if (ptr == 0) continue;
		count++;
		while (ptr < ddb->data + ddb->dataSize && *ptr != endMarker)
		{
			ptr++;
			total++;
		}
	}
	print("%5d bytes in %d %s\n", total, count, name);
	return total;

}

int DDB_DumpMessageTableMetrics (DDB* ddb, DDB_MsgType type, DDB_PrintFunc print)
{
	int count;
	int total = 0;
	uint16_t* table;
	const char* name;

	switch (type)
	{
		case DDB_LOCDESC: 
			return DDB_DumpMessageMetrics(ddb, ddb->locDescriptions, "location descriptions", print);
		case DDB_OBJNAME: 
			table = ddb->objNamTable;
			count = ddb->numObjects; 
			name = "object names";
			break;
		case DDB_MSG:     
			return DDB_DumpMessageMetrics(ddb, ddb->messages, "messages", print);
		case DDB_SYSMSG:  
			table = ddb->sysMsgTable;
			count = ddb->numSystemMessages; 
			name = "system messages";
			break;
		default:          
			return 0;
	}

	uint8_t endMarker = (ddb->version == DDB_VERSION_PAWS ? 0x1F : 0x0A) ^ 0xFF;
	
	for (int n = 0 ; n < count; n++)
	{
		uint16_t offset = table[n];
		if (offset == 0) continue;
		uint8_t* ptr = ddb->data + offset;
		while (ptr < ddb->data + ddb->dataSize && *ptr != endMarker)
		{
			ptr++;
			total++;
		}
		total += 3;
	}
	print("%5d bytes in %d %s\n", total, count, name);
	return total;
}

void DDB_DumpMetrics (DDB* ddb, DDB_PrintFunc print)
{
	int total = 0;
	int count = 0;

	print("-------------------------------------------\n");

	if (ddb->hasTokens) 
	{
		print("%5d bytes in tokens\n", (int)ddb->tokenBlockSize);
		total += ddb->tokenBlockSize;
	}

	count = 0;
	for (uint8_t* ptr = ddb->vocabulary; *ptr; ptr += 7)
	{
		total += 7;
		count++;
	}
	print("%5d bytes in vocabulary (%d words)\n", total, count);

	total += DDB_DumpMessageTableMetrics(ddb, DDB_SYSMSG, print);
	total += DDB_DumpMessageTableMetrics(ddb, DDB_MSG, print);
	total += DDB_DumpMessageTableMetrics(ddb, DDB_OBJNAME, print);
	total += DDB_DumpMessageTableMetrics(ddb, DDB_LOCDESC, print);

	count = ddb->numObjects * 4;
	if (ddb->objExAttrTable)
		count += 2*ddb->numObjects;
	print("%5d bytes in object tables (%d objects)\n", count, ddb->numObjects);
	total += count;

	count = 0;
	for (int n = 0; n < ddb->numLocations; n++)
	{
		uint8_t* ptr = ddb->locConnections[n];
		if (ptr == 0) continue;
		while (ptr < ddb->data + ddb->dataSize && *ptr != 0xFF)
			ptr++, count++;
		count++;
	}
	print("%5d bytes in connections (%d locations)\n", count, ddb->numLocations);
	total += count;

	count = 2 * ddb->numProcesses;
	for (int n = 0; n < ddb->numProcesses; n++)
	{
		uint8_t* entry = ddb->data + ddb->processTable[n];
		while (entry < ddb->data + ddb->dataSize)
		{
			if (*entry == 0)
			{
				count++;
				break;
			}

			// uint8_t  verb   = entry[0];
			// uint8_t  noun   = entry[1];
			uint16_t offset = *(uint16_t*)(entry + 2);
			uint8_t* code   = ddb->data + offset;
			uint8_t* start  = code;
			if (code >= ddb->data + ddb->dataSize || code == ddb->data)
				break;

			count += 4;
			entry += 4;

			while (*code != 0xFF)
			{
				uint8_t condactIndex = *code & 0x7F;
				uint8_t condact = ddb->condactMap[condactIndex].condact;
				int parameters = ddb->condactMap[condactIndex].parameters;
				code += parameters + 1;

				if (condact == CONDACT_DONE || 
					condact == CONDACT_NOTDONE ||
					condact == CONDACT_OK ||
					condact == CONDACT_SKIP ||
					condact == CONDACT_RESTART ||
					condact == CONDACT_REDO)
					break;
			}

			count += code - start;
		}
	}
	print("%5d bytes in processes (%d processes)\n", count, ddb->numProcesses);
	total += count;

	print("-------------------------------------------\n");
	print("%5d bytes in total\n", total);
}
