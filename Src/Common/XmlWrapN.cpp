//////////////////////////////////////////////////////////////////
/// @file XmlWrapN.cpp NULL XML wrapper
//
// BasicSynth
//
// NULL XmlWrapper - dummy functions for systems that don't
// load instruments and projects from XML files. This will
// resolve references, but always returns error.
//
// To implement configuration storage using something other
// than MSXML and LIBXML2, copy this file, implement each
// method, and link in your object instead of this one.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <SynthString.h>
#include <XmlWrap.h>
#if !defined(USE_MSXML) && !defined(USE_LIBXML) && !defined(USE_TINYXML)

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
}

XmlSynthElem::~XmlSynthElem()
{
}

void XmlSynthElem::Clear()
{
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	return 0;
}

XmlSynthElem *XmlSynthElem::FirstChild(XmlSynthElem *p)
{
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling(XmlSynthElem *p)
{
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag, XmlSynthElem *p)
{
	return 0;
}

const char *XmlSynthElem::TagName()
{
	return "";
}

int XmlSynthElem::GetAttribute(const char *attrName, short& val)
{
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, long& val)
{
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, float& val)
{
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, double& val)
{
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, char **val)
{
	*val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, bsString& val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, short val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, long val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, float val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, double val)
{
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, const char *val)
{
	return -1;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	return -1;
}

int XmlSynthElem::SetContent(const char *data)
{
	return -1;
}

int XmlSynthElem::GetContent(char **data)
{
	*data = 0;
	return -1;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
}

XmlSynthDoc::~XmlSynthDoc()
{
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	return 0;
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag)
{
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname)
{
	return 0;
}

XmlSynthElem *OpenXmlSynthDoc::(const char *fname, XmlSynthElem *rootElem)
{
	return 0;
}

int XmlSynthDoc::Save(const char *fname)
{
	return -1;
}

int XmlSynthDoc::Close()
{
	return -1;
}

#else
int _not_using_xml_null = 1;
#endif
