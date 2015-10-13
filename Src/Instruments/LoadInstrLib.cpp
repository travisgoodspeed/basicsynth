//////////////////////////////////////////////////////////////////////
/// @file LoadInstrLib.cpp Implementation of functions to load instrument XML files.
//
// BasicSynth instrument library load functions
//
// Note: These are now redundant since the implementation
//       has been moved into the instrument manager.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "LoadInstrLib.h"

// Load and instrument library from the file "fname"
// The file must be an XML file with a document node of
// "instrlib" - all instrument instances are added
// to the instrument manager "mgr"
int LoadInstrLib(InstrManager& mgr, const char *fname)
{
	return mgr.LoadInstrLib(fname);
}

// Load and instrument library from the XML document
// node "root". root must point to a node of type
// "instrlib" - all instrument instances are added
// to the instrument manager "mgr"
int LoadInstrLib(InstrManager& mgr, XmlSynthElem *root)
{
	return mgr.LoadInstrLib(root);
}

// Load an instrument defintion from the XML node
// "instr" which must be of type <instr>.
InstrConfig *LoadInstr(InstrManager& mgr, XmlSynthElem *instr)
{
	if (instr == 0)
		return 0;
	return mgr.LoadInstr(instr);
}

bsInt16 InstrParamMap::SearchParamID(const char *name, InstrParamMap *map, int n)
{
	int hi = n - 1;
	int lo = 0;
	while (hi > lo)
	{
		int ndx = lo + ((hi - lo) / 2);
		int eq = strcmp(name, map[ndx].name);
		if (eq == 0)
			return map[ndx].id;
		if (eq < 0)
			hi = ndx - 1;
		else
			lo = ndx + 1;
	}
	if (strcmp(name, map[lo].name) == 0)
		return map[lo].id;
	return -1;
}

const char *InstrParamMap::SearchParamName(bsInt16 id, InstrParamMap *map, int count)
{
	for (int index = 0; index < count; index++)
	{
		if (map->id == id)
			return map->name;
		map++;
	}
	return "";
}

const char *InstrParamMap::ParamNum(const char *str, int *val)
{
	while (!isdigit(*str))
	{
		if (*str == 0)
			return str;
		str++;
	}
	int n = 0;
	while (isdigit(*str))
		n = (n * 10) + (*str++ - '0');
	*val = n;
	return str;
}

void InstrParamMap::FormatNum(bsInt16 n, char *pdig)
{
	if (n >= 100)
		*pdig++ = (n / 100) + '0';
	if (n >= 10)
		*pdig++ = ((n / 10) % 10) + '0';
	*pdig++ = (n % 10) + '0';
	*pdig = '\0';
}

