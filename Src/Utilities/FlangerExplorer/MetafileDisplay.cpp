//////////////////////////////////////////////////////////////////////
// Load and draw a metafile in a window.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MetafileDisplay.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMetafileDisplay::CMetafileDisplay()
{
	enhMF = NULL;
}

CMetafileDisplay::~CMetafileDisplay()
{
	if (enhMF != NULL)
	{
		DeleteEnhMetaFile(enhMF);
		enhMF = NULL;
	}
}

int CMetafileDisplay::CreateFromResourceName(LPCTSTR resName)
{
	HMODULE hmod = _Module.GetResourceInstance();
	HRSRC hres = FindResource(hmod, resName, _T("METAFILE"));
	return LoadBits((UINT)SizeofResource(hmod, hres),
	                      LockResource(LoadResource(hmod, hres)));
}

int CMetafileDisplay::CreateFromResourceID(UINT resID)
{
	HMODULE hmod = _Module.GetResourceInstance();
	HRSRC hres = FindResource(hmod, MAKEINTRESOURCE(resID), _T("METAFILE"));
	return LoadBits((UINT)SizeofResource(hmod, hres),
	                      LockResource(LoadResource(hmod, hres)));
}

int CMetafileDisplay::CreateFromFile(LPCTSTR fileName)
{
	if (enhMF != NULL)
	{
		DeleteEnhMetaFile(enhMF);
		enhMF = NULL;
	}

	int ret = 0;
	// Open the file, find out how big it is, allocate memory then read it in
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != (HANDLE)-1)
	{
		LPVOID pBuf = NULL;
		DWORD dwFileHigh = 0;
		DWORD dwFile = GetFileSize(hFile, &dwFileHigh);
		if (dwFileHigh == 0 && (pBuf = LocalAlloc(0, dwFile)) != NULL)
		{
			DWORD dwRead;
			if (ReadFile(hFile, pBuf, dwFile, &dwRead, NULL) && dwRead == dwFile)
				ret = LoadBits((UINT) dwFile, pBuf);
			LocalFree(pBuf);
		}
		else
			ret = -1;
		CloseHandle(hFile);
	}
	else
	{
		ret = -1;
		ATLTRACE(_T("Metafile Load: %d\n"), GetLastError());
	}
	return ret;
}

int CMetafileDisplay::CreateFromClipboard( void)
{
	if (enhMF != NULL)
	{
		DeleteEnhMetaFile(enhMF);
		enhMF = NULL;
	}
	OpenClipboard();
	HENHMETAFILE enhMFNew = (HENHMETAFILE) GetClipboardData(CF_ENHMETAFILE);
	CloseClipboard();
	if (enhMFNew != NULL)
		enhMF = CopyEnhMetaFile(enhMFNew, NULL);
	return enhMF == NULL ? -1 : 0;
}

int CMetafileDisplay::LoadBits(UINT uiBuf, LPVOID pBuf)
{
#pragma pack(2)
typedef struct 
{
	DWORD   key;
	WORD	hmf;
	short	left;
	short	top;
	short	right;
	short	bottom;
	WORD    inch;
	DWORD	reserved;
	WORD    checksum;
} AMETAHEADER;
#pragma pack()

	if (enhMF != NULL)
	{
		DeleteEnhMetaFile(enhMF);
		enhMF = NULL;
	}

	// See if this is an enhanced metafile
	ENHMETAHEADER *emhdr = (ENHMETAHEADER *)pBuf;
	if (uiBuf >= sizeof(ENHMETAHEADER)
	 && emhdr->iType == EMR_HEADER
	 && emhdr->dSignature == ENHMETA_SIGNATURE)
	{
		enhMF = SetEnhMetaFileBits(uiBuf, (CONST BYTE *)pBuf);
	}
	else if (uiBuf >= sizeof(METAHEADER))
	{
		METAFILEPICT mp = {MM_ANISOTROPIC, 1000, 1000, NULL};
		AMETAHEADER *ahdr = (AMETAHEADER *)pBuf;
		METAHEADER *mhdr = (METAHEADER *)pBuf;
		// See if this is a placeable metafile
		if (ahdr->key == 0x9AC6CDD7)
		{
			mp.xExt = ((LONG)(ahdr->right - ahdr->left) * 25401) / (LONG)ahdr->inch;
			mp.yExt = ((LONG)(ahdr->bottom - ahdr->top) * 25401) / (LONG)ahdr->inch;
			mhdr = (METAHEADER *) (ahdr + 1);
		}
		UINT uiSizeMF = (UINT) mhdr->mtSize * 2;
		HDC dcRef = ::GetDC(NULL);
		enhMF = SetWinMetaFileBits(uiSizeMF, (LPBYTE)mhdr, dcRef, &mp);
		::ReleaseDC(NULL, dcRef);
	}

	return (enhMF == NULL) ? -1 : 0;
}

LRESULT CMetafileDisplay::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC dc(m_hWnd);

	if (enhMF != NULL)
	{
		RECT rc;
		GetClientRect(&rc);
		int cx = rc.right - rc.left;
		int cy = rc.bottom - rc.top;
		int tmp;

		SIZE szPic;
		ENHMETAHEADER hdr;
		::GetEnhMetaFileHeader(enhMF, sizeof(hdr), &hdr);
		szPic.cx = hdr.rclBounds.right - hdr.rclBounds.left;
		szPic.cy = hdr.rclBounds.bottom - hdr.rclBounds.top;

		if (szPic.cx == 0)
			szPic.cx = 1;
		if (szPic.cy == 0)
			szPic.cy = 1;
		if (szPic.cx == szPic.cy)
		{
			// make drawing rect square
			if (cx > cy)
			{
				rc.left = (cx - cy) / 2;
				rc.right = rc.left + cy;
			}
			else if (cy > cx)
			{
				rc.top = (cy - cx) / 2;
				rc.bottom = rc.top + cx;
			}
		}
		else
		{
			float r1 = (float) szPic.cx / (float) szPic.cy;
			float r2 = (float) cx / (float) cy;
			if (r1 < 1)
			{
				// h > w
				if (r2 < r1)
				{
					tmp = (int) (r1 * (float) cy);
					rc.left = (cx - tmp) / 2;
					rc.right = rc.left + tmp;
				}
			}
			else
			{
				// w > h
				if (r2 < r1)
				{
					tmp = (int) ((float) cx / r1);
					rc.top = (cy - tmp) / 2;
					rc.bottom = rc.top + tmp;
				}
			}
		}
		rc.top += 2;
		rc.left += 2;
		rc.right -= 2;
		rc.bottom -=2;
		PlayEnhMetaFile(dc, enhMF, &rc);
	}
	return 0;
}
