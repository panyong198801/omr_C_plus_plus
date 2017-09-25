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
	CString strIniPath;//≈‰÷√Œƒº˛¬∑æ∂
	
	int		m_nTraceMode;
};

#endif