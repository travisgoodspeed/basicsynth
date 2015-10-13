#pragma once

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