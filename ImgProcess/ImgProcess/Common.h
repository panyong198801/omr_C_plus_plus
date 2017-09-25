<<<<<<< HEAD
#ifndef _COMMOM_Printer_H
#define _COMMOM_Printer_H

typedef unsigned char uchar ;

class CCommon 
{
public:
	CCommon();
	~CCommon();
	void CreateLogFile(CString strFileName);
	void WriteSysFile(CString strClass, CString strDebugInfo,int nTraceMode=0);

	void WriteSysFile(CString strInfo,int nTraceMode=0);
	
private:
	//HANDLE	mHandle;
	//CFile	m_file;
	//CFile	m_FileTmp;
	CFile	m_file;
	bool bWriteLog;
	CString strIniPath;//配置文件路径
	
	int		m_nTraceMode;
};

=======
#ifndef _COMMOM_Printer_H
#define _COMMOM_Printer_H

typedef unsigned char uchar ;

class CCommon 
{
public:
	CCommon();
	~CCommon();
	void CreateLogFile(CString strFileName);
	void WriteSysFile(CString strClass, CString strDebugInfo,int nTraceMode=0);

	void WriteSysFile(CString strInfo,int nTraceMode=0);
	
private:
	//HANDLE	mHandle;
	//CFile	m_file;
	//CFile	m_FileTmp;
	CFile	m_file;
	bool bWriteLog;
	CString strIniPath;//配置文件路径
	
	int		m_nTraceMode;
};

>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
#endif