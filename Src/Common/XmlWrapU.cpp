/////////////////////////////////////////////////////////////////
/// @file XmlWrapU.cpp libxml2 wrapper
//
// XML implementation for Unix systems using libxml2
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////

#include <string.h>
#include <SynthString.h>
#include <XmlWrap.h>
#if defined(USE_LIBXML)

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
	doc = p;
	pElem = 0;
}

XmlSynthElem::~XmlSynthElem()
{
}

void XmlSynthElem::Clear()
{
	pElem = 0;
}

xmlNodePtr XmlSynthElem::NextElement(xmlNodePtr e)
{
	while (e && e->type != XML_ELEMENT_NODE)
		e = e->next;
	return e;
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	if (pElem)
	{
		XmlSynthElem *childElem = new XmlSynthElem(doc);
		if (FirstChild(childElem))
			return childElem;
		delete childElem;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::FirstChild(XmlSynthElem *childElem)
{
	if (pElem)
	{
		xmlNodePtr child = NextElement(pElem->children);
		if (child)
		{
			childElem->SetNode(child);
			return childElem;
		}
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	if (pElem)
	{
		XmlSynthElem *childElem = new XmlSynthElem(doc);
		if (NextSibling(childElem))
			return childElem;
		delete childElem;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling(XmlSynthElem *childElem)
{
	if (pElem)
	{
		xmlNodePtr child = NextElement(pElem->next);
		if (child)
		{
			childElem->SetNode(child);
			return childElem;
		}
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	if (doc)
		return doc->CreateElement(this, childTag, 0);
	return NULL;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag, XmlSynthElem *childElem)
{
	if (doc)
		return doc->CreateElement(this, childTag, childElem);
	return NULL;
}

const char *XmlSynthElem::TagName()
{
	if (pElem)
		return (const char*)pElem->name;
	return "";
}

int XmlSynthElem::GetAttribute(char *attrName, short& val)
{
	long tmp;
	int rv = GetAttribute(attrName, tmp);
	if (rv == 0)
		val = (short) tmp;
	else
		val = 0;
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, long& val)
{
	int rv = -1;
	if (pElem)
	{
		xmlChar *str = xmlGetProp(pElem, (const xmlChar *)attrName);
		if (str)
		{
			val = atol((const char*)str);
			rv = 0;
		}
		else
		{
			val = 0;
		}
	}
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, float& val)
{
	double tmp;
	int rv = GetAttribute(attrName, tmp);
	if (rv == 0)
		val = tmp;
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, double& val)
{
	int rv = -1;
	if (pElem)
	{
		xmlChar *str = xmlGetProp(pElem, (const xmlChar *)attrName);
		if (str)
		{
			val = atof((const char*)str);
			rv = 0;
		}
		else
		{
			val = 0;
		}
	}
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, char **val)
{
	int rv = -1;
	*val = NULL;
	if (pElem)
	{
		xmlChar *str = xmlGetProp(pElem, (const xmlChar *)attrName);
		if (str)
		{
			*val = new char[xmlStrlen(str)+1];
			if (*val)
			{
				strcpy(*val, (const char *)str);
				rv = 0;
			}
			xmlFree(str);
		}
	}
	return rv;
}

int XmlSynthElem::GetAttribute(char *attrName, bsString& val)
{
	int rv = -1;
	if (pElem)
	{
		xmlChar *str = xmlGetProp(pElem, (const xmlChar *)attrName);
		if (str)
		{
			val = (const char*)str;
			rv = 0;
		}
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, short val)
{
	return SetAttribute(attrName, (long) val);
}

int XmlSynthElem::SetAttribute(const char *attrName, long val)
{
	int rv = -1;
	if (pElem)
	{
		char valstr[32];
		sprintf(valstr, "%ld", val);
		xmlNewProp(pElem, (const xmlChar*)attrName, (const xmlChar *)valstr);
		rv = 0;
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, float val)
{
	return SetAttribute(attrName, (double) val);
}

int XmlSynthElem::SetAttribute(const char *attrName, double val)
{
	int rv = -1;
	if (pElem)
	{
		char valstr[100];
		sprintf(valstr, "%f", val);
		xmlNewProp(pElem, (const xmlChar*)attrName, (const xmlChar *)valstr);
		rv = 0;
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, const char *val)
{
	int rv = -1;
	if (pElem)
	{
		xmlNewProp(pElem, (const xmlChar*)attrName, (const xmlChar *)val);
		rv = 0;
	}
	return rv;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	if (pElem)
		return xmlStrcmp((const xmlChar*)tag, pElem->name) == 0;
	return tag == NULL;
}

int XmlSynthElem::SetContent(const char *data)
{
	int rv = -1;
	if (pElem)
	{
		xmlNodeSetContent(pElem, (const xmlChar *)data);
		rv = 0;
	}
	return rv;
}

int XmlSynthElem::GetContent(char **data)
{
	int rv = -1;
	if (pElem)
	{
		xmlChar *cp = xmlNodeGetContent(pElem);
		if (cp)
		{
			*data = new char[xmlStrlen(cp)+1];
			if (*data)
			{
				strcpy(*data, (const char *)cp);
				rv = 0;
			}
			xmlFree(cp);
		}
	}
	return rv;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
	doc = NULL;
	root = NULL;
}

XmlSynthDoc::~XmlSynthDoc()
{
	Close();
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	return CreateElement(parent, tag, 0);
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag, XmlSynthElem *newElem)
{
	if (newElem == 0)
		newElem = new XmlSynthElem(this);
	if (doc)
	{
		xmlNodePtr newNode = xmlNewTextChild(parent->pElem, NULL, (const xmlChar *)tag, NULL);
		newElem->SetNode(newNode);
	}
	return newElem;
}

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag)
{
	XmlSynthElem *rootElem = new XmlSynthElem(this);
	if (NewDoc(roottag, rootElem))
		return rootElem;
	delete rootElem;
	return 0;
}

XmlSynthElem *XmlSynthDoc::NewDoc(char *roottag, XmlSynthElem *rootElem)
{
	doc = xmlNewDoc((const xmlChar*)"1.0");
	if (doc)
	{
		if (roottag == NULL)
			roottag = "synth";
		root = xmlNewDocNode(doc, NULL, (const xmlChar *)roottag, NULL);
		rootElem->SetNode(root);
		return rootElem;
	}
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(char *fname)
{
	XmlSynthElem *rootElem = new XmlSynthElem(this);
	if (Open(fname, rootElem))
		return rootElem;
	delete rootElem;
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(char *fname, XmlSynthElem *rootElem)
{
	if (fname == NULL || strlen(fname) == 0)
		return NULL;
	doc = xmlParseFile(fname);
	if (doc)
	{
		root = xmlDocGetRootElement(doc);
		if (root)
		{
			rootElem->SetNode(root);
			return rootElem;
		}
	}
	return 0;
}

int XmlSynthDoc::Save(char *fname)
{
	int rv = -1;
	if (doc)
	{
		xmlSaveFormatFile(fname, doc, 1);
		rv = 0;
	}
	return rv;
}

int XmlSynthDoc::Close()
{
	if (doc)
		xmlFreeDoc(doc);
	doc = NULL;
	root = NULL;
	return 0;
}

#else
int _not_using_libxml = 1;
#endif
