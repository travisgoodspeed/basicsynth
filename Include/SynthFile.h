/////////////////////////////////////////////////////
// BasicSynth File classes.
//
/// @file SynthFile.h File input and output.
/// Two simple file classes, one for unbuffered output
/// and one for buffered input. These are optimized for
/// the needs of the synthesizer and make direct OS API
/// calls rather than the C++ library FILE or stream IO.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////
// @addtogroup grpIO
//@{

#ifndef _SYNTHFILE_H_
#define _SYNTHFILE_H_

#if defined(WIN32)
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

/// Unbuffered file write. This is used to write a buffer of values
/// to an opened file. It is assumed the caller is buffering output,
/// as in the case of WaveOutFile. Currently, no buffered file output
/// class is available.
class FileWriteUnBuf
{
private:
#if defined(WIN32)
	HANDLE fh;
#endif
#if defined(UNIX)
	int fd;
#endif

public:
	FileWriteUnBuf();
	~FileWriteUnBuf();

	/// Open a file for writing. If the file exists it is truncated to zero length.
	/// If the file does not exist, it is created. This does not create missing
	/// directories in the path.
	/// @param fname path to the file
	/// @return 0 on error, a negative value on errors
	int FileOpen(const char *fname);

	/// Close the file
	int FileClose();

	/// Write to the file.
	/// @param buf buffer to write
	/// @param siz number of bytes to write
	/// @return the number of bytes written
	int FileWrite(void *buf, size_t siz);

	/// Move the file position. The position is absolute, like lseek from the file start.
	/// @param pos number of bytes from the beginning of the file
	/// @return the new file position
	int FileRewind(int pos = 0);
};

/// Buffered file read. The size of the buffer may be set before the file is
/// first opened by calling SetBufSize. Otherwise a default buffer size
/// is used.
class FileReadBuf
{
private:
	int doneAll;
	bsUint8 *inbuf;
#if defined(WIN32)
	HANDLE fh;
	DWORD insize;
	DWORD inread;
	DWORD inpos;
#endif
#if defined(UNIX)
	int fd;
	off_t insize;
	off_t inread;
	off_t inpos;
#endif

public:
	FileReadBuf();
	~FileReadBuf();
	/// Set the buffer size. This only sets the size, but does not allocate
	/// the buffer. The buffer is allocated on the first FileOpen() call.
	/// @note SetBufSize has no effect unless called before FileOpen
	/// @param sz size of the buffer in bytes
	void SetBufSize(size_t sz);

	/// Open a file.
	/// @param fname path name to the file
	/// @return 0 on error, a negative value on errors
	int FileOpen(const char *fname);

	/// Read from the file. Up to \e rdsiz bytes are read.
	/// @param rdbuf buffer for input
	/// @param rdsiz number of bytes to read
	/// @return the number of bytes actually read or -1 on EOF.
	int FileRead(void *rdbuf, int rdsiz);

	/// Read one character from the file. The value is in the range 0-255.
	/// @return the next byte from the file or -1 at EOF
	int ReadCh();

	/// Skip forward in the file. The position is relative to the current file position
	/// and may be positive or negative. The new file position is returned.
	/// @param n the number of bytes to skip forward
	/// @return the new file position
	int FileSkip(int n);

	/// Rewind to a specific file position. The position is relative to the
	/// beginning of the file and should be positive.
	/// @param pos the file position in bytes offset
	/// @return the new file position
	int FileRewind(int pos = 0);

	/// Get the file position.
	/// @return the current file position
	int FilePosition();

	/// Close the file. The SynthFile object can be reused after the file is closed
	/// by calling FileOpen().
	int FileClose();
};

/// Check for existence of a file or directory.
/// @param fname full path to the file or directory.
int SynthFileExists(const char *fname);

/// Copy a file
/// @param oldName full path to existing file
/// @param newName full path to copied file
int SynthCopyFile(const char *oldName, const char *newName);

/// Create a file
/// @param fname full path to new file
/// @param data optional initial content
/// @param datalen length of data
int SynthCreateFile(const char *fname, void *data, size_t datalen);
//@}
#endif
