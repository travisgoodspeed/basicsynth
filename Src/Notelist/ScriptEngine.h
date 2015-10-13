//////////////////////////////////////////////////////////////////////
/// @file ScriptEngine.h Definition of the interface for external scripting.
//
// Script engine interface
// This is a pure virutal class that defines the interface between
// the Notelist converter and a plugin script enginge.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _SCRIPTENGINE_H_
#define _SCRIPTENGINE_H_

class nlScriptEngine
{
public:
	virtual void SetConverter(nlConverter *cp) = 0;
	virtual void SetGenerater(nlGenerate *gp) = 0;
	virtual int LoadScript(const char *script) = 0;
	virtual int CallScript(const char *call) = 0;
	virtual int EvalScript(const char *script, nlVarValue& out) = 0;
};

#endif
