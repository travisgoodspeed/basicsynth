
///////////////////////////////////////////////////////////
// BasicSynth -
//
/// @file SequenceFile.h Sequence file loader
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
/// @addtogroup grpSeq
//@{

#ifndef _SEQUENCEFILE_H_
#define _SEQUENCEFILE_H_

/// maximum line length for a sequence file
#define SEQ_MAX_LINE 2048
/// maximum number of arguments on a sequence file line
#define SEQ_MAX_ARG  256
#define SEQ_PARAM_GROW 10

/// Sequence file paramater map.
/// Used internally by the SequenceFileLoader and
/// should not be called directly.
class SeqFileMap : public SynthList<SeqFileMap>
{
public:
	int inum;
	int maxent;
	int entries;
	int *map;

	SeqFileMap()
	{
		inum = -1;
		maxent = 0;
		entries = -1;
		map = 0;
	}

	~SeqFileMap()
	{
		delete map;
	}

	void AddEntry(int argn, int mapn)
	{
		if (argn >= maxent)
		{
			int *newmap = new int[maxent+SEQ_PARAM_GROW];
			if (newmap == NULL)
				return;
			if (maxent)
			{
				memcpy(newmap, map, maxent*sizeof(int));
				delete map;
			}
			memset(&newmap[maxent], 0, SEQ_PARAM_GROW*sizeof(int));
			map = newmap;
			maxent += 10;
		}
		map[argn] = mapn;
		if (argn > entries)
			entries = argn;
	}

	int MapParam(int n)
	{
		if (n <= entries)
			return map[n];
		return n;
	}
};

/// Sequencer file loader.
/// A sequencer file is a line of timed events. Each event line
/// in the file must start with the values required by the
/// sequencer event base class. The format of one event line
/// in the sequence file is as follows:
/// @code
///    [+|-|&]inum chnl time duration track { param }
/// @endcode
/// A line beginning with a plus (+) will generate a PARAM event.
/// Lines beginning with a minus (-) will generate a STOP event.
/// A line beginning with ampersand will generate a RESTART event.
/// All other lines (except as noted below) will produce a START event.
///
/// Fields are separated by one or more spaces and may be either
/// a number or a character string. Numbers (other than the inum field)
/// may be entered as a real or integer value and may also include
/// a leading negative sign. Character strings are enclosed in either
/// single or double quote marks. The maximum field length is 256 characters.
/// The first four fields are required. The inum field is an integer
/// value that matches an entry in the instrument definition table.
/// The chnl field indicates the output channel, usually a mixer input.
/// The time and duration fields are numbers that specify the event
/// time and duration in seconds.
///
/// The params are optional and specific to the instrument inum.
/// Any number of param fields may be included, up to a maximum line
/// length of 2048 characters.
///
/// A line beginning with equal (=) defines a parameter map. Since
/// the number of parameters is potentially large, the parameter map
/// allows selecting only those parameters that are essential. The map
/// statement has the form:
/// @code
/// =inum p1 p2 p3...
/// @endcode
/// Each pn value represents the actual parameter ID value to use for
/// parameter number n.
class SequenceFile
{
private:
	InstrManager *inMgr;
	Sequencer *seq;
	SeqFileMap *map;
	SeqFileMap *curmap;
	int error;
	int lineno;
	bsString errlin;
	bsInt32 lastID;
	FileReadBuf fp;

	const char *NextParam(const char *pin, char *pout);
	int Comment(const char *line);
	int ReadLine(char *buf);


public:
	SequenceFile()
	{
		inMgr = NULL;
		seq = NULL;
		map = NULL;
		error = 0;
		lineno = 0;
		lastID = 0;
		map = NULL;
		curmap = NULL;
	}

	~SequenceFile()
	{
		SeqFileMap *p;
		while ((p = map) != NULL)
		{
			map = p->next;
			delete p;
		}
	}

	/// Initialize.
	/// The InstrManager instance is used to locate the
	/// event factory by instrument id. The sequencer is called to
	/// store each event as it is parsed.
	/// @param im instrument manager
	/// @param s sequencer
	void Init(InstrManager *im, Sequencer *s)
	{
		inMgr = im;
		seq = s;
	}

	/// Parse the sequence line from a memory buffer.
	/// @param linbuf buffer with one sequencer line
	int ParseMem(const char *linbuf);

	/// Load the file from memory.
	/// @param fileName pointer to buffer to load
	int LoadMem(const char *fileName);

	/// Load the file from a disk file
	/// @param fileName path to file to load
	int LoadFile(const char *fileName);

	/// Return the error during loading
	int GetError(bsString& buf)
	{
		if (!error)
			return 0;
		buf = errlin;
		return lineno;
	}
};
//@}
#endif
