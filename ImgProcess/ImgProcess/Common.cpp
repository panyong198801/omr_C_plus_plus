<<<<<<< HEAD

#include "stdafx.h"
#include "Common.h"


CCommon::CCommon()
{
	bWriteLog = true;
	m_nTraceMode=0;
	//m_file = new CFile(); 
}



CCommon::~CCommon()
{
	if (m_file)
		m_file.Close();
}

void CCommon::CreateLogFile(CString strFileName)
{
	CTime tNow;
	//DeleteFile();
	if(INVALID_HANDLE_VALUE != (HANDLE)m_file.m_hFile)		
		return;
	
	//CFile	m_FileTmp;
	if(!m_file.Open(strFileName,
		CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
	{
		m_file.m_hFile = NULL;
	}

}



/*---------------------------------------------------------
函数说明:记录日志信息(时间记录的毫秒)
输入参数： 
	strDebugInfo:	需要记录的日志信息
---------------------------------------------------------*/
void CCommon::WriteSysFile(CString strDebugInfo,int nTraceMode)
{
	if(nTraceMode > m_nTraceMode)	return;

	if(!bWriteLog)
		return;

	if (m_file.m_hFile == NULL)
		return;

	SYSTEMTIME systime;	
	GetLocalTime(&systime);
//	CTime tNow;
	CString strLogText;

	if (strDebugInfo == _T(""))
		return;

//	tNow = CTime::GetCurrentTime();
	strLogText.Format(_T("%.4u-%.2u-%.2u %.2u:%.2u:%.2u:%.3u	%s \r\n"), systime.wYear, systime.wMonth, systime.wDay,
				systime.wHour, systime.wMinute, systime.wSecond,systime.wMilliseconds, strDebugInfo);

	if (m_file.m_hFile == NULL || INVALID_HANDLE_VALUE == (HANDLE)m_file.m_hFile)
	{
		//JYSH	20120531	当文件对象不存在时，先创建一个临时文件。
	/*	if(NULL == m_FileTmp.m_hFile || INVALID_HANDLE_VALUE == (HANDLE)m_file.m_hFile)
		{
			if(m_FileTmp.Open(L"CommonTmp.txt",CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
			{
				m_FileTmp.SeekToEnd();
				USES_CONVERSION;
				m_FileTmp.Write(T2A(strLogText), strlen(T2A(strLogText)));
			}
		}*/
		return;
	}
	
	
	m_file.SeekToEnd();
	USES_CONVERSION;
	m_file.Write(T2A(strLogText), strlen(T2A(strLogText)));
	//strLogText.ReleaseBuffer ();
}


void CCommon::WriteSysFile(CString strClass,CString strDebugInfo,int nTraceMode)
{
	CString	strInfo;
	strInfo =  strClass + L"[" + strDebugInfo  + L"]" ;

	WriteSysFile(strInfo,nTraceMode);
}

=======

#include "stdafx.h"
#include "Common.h"


CCommon::CCommon()
{
	bWriteLog = true;
	m_nTraceMode=0;
	//m_file = new CFile(); 
}



CCommon::~CCommon()
{
	if (m_file)
		m_file.Close();
}

void CCommon::CreateLogFile(CString strFileName)
{
	CTime tNow;
	//DeleteFile();
	if(INVALID_HANDLE_VALUE != (HANDLE)m_file.m_hFile)		
		return;
	
	//CFile	m_FileTmp;
	if(!m_file.Open(strFileName,
		CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
	{
		m_file.m_hFile = NULL;
	}

}



/*---------------------------------------------------------
函数说明:记录日志信息(时间记录的毫秒)
输入参数： 
	strDebugInfo:	需要记录的日志信息
---------------------------------------------------------*/
void CCommon::WriteSysFile(CString strDebugInfo,int nTraceMode)
{
	if(nTraceMode > m_nTraceMode)	return;

	if(!bWriteLog)
		return;

	if (m_file.m_hFile == NULL)
		return;

	SYSTEMTIME systime;	
	GetLocalTime(&systime);
//	CTime tNow;
	CString strLogText;

	if (strDebugInfo == _T(""))
		return;

//	tNow = CTime::GetCurrentTime();
	strLogText.Format(_T("%.4u-%.2u-%.2u %.2u:%.2u:%.2u:%.3u	%s \r\n"), systime.wYear, systime.wMonth, systime.wDay,
				systime.wHour, systime.wMinute, systime.wSecond,systime.wMilliseconds, strDebugInfo);

	if (m_file.m_hFile == NULL || INVALID_HANDLE_VALUE == (HANDLE)m_file.m_hFile)
	{
		//JYSH	20120531	当文件对象不存在时，先创建一个临时文件。
	/*	if(NULL == m_FileTmp.m_hFile || INVALID_HANDLE_VALUE == (HANDLE)m_file.m_hFile)
		{
			if(m_FileTmp.Open(L"CommonTmp.txt",CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate|CFile::shareDenyNone,NULL))
			{
				m_FileTmp.SeekToEnd();
				USES_CONVERSION;
				m_FileTmp.Write(T2A(strLogText), strlen(T2A(strLogText)));
			}
		}*/
		return;
	}
	
	
	m_file.SeekToEnd();
	USES_CONVERSION;
	m_file.Write(T2A(strLogText), strlen(T2A(strLogText)));
	//strLogText.ReleaseBuffer ();
}


void CCommon::WriteSysFile(CString strClass,CString strDebugInfo,int nTraceMode)
{
	CString	strInfo;
	strInfo =  strClass + L"[" + strDebugInfo  + L"]" ;

	WriteSysFile(strInfo,nTraceMode);
}

>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
