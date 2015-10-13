//////////////////////////////////////////////////////////////////////
/// @file InitRes.cpp Load resources
/// Resources define the dialogs. They can be stored
/// in an XML file or compiled into the program. During
/// development it is better to just compile in the resources
/// so that we don't have to worry about having the file in 
/// a known directory. For the installed release version, we
/// want the resources in an external file so that they can be
/// localized.
///////////////////////////////////////////////////////////////////////
#include "globinc.h"

#ifndef USE_COMPILED_RESOURCE
#define USE_COMPILED_RESOURCE 0
#endif

#if USE_COMPILED_RESOURCE
#include "resource.cpp"

bool InitResources()
{
	wxXmlResource::Get()->InitAllHandlers();
	InitXmlResource();
	return true;
}

#else

bool InitResources()
{
	// TODO: look for $(LANG)resource.xml first...
	wxXmlResource *res = wxXmlResource::Get();
	res->InitAllHandlers();
	wxFileName fn;
	fn.SetName("resource.xml");
	if (!fn.FileExists())
	{
		fn.SetPath(wxStandardPaths::Get().GetDataDir());
		if (!fn.FileExists())
			return false;
	}
	return res->Load(fn.GetFullPath());
}

#endif
