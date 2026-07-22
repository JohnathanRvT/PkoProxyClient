
/* * * * * * * * * * * * * * * * * * * *

    ADO 封装
    Jampe
    2006/3/28

 * * * * * * * * * * * * * * * * * * * */

#if !defined __CONNECTION_H__
#define __CONNECTION_H__


#include "lock.h"


#import "c:\Program Files\Common Files\System\ADO\msado60.tlb" \
        no_namespace rename("BOF", "ADOBOF") rename("EOF", "ADOEOF")


class jConnection
{
public:

    jConnection();
    jConnection(const char* szServer, const char* szUsername, const char* szPassword, const char* szDatabase);  //  for sqlserver
    jConnection(const char* szFilepath);    //  for access
    virtual ~jConnection();

    virtual bool Connect();
    virtual bool Close();
    virtual int Execute(const char* szSQLStr);
    virtual int Execute(const char* szSQLStr, _RecordsetPtr& lpRecordset);

    virtual bool IsConnect();

    virtual void SetConnectionString(const char* szConStr);

    virtual void SetAccessFilePath(const char* szFilepath);
    virtual void SetSQLServerInfo(const char* szServer, const char* szUsername, const char* szPassword, const char* szDatabase);
    virtual void Init();
    virtual void Uninit();

public:
	_ConnectionPtr m_Connection;

protected:
    char            m_ConStr[1024];
    Lock            m_lock;


};



#endif


