//////////////////////////////////////////////////////////////////
/// @file SynthString.cpp Implementation of bsString
//
// BasicSynth Library
//
// String class implementation
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <SynthString.h>
#include <math.h>

const char *bsString::nulStr = "";

char *bsString::Allocate(size_t n)
{
	n++; // for null terminator
	if (n >= maxLen)
	{
		// round up to 16-byte boundary
		n = (n + 15) & ~0xF;
		char *newStr = new char[n];
		if (newStr == 0)
			return 0;
		memset(newStr, 0, n);
		if (theStr)
		{
			memcpy(newStr, theStr, maxLen);
			delete theStr;
		}
		theStr = newStr;
		maxLen = n;
	}
	return theStr;
}

int bsString::SetLen(int newLen)
{
	if (theStr == NULL)
		curLen = 0;
	else if (newLen == -1)
		curLen = strlen(theStr);
	else
	{
		if ((size_t)newLen >= maxLen)
			Allocate((size_t)newLen);
		curLen = newLen;
		theStr[curLen] = 0;
	}
	return (int) curLen;
}

bsString& bsString::Assign(const char *s)
{
	size_t n = 0;
	if (s && (n = strlen(s)) > 0)
	{
		if (Allocate(n))
		{
			memcpy(theStr, s, n+1);
			curLen = n;
		}
	}
	else
	{
		curLen = 0;
		if (theStr)
			*theStr = 0;
	}
	return *this;
}

bsString& bsString::Assign(const bsString& s)
{
	if (s.curLen == 0 || s.theStr == 0)
		curLen = 0;
	else if (Allocate(s.curLen))
	{
		memcpy(theStr, s.theStr, s.curLen+1);
		curLen = s.curLen;
	}
	return *this;
}


bsString& bsString::Assign(const wchar_t *s)
{
	if (s && *s)
	{
		if (Allocate(utf8Len(s)))
			curLen = utf8(s, theStr, maxLen);
	}
	else
		Empty();
	return *this;
}

bsString& bsString::Assign(long n, int base, bool sgn)
{
	if (base > 16)
		base = 16;
	else if (base < 2)
		base = 2;
	// 22 chars will allow 64-bit values with sign in any base from 8-16
	// 16: ffffffffffffffff
	// 10: 9223372036854775807
	//  8: 777777777777777777777
	//  2: 111111111111111111111111111111111111111111111111111111111111111
	size_t toalloc = 22;
	if (base < 8)
		toalloc = 65;
	if (Allocate(toalloc))
		curLen = NumToStr(n, theStr, maxLen, base);
	return *this;
}

bsString& bsString::Assign(double d)
{
	// todo: choose maximum based on value and precision
	if (Allocate(64))
		curLen = FlpToStr(d, theStr, 64);
	return *this;
}

bsString& bsString::Append(const char *s)
{
	if (s && *s)
	{
		size_t n = curLen + strlen(s);
		if (Allocate(n))
		{
			strcat(theStr, s);
			curLen = n;
		}
	}
	return *this;
}

bsString& bsString::Append(long n)
{
	if (Allocate(curLen+20))
		curLen += NumToStr(n, &theStr[curLen], 20);
	return *this;
}

bsString& bsString::Append(double d)
{
	if (Allocate(curLen+40))
		curLen += FlpToStr(d, &theStr[curLen], 40);
	return *this;
}

bsString& bsString::Append(const wchar_t *ws)
{
	if (ws && *ws)
	{
		if (Allocate(curLen+utf8Len(ws)))
			curLen = utf8(ws, &theStr[curLen], maxLen - curLen);
	}
	return *this;
}

// return < 0 if this < s1
// return > 0 if this > s1
// return 0 if this == s1
// This is ANSI only, not true MBCS
// Most of the code in BS only looks for ==

int bsString::CompareNC(const char *s1)
{
	char *s2 = theStr;
	if (!s1)
	{
		if (s2)
			return 1;
		return 0;
	}
	if (!s2)
		return -1;

	char c1, c2;
	while (*s1 && *s2)
	{
		if ((c1 = *s1++) >= 'a' && c1 <= 'z')
			c1 = 'A' + c1 - 'a';
		if ((c2 = *s2++) >= 'a' && c2 <= 'z')
			c2 = 'A' + c2 - 'a';
		if (c1 != c2)
			return c2 - c1;
	}
	return *s2 - *s1;
}

int bsString::Compare(const char *s1)
{
	if (!s1)
	{
		if (theStr)
			return 1;
		return 0;
	}
	if (!theStr)
		return -1;
	return strcmp(theStr, s1);
}

bsString& bsString::Upper()
{
	if (curLen > 0)
	{
		char ch;
		char *s = theStr;
		for (size_t n = curLen; n > 0;  n--)
		{
			if ((ch = *s) >= 'a' && ch <= 'z')
				*s = ch - 'a' + 'A';
			s++;
		}
	}
	return *this;
}

bsString& bsString::Lower()
{
	if (curLen > 0)
	{
		char ch;
		char *s = theStr;
		for (size_t n = curLen; n > 0;  n--)
		{
			if ((ch = *s) >= 'A' && ch <= 'Z')
				*s = ch - 'A' + 'a';
			s++;
		}
	}
	return *this;
}

int bsString::Find(int start, int ch)
{
	if (theStr == 0)
		return -1;
	if (start >= (int)curLen)
		start = (int)curLen-1;
	if (start < 0)
		start = 0;
	const char *p1 = strchr(&theStr[start], ch);
	if (p1)
		return (int) (p1 - theStr);
	return -1;
}

int bsString::FindReverse(int start, int ch)
{
	if (theStr == 0)
		return -1;
	if (start >= (int)curLen)
		return -1;
	if (start < 0)
		start = 0;
	const char *p1 = strrchr(&theStr[start], ch);
	if (p1)
		return (int) (p1 - theStr);
	return -1;
}

size_t bsString::SubString(bsString& out, int start, size_t len)
{
	if (len > curLen || len == 0)
		len = curLen;
	if (start < 0)
		start = 0;
	out.Allocate(len);
	out.curLen = 0;
	if (start < (int)curLen)
	{
		char *p1 = &theStr[start];
		char *p2 = out.theStr;
		while (len > 0 && start++ < (int)curLen)
		{
			*p2++ = *p1++;
			len--;
			out.curLen++;
		}
		*p2 = 0;
	}
	return out.curLen;
}

int bsString::SplitPath(bsString& base, bsString& file, int inclSep)
{
	if (curLen == 0)
	{
		base = (const char*)0;
		file = (const char*)0;;
		return 0;
	}
	int slash = FindReverse(0, '\\');
	if (slash < 0)
	{
		if ((slash = FindReverse(0, '/')) < 0)
			slash = Find(0, ':');
	}
	if (slash >= 0)
	{
		SubString(base, 0, slash + (inclSep ? 1 : 0));
		SubString(file, slash+1, 0);
		return 2;
	}
	base = (const char*)0;
	file = theStr;
	return 1;
}

void bsString::Attach(char *str, int cl, int ml)
{
	if (theStr)
		delete theStr;
	theStr = str;
	if (str == 0)
	{
		curLen = 0;
		maxLen = 0;
	}
	else
	{
		if (cl < 0)
		curLen = strlen(str);
		else
			curLen = cl;
		if (ml < 0)
			maxLen = curLen + 1;
		else
			maxLen = ml;
	}
}

char *bsString::Detach(int *cl, int *ml)
{
	char *str = theStr;
	theStr = 0;
	if (cl)
		*cl = (int)curLen;
	if (ml)
		*ml = (int)maxLen;
	curLen = 0;
	maxLen = 0;
	return str;
}

char *bsString::FmtDig(char *s, unsigned long n, unsigned long base, size_t len)
{
	static char digstr[] = "0123456789ABCDEF";
	if (len > 1 && n >= base)
		s = FmtDig(s, n/base, base, len-1);
	*s++ = digstr[n % base];
	*s = '\0';
	return s;
}

long bsString::StrToNum(const char *s, int base)
{
	long val = 0L;
	while (*s == ' ' || *s == '\t')
		s++;
	int neg = 0;
	if (*s == '-')
	{
		s++;
		neg = 1;
	}
	long add;
	while (*s)
	{
		if (*s >= '0' && *s <= '9')
			add = (long)(*s++ - '0');
		else if (*s >= 'a' && *s <= 'f')
			add = (long)(*s++ - 'a' + 10);
		else if (*s >= 'A' && *s <= 'F')
			add = (long)(*s++ - 'A' + 10);
		else
			break;
		if (add > base)
			break;
		val = (val * base) + add;
	}
	if (neg)
		val = -val;
	return val;
}

size_t bsString::NumToStr(long n, char *str, size_t len, int base)
{
	if (len == 0)
		return 0;

	char *s = str;
	if (n < 0)
	{
		*s++ = '-';
		len--;
		n = -n;
	}
	s = FmtDig(s, (unsigned long)n, (unsigned long)base, len);
	return (size_t)(s - str);
}

double bsString::FlpMinimum = 0.0000009;
int bsString::FlpPrecision = 6;

/// Convert string to floating point.
/// Supports 'e' format.
/// @param str string to convert.
/// @returns value
double bsString::StrToFlp(const char *str, int dp)
{
	double val = 0.0;
	int neg = 0;
	while (*str == ' ' || *str == '\t')
		str++;
	if (*str == '-')
	{
		neg++;
		str++;
	}
	while (*str >= '0' && *str <= '9')
		val = (val * 10.0) + (double)(*str++ - '0');
	if (*str == dp)
	{
		double div = 10.0;
		str++;
		while (*str >= '0' && *str <= '9')
		{
			val += (double)(*str++ - '0') / div;
			div *= 10.0;
		}
	}
	if (*str == 'e' || *str == 'E')
	{
		str++;
		int nege = 0;
		if (*str == '+')
			str++;
		else if (*str == '-')
		{
			str++;
			nege = 1;
		}
		double e = 0.0;
		while (*str >= '0' && *str <= '9')
			e = (e * 10.0) + (double)(*str++ - '0');
		val *= pow(10.0, nege ? -e : e);
	}
	if (neg)
		return -val;
	return val;
}

/// Format floating point value.
/// This always uses '.' as the decimal point.
/// @param val the value to format.
/// @param str the output string
/// @param len maximum length of the string
/// @returns pointer to the string
size_t bsString::FlpToStr(double val, char *str, size_t len, int dp)
{
	if (len == 0)
		return 0;

	// TODO: Use 'e' format if the value is too big.
	// i.e, val >= pow(10, len) || val <= pow(10, -len)
	// Synthesizer params are typically normalized to [1,-1]
	// and only need about 5-6 digits precision, 
	// so this is not a problem for now.
	char *s1 = str;
	if (val < 0.0)
	{
		*s1++ = '-';
		len--;
		val = -val;
	}
	double fpart = val;
	double ipart = floor(val);
	char *s2 = FmtDig(s1, (long)ipart, 10L, len);
	int dig = (int)(len - strlen(str) - 1);
	fpart -= ipart;
	if (fpart > FlpMinimum && dig > 0)
	{
		double fmin = FlpMinimum;
		if (dig > FlpPrecision)
			dig = FlpPrecision;
		*s2++ = (char)dp;
		do
		{
			fpart *= 10.0;
			ipart = floor(fpart); // 0 <= ipart < 10
			*s2++ = ((int)ipart) + '0';
			fpart -= ipart;
			fmin *= 10.0;
		} while (--dig > 0 && fpart > fmin);
		*s2 = '\0';
	}
	return (size_t)(s2 - str);
}

// return the number of UTF=8 bytes in a UTF-16 string
// N.B.: does not include the null byte
size_t bsString::utf8Len(const wchar_t *ws)
{
	size_t len = 0;
	wchar_t wc;
	while ((wc = *ws++) != 0)
	{
		if (wc > 0xD7FF && wc < 0xE000)
			len += 4;
		else if (wc > 0x7FF)
			len += 3;
		else if (wc > 0x7F)
			len += 2;
		else
			len++;
	}
	return len;
}

// return the number of UTF-16 characters in a UTF-8 string.
// N.B.: does not include the null byte
size_t bsString::utf16Len(const char *s)
{
	size_t len = 0;
	wchar_t wc;
	while ((wc = (wchar_t)*s++) != 0)
	{
		if ((wc & 0xC0) == 0xC0)
		{
			if (wc & 0x20) // 0xE0
			{
				if (wc & 0x10) // UTF+1FFFFF
				{
					s++;
					len++;
				}
				//else UTF+FFFF
				s++;
			}
			// else UTF+07FF
			s++;
		}
		// else UTF+007F
		len++;
	}
	return len;
}

// convert UTF-8 code to UTF-16
size_t bsString::utf16(const char *cs, wchar_t *ws, size_t wlen)
{
	if (cs == NULL)
	{
		*ws = 0;
		return 0;
	}

	wchar_t *sav = ws;
	wchar_t uc;
	while (wlen > 1 && (uc = *cs++) != 0)
	{
		if ((uc & 0xC0) == 0xC0)
		{
			if (uc & 0x20) // 0xE0
			{
				if (uc & 0x10) // UTF+1FFFFF
				{
					// 21 bits  U+1FFFFF  11110aaa 10bbbbbb 10cccccc 10dddddd -> 110110aabbbbbbcc 110111ccccdddddd
					if ((cs[0] & 0xC0) == 0x80
					 && (cs[1] & 0xC0) == 0x80
					 && (cs[2] & 0xC0) == 0x80
					 && wlen > 2)
					{
						// form the 32-bit value.
						long lc = ((long)uc & 0x03) << 18;
						lc |= ((long)*cs++ & 0x3f) << 12;
						lc |= ((long)*cs++ & 0x3f) << 6;
						lc |= ((long)*cs & 0x3f);
						// high surrogate
						*ws++ = (wchar_t)(0xD800 | ((lc >> 10) & 0x3FF));
						// low surrogate
						uc = (wchar_t)(0xDC00 | (lc & 0x3FF));
						wlen--;
					}
				}
				else
				{
					// 16 bits: U+FFFF 1110aaaa 10bbbbbb 10cccccc -> aaaabbbbbbcccccc
					if ((cs[0] & 0xC0) == 0x80
					 && (cs[1] & 0xC0) == 0x80)
					{
						uc = (uc & 0x0f) << 12;
						uc |= ((wchar_t)*cs++ & 0x3f) << 6;
						uc |= ((wchar_t)*cs++ & 0x3f);
					}
				}
			}
			else
			{
				// 11 bits: U+07FF 110aaaaa 10bbbbbb -> 00000aaaaabbbbbb
				if ((cs[0] & 0xC0) == 0x80)
				{
					uc = ((uc & 0x1F) << 6);
					uc |= ((wchar_t)*cs++ & 0x3f);
				}
			}
		}
		// invalid utf8 is possibly windows-1252, iso-8259 or similar.
		*ws++ = uc;
		wlen--;
	}
	*ws = 0;
	return (size_t)(ws - sav);
}

// convert UTF-16 to UTF-8
size_t bsString::utf8(const wchar_t *ws, char *cs, size_t clen)
{
	if (ws == NULL)
	{
		*cs = 0;
		return 0;
	}

	char *sav = cs;
	wchar_t wc;
	while (clen > 1 && (wc = *ws++) != 0)
	{
		if (wc > 0x7F)       // 7 bits: U+007F 0aaaaaaa
		{
			if (wc > 0x7FF)
			{
				if ((wc & 0xD800) == 0xD800 && (*ws & 0xDC00) == 0xDC00)
				{
					if (clen < 5)
						break;
					clen -= 4;
					// supplementary planes - 21 bits  U+1FFFFF 
					// utf-16: 110110aaaaaaaaaa 110111bbbbbbbbbb   <- 1aaaaaaaaaabbbbbbbbbb (bit 21 implied)
					// utf-8:  11110aaa 10bbbbbb 10cccccc 10dddddd <- aaabbbbbbccccccdddddd
					long lc = 0x100000 | ((long)(wc & 0x03FF) << 10) | (long)(*ws++ & 0x03FF);
					*cs++ = (char) (0xF0 | ((lc >> 18) & 0x07));
					*cs++ = (char) (0x80 | ((lc >> 12) & 0x3F));
					*cs++ = (char) (0x80 | ((lc >> 6) & 0x3F));
					*cs++ = (char) (0x80 | (lc & 0x3F));
				}
				else  // 16 bits: U+FFFF 1110aaaa 10bbbbbb 10cccccc <- aaaabbbbbbcccccc
				{
					if (clen < 4)
						break;
					clen -= 3;
					*cs++ = (char) (0xE0 | ((wc >> 12) & 0x0F));
					*cs++ = (char) (0x80 | ((wc >> 6) & 0x3F));
					*cs++ = (char) (0x80 | (wc & 0x3F));
				}
			}
			else // 11 bits: U+07FF 110aaaaa 10bbbbbb <- 00000aaaaabbbbbb
			{
				if (clen < 3)
					break;
				clen -= 2;
				*cs++ = (char) (0xC0 | ((wc >> 6) & 0x1F));
				*cs++ = (char) (0x80 | (wc & 0x3F));
			}
		}
		else
		{
			*cs++ = (char)wc;
			clen--;
		}
	}
	*cs = '\0';
	return (size_t)(cs - sav);
}
