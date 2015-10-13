//////////////////////////////////////////////////////////////////
/// @file SynthFileW.cpp File I/O for Windows
//
// BasicSynth Library
//
// File I/O implementation for Windows API
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthFile.h>

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

FileWriteUnBuf::FileWriteUnBuf()
{
	fh = INVALID_HANDLE_VALUE;
}

FileWriteUnBuf::~FileWriteUnBuf()
{
	if (fh != INVALID_HANDLE_VALUE)
		CloseHandle(fh);
}

int FileWriteUnBuf::FileOpen(const char *fname)
{
	size_t wlen = bsString::utf16Len(fname) + 1;
	wchar_t *wbuf = new wchar_t[wlen];
	bsString::utf16(fname, wbuf, wlen);
	fh = CreateFileW(wbuf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	delete wbuf;
	if (fh == INVALID_HANDLE_VALUE)
		return -1;
	return 0;
}

int FileWriteUnBuf::FileClose()
{
	CloseHandle(fh);
	fh = INVALID_HANDLE_VALUE;
	return 0;
}

int FileWriteUnBuf::FileWrite(void *buf, size_t siz)
{
	DWORD nwrit = 0;
	if (WriteFile(fh, buf, (DWORD) siz, &nwrit, 0))
		return (int) nwrit;
	return -1;
}

int FileWriteUnBuf::FileRewind(int pos)
{
	DWORD ret = SetFilePointer(fh, (LONG) pos, NULL, FILE_BEGIN);
	if (ret == INVALID_SET_FILE_POINTER)
		return -1;
	return (int) LOWORD(ret);
}

//////////////////////////////////////////////////////////////////

FileReadBuf::FileReadBuf()
{
	fh = INVALID_HANDLE_VALUE;
	doneAll = 0;
	insize = 1024*16;
	inread = 0;
	inpos = 0;
	inbuf = 0;
}

FileReadBuf::~FileReadBuf()
{
	if (inbuf)
		delete inbuf;
	if (fh != INVALID_HANDLE_VALUE)
		CloseHandle(fh);
}

void FileReadBuf::SetBufSize(size_t sz)
{
	if (inbuf == NULL)
		insize = (DWORD) sz;
}

int FileReadBuf::FileOpen(const char *fname)
{
	if (inbuf == 0)
	{
		inbuf = new bsUint8[insize];
		if (inbuf == 0)
			return -1;
	}
	size_t wlen = bsString::utf16Len(fname) + 1;
	wchar_t *wbuf = new wchar_t[wlen];
	bsString::utf16(fname, wbuf, wlen);
	fh = CreateFileW(wbuf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	delete wbuf;
	if (fh == INVALID_HANDLE_VALUE)
		return -1;
	inread = 0;
	inpos = 0;
	doneAll = 0;
	return 0;
}

int FileReadBuf::FileRead(void *rdbuf, int rdsiz)
{
	bsUint8 *bp = (bsUint8 *)rdbuf;
	DWORD nread = 0;
	DWORD toread = (DWORD) rdsiz;
	while (nread < (DWORD) rdsiz)
	{
		if (inpos >= inread)
		{
			inread = 0;
			inpos = 0;
			if (doneAll)
				break;
			ReadFile(fh, (LPVOID) inbuf, insize, &inread, NULL);
			if (inread < insize)
				doneAll = 1;
		}
		else
		{
			if (toread > (insize-inpos))
				toread = insize-inpos;
			memcpy(&bp[nread], &inbuf[inpos], toread);
			inpos += toread;
			nread += toread;
			toread = (DWORD) rdsiz - nread;
		}
	}
	return (int) nread;
}

int FileReadBuf::ReadCh()
{
	if (fh == INVALID_HANDLE_VALUE)
		return -1;
	bsUint8 ch;
	if (inpos < inread)
		ch = inbuf[inpos++];
	else if (FileRead(&ch, 1) != 1)
		return -1;
	return ((int) ch) & 0xFF;
}

int FileReadBuf::FileSkip(int n)
{
	int skip = 0;
	int newpos = (int)inpos + n;
	if (newpos > (int)inread)
	{
		skip = newpos - (int)inread;
		inpos = 0;
		inread = 0;
	}
	else if (newpos < 0)
	{
		skip = newpos;
		inpos = 0;
		inread = 0;
	}
	else
        inpos = (DWORD)newpos;
	doneAll = 0;
	DWORD ret = SetFilePointer(fh, (LONG) skip, NULL, FILE_CURRENT);
	if (ret == INVALID_SET_FILE_POINTER)
		return -1;
	return (int) LOWORD(ret) - inread + inpos;
}

int FileReadBuf::FileRewind(int pos)
{
	inpos = 0;
	inread = 0;
	doneAll = 0;
	DWORD ret = SetFilePointer(fh, (LONG) pos, NULL, FILE_BEGIN);
	if (ret == INVALID_SET_FILE_POINTER)
		return -1;
	return (int) LOWORD(ret);
}

int FileReadBuf::FilePosition()
{
	DWORD pos = SetFilePointer(fh, 0, NULL, FILE_CURRENT);
	return (int) pos + inpos - inread;
}

int FileReadBuf::FileClose()
{
	CloseHandle(fh);
	fh = INVALID_HANDLE_VALUE;
	inpos = 0;
	inread = 0;
	return 0;
}

int SynthFileExists(const char *fname)
{
	DWORD attr = 0;

	size_t wlen = bsString::utf16Len(fname) + 1;
	wchar_t *wbuf = new wchar_t[wlen];
	if (wbuf)
	{
		bsString::utf16(fname, wbuf, wlen);
		attr = GetFileAttributesW(wbuf);
		delete wbuf;
	}
	else
		attr = ::GetFileAttributesA(fname);

	return (attr == INVALID_FILE_ATTRIBUTES) ? 0 : 1;
}

int SynthCopyFile(const char *oldName, const char *newName)
{
	BOOL res = 0;

	size_t oldlen = bsString::utf16Len(oldName) + 1;
	wchar_t *wold = new wchar_t[oldlen];

	size_t newlen = bsString::utf16Len(newName) + 1;
	wchar_t *wnew = new wchar_t[newlen];

	if (wold && wnew)
	{
		bsString::utf16(oldName, wold, oldlen);
		bsString::utf16(newName, wnew, newlen);
		res = ::CopyFileW(wold, wnew, 0);
	}
	else
		res = ::CopyFileA(oldName, newName, 0);

	delete wold;
	delete wnew;

	return res ? 0 : -1;
}
