<<<<<<< HEAD
#pragma once
#include <afxinet.h>
#define MAX_FTP_COUNT 256
class CFtpProcess
{
public:
	CFtpProcess(void);
	~CFtpProcess(void);

	CInternetSession *m_pNetSession[MAX_FTP_COUNT];
	CFtpConnection *m_ftpConnect[MAX_FTP_COUNT];
	
	CFtpFileFind *m_pFtpFileFind[MAX_FTP_COUNT];
	bool m_bOccupy[MAX_FTP_COUNT];

	int m_nCurCount;
	bool ConnectFtp(CString strIP, CString strUser, CString strPsw, int nConnCount); //ftp连接 
	bool uploadFtpFile(CString strRemoteFile, CString strCurFile);
	bool downLoadFile(CString strRemoteFile, CString strCurFile);
};

=======
#pragma once
#include <afxinet.h>
#define MAX_FTP_COUNT 256
class CFtpProcess
{
public:
	CFtpProcess(void);
	~CFtpProcess(void);

	CInternetSession *m_pNetSession[MAX_FTP_COUNT];
	CFtpConnection *m_ftpConnect[MAX_FTP_COUNT];
	
	CFtpFileFind *m_pFtpFileFind[MAX_FTP_COUNT];
	bool m_bOccupy[MAX_FTP_COUNT];

	int m_nCurCount;
	bool ConnectFtp(CString strIP, CString strUser, CString strPsw, int nConnCount); //ftp连接 
	bool uploadFtpFile(CString strRemoteFile, CString strCurFile);
	bool downLoadFile(CString strRemoteFile, CString strCurFile);
};

>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
