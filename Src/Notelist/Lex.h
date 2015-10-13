//////////////////////////////////////////////////////////////////////
/// @file Lex.h Definition of the lexical scanner classes
//
// nlLexIn is the pure-virtual base class for stream input
// nlLexFileIn provides support for reading from a file
// nlLexFileMem provides support for reading from a memory buffer
// nlLex is the lexical scanner
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_LEX_H_)
#define _LEX_H_

#pragma once

#ifndef EOF
#define EOF (-1)
#endif

/// Interface to a lex buffer.
class nlLexIn
{
public:
	virtual int Getc() = 0;
	virtual void Ungetc(int ch) = 0;
	virtual int Open() = 0;
	virtual int Close() = 0;
	virtual int Position() = 0;
};

/// Lex buffer for a file
class nlLexFileIn : public nlLexIn
{
private:
	char *filename;
	FileReadBuf input;
	int savch;
	int position;
public:
	nlLexFileIn(const char *fn)
	{
		position = 0;
		filename = StrMakeCopy(fn);
		savch = -1;
	}
	virtual ~nlLexFileIn()
	{
		delete filename;
		input.FileClose();
	}

	virtual int Getc()
	{
		int ch;
		if (savch != -1)
		{
			ch = savch;
			savch = -1;
		}
		else
		{
			ch = input.ReadCh();
			if (ch == -1)
				return EOF;
		}
		position++;
		return ch;
	}

	virtual void Ungetc(int ch)
	{
		savch = ch;
		position--;
	}

	virtual int Open()
	{
		position = 0;
		if (input.FileOpen(filename) == 0)
		{
			// Skip BOM on UTF-8 file
			if (input.ReadCh() == 0xEF
			 && input.ReadCh() == 0xBB
			 && input.ReadCh() == 0xBF)
			{
				position = 3;
			}
			else
			{
				input.FileRewind();
			}
			return 1;
		}
		return 0;
	}

	virtual int Close()
	{
		input.FileClose();
		return 0;
	}

	virtual int Position()
	{
		return position;
	}
};

/// Lex buffer for in-memory script.
class nlLexFileMem : public nlLexIn
{
private:
	unsigned char *current;
	unsigned char *start;
	unsigned char *end;
	int  savch;
public:
	nlLexFileMem(const char *p, size_t n)
	{
		savch = -1;
		start = (unsigned char *)p;
		current = start;
		end = start + n;
		// Skip UTF-8 BOM
		if (n >= 3
		 && start[0] == 0xEF
		 && start[1] == 0xBB
		 && start[2] == 0xBF)
		{
			current += 3;
		}
	}

	virtual ~nlLexFileMem()
	{
	}

	virtual int Open()
	{
		savch = -1;
		return start != NULL;
	}

	virtual int Close()
	{
		savch = -1;
		start = end = current = NULL;
		return 0;
	}

	virtual int Getc()
	{
		int ch;
		if (savch != -1)
		{
			ch = savch;
			savch = -1;
			return ch;
		}
		if (current < end)
		{
			return (int) *current++;
		}
		return EOF;
	}

	virtual int Look()
	{
		if (savch != -1)
			return savch;
		if (current < end)
			return (int) *current;
		return EOF;
	}

	virtual void Ungetc(int ch)
	{
		savch = ch;
	}

	virtual int Position()
	{
		int p = (int) (current - start);
		if (savch != -1)
			p--;
		return p;
	}
};

/// @brief Lexical scanner. 
/// @details The nlLex object performs token scanning for nlParse.
/// The lexer requires the caller to first set a buffer object. The
/// buffer retrieves characters one at a time for the lexer. A program
/// can implement a custom lex buffer to read the script from a source
/// other than a file or memory buffer.
class nlLex  
{
private:
	char *cTokbuf;
	int   theToken;
	int   nLineno;
	int   position;
	double version;
	nlLexIn *in;
			
public:
	nlLex();
	virtual ~nlLex();

	inline nlLexIn *GetLexIn() { return in; }
	inline void SetLexIn(nlLexIn *p) { in = p; }
	inline void Version(double v) { version = v; }

	int Open(nlLexIn *p);
	void Close();
	int Next();

	inline bool SymbolStartChar(int ch)
	{
		return (ch >= 'a' && ch <= 'z')
			|| (ch >= 'A' && ch <= 'Z')
			|| ch == '_'
			|| ch == '$'
			|| ch > 127;
	}

	inline bool SymbolChar(int ch)
	{
		return (ch >= 'a' && ch <= 'z')
			|| (ch >= 'A' && ch <= 'Z')
			|| (ch >= '0' && ch <= '9')
			|| ch == '_'
			|| ch == '$'
			|| ch > 127;
	}

	inline const char *Tokbuf()
	{
		return cTokbuf;
	}

	inline int Lineno()
	{
		return nLineno;
	}

	inline int Position()
	{
		return position;
	}

	static int IsKeyword(const char *cTokbuf);
};

#endif // !defined(_LEX_H_)
