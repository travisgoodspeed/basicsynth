//////////////////////////////////////////////////////////////////////
// MetafileDisplay.h: window class to display a metafile
//
//////////////////////////////////////////////////////////////////////

#if !defined(_METAFILEDISPLAY_H_)
#define _METAFILEDISPLAY_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMetafileDisplay  : public CWindowImpl<CMetafileDisplay>
{
private:
	HENHMETAFILE enhMF;

public:
	CMetafileDisplay();
	virtual ~CMetafileDisplay();

	DECLARE_WND_CLASS(_T("MetafileDisplay"))

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	int CreateFromResourceName(LPCTSTR resName);
	int CreateFromResourceID(UINT id);
	int CreateFromFile(LPCTSTR filename);
	int CreateFromClipboard( void);

	int LoadBits(UINT uiBuf, LPVOID pBuf);
};

#endif // !defined(AFX_METAFILEDISPLAY_H__026E0B70_DD48_4A33_BCF2_D11BB5FBEEEE__INCLUDED_)
