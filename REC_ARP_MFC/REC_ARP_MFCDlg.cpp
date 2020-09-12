
// REC_ARP_MFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "REC_ARP_MFC.h"
#include "REC_ARP_MFCDlg.h"
#include "afxdialogex.h"
#include "winsock2.h" 
#include "iphlpapi.h"  
#include "pcap.h" 
#include <windows.h>
#include <ws2ipdef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

pcap_if_t* Dev, * allDevs;
pcap_t* currentOpenDev;
CString str;
PIP_ADAPTER_INFO pAdapter = 0;
PIP_ADAPTER_INFO pAdapterInf = 0;
PIP_ADAPTER_INFO SelectedAdapter = 0;
ULONG uBuf = 0;
DWORD opinf;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRECARPMFCDlg 对话框



CRECARPMFCDlg::CRECARPMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REC_ARP_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRECARPMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INF, INF);
	DDX_Control(pDX, IDC_START_BUT, START);
	DDX_Control(pDX, IDC_END_BUT, END);
	DDX_Control(pDX, IDC_DEC_COM, DEC_COM);
}

BEGIN_MESSAGE_MAP(CRECARPMFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_BUT, &CRECARPMFCDlg::OnBnClickedStartBut)
	ON_BN_CLICKED(IDC_END_BUT, &CRECARPMFCDlg::OnBnClickedEndBut)
	ON_CBN_SELCHANGE(IDC_DEC_COM, &CRECARPMFCDlg::OnCbnSelchangeDecCom)
END_MESSAGE_MAP()


// CRECARPMFCDlg 消息处理程序

BOOL CRECARPMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	opinf = GetAdaptersInfo(pAdapter, &uBuf);
	if (opinf == ERROR_BUFFER_OVERFLOW)
	{
		pAdapter = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, uBuf);//获取本地所有适配器并赋值
		pAdapterInf = pAdapter;	//将适配器信息赋值，防止接下来出现错误
		opinf = GetAdaptersInfo(pAdapter, &uBuf);	//获取本地网络信息，并赋值返回值
		if (opinf == ERROR_SUCCESS)
		{
			while (pAdapterInf)//当适配器信息不空
			{
				DEC_COM.AddString(pAdapterInf->Description);	//添加适配器信息至ComboBox中
				pAdapterInf = pAdapterInf->Next;	//继续下一个适配器
			}
		}
	}
	m_Thread = NULL;

	INF.SetHorizontalExtent(2000);

	//默认将停止按钮设置为FALSE防止误触
	END.EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRECARPMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRECARPMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		//CDialogEx::OnPaint();
		CPaintDC   dc(this);
		CRect rect;
		GetClientRect(&rect);
		CDC   dcMem;
		dcMem.CreateCompatibleDC(&dc);
		CBitmap   bmpBackground;
		bmpBackground.LoadBitmap(IDB_BITMAP1);  //对话框的背景图片  

		BITMAP   bitmap;
		bmpBackground.GetBitmap(&bitmap);
		CBitmap* pbmpOld = dcMem.SelectObject(&bmpBackground);
		dc.StretchBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRECARPMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




DWORD WINAPI RecInf(LPVOID lpParameter)
{
	CRECARPMFCDlg* crlg = (CRECARPMFCDlg*)lpParameter;

	char errBuf[PCAP_ERRBUF_SIZE]; //存放错误信息的缓冲
	pcap_findalldevs(&allDevs, errBuf);//列举所有设备 

	CString a = SelectedAdapter->AdapterName;

	CString subfromIpHelper = a.Mid(a.ReverseFind('{') + 1, 4);
	for (Dev = allDevs; Dev; Dev = Dev->next)
	{
		a = Dev->name;
		CString subfromWinpcap = a.Mid(a.ReverseFind('{') + 1, 4);
		if (subfromWinpcap == subfromIpHelper)
		{
			break;
		}
	}
	if ((currentOpenDev = pcap_open(Dev->name, 65535, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errBuf)) == NULL)//打开设备 
	{
		return 0;
	}

	pcap_pkthdr* hdr;
	const u_char* pkt_data;

	while (true)
	{
		//使用pcap_next_ex捕获数据报
		if (pcap_next_ex(currentOpenDev, &hdr, &pkt_data) > 0)
		{

			unsigned char* data = NULL;	//定义获取的数据包

			data = (unsigned char*)pkt_data;	//将捕获的数据包赋值

			unsigned char GetFrame[60];		//用于转换格式


			for (int i = 0; i < 60; i++) //将data转换为char[60] 方便获取数据
			{
				GetFrame[i] = *data;
				*data = *data + 1;
			}

			//限定格式，使捕获的数据包为指定的数据包
			if (data[12] == 0x08 && data[13] == 0x06 && data[20] == 0x00 && data[21] == 0x02)
			{
				//将发送IP写入字符串中
				str.Format("%d.%d.%d.%d", data[28], data[29], data[30], data[31]);
				in_addr ipAddress;

				//将点分十进制的IP转换成一个长整数型数存入定义的ipAddress中
				ipAddress.S_un.S_addr = inet_addr(str);

				//定义主机信息结构体
				hostent* pht = NULL;

				//通过ip地址获取设备信息
				pht = gethostbyaddr((char*)&ipAddress, sizeof(ipAddress), AF_INET);

				//当主机信息不空时，输出主机名
				if (pht != NULL)
				{
					str.Format("协议类型：%02x%02x，ETH_IP      帧类型：%02x%02x，ETH_ARP     操作类型：%02x%02x，ARP_RESPONSE     发送方ip:%d.%d.%d.%d     发送方mac:%02X-%02X-%02X-%02X-%02X-%02X        目标mac地址：%02x-%02x-%02x-%02x-%02x-%02x        目标ip地址：%d.%d.%d.%d          发送主机名:%15s",
						data[16], data[17],
						data[12], data[13],
						data[20], data[21],
						data[28], data[29], data[30], data[31],
						data[6], data[7], data[8], data[9], data[10], data[11],
						data[32], data[33], data[34], data[35], data[36], data[37],
						data[38], data[39], data[40], data[41],
						pht->h_name
					);
				}
				//无法获取主机信息则输出未知主机名
				else
				{
					str.Format("协议类型：%02x%02x，ETH_IP      帧类型：%02x%02x，ETH_ARP     操作类型：%02x%02x，ARP_RESPONSE     发送方ip:%d.%d.%d.%d       发送方mac:%02X-%02X-%02X-%02X-%02X-%02X        目标mac地址：%02x-%02x-%02x-%02x-%02x-%02x        目标ip地址：%d.%d.%d.%d          发送主机名:%15s",
						data[16], data[17],
						data[12], data[13],
						data[20], data[21],
						data[28], data[29], data[30], data[31],
						data[6], data[7], data[8], data[9], data[10], data[11],
						data[32], data[33], data[34], data[35], data[36], data[37],
						data[38], data[39], data[40], data[41],
						"未知主机名"
					);
				}
				//通过AddString方法直接向主线程界面更新数据
				crlg->INF.AddString(str);

			}
			crlg->INF.Invalidate();	//这句是刷新界面,True刷新，False则不刷新
		}
	}
}


//开始按钮的设计
void CRECARPMFCDlg::OnBnClickedStartBut()
{
	//将所有设备信息赋值
	SelectedAdapter = pAdapter;

	CString selectedItem;

	//获取当前ComboBox所选数据
	DEC_COM.GetLBText(DEC_COM.GetCurSel(), selectedItem);

	//循环将选择设备赋值
	while (SelectedAdapter->Description != selectedItem)
	{
		SelectedAdapter = SelectedAdapter->Next;
	}

	// TODO: 在此添加控件通知处理程序代码
	if (SelectedAdapter == 0)
	{
		MessageBox("请先于左侧框内选择设备！");
	}
	else
	{
		//将停止按钮恢复，将开始按钮设为FALSE
		END.EnableWindow(TRUE);
		START.EnableWindow(FALSE);

		//若线程为空，则开始线程工作
		if (m_Thread == NULL)
		{
			m_Thread = CreateThread(NULL, 0, RecInf, this, 0, NULL);
		}
		else 
		{
			ResumeThread(m_Thread);
		}

	}

}


//暂停按钮的设计
void CRECARPMFCDlg::OnBnClickedEndBut()
{
	//将停止按钮设为FALSE，启动按钮恢复工作
	END.EnableWindow(FALSE);
	START.EnableWindow(TRUE);

	//暂停线程的工作
	SuspendThread(m_Thread);
}


void CRECARPMFCDlg::OnCbnSelchangeDecCom()
{
	// TODO: 在此添加控件通知处理程序代码
}
