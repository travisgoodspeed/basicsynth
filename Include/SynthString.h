/////////////////////////////////////////////////////////
// BasicSynth string object
//
/// \file SynthString.h
/// Defines a string class.
/// This is not a very clever or extensive string class,
/// but it does all that BasicSynth needs.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
////////////////////////////////////////////////////////
/// \addtogroup grpGeneral
#ifndef _SYNTHSTRING_
#define _SYNTHSTRING_
/*@{*/

#include <string.h>

/// String management class. bsString maintains a variable
/// length buffer and string size. Operations are avaialble to assign, append
/// and compare strings.
class bsString
{
private:
	char *theStr;        // character string buffer
	size_t maxLen;       // size of allocated buffer
	size_t curLen;       // current actual string length
	static const char *nulStr;
	static char *FmtDig(char *s, unsigned long n, unsigned long base, size_t len);
public:
	bsString()
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
	}

	bsString(const char *s)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(s);
	}

	explicit bsString(bsString& s)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(s);
	}

	bsString(int n)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign((long)n);
	}

	explicit bsString(long n)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(n);
	}

	explicit bsString(double d)
	{
		theStr = NULL;
		maxLen = 0;
		curLen = 0;
		Assign(d);
	}

	~bsString()
	{
		delete theStr;
	}

	/// Get the length of the string.
	size_t Length() const
	{
		return curLen;
	}

	/// Set to a null string
	bsString& Empty()
	{
		curLen = 0;
		if (theStr)
			*theStr = 0;
		return *this; 
	}

	/// Cast to a char* type
	operator const char *()
	{
		if (theStr)
			return theStr;
		return nulStr;
	}

	/*
	/// Cast to a long type
	operator long ()
	{
		if (theStr)
			return ToInt();
		return 0;
	}

	/// Cast to a double type
	operator double ()
	{
		if (theStr)
			return ToFloat();
		return 0.0;
	}
	*/

	static size_t NumToStr(long n, char *str, size_t len, int base = 10);
	static long StrToNum(const char *s, int base = 10);

	/// Convert to long integer.
	long ToInt(int base = 10)
	{
		if (curLen == 0)
			return 0;
		return StrToNum(theStr, base);
	}

	static double FlpMinimum;
	static int FlpPrecision;
	static double StrToFlp(const char *s, int dp = '.');
	static size_t FlpToStr(double val, char *str, size_t len, int dp = '.');

	/// Convert to floating point.
	double ToFloat()
	{
		if (curLen == 0)
			return 0.0;
		return StrToFlp(theStr);
	}

	/// Get a character at a specific position.
	char operator[](size_t n)
	{
		if (n < curLen)
			return theStr[n];
		return 0;
	}

	/// Allocate the internal buffer. This will be called automatically when
	/// needed, but may also be called directly to pre-allocate a buffer. This
	/// is useful if multple appends to the string are intended.
	/// \param n size of buffer to allocate
	char *Allocate(size_t n);

	/// Set the string length. If the new length is less than the current length,
	/// the string is truncated. If the length is longer, the string buffer is extended.
	/// If -1 is used for the new length, the actual string length is calculated from
	/// the buffer content.
	/// \param newLen new length of the string
	int SetLen(int newLen = -1);

	/// \name String assignment
	/// Assignment to the string
	/// The character string is copied to the internal buffer.
	/// The internal buffer is allocated or extended as needed.
	/// \param s string to assign to the object
	//@{
	bsString& Assign(const char *s);
	bsString& Assign(const bsString& s);
	bsString& Assign(const wchar_t *ws);

	/// \param n number to format.
	/// \param base number base (8,10 or 16)
	/// \param sgn treat as unsigned if false
	bsString& Assign(long n, int base = 10L, bool sgn = true);
	bsString& Assign(double n);

	bsString& operator=(const char *s)
	{
		return Assign(s);
	}

	bsString& operator=(const bsString& s)
	{
		return Assign(s);
	}

	bsString& operator=(long n)
	{
		return Assign(n);
	}

	bsString operator=(double d)
	{
		return Assign(d);
	}

	//@}

	/// \name String concatenation
	/// Append to the string.
	/// New characters are added to the end of the buffer.
	/// If needed, the buffer is automatically extended. The new string length is calculated as well.
	/// \param s string to append
	//@{
	bsString& Append(const char *s);
	bsString& Append(const wchar_t *ws);
	bsString& Append(long n);
	bsString& Append(double d);

	bsString& operator+=(const char *s)
	{
		return Append(s);
	}

	bsString& operator+=(const bsString& s)
	{
		return Append(s.theStr);
	}

	bsString& operator+=(char ch)
	{
		if (Allocate(curLen+1))
		{
			theStr[curLen++] = ch;
			theStr[curLen] = 0;
		}
		return *this;
	}

	bsString& operator+=(long n)
	{
		return Append(n);
	}

	bsString& operator+=(double d)
	{
		return Append(d);
	}

	//@}

	/// \name String Comparison
	/// Compare two strings. String comparison is character by character and may be
	/// case sensitive or not. The return value follows the C library strcmp convention.
	/// \param s string to compare with
	//@{
	int Compare(const char *s);

	int Compare(bsString& s)
	{
		return Compare(s.theStr);
	}

	int operator==(const char *s)
	{
		return Compare(s) == 0;
	}

	int operator==(bsString& s)
	{
		return Compare(s) == 0;
	}

	int operator!=(const char *s)
	{
		return Compare(s) != 0;
	}

	int operator!=(bsString& s)
	{
		return Compare(s) != 0;
	}

	int CompareNC(const char *s1);

	int CompareNC(bsString& s)
	{
		return CompareNC(s.theStr);
	}
	//@}

	/// Convert to upper case.
	bsString& Upper();
	/// Convert to lower case.
	bsString& Lower();

	/// Find the first instance of ch after start
	int Find(int start, int ch);

	/// Find the last instance of ch before start
	int FindReverse(int start, int ch);

	/// Extract the sub-string
	size_t SubString(bsString& out, int start, size_t len);

	/// Split a file path into directory and file parts
	int SplitPath(bsString& base, bsString& file, int inclSep = 1);

	/// Pass ownership of a buffer to this object.
	void Attach(char *str, int cl = -1, int ml = -1);

	/// Take ownership of the buffer from this object.
	char *Detach(int *cl = 0, int *ml = 0);

	/// \name unicode support
	//@{
	/// Calculate UTF-16 string length from UTF-8.
	/// @param ws wide character string
	static size_t utf8Len(const wchar_t *ws);
	/// Calculate UTF-8 string length from UTF-16.
	/// @param cs multi-byte character string
	static size_t utf16Len(const char *cs);
	/// Convert UTF-16 string to UTF-8.
	/// @param ws wide character string
	/// @param cs multi-byte character string
	/// @param clen size of cs buffer
	static size_t utf8(const wchar_t *ws, char *cs, size_t clen);
	/// Convert UTF-8 string to UTF-16.
	/// @param cs multi-byte character string
	/// @param ws wide character string
	/// @param wlen size of ws buffer
	static size_t utf16(const char *cs, wchar_t *ws, size_t wlen);
	//@}
};
/*@}*/
#endif
