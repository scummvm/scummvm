#include "hpl1/engine/libraries/angelscript/add-ons/scriptstdstring.h"
#ifndef __psp2__
#include <locale.h> // setlocale()
#endif

#include "common/str.h"
#include "hpl1/std/map.h"
#include "hpl1/debug.h"

// This macro is used to avoid warnings about unused variables.
// Usually where the variables are only used in debug mode.
#define UNUSED_VAR(x) (void)(x)

struct StrComp {
	bool operator()(const Common::String *a, const Common::String *b) {
		return *a < *b;
	}
};

using map_t = Hpl1::Std::map<const Common::String *, int, StrComp>;

BEGIN_AS_NAMESPACE
class CStdStringFactory : public asIStringFactory {
public:
	CStdStringFactory() {}
	~CStdStringFactory() {
		// The script engine must release each string
		// constant that it has requested
		assert(stringCache.size() == 0);
	}

	const void *GetStringConstant(const char *data, asUINT length) {
		// The string factory might be modified from multiple
		// threads, so it is necessary to use a mutex.
		asAcquireExclusiveLock();

		Common::String str(data, length);
		map_t::iterator it = stringCache.find(&str);
		if (it != stringCache.end()) {
			it->second++;
		} else {
			// deleted in ReleaseStringConstant
			it = stringCache.insert(map_t::value_type(new Common::String(data, length), 1)).first;
		}
		asReleaseExclusiveLock();

		return reinterpret_cast<const void *>(it->first);
	}

	int  ReleaseStringConstant(const void *str) {
		if (str == 0)
			return asERROR;

		int ret = asSUCCESS;

		// The string factory might be modified from multiple
		// threads, so it is necessary to use a mutex.
		asAcquireExclusiveLock();

		map_t::iterator it = stringCache.find(reinterpret_cast<const Common::String *>(str));
		if (it == stringCache.end())
			ret = asERROR;
		else {
			it->second--;
			if (it->second == 0) {
				delete it->first;
				stringCache.erase(it);
			}
		}

		asReleaseExclusiveLock();

		return ret;
	}

	int  GetRawStringData(const void *str, char *data, asUINT *length) const {
		if (str == 0)
			return asERROR;

		if (length)
			*length = (asUINT)reinterpret_cast<const Common::String *>(str)->size();

		if (data)
			memcpy(data, reinterpret_cast<const Common::String *>(str)->c_str(), reinterpret_cast<const Common::String *>(str)->size());

		return asSUCCESS;
	}

	// THe access to the string cache is protected with the common mutex provided by AngelScript
	map_t stringCache;
};

static CStdStringFactory *stringFactory = 0;

// TODO: Make this public so the application can also use the string
//       factory and share the string constants if so desired, or to
//       monitor the size of the string factory cache.
CStdStringFactory *GetStdStringFactorySingleton() {
	if (stringFactory == 0) {
		// The following instance will be destroyed by the global
		// CStdStringFactoryCleaner instance upon application shutdown
		stringFactory = new CStdStringFactory();
	}
	return stringFactory;
}

static void ConstructString(Common::String *thisPointer) {
	new (thisPointer) Common::String();
}

static void CopyConstructString(const Common::String &other, Common::String *thisPointer) {
	new (thisPointer) Common::String(other);
}

static void DestructString(Common::String *thisPointer) {
	thisPointer->Common::String::~String();
}

static Common::String &AddAssignStringToString(const Common::String &str, Common::String &dest) {
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration.
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	dest += str;
	return dest;
}

// bool string::isEmpty()
// bool string::empty() // if AS_USE_STLNAMES == 1
static bool StringIsEmpty(const Common::String &str) {
	// We don't register the method directly because some compilers
	// and standard libraries inline the definition, resulting in the
	// linker being unable to find the declaration
	// Example: CLang/LLVM with XCode 4.3 on OSX 10.7
	return str.size() > 0;
}

static Common::String &AssignUInt64ToString(asQWORD i, Common::String &dest) {
	dest = Common::String::format("%lu", i);
	return dest;
}

static Common::String &AddAssignUInt64ToString(asQWORD i, Common::String &dest) {
	dest += Common::String::format("%lu", i);
	return dest;
}

static Common::String AddStringUInt64(const Common::String &str, asQWORD i) {
	return str + Common::String::format("%lu", i);
}

static Common::String AddInt64String(asINT64 i, const Common::String &str) {
	return Common::String::format("%li", i) + str;
}

static Common::String &AssignInt64ToString(asINT64 i, Common::String &dest) {
	dest = Common::String::format("%li", i);
	return dest;
}

static Common::String &AddAssignInt64ToString(asINT64 i, Common::String &dest) {
	dest += Common::String::format("%li", i);
	return dest;
}

static Common::String AddStringInt64(const Common::String &str, asINT64 i) {
	return str + Common::String::format("%li", i);
}

static Common::String AddUInt64String(asQWORD i, const Common::String &str) {
	return Common::String::format("%li", i) + str;
}

static Common::String &AssignDoubleToString(double f, Common::String &dest) {
	dest = Common::String::format("%f", f);
	return dest;
}

static Common::String &AddAssignDoubleToString(double f, Common::String &dest) {
	dest += Common::String::format("%f", f);
	return dest;
}

static Common::String &AssignFloatToString(float f, Common::String &dest) {
	dest = Common::String::format("%f", f);
	return dest;
}

static Common::String &AddAssignFloatToString(float f, Common::String &dest) {
	dest += Common::String::format("%f", f);
	return dest;
}

static Common::String &AssignBoolToString(bool b, Common::String &dest) {
	dest = (b ? "true" : "false");
	return dest;
}

static Common::String &AddAssignBoolToString(bool b, Common::String &dest) {
	dest += (b ? "true" : "false");
	return dest;
}

static Common::String AddStringDouble(const Common::String &str, double f) {
	return str + Common::String::format("%f", f);
}

static Common::String AddDoubleString(double f, const Common::String &str) {
	return Common::String::format("%f", f) + str;
}

static Common::String AddStringFloat(const Common::String &str, float f) {
	return str + Common::String::format("%f", f);
}

static Common::String AddFloatString(float f, const Common::String &str) {
	return Common::String::format("%f", f) + str;
}

static Common::String AddStringBool(const Common::String &str, bool b) {
	return str + (b ? "true" : "false");
}

static Common::String AddBoolString(bool b, const Common::String &str) {
	return (b ? "true" : "false") + str;
}

static char *StringCharAt(unsigned int i, Common::String &str) {
	if (i >= str.size()) {
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		// Return a null pointer
		return 0;
	}

	return str.begin() + i;
}

// AngelScript signature:
// int string::opCmp(const string &in) const
static int StringCmp(const Common::String &a, const Common::String &b) {
	int cmp = 0;
	if (a < b) cmp = -1;
	else if (a > b) cmp = 1;
	return cmp;
}

// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findFirst(const string &in sub, uint start = 0) const
static int StringFindFirst(const Common::String &sub, asUINT start, const Common::String &str) {
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.find(sub, (size_t)start);
}

// This function returns the index of the first position where the one of the bytes in substring
// exists in the input string. If the characters in the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findFirstOf(const string &in sub, uint start = 0) const
static int StringFindFirstOf(const Common::String &sub, asUINT start, const Common::String &str) {
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.findFirstOf(sub, (size_t)start);
}

// This function returns the index of the last position where the one of the bytes in substring
// exists in the input string. If the characters in the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findLastOf(const string &in sub, uint start = -1) const
static int StringFindLastOf(const Common::String &sub, asUINT start, const Common::String &str) {
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.findLastOf(sub, (size_t)start);
}

// This function returns the index of the first position where a byte other than those in substring
// exists in the input string. If none is found -1 is returned.
//
// AngelScript signature:
// int string::findFirstNotOf(const string &in sub, uint start = 0) const
static int StringFindFirstNotOf(const Common::String &sub, asUINT start, const Common::String &str) {
	// We don't register the method directly because the argument types change between 32bit and 64bit platforms
	return (int)str.findFirstNotOf(sub, (size_t)start);
}

// This function returns the index of the last position where a byte other than those in substring
// exists in the input string. If none is found -1 is returned.
//
// AngelScript signature:
// int string::findLastNotOf(const string &in sub, uint start = -1) const
static int StringFindLastNotOf(const Common::String &sub, asUINT start, const Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}

// This function returns the index of the last position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int string::findLast(const string &in sub, int start = -1) const
static int StringFindLast(const Common::String &sub, int start, const Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// void string::insert(uint pos, const string &in other)
static void StringInsert(unsigned int pos, const Common::String &other, Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// void string::erase(uint pos, int count = -1)
static void StringErase(unsigned int pos, int count, Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}


// AngelScript signature:
// uint string::length() const
static asUINT StringLength(const Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}


// AngelScript signature:
// void string::resize(uint l)
static void StringResize(asUINT l, Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// string formatInt(int64 val, const string &in options, uint width)
static Common::String formatInt(asINT64 value, const Common::String &options, asUINT width) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// string formatUInt(uint64 val, const string &in options, uint width)
static Common::String formatUInt(asQWORD value, const Common::String &options, asUINT width) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// string formatFloat(double val, const string &in options, uint width, uint precision)
static Common::String formatFloat(double value, const Common::String &options, asUINT width, asUINT precision) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// int64 parseInt(const string &in val, uint base = 10, uint &out byteCount = 0)
static asINT64 parseInt(const Common::String &val, asUINT base, asUINT *byteCount) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// uint64 parseUInt(const string &in val, uint base = 10, uint &out byteCount = 0)
static asQWORD parseUInt(const Common::String &val, asUINT base, asUINT *byteCount) {
	HPL1_UNIMPLEMENTED(__func__);
}

// AngelScript signature:
// double parseFloat(const string &in val, uint &out byteCount = 0)
double parseFloat(const Common::String &val, asUINT *byteCount) {
	HPL1_UNIMPLEMENTED(__func__);
}

// This function returns a string containing the substring of the input string
// determined by the starting index and count of characters.
//
// AngelScript signature:
// string string::substr(uint start = 0, int count = -1) const
static Common::String StringSubString(asUINT start, int count, const Common::String &str) {
	HPL1_UNIMPLEMENTED(__func__);
}

// String equality comparison.
// Returns true iff lhs is equal to rhs.
//
// For some reason gcc 4.7 has difficulties resolving the
// asFUNCTIONPR(operator==, (const string &, const string &)
// makro, so this wrapper was introduced as work around.
static bool StringEquals(const Common::String &lhs, const Common::String &rhs) {
	return lhs == rhs;
}

void RegisterStdString_Native(asIScriptEngine *engine) {
	int r = 0;
	UNUSED_VAR(r);

	// Register the string type
#ifdef AS_CAN_USE_CPP11
	// With C++11 it is possible to use asGetTypeTraits to automatically determine the correct flags to use
	r = engine->RegisterObjectType("string", sizeof(Common::String), asOBJ_VALUE | asGetTypeTraits<Common::String>());
	assert(r >= 0);
#else
	r = engine->RegisterObjectType("string", sizeof(Common::String), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
	assert(r >= 0);
#endif

	r = engine->RegisterStringFactory("string", GetStdStringFactorySingleton());

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructString),  asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asMETHODPR(Common::String, operator =, (const Common::String &), Common::String &), asCALL_THISCALL);
	assert(r >= 0);
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asFUNCTION(AddAssignStringToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
//	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asMETHODPR(string, operator+=, (const string&), string&), asCALL_THISCALL); assert( r >= 0 );

	// Need to use a wrapper for operator== otherwise gcc 4.7 fails to compile
	r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTIONPR(StringEquals, (const Common::String &, const Common::String &), bool), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int opCmp(const string &in) const", asFUNCTION(StringCmp), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTIONPR(Common::operator +, (const Common::String &, const Common::String &), Common::String), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);

	// The string length can be accessed through methods or through virtual property
	// TODO: Register as size() for consistency with other types
#if AS_USE_ACCESSORS != 1
	r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
#endif
	r = engine->RegisterObjectMethod("string", "void resize(uint)", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
#if AS_USE_STLNAMES != 1 && AS_USE_ACCESSORS == 1
	// Don't register these if STL names is used, as they conflict with the method size()
	r = engine->RegisterObjectMethod("string", "uint get_length() const property", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void set_length(uint) property", asFUNCTION(StringResize), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
#endif
	// Need to use a wrapper on Mac OS X 10.7/XCode 4.3 and CLang/LLVM, otherwise the linker fails
//	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asMETHOD(string, empty), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	// Register the index operator, both as a mutator and as an inspector
	// Note that we don't register the operator[] directly, as it doesn't do bounds checking
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAt), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddStringDouble), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDoubleString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloatToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloatToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddStringFloat), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloatString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt64ToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddStringInt64), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt64String), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt64ToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddStringUInt64), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt64String), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBoolToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBoolToString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddStringBool), asCALL_CDECL_OBJFIRST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBoolString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);

	// Utilities
	r = engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirst(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirstOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstOf), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirstNotOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstNotOf), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLast(const string &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLastOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastOf), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLastNotOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastNotOf), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void insert(uint pos, const string &in other)", asFUNCTION(StringInsert), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void erase(uint pos, int count = -1)", asFUNCTION(StringErase), asCALL_CDECL_OBJLAST);
	assert(r >= 0);


	r = engine->RegisterGlobalFunction("string formatInt(int64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatInt), asCALL_CDECL);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatUInt(uint64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatUInt), asCALL_CDECL);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatFloat(double val, const string &in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat), asCALL_CDECL);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt), asCALL_CDECL);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 parseUInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt), asCALL_CDECL);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("double parseFloat(const string &in, uint &out byteCount = 0)", asFUNCTION(parseFloat), asCALL_CDECL);
	assert(r >= 0);

#if AS_USE_STLNAMES == 1
	// Same as length
	r = engine->RegisterObjectMethod("string", "uint size() const", asFUNCTION(StringLength), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	// Same as isEmpty
	r = engine->RegisterObjectMethod("string", "bool empty() const", asFUNCTION(StringIsEmpty), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	// Same as findFirst
	r = engine->RegisterObjectMethod("string", "int find(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
	// Same as findLast
	r = engine->RegisterObjectMethod("string", "int rfind(const string &in, int start = -1) const", asFUNCTION(StringFindLast), asCALL_CDECL_OBJLAST);
	assert(r >= 0);
#endif

	// TODO: Implement the following
	// findAndReplace - replaces a text found in the string
	// replaceRange - replaces a range of bytes in the string
	// multiply/times/opMul/opMul_r - takes the string and multiplies it n times, e.g. "-".multiply(5) returns "-----"
}

static void ConstructStringGeneric(asIScriptGeneric *gen) {
	new (gen->GetObject()) Common::String();
}

static void CopyConstructStringGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetArgObject(0));
	new (gen->GetObject()) Common::String(*a);
}

static void DestructStringGeneric(asIScriptGeneric *gen) {
	Common::String *ptr = static_cast<Common::String *>(gen->GetObject());
	ptr->Common::String::~String();
}

static void AssignStringGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetArgObject(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = *a;
	gen->SetReturnAddress(self);
}

static void AddAssignStringGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetArgObject(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += *a;
	gen->SetReturnAddress(self);
}

static void StringEqualsGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	Common::String *b = static_cast<Common::String *>(gen->GetArgAddress(0));
	*(bool *)gen->GetAddressOfReturnLocation() = (*a == *b);
}

static void StringCmpGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	Common::String *b = static_cast<Common::String *>(gen->GetArgAddress(0));

	int cmp = 0;
	if (*a < *b) cmp = -1;
	else if (*a > *b) cmp = 1;

	*(int *)gen->GetAddressOfReturnLocation() = cmp;
}

static void StringAddGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	Common::String *b = static_cast<Common::String *>(gen->GetArgAddress(0));
	Common::String ret_val = *a + *b;
	gen->SetReturnObject(&ret_val);
}

static void StringLengthGeneric(asIScriptGeneric *gen) {
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*static_cast<asUINT *>(gen->GetAddressOfReturnLocation()) = (asUINT)self->size();
}

static void StringIsEmptyGeneric(asIScriptGeneric *gen) {
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<bool *>(gen->GetAddressOfReturnLocation()) = StringIsEmpty(*self);
}

static void StringResizeGeneric(asIScriptGeneric *gen) {
	HPL1_UNIMPLEMENTED(StringResizeGeneric);
}

static void StringInsert_Generic(asIScriptGeneric *gen) {
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	asUINT pos = gen->GetArgDWord(0);
	Common::String *other = reinterpret_cast<Common::String *>(gen->GetArgAddress(1));
	StringInsert(pos, *other, *self);
}

static void StringErase_Generic(asIScriptGeneric *gen) {
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	asUINT pos = gen->GetArgDWord(0);
	int count = int(gen->GetArgDWord(1));
	StringErase(pos, count, *self);
}

static void StringFindFirst_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirst(*find, start, *self);
}

static void StringFindLast_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLast(*find, start, *self);
}

static void StringFindFirstOf_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstOf(*find, start, *self);
}

static void StringFindLastOf_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastOf(*find, start, *self);
}

static void StringFindFirstNotOf_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindFirstNotOf(*find, start, *self);
}

static void StringFindLastNotOf_Generic(asIScriptGeneric *gen) {
	Common::String *find = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT start = gen->GetArgDWord(1);
	Common::String *self = reinterpret_cast<Common::String *>(gen->GetObject());
	*reinterpret_cast<int *>(gen->GetAddressOfReturnLocation()) = StringFindLastNotOf(*find, start, *self);
}

static void formatInt_Generic(asIScriptGeneric *gen) {
	asINT64 val = gen->GetArgQWord(0);
	Common::String *options = reinterpret_cast<Common::String *>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new (gen->GetAddressOfReturnLocation()) Common::String(formatInt(val, *options, width));
}

static void formatUInt_Generic(asIScriptGeneric *gen) {
	asQWORD val = gen->GetArgQWord(0);
	Common::String *options = reinterpret_cast<Common::String *>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	new (gen->GetAddressOfReturnLocation()) Common::String(formatUInt(val, *options, width));
}

static void formatFloat_Generic(asIScriptGeneric *gen) {
	double val = gen->GetArgDouble(0);
	Common::String *options = reinterpret_cast<Common::String *>(gen->GetArgAddress(1));
	asUINT width = gen->GetArgDWord(2);
	asUINT precision = gen->GetArgDWord(3);
	new (gen->GetAddressOfReturnLocation()) Common::String(formatFloat(val, *options, width, precision));
}

static void parseInt_Generic(asIScriptGeneric *gen) {
	Common::String *str = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT *>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseInt(*str, base, byteCount));
}

static void parseUInt_Generic(asIScriptGeneric *gen) {
	Common::String *str = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT base = gen->GetArgDWord(1);
	asUINT *byteCount = reinterpret_cast<asUINT *>(gen->GetArgAddress(2));
	gen->SetReturnQWord(parseUInt(*str, base, byteCount));
}

static void parseFloat_Generic(asIScriptGeneric *gen) {
	Common::String *str = reinterpret_cast<Common::String *>(gen->GetArgAddress(0));
	asUINT *byteCount = reinterpret_cast<asUINT *>(gen->GetArgAddress(1));
	gen->SetReturnDouble(parseFloat(*str, byteCount));
}

static void StringCharAtGeneric(asIScriptGeneric *gen) {
	unsigned int index = gen->GetArgDWord(0);
	Common::String *self = static_cast<Common::String *>(gen->GetObject());

	if (index >= self->size()) {
		// Set a script exception
		asIScriptContext *ctx = asGetActiveContext();
		ctx->SetException("Out of range");

		gen->SetReturnAddress(0);
	} else {
		gen->SetReturnAddress(self->begin() + index);
	}
}

static void AssignInt2StringGeneric(asIScriptGeneric *gen) {
	asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = Common::String::format("%ld", *a);
	gen->SetReturnAddress(self);
}

static void AssignUInt2StringGeneric(asIScriptGeneric *gen) {
	asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = Common::String::format("%lu", *a);
	gen->SetReturnAddress(self);
}

static void AssignDouble2StringGeneric(asIScriptGeneric *gen) {
	double *a = static_cast<double *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = Common::String::format("%f", *a);
	gen->SetReturnAddress(self);
}

static void AssignFloat2StringGeneric(asIScriptGeneric *gen) {
	float *a = static_cast<float *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = Common::String::format("%f", *a);
	gen->SetReturnAddress(self);
}

static void AssignBool2StringGeneric(asIScriptGeneric *gen) {
	bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self = (*a ? "true" : "false");
	gen->SetReturnAddress(self);
}

static void AddAssignDouble2StringGeneric(asIScriptGeneric *gen) {
	double *a = static_cast<double *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += Common::String::format("%f", *a);
	gen->SetReturnAddress(self);
}

static void AddAssignFloat2StringGeneric(asIScriptGeneric *gen) {
	float *a = static_cast<float *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += Common::String::format("%f", *a);
	gen->SetReturnAddress(self);
}

static void AddAssignInt2StringGeneric(asIScriptGeneric *gen) {
	asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += Common::String::format("%ld", *a);
	gen->SetReturnAddress(self);
}

static void AddAssignUInt2StringGeneric(asIScriptGeneric *gen) {
	asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += Common::String::format("%lu", *a);
	gen->SetReturnAddress(self);
}

static void AddAssignBool2StringGeneric(asIScriptGeneric *gen) {
	bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
	Common::String *self = static_cast<Common::String *>(gen->GetObject());
	*self += (*a ? "true" : "false");
	gen->SetReturnAddress(self);
}

static void AddString2DoubleGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	double *b = static_cast<double *>(gen->GetAddressOfArg(0));
	Common::String ret_val = *a + Common::String::format("%f", *b);
	gen->SetReturnObject(&ret_val);
}

static void AddString2FloatGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	float *b = static_cast<float *>(gen->GetAddressOfArg(0));
	Common::String ret_val = *a + Common::String::format("%f", *b);
	gen->SetReturnObject(&ret_val);
}

static void AddString2IntGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	asINT64 *b = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	Common::String ret_val = *a + Common::String::format("%ld", *b);
	gen->SetReturnObject(&ret_val);
}

static void AddString2UIntGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	asQWORD *b = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	Common::String ret_val = *a + Common::String::format("%lu", *b);
	gen->SetReturnObject(&ret_val);
}

static void AddString2BoolGeneric(asIScriptGeneric *gen) {
	Common::String *a = static_cast<Common::String *>(gen->GetObject());
	bool *b = static_cast<bool *>(gen->GetAddressOfArg(0));
	Common::String ret_val = *a + (*b ? "true" : "false");
	gen->SetReturnObject(&ret_val);
}

static void AddDouble2StringGeneric(asIScriptGeneric *gen) {
	double *a = static_cast<double *>(gen->GetAddressOfArg(0));
	Common::String *b = static_cast<Common::String *>(gen->GetObject());
	Common::String ret_val = Common::String::format("%f", *a) + *b;
	gen->SetReturnObject(&ret_val);
}

static void AddFloat2StringGeneric(asIScriptGeneric *gen) {
	float *a = static_cast<float *>(gen->GetAddressOfArg(0));
	Common::String *b = static_cast<Common::String *>(gen->GetObject());
	Common::String ret_val = Common::String::format("%f", *a) + *b;
	gen->SetReturnObject(&ret_val);
}

static void AddInt2StringGeneric(asIScriptGeneric *gen) {
	asINT64 *a = static_cast<asINT64 *>(gen->GetAddressOfArg(0));
	Common::String *b = static_cast<Common::String *>(gen->GetObject());
	Common::String ret_val = Common::String::format("%ld", *a) + *b;
	gen->SetReturnObject(&ret_val);
}

static void AddUInt2StringGeneric(asIScriptGeneric *gen) {
	asQWORD *a = static_cast<asQWORD *>(gen->GetAddressOfArg(0));
	Common::String *b = static_cast<Common::String *>(gen->GetObject());
	Common::String ret_val = Common::String::format("%lu", *a) + *b;
	gen->SetReturnObject(&ret_val);
}

static void AddBool2StringGeneric(asIScriptGeneric *gen) {
	bool *a = static_cast<bool *>(gen->GetAddressOfArg(0));
	Common::String *b = static_cast<Common::String *>(gen->GetObject());
	Common::String ret_val = (*a ? "true" : "false") + *b;
	gen->SetReturnObject(&ret_val);
}

static void StringSubString_Generic(asIScriptGeneric *gen) {
	// Get the arguments
	Common::String *str   = (Common::String *)gen->GetObject();
	asUINT  start = *(int *)gen->GetAddressOfArg(0);
	int     count = *(int *)gen->GetAddressOfArg(1);

	// Return the substring
	new (gen->GetAddressOfReturnLocation()) Common::String(StringSubString(start, count, *str));
}

void RegisterStdString_Generic(asIScriptEngine *engine) {
	int r = 0;
	UNUSED_VAR(r);

	// Register the string type
	r = engine->RegisterObjectType("string", sizeof(Common::String), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK);
	assert(r >= 0);

	r = engine->RegisterStringFactory("string", GetStdStringFactorySingleton());

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                    asFUNCTION(ConstructStringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",    asFUNCTION(CopyConstructStringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                    asFUNCTION(DestructStringGeneric),  asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asFUNCTION(AssignStringGeneric),    asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(const string &in)", asFUNCTION(AddAssignStringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "bool opEquals(const string &in) const", asFUNCTION(StringEqualsGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int opCmp(const string &in) const", asFUNCTION(StringCmpGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(const string &in) const", asFUNCTION(StringAddGeneric), asCALL_GENERIC);
	assert(r >= 0);

	// Register the object methods
#if AS_USE_ACCESSORS != 1
	r = engine->RegisterObjectMethod("string", "uint length() const", asFUNCTION(StringLengthGeneric), asCALL_GENERIC);
	assert(r >= 0);
#endif
	r = engine->RegisterObjectMethod("string", "void resize(uint)",   asFUNCTION(StringResizeGeneric), asCALL_GENERIC);
	assert(r >= 0);
#if AS_USE_STLNAMES != 1 && AS_USE_ACCESSORS == 1
	r = engine->RegisterObjectMethod("string", "uint get_length() const property", asFUNCTION(StringLengthGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void set_length(uint) property", asFUNCTION(StringResizeGeneric), asCALL_GENERIC);
	assert(r >= 0);
#endif
	r = engine->RegisterObjectMethod("string", "bool isEmpty() const", asFUNCTION(StringIsEmptyGeneric), asCALL_GENERIC);
	assert(r >= 0);

	// Register the index operator, both as a mutator and as an inspector
	r = engine->RegisterObjectMethod("string", "uint8 &opIndex(uint)", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "const uint8 &opIndex(uint) const", asFUNCTION(StringCharAtGeneric), asCALL_GENERIC);
	assert(r >= 0);

	// Automatic conversion from values
	r = engine->RegisterObjectMethod("string", "string &opAssign(double)", asFUNCTION(AssignDouble2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(double)", asFUNCTION(AddAssignDouble2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(double) const", asFUNCTION(AddString2DoubleGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(double) const", asFUNCTION(AddDouble2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(float)", asFUNCTION(AssignFloat2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(float)", asFUNCTION(AddAssignFloat2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(float) const", asFUNCTION(AddString2FloatGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(float) const", asFUNCTION(AddFloat2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(int64)", asFUNCTION(AssignInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(int64)", asFUNCTION(AddAssignInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(int64) const", asFUNCTION(AddString2IntGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(int64) const", asFUNCTION(AddInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(uint64)", asFUNCTION(AssignUInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(uint64)", asFUNCTION(AddAssignUInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(uint64) const", asFUNCTION(AddString2UIntGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(uint64) const", asFUNCTION(AddUInt2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string &opAssign(bool)", asFUNCTION(AssignBool2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string &opAddAssign(bool)", asFUNCTION(AddAssignBool2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd(bool) const", asFUNCTION(AddString2BoolGeneric), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "string opAdd_r(bool) const", asFUNCTION(AddBool2StringGeneric), asCALL_GENERIC);
	assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string substr(uint start = 0, int count = -1) const", asFUNCTION(StringSubString_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirst(const string &in, uint start = 0) const", asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirstOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstOf_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findFirstNotOf(const string &in, uint start = 0) const", asFUNCTION(StringFindFirstNotOf_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLast(const string &in, int start = -1) const", asFUNCTION(StringFindLast_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLastOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastOf_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "int findLastNotOf(const string &in, int start = -1) const", asFUNCTION(StringFindLastNotOf_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void insert(uint pos, const string &in other)", asFUNCTION(StringInsert_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterObjectMethod("string", "void erase(uint pos, int count = -1)", asFUNCTION(StringErase_Generic), asCALL_GENERIC);
	assert(r >= 0);


	r = engine->RegisterGlobalFunction("string formatInt(int64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatInt_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatUInt(uint64 val, const string &in options = \"\", uint width = 0)", asFUNCTION(formatUInt_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("string formatFloat(double val, const string &in options = \"\", uint width = 0, uint precision = 0)", asFUNCTION(formatFloat_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("int64 parseInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseInt_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("uint64 parseUInt(const string &in, uint base = 10, uint &out byteCount = 0)", asFUNCTION(parseUInt_Generic), asCALL_GENERIC);
	assert(r >= 0);
	r = engine->RegisterGlobalFunction("double parseFloat(const string &in, uint &out byteCount = 0)", asFUNCTION(parseFloat_Generic), asCALL_GENERIC);
	assert(r >= 0);
}

void RegisterStdString(asIScriptEngine *engine) {
	if (strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY"))
		RegisterStdString_Generic(engine);
	else
		RegisterStdString_Native(engine);
}

void cleanupRegisteredString() {
	if (stringFactory && stringFactory->stringCache.empty()) {
		delete stringFactory;
		stringFactory = nullptr;
	}
}

END_AS_NAMESPACE
