#pragma once


// Диалоговое окно CDialogEdit1

class CDialogEdit1 : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogEdit1)

public:
	CDialogEdit1(CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~CDialogEdit1();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_EDIT1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CString m_tcpPort;
	CIPAddressCtrl m_IP;
	afx_msg void OnBnClickedOk();
};
