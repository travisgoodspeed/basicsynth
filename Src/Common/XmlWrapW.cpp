//////////////////////////////////////////////////////////////////
/// @file XmlWrapW.cpp MSXML wrapper
//
// BasicSynth
//
// XmlWrapper for Windows MSXML
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <SynthString.h>
#include <XmlWrap.h>
#if defined(USE_MSXML)

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
	doc = p;
	nodeTag = NULL;
}

XmlSynthElem::~XmlSynthElem()
{
	pElem.Release();
}

void XmlSynthElem::Clear()
{
	pElem.Release();
	delete nodeTag;
	nodeTag = NULL;
}

char *WideToMulti(LPOLESTR str, UINT len)
{
	int need = WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
	char *ret = new char[need+1];
	if (ret != NULL)
	{
		WideCharToMultiByte(CP_UTF8, 0, str, len, ret, need+1, NULL, NULL);
		ret[need] = 0;
	}
	return ret;
}

void XmlSynthElem::SetNode(IUnknown *pnode)
{
	if (nodeTag)
	{
		delete nodeTag;
		nodeTag = NULL;
	}

	pElem = pnode;
	if (pElem)
	{
		CComBSTR bsTag;
		pElem->get_tagName(&bsTag.m_str);
		nodeTag = WideToMulti(bsTag.m_str, bsTag.Length());
	}
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	XmlSynthElem *childElem = new XmlSynthElem(doc);
	if (FirstChild(childElem))
		return childElem;
	delete childElem;
	return NULL;
}

XmlSynthElem *XmlSynthElem::FirstChild(XmlSynthElem *childElem)
{
	if (pElem)
	{
		CComPtr<IXMLDOMNode> pnode;
		HRESULT hr = pElem->get_firstChild(&pnode);
		if (hr == S_OK && pnode)
		{
			childElem->doc = doc;
			childElem->SetNode(pnode);
			return childElem;
		}
	}
	return NULL;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	XmlSynthElem *childElem = new XmlSynthElem(doc);
	if (NextSibling(childElem))
		return childElem;
	delete childElem;
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling(XmlSynthElem *childElem)
{
	if (pElem)
	{
		CComPtr<IXMLDOMNode> pnode;
		HRESULT hr = pElem->get_nextSibling(&pnode);
		if (hr == S_OK && pnode)
		{
			childElem->doc = doc;
			childElem->SetNode(pnode);
			return childElem;
		}
	}
	return NULL;
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
	return nodeTag;
}

int XmlSynthElem::GetAttribute(char *attrName, short& val)
{
	long tmp;
	int rv = GetAttribute(attrName, tmp);
	if (rv == 0)
		val = (short) tmp;
	return rv;
}

int XmlSynthElem::GetAttribute(const char *attrName, long& val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_I4);
			val = v.lVal;
			rv = 0;
		}
		else
			val = 0;
	}
	return rv;
}

int XmlSynthElem::GetAttribute(const char *attrName, float& val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_R4);
			val = v.fltVal;
			rv = 0;
		}
		else
			val = 0;
	}
	return rv;
}

int XmlSynthElem::GetAttribute(const char *attrName, double& val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_R8);
			val = v.dblVal;
			rv = 0;
		}
		else
			val = 0;
	}
	return rv;
}

int XmlSynthElem::GetAttribute(const char *attrName, char **val)
{
	int rv = -1;
	*val = NULL;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_BSTR);
			*val = WideToMulti(v.bstrVal, SysStringLen(v.bstrVal));
			if (*val)
				rv = 0;
		}
	}
	return rv;
}

int XmlSynthElem::GetAttribute(const char *attrName, bsString& val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v;
		if (pElem->getAttribute(name, &v) == S_OK)
		{
			v.ChangeType(VT_BSTR);
			val.Assign(v.bstrVal);
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
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, float val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, double val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
	return rv;
}

int XmlSynthElem::SetAttribute(const char *attrName, const char *val)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR name(attrName);
		// todo: CComVariant uses the system code page.
		// this needs to use CP_UTF8
		CComVariant v(val);
		if (pElem->setAttribute(name, v) == S_OK)
			rv = 0;
	}
	return rv;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	if (nodeTag != NULL)
		return strcmp(nodeTag, tag) == 0;
	return tag == NULL;
}

int XmlSynthElem::SetContent(const char *data)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR v(data);
		if (pElem->put_text(v) == S_OK)
			rv = 0;
	}
	return rv;
}

int XmlSynthElem::GetContent(char **data)
{
	int rv = -1;
	if (pElem)
	{
		CComBSTR v;
		if (pElem->get_text(&v.m_str) == S_OK)
		{
			*data = WideToMulti(v.m_str, v.Length());
			if (*data)
				rv = 0;
		}
	}
	return rv;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
}

XmlSynthDoc::~XmlSynthDoc()
{
}

int XmlSynthDoc::GetXmlDoc()
{
	if (!pDoc)
	{
		HRESULT hr = pDoc.CoCreateInstance(L"MSXML2.DomDocument");
		if (hr != S_OK)
			return -1;
	}

	return 0;
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	return CreateElement(parent, tag, 0);
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag, XmlSynthElem *newElem)
{
	if (newElem == 0)
		newElem = new XmlSynthElem(this);
	if (pDoc)
	{
		CComBSTR bsTag(tag);
		CComPtr<IXMLDOMElement> pnew;
		CComPtr<IXMLDOMNode> pout;
		pDoc->createElement(bsTag, &pnew);
		parent->pElem->appendChild(pnew, &pout);
		//newElem->pElem = pout;
		newElem->doc = this;
		newElem->SetNode(pout);
	}
	return newElem;
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag)
{
	XmlSynthElem *rootElem = new XmlSynthElem(this);
	if (NewDoc(roottag, rootElem))
		return rootElem;
	delete rootElem;
	return NULL;
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag, XmlSynthElem *rootElem)
{
	if (!GetXmlDoc())
	{
		HRESULT hr;
		CComPtr<IXMLDOMNode> pout;
		CComPtr<IXMLDOMProcessingInstruction> pi;
		pDoc->createProcessingInstruction(L"xml", L"version='1.0'", &pi);
		pDoc->appendChild(pi, &pout);
		pout.Release();

		if (roottag == NULL)
			roottag = "synth";

		CComBSTR bsTag(roottag);
		CComPtr<IXMLDOMElement> pnew;
		hr = pDoc->createElement(bsTag, &pnew);
		if (hr == S_OK)
		{
			pDoc->appendChild(pnew, &pout);
			rootElem->SetNode(pout);
			rootElem->SetAttribute("version", 2);
			return rootElem;
		}
	}
	return NULL;
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname)
{
	XmlSynthElem *rootElem = new XmlSynthElem(this);
	if (Open(fname, rootElem))
		return rootElem;
	delete rootElem;
	return NULL;
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname, XmlSynthElem *rootElem)
{
	if (fname == NULL || strlen(fname) == 0)
		return NULL;
	if (!GetXmlDoc())
	{
		HRESULT hr;
		CComVariant vname(fname);
		VARIANT_BOOL bSucess = 0;
		//pDoc->put_preserveWhiteSpace(VARIANT_TRUE);
		hr = pDoc->load(vname, &bSucess);
		if (hr == S_OK && bSucess)
		{
			CComPtr<IXMLDOMElement> xmlroot;
			pDoc->get_documentElement(&xmlroot);
			rootElem->SetNode(xmlroot);
			return rootElem;
		}
	}
	return NULL;
}

int XmlSynthDoc::Save(const char *fname)
{
	int rv = -1;
	if (pDoc)
	{
		//pDoc->put_preserveWhiteSpace(VARIANT_TRUE);
		CComVariant vdest(fname);
		rv = pDoc->save(vdest) == S_OK ? 0 : -1;
	}
	return rv;
}

int XmlSynthDoc::Close()
{
	pDoc.Release();
	return 0;
}

#else
int _not_using_msxml = 1;
#endif
