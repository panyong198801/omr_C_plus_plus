<<<<<<< HEAD
#include "StdAfx.h"
#include "FtpProcess.h"

CFtpProcess::CFtpProcess(void)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		m_pNetSession[i] = NULL;
		m_ftpConnect[i] = NULL; 
		m_pFtpFileFind[i] = NULL;
		m_bOccupy[i] = false;
	}

	m_nCurCount = 30;
}

CFtpProcess::~CFtpProcess(void)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		if (m_pNetSession[i] != NULL)
		{
			m_pFtpFileFind[i]->Close();
			m_ftpConnect[i]->Close();
			InternetCloseHandle(m_pFtpFileFind[i]);
			
			InternetCloseHandle( m_ftpConnect[i]);
			InternetCloseHandle( m_pNetSession[i] );

			
			
			delete m_pFtpFileFind[i];
			m_pFtpFileFind[i] = NULL;
			
			delete m_ftpConnect[i];
			m_ftpConnect[i] = NULL;

			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
		}
	}
}


bool CFtpProcess::ConnectFtp(CString strIP, CString strUser, CString strPsw, int nConnCount)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		if (m_pNetSession[i] != NULL)
		{
			InternetCloseHandle(m_pFtpFileFind[i]);
			InternetCloseHandle( m_ftpConnect[i]);
			InternetCloseHandle( m_pNetSession[i] );

			delete m_pFtpFileFind[i];
			m_pFtpFileFind[i] = NULL;

			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
		}

		m_bOccupy[i] = false;
	}

	m_nCurCount = nConnCount;

	if (m_nCurCount < 0)
		m_nCurCount = 1;

	if (m_nCurCount > MAX_FTP_COUNT)
		m_nCurCount = MAX_FTP_COUNT;

	for (int i=0; i<m_nCurCount; i++)
	{
		m_pNetSession[i] = new CInternetSession(NULL,1,PRE_CONFIG_INTERNET_ACCESS);

		try 
		{ 
			//新建连接对象
			m_ftpConnect[i]= m_pNetSession[i]->GetFtpConnection(strIP,strUser,strPsw); 
		} 
		catch(CInternetException *pEx)  //有异常
		{
			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
			m_ftpConnect[i] = NULL;
			pEx->Delete();
			return false;
		}

		if (!m_pFtpFileFind[i])
			m_pFtpFileFind[i] =  new CFtpFileFind(m_ftpConnect[i]);
	}

	return true;
}

bool CFtpProcess::downLoadFile(CString strRemoteFile, CString strCurFile)
{
	bool bFind = false;
	int nCount = 0;
	while(!bFind && nCount <= 300000)
	{
		for (int i=0; i<m_nCurCount; i++)
		{
			if (!m_bOccupy[i] && m_ftpConnect[i])
			{
				bFind = true;
				m_bOccupy[i] = true; //已经被占用 pany 2016.05.12
				
				//判断本地文件是否存在
				if (GetFileAttributes(strCurFile)!= 0xFFFFFFFF)
				{
					::DeleteFile(strCurFile);
				}
				
				if (!m_pFtpFileFind[i]->FindFile(strRemoteFile)) //ftp上没有该文件
				{
					m_bOccupy[i] = false;
					return false;
				}

				if (!m_ftpConnect[i]->GetFile(strRemoteFile, strCurFile))
				{
					m_bOccupy[i] = false;
					return false;
				}
				else 
				{
					m_bOccupy[i] = false;
					return TRUE;
				}
			}
		}

		if (!bFind)
		{
			int nSleepTime = 200;
			Sleep(nSleepTime);
			nCount += nSleepTime;
		}
	}

	return false;
}

bool CFtpProcess::uploadFtpFile(CString strRemoteFile, CString strCurFile)
{
	bool bFind = false;
	int nCount = 0;
	while(!bFind && nCount <= 300000)
	{
		for (int i=0; i<m_nCurCount; i++)
		{
			if (!m_bOccupy[i] && m_ftpConnect[i])
			{
				bFind = true;
				m_bOccupy[i] = true; //已经被占用 pany 2016.05.12

				//判断本地文件不存在
				if (GetFileAttributes(strCurFile) == 0xFFFFFFFF)
				{
					m_bOccupy[i] = false;
					return false;
				}

				if (m_pFtpFileFind[i]->FindFile(strRemoteFile)) //ftp上有该文件
				{
					//m_bOccupy[i] = false;
					m_ftpConnect[i]->Remove(strRemoteFile);
					//return false;
				}

				if (!m_ftpConnect[i]->PutFile(strCurFile, strRemoteFile))
				{
					m_bOccupy[i] = false;
					return false;
				}
				else 
				{
					m_bOccupy[i] = false;
					return TRUE;
				}
			}
		}

		if (!bFind)
		{
			int nSleepTime = 200;
			Sleep(nSleepTime);
			nCount += nSleepTime;
		}
	}


	return false;
=======
#include "StdAfx.h"
#include "FtpProcess.h"

CFtpProcess::CFtpProcess(void)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		m_pNetSession[i] = NULL;
		m_ftpConnect[i] = NULL; 
		m_pFtpFileFind[i] = NULL;
		m_bOccupy[i] = false;
	}

	m_nCurCount = 30;
}

CFtpProcess::~CFtpProcess(void)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		if (m_pNetSession[i] != NULL)
		{
			m_pFtpFileFind[i]->Close();
			m_ftpConnect[i]->Close();
			InternetCloseHandle(m_pFtpFileFind[i]);
			
			InternetCloseHandle( m_ftpConnect[i]);
			InternetCloseHandle( m_pNetSession[i] );

			
			
			delete m_pFtpFileFind[i];
			m_pFtpFileFind[i] = NULL;
			
			delete m_ftpConnect[i];
			m_ftpConnect[i] = NULL;

			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
		}
	}
}


bool CFtpProcess::ConnectFtp(CString strIP, CString strUser, CString strPsw, int nConnCount)
{
	for (int i=0; i<MAX_FTP_COUNT; i++)
	{
		if (m_pNetSession[i] != NULL)
		{
			InternetCloseHandle(m_pFtpFileFind[i]);
			InternetCloseHandle( m_ftpConnect[i]);
			InternetCloseHandle( m_pNetSession[i] );

			delete m_pFtpFileFind[i];
			m_pFtpFileFind[i] = NULL;

			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
		}

		m_bOccupy[i] = false;
	}

	m_nCurCount = nConnCount;

	if (m_nCurCount < 0)
		m_nCurCount = 1;

	if (m_nCurCount > MAX_FTP_COUNT)
		m_nCurCount = MAX_FTP_COUNT;

	for (int i=0; i<m_nCurCount; i++)
	{
		m_pNetSession[i] = new CInternetSession(NULL,1,PRE_CONFIG_INTERNET_ACCESS);

		try 
		{ 
			//新建连接对象
			m_ftpConnect[i]= m_pNetSession[i]->GetFtpConnection(strIP,strUser,strPsw); 
		} 
		catch(CInternetException *pEx)  //有异常
		{
			delete m_pNetSession[i];
			m_pNetSession[i] = NULL;
			m_ftpConnect[i] = NULL;
			pEx->Delete();
			return false;
		}

		if (!m_pFtpFileFind[i])
			m_pFtpFileFind[i] =  new CFtpFileFind(m_ftpConnect[i]);
	}

	return true;
}

bool CFtpProcess::downLoadFile(CString strRemoteFile, CString strCurFile)
{
	bool bFind = false;
	int nCount = 0;
	while(!bFind && nCount <= 300000)
	{
		for (int i=0; i<m_nCurCount; i++)
		{
			if (!m_bOccupy[i] && m_ftpConnect[i])
			{
				bFind = true;
				m_bOccupy[i] = true; //已经被占用 pany 2016.05.12
				
				//判断本地文件是否存在
				if (GetFileAttributes(strCurFile)!= 0xFFFFFFFF)
				{
					::DeleteFile(strCurFile);
				}
				
				if (!m_pFtpFileFind[i]->FindFile(strRemoteFile)) //ftp上没有该文件
				{
					m_bOccupy[i] = false;
					return false;
				}

				if (!m_ftpConnect[i]->GetFile(strRemoteFile, strCurFile))
				{
					m_bOccupy[i] = false;
					return false;
				}
				else 
				{
					m_bOccupy[i] = false;
					return TRUE;
				}
			}
		}

		if (!bFind)
		{
			int nSleepTime = 200;
			Sleep(nSleepTime);
			nCount += nSleepTime;
		}
	}

	return false;
}

bool CFtpProcess::uploadFtpFile(CString strRemoteFile, CString strCurFile)
{
	bool bFind = false;
	int nCount = 0;
	while(!bFind && nCount <= 300000)
	{
		for (int i=0; i<m_nCurCount; i++)
		{
			if (!m_bOccupy[i] && m_ftpConnect[i])
			{
				bFind = true;
				m_bOccupy[i] = true; //已经被占用 pany 2016.05.12

				//判断本地文件不存在
				if (GetFileAttributes(strCurFile) == 0xFFFFFFFF)
				{
					m_bOccupy[i] = false;
					return false;
				}

				if (m_pFtpFileFind[i]->FindFile(strRemoteFile)) //ftp上有该文件
				{
					//m_bOccupy[i] = false;
					m_ftpConnect[i]->Remove(strRemoteFile);
					//return false;
				}

				if (!m_ftpConnect[i]->PutFile(strCurFile, strRemoteFile))
				{
					m_bOccupy[i] = false;
					return false;
				}
				else 
				{
					m_bOccupy[i] = false;
					return TRUE;
				}
			}
		}

		if (!bFind)
		{
			int nSleepTime = 200;
			Sleep(nSleepTime);
			nCount += nSleepTime;
		}
	}


	return false;
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
}