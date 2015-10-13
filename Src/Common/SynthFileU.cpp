/////////////////////////////////////////////////////////////////
/// @file SynthFileU.cpp File I/O implementation using Unix system calls
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <sys/types.h>
//#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <SynthFile.h>

FileWriteUnBuf::FileWriteUnBuf()
{
	fd = -1;
}

FileWriteUnBuf::~FileWriteUnBuf()
{
	if (fd != -1)
		close(fd);
}

int FileWriteUnBuf::FileOpen(const char *fname)
{
	fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fd < 0)
		return -1;
	return 0;
}

int FileWriteUnBuf::FileClose()
{
	close(fd);
	fd = -1;
	return 0;
}

int FileWriteUnBuf::FileWrite(void *buf, size_t siz)
{
	return (int) write(fd, buf, (size_t) siz);
}

int FileWriteUnBuf::FileRewind(int pos)
{
	return (int) lseek(fd, (off_t) pos, SEEK_SET);
}

FileReadBuf::FileReadBuf()
{
	doneAll = 0;
	insize = 1024*16;
	inread = 0;
	inpos = 0;
	inbuf = 0;
	fd = -1;
}

FileReadBuf::~FileReadBuf()
{
	if (inbuf)
		delete inbuf;
	if (fd != -1)
		close(fd);
}

void FileReadBuf::SetBufSize(size_t sz)
{
	if (inbuf == NULL)
		insize = (off_t) sz;
}

int FileReadBuf::FileOpen(const char *fname)
{
	if (inbuf == 0)
	{
		inbuf = new bsUint8[insize];
		if (inbuf == 0)
			return -1;
	}
	if ((fd = open(fname, O_RDONLY)) < 0)
		return -1;
	inread = 0;
	inpos = 0;
	doneAll = 0;
	return 0;
}

int FileReadBuf::FileRead(void *rdbuf, int rdsiz)
{
	if (fd < 0)
		return -1;
	bsUint8 *bp = (bsUint8 *)rdbuf;
	off_t nread = 0;
	off_t toread = (off_t) rdsiz;
	while (nread < (off_t) rdsiz)
	{
		if (inpos >= inread)
		{
			inread = 0;
			inpos = 0;
			if (doneAll)
				break;
			if ((inread = read(fd, inbuf, insize)) < insize)
				doneAll = 1;
		}
		else
		{
			if (toread > (inread-inpos))
				toread = inread-inpos;
			memcpy(&bp[nread], &inbuf[inpos], toread);
			inpos += toread;
			nread += toread;
			toread = (off_t) rdsiz - nread;
		}
	}
	return (int) nread;
}

int FileReadBuf::ReadCh()
{
	if (fd < 0)
		return -1;
	bsUint8 ch;
	if (inpos < inread)
		ch = inbuf[inpos++];
	else if (FileRead(&ch, 1) != 1)
		return -1;
	return ((int)ch) & 0xFF;
}

int FileReadBuf::FileSkip(int n)
{
	if (fd < 0)
		return -1;
	off_t skip = 0;
	inpos += (off_t) n;
	if (inpos > inread)
	{
		skip = inpos - inread;
		inpos = 0;
		inread = 0;
	}
	else if (inpos < 0)
	{
		skip = inpos;
		inpos = 0;
		inread = 0;
	}
	doneAll = 0;
	return (int) (inpos + lseek(fd, skip, SEEK_CUR));
}

int FileReadBuf::FileRewind(int pos)
{
	if (fd < 0)
		return -1;
	inpos = 0;
	inread = 0;
	doneAll = 0;
	return (int) lseek(fd, (off_t) pos, SEEK_SET);
}

int FileReadBuf::FilePosition()
{
	return (int) lseek(fd, 0, SEEK_CUR) - inread + inpos;
}

int FileReadBuf::FileClose()
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
	return 0;
}

int SynthFileExists(const char *fname)
{
	struct stat info;
	if (stat(fname, &info) == -1)
		return 0;
	if (info.st_mode & S_IFREG)
		return 1;
	return 0;
}

int SynthCopyFile(const char *oldName, const char *newName)
{
	int fdin = open(oldName, O_RDONLY);
	if (fdin < 0)
		return -1;
	int fdout = open(newName, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (fdout < 0)
	{
		close(fdin);
		return -1;
	}

	char *buf = new char[32768];
	int nread;
	while ((nread = read(fdin, buf, 32768)) > 0)
	{
		if (write(fdout, buf, nread) != nread)
			break;
	}
	close(fdin);
	close(fdout);
	delete buf;
	if (nread == 0)
		return 0;
	return -1;
}

