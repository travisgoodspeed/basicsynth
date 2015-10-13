//////////////////////////////////////////////////////////////////////
/// @file Includes.h Required include files for instruments
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#if !defined(_INCLUDES_H_)
#define _INCLUDES_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>

/// @brief Structure to define instrument parameter names.
/// @details An instrument parameter is mapped from
/// a parameter name to the actual parameter
/// id value using this map.
struct InstrParamMap
{
	const char *name;  ///< name of the parameter
	bsInt16 id;        ///< actual instrument parameter.

	static bsInt16 SearchParamID(const char *name, InstrParamMap *map, int n);
	static const char *SearchParamName(bsInt16 id, InstrParamMap *map, int count);
	static const char *ParamNum(const char *str, int *val);
	static void FormatNum(bsInt16 n, char *pdig);
};

#endif
//@}
