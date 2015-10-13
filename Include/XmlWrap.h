/////////////////////////////////////////////////////
// BasicSynth XML Wrapper class definitions
//
/// @file XmlWrap.h Wrapper classes to provide a portable abstraction over the XML DOM.
//
///  - define USE_MSXML to use the MS Windows msxml dll
///  - define USE_LIBXML to use the libxml2 library
///  - define USE_TINYXML to use the tinyxml library
///  - define none to create a dummy XML class
//
// Compile the appropriate .cpp file as well:
//  XmlWrapW.cpp - MSXML
//  XmlWrapU.cpp - Libxml2
//  XmlWrapN.cpp - no XML support
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////
#if !defined(_XMLWRAP_H_)
#define _XMLWRAP_H

#if defined(USE_MSXML)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <atlbase.h>
//#import "msxml3.dll" raw_interfaces_only named_guids
//using namespace MSXML2;
#include <msxml2.h>
#endif
#if defined(USE_LIBXML)
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#endif
#if defined(USE_TINYXML)
#include <tinyxml.h>
#endif

class XmlSynthDoc;

/// An XML element node.
class XmlSynthElem
{
private:
	XmlSynthDoc *doc;

#if defined(USE_MSXML)
	char *nodeTag;
	CComQIPtr<IXMLDOMElement> pElem;
	void SetNode(IUnknown *pnode);
#endif
#if defined(USE_LIBXML)
	xmlNodePtr pElem;
	void SetNode(xmlNodePtr pnode) { pElem = pnode; }
	xmlNodePtr NextElement(xmlNodePtr e);
#endif
#if defined(USE_TINYXML)
	TiXmlElement *pElem;
	void SetNode(TiXmlElement *pnode) { pElem = pnode; }
	void ConvertUTF8(const char *in, bsString& out);
#endif

	friend class XmlSynthDoc;

public:
	XmlSynthElem(XmlSynthDoc *p);
//	XmlSynthElem(XmlSynthDoc *p = 0);
	~XmlSynthElem();
	XmlSynthDoc *Document() { return doc; }
	void Clear();
	/// Get the first child element of this element.
	/// This method allocates a new wrapper object.
	XmlSynthElem *FirstChild();
	/// Get the first child element of this element.
	/// This method reuses an existing wrapper object.
	XmlSynthElem *FirstChild(XmlSynthElem *childElem);
	/// Get the next sibling element of this element.
	/// This method allocates a new wrapper object.
	XmlSynthElem *NextSibling();
	/// Get the next sibling element of this element.
	/// This method reuses an existing wrapper object.
	XmlSynthElem *NextSibling(XmlSynthElem *childElem);
	/// Add a child element to this element.
	/// This method allocates a new wrapper object for the child.
	/// @param childTag tag for the new child element
	XmlSynthElem *AddChild(const char *childTag);
	/// Add a child element to this element.
	/// This method reuses an existing wrapper object.
	/// @param childTag tag for the new child element
	/// @param childElem wrapper object for the new child.
	XmlSynthElem *AddChild(const char *childTag, XmlSynthElem *childElem);
	const char *TagName();
	int GetAttribute(const char *attrName, short& val);
	int GetAttribute(const char *attrName, long& val);
	int GetAttribute(const char *attrName, float& val);
	int GetAttribute(const char *attrName, double& val);
	int GetAttribute(const char *attrName, char **val);
	int GetAttribute(const char *attrName, bsString& val);
	int SetAttribute(const char *attrName, short val);
	int SetAttribute(const char *attrName, long val);
	int SetAttribute(const char *attrName, float val);
	int SetAttribute(const char *attrName, double val);
	int SetAttribute(const char *attrName, const char *val);
	int TagMatch(const char *tag);
	int SetContent(const char *data);
	int GetContent(char **data);
};

/// An XML document.
class XmlSynthDoc
{
private:
	int isUTF8;
	double prjVersion;
	bsString xmlEncoding;
	bsString xmlLocale;

#if defined(USE_MSXML)
	CComQIPtr<IXMLDOMDocument> pDoc;
	int GetXmlDoc();
#endif
#if defined(USE_LIBXML)
	xmlDocPtr doc;
	xmlNodePtr root;
#endif
#if defined(USE_TINYXML)
	TiXmlDocument *doc;
#endif

public:
	XmlSynthDoc();
	~XmlSynthDoc();
	/// Create a new XML element.
	/// The new element is added as a child of parent with the given tag
	XmlSynthElem *CreateElement(XmlSynthElem *parent, const char *tag);
	/// Create a new element.
	/// The new element is added as a child of parent with the given tag
	XmlSynthElem *CreateElement(XmlSynthElem *parent, const char *tag, XmlSynthElem *childElem);
	/// Create a new XML doc.
	XmlSynthElem *NewDoc(const char *roottag);
	/// Create a new XML doc.
	XmlSynthElem *NewDoc(const char *roottag, XmlSynthElem *rootElem);
	/// Open an XML file.
	/// @param fname path to the XML file
	/// @return the root node of the XML document.
	XmlSynthElem *Open(const char *fname);
	/// Open an XML file.
	/// @param fname path to the XML file
	/// @param rootElem wrapper to hold the root element
	/// @return the root node of the XML document.
	XmlSynthElem *Open(const char *fname, XmlSynthElem *rootElem);
	/// Save the XML file
	int Save(const char *fname);
	/// Close the XML file.
	int Close();
	/// Is encoding utf-8
	int UTF8() { return isUTF8; }
	double Version() { return prjVersion; }
	const char *Encoding() { return (const char*)xmlEncoding; }
	const char *Locale() { return (const char *)xmlLocale; }
};

#endif
