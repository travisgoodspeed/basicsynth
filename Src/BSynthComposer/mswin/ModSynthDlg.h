//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MODSYNTH_DLG_H
#define MODSYNTH_DLG_H

#include "resource.h"

class ModSynthConfig : public CDialogImpl<ModSynthConfig>
{
private:
	ModSynth *ms;
	int changed;
	CButton btnUp;
	CButton btnDn;
	CButton btnAdd;
	CButton btnRem;
	CListBox lbType;
	CListBox lbUgens;
	CEdit edName;

	void EnableButtons();

public:
	ModSynthConfig(ModSynth *in)
	{
		changed = 0;
		ms = in;
	}

	enum { IDD = IDD_MODSYNTH_CONFIG };

	BEGIN_MSG_MAP(ModSynthConfig)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_MS_ADD, BN_CLICKED, OnAdd)
		COMMAND_HANDLER(IDC_MS_REM, BN_CLICKED, OnRemove)
		COMMAND_HANDLER(IDC_MS_MVUP, BN_CLICKED, OnMoveUp)
		COMMAND_HANDLER(IDC_MS_MVDN, BN_CLICKED, OnMoveDown)
		COMMAND_HANDLER(IDC_MS_TYPE, LBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(IDC_MS_UGENS, LBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(IDC_MS_NAME, EN_CHANGE, OnChange)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

class ModSynthConnect : public CDialogImpl<ModSynthConnect>
{
private:
	ModSynth *ms;
	int changed;
	CListBox lbConn;
	CListBox lbSrc;
	CListBox lbDst;
	CListBox lbParam;
	CButton btnAdd;
	CButton btnRem;

	struct ConnInfo
	{
		ModSynthUG *src;
		ModSynthUG *dst;
		int index;
	};

	void ClearConnInfo();
	void EnableButtons();

public:
	ModSynthConnect(ModSynth *in)
	{
		ms = in;
		changed = 0;
	}

	enum { IDD = IDD_MODSYNTH_CONNECT };

	BEGIN_MSG_MAP(ModSynthConnect)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_MC_ADD, BN_CLICKED, OnAdd)
		COMMAND_HANDLER(IDC_MC_REM, BN_CLICKED, OnRemove)
		COMMAND_HANDLER(IDC_MC_SRC, LBN_SELCHANGE, OnSelSrc)
		COMMAND_HANDLER(IDC_MC_DST, LBN_SELCHANGE, OnSelDst)
		COMMAND_HANDLER(IDC_MC_PARAM, LBN_SELCHANGE, OnSelChange)
		COMMAND_HANDLER(IDC_MC_CONN, LBN_SELCHANGE, OnSelChange)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelSrc(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelDst(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};
#endif
