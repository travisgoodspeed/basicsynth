//////////////////////////////////////////////////////////////////
/// @file XmlWrapT.cpp TinyXML wrapper
//
// BasicSynth
//
// TinyXML XmlWrapper - see: www.sourceforge.net/projects/tinyxml
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////
#include <SynthString.h>
#include <XmlWrap.h>
#if defined(USE_TINYXML)
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>

XmlSynthElem::XmlSynthElem(XmlSynthDoc *p)
{
	doc = p;
	pElem = 0;
}

XmlSynthElem::~XmlSynthElem()
{
}

// Early versions of this code marked the file
// as encoding=UTF-8 but did not really convert
// the text to UTF-8.  A partial work-around
// is to simply change the single-byte characters
// into wide characters, then converts to UTF-8
// and hope for the best. This works for ISO-8859-1,
// but will store invalid characters for almost
// everything else. A better workaround is to
// change the encoding attribute in the file,
// then use setlocale(LC_CTYPE, encoding) and
// mbstowcs() to convert to UNICODE.
void XmlSynthElem::ConvertUTF8(const char *in, bsString& out)
{
/* TODO:
	if (in is not valid UTF-8)
	{
		// use the locale from the xml file to set the codepage,
		// call mbstowcs to get a UTF-16 string,
		// set the bsString using the UTF-16, which
		// wil then automatically convert to UTF-8.
	}
	else
*/
	out.Assign(in);
}

void XmlSynthElem::Clear()
{
	pElem = 0;
}

XmlSynthElem *XmlSynthElem::FirstChild()
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (FirstChild(ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::FirstChild(XmlSynthElem *ret)
{
	if (pElem)
	{
		const TiXmlElement *n = pElem->FirstChildElement();
		if (n)
		{
			ret->doc = doc;
			ret->SetNode((TiXmlElement *)n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling()
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (NextSibling(ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::NextSibling(XmlSynthElem *ret)
{
	if (pElem)
	{
		const TiXmlElement *n = pElem->NextSiblingElement();
		if (n)
		{
			ret->doc = doc;
			ret->SetNode((TiXmlElement*)n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag)
{
	if (pElem)
	{
		XmlSynthElem *ret = new XmlSynthElem(doc);
		if (AddChild(childTag, ret))
			return ret;
		delete ret;
	}
	return 0;
}

XmlSynthElem *XmlSynthElem::AddChild(const char *childTag, XmlSynthElem *ret)
{
	if (pElem && childTag)
	{
		TiXmlElement *n = new TiXmlElement(childTag);
		if (n)
		{
			pElem->LinkEndChild(n);
			ret->doc = doc;
			ret->SetNode(n);
			return ret;
		}
	}
	ret->SetNode(0);
	return 0;
}

const char *XmlSynthElem::TagName()
{
	if (pElem)
		return pElem->Value();
	return "";
}

int XmlSynthElem::GetAttribute(const char *attrName, short& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = (short) bsString::StrToNum(str);
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, long& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = bsString::StrToNum(str);
			return 0;
		}
	}
	val = 0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, float& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = (float) bsString::StrToFlp(str);
			return 0;
		}
	}

	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, double& val)
{
	if (pElem)
	{
		const char *str = pElem->Attribute(attrName);
		if (str)
		{
			val = bsString::StrToFlp(str);
			return 0;
		}
	}
	val = 0.0;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, char **val)
{
	if (pElem)
	{
		const char *s = pElem->Attribute(attrName);
		if (s)
		{
			bsString tmp;
			ConvertUTF8(s, tmp);
			*val = tmp.Detach();
			return 0;
		}
	}
	*val = NULL;
	return -1;
}

int XmlSynthElem::GetAttribute(const char *attrName, bsString& val)
{
	if (pElem)
	{
		const char *s = pElem->Attribute(attrName);
		if (s)
		{
			ConvertUTF8(s, val);
			return 0;
		}
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, short val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, long val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, (int) val);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, float val)
{
	return SetAttribute(attrName, (double) val);
}

int XmlSynthElem::SetAttribute(const char *attrName, double val)
{
	if (pElem)
	{
		bsString strval(val);
		pElem->SetAttribute(attrName, (const char *)strval);
		return 0;
	}
	return -1;
}

int XmlSynthElem::SetAttribute(const char *attrName, const char *val)
{
	if (pElem)
	{
		pElem->SetAttribute(attrName, val ? val : "");
		return 0;
	}
	return -1;
}

int XmlSynthElem::TagMatch(const char *tag)
{
	if (pElem)
	{
		if (strcmp(pElem->Value(), tag) == 0)
			return 1;
	}
	return 0;
}

int XmlSynthElem::SetContent(const char *data)
{
	if (pElem)
	{
		TiXmlText *t = new TiXmlText(data ? data : "");
		if (t)
		{
			pElem->LinkEndChild(t);
			return 0;
		}
	}
	return -1;
}

int XmlSynthElem::GetContent(char **data)
{
	if (pElem)
	{
		const TiXmlNode *n = pElem->FirstChild();
		const TiXmlText *t;
		while (n)
		{
			if ((t = n->ToText()))
			{
				const char *s = t->Value();
				if (s)
				{
					bsString tmp;
					ConvertUTF8(s, tmp);
					*data = tmp.Detach();
					return 0;
				}
			}
			n = n->NextSibling();
		}
	}
	*data = 0;
	return -1;
}

//////////////////////////////////////

XmlSynthDoc::XmlSynthDoc()
{
	isUTF8 = 0;
	prjVersion = 0.0;
}

XmlSynthDoc::~XmlSynthDoc()
{
}

XmlSynthElem *XmlSynthDoc::CreateElement(XmlSynthElem *parent, const char *tag)
{
	return parent->AddChild(tag);
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag)
{
	return NewDoc(roottag, 0);
}

XmlSynthElem *XmlSynthDoc::NewDoc(const char *roottag, XmlSynthElem *root)
{
	xmlEncoding = "UTF-8";
	xmlLocale = setlocale(LC_CTYPE, NULL);
	prjVersion = 2.0;
	isUTF8 = true;

	doc = new TiXmlDocument;
	if (doc)
	{
		if (root == 0)
			root = new XmlSynthElem(this);
		else
			root->doc = this;
		if (root)
		{
			TiXmlDeclaration *d = new TiXmlDeclaration("1.0","UTF-8","yes");
			if (d)
				doc->LinkEndChild(d);
			TiXmlElement *e = new TiXmlElement(roottag ? roottag : "synth");
			if (e)
			{
				doc->LinkEndChild(e);
				root->SetNode(e);
				root->SetAttribute("version", prjVersion);
				root->SetAttribute("locale", xmlLocale);
				return root;
			}
		}
	}
	return 0;
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname)
{
	return Open(fname, 0);
}

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
static FILE *OpenXmlFile(const char *fname, const char *mode)
{
	wchar_t wbuf[MAX_PATH];
#if _WIN32
    // convert to UNICODE
	bsString::utf16(fname, wbuf, MAX_PATH);
	wchar_t wmode[20];
	bsString::utf16(mode, wmode, 20);
	return _wfopen(wbuf, wmode);
#else
	char cbuf[MAX_PATH];
    char *lc = setlocale(LC_CTYPE, NULL);
    if (lc && strcmp(lc, "C"))
    {
        char *dot = strchr(lc, '.');
        if (dot)
        {
            // the user specified a codepage.
            // if it is not UTF-8, convert...
            if (strcmp(dot, ".UTF-8") != 0
			 && strcmp(dot, ".utf-8") != 0)
            {
				bsString::utf16(fname, wbuf, MAX_PATH);
				cbuf[0] = 0;
				if (wcstombs(cbuf, wbuf, wcslen(wbuf)+1) != (size_t)-1)
	            	fname = cbuf;
            }
        }
    }
	return fopen(fname, mode);
#endif
}

XmlSynthElem *XmlSynthDoc::Open(const char *fname, XmlSynthElem* root)
{
	doc = new TiXmlDocument;
	doc->SetTabSize(0);
	if (fname)
	{
		FILE *fp = OpenXmlFile(fname, "rb");
		if (fp)
		{
			isUTF8 = 0;
			prjVersion = 0;

			int res = doc->LoadFile(fp, TIXML_ENCODING_UNKNOWN);
			fclose(fp);
			if (res)
			{
				TiXmlNode *n = 0;
				while ((n = doc->IterateChildren(n)) != 0)
				{
					if (n->ToDeclaration())
					{
						TiXmlDeclaration *decl = n->ToDeclaration();
						xmlEncoding = decl->Encoding();
						if (xmlEncoding.CompareNC("UTF-8"))
							isUTF8 = 1;
						break;
					}
				}

				TiXmlElement *e = doc->RootElement();
				if (e)
				{
                    prjVersion = 0;
                    xmlLocale = e->Attribute("locale");
                    e->Attribute("version", &prjVersion);
                    if (root == 0)
                        root = new XmlSynthElem(this);
                    else
                        root->doc = this;
                    root->SetNode(e);
                    return root;
				}
			}
		}
	}
	return 0;
}

int XmlSynthDoc::Save(const char *fname)
{
	if (doc && fname)
	{
		FILE *fp = OpenXmlFile(fname, "w");
		if (fp)
		{
			int res = doc->SaveFile(fp);
			fclose(fp);
			if (res)
				return 0;
		}
	}
	return -1;
}

int XmlSynthDoc::Close()
{
	delete doc;
	doc = 0;
	return 0;
}

#else
int _not_using_tiny_xml = 1;
#endif
