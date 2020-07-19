
// REC_ARP_MFCDlg.h: 头文件
//

#pragma once


// CRECARPMFCDlg 对话框
class CRECARPMFCDlg : public CDialogEx
{
// 构造
public:
	CRECARPMFCDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REC_ARP_MFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CListBox INF;
	CButton START;
	CButton END;
	HANDLE m_Thread;
	CComboBox DEC_COM;
	afx_msg void OnBnClickedStartBut();
	afx_msg void OnBnClickedEndBut();
	afx_msg void OnCbnSelchangeDecCom();
};
