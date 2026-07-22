

#include "ADO.h"
#include "log.h"
#include <string.h>
#include "infotype.h"

//  初始化COM
void InitCom()
{
    CoInitialize(0);
}

void UninitCom()
{
    CoUninitialize();
}



jConnection::jConnection()
{
    Init();
}


jConnection::~jConnection()
{
    Uninit();
}


jConnection::jConnection(const char* szServer, const char* szUsername, const char* szPassword, const char* szDatabase)
{
    SetSQLServerInfo(szServer, szUsername, szPassword, szDatabase);
    Init();
}

jConnection::jConnection(const char* szFilepath)
{
    SetAccessFilePath(szFilepath);
    Init();
}


void jConnection::Init(void)
{
__BEGIN_TRY
    InitCom();
    m_Connection.CreateInstance("ADODB.Connection");
__COM_TRY
__END_TRY
}


void jConnection::Uninit(void)
{
__BEGIN_TRY
    if(m_Connection.GetInterfacePtr())
    {
        m_Connection.Release();
    }
    UninitCom();
__COM_TRY
__END_TRY
}


bool jConnection::Connect()
{
    m_lock.lock();
__BEGIN_TRY
    if(adStateClosed != m_Connection->GetState())
    {
        m_lock.unlock();
        //对象已打开
        return true;
    }
    HRESULT ret = m_Connection->Open((_bstr_t) m_ConStr, "", "", adConnectUnspecified);
    m_lock.unlock();
    return S_OK == ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool jConnection::Close()
{
    m_lock.lock();
__BEGIN_TRY
    if(adStateClosed == m_Connection->GetState())
    {
        m_lock.unlock();
        //对象已关闭
        return true;
    }
    HRESULT ret = m_Connection->Close();
    m_lock.unlock();
    return S_OK == ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


int jConnection::Execute(const char* szSQLStr)
{
    m_lock.lock();
__BEGIN_TRY
    _variant_t aff;//影响行数
    m_Connection->Execute((_bstr_t)szSQLStr, &aff, adCmdUnknown);
    m_lock.unlock();
    return (int)aff <= 0 ? 0 : (int)aff;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return 0;
}


int jConnection::Execute(const char* szSQLStr, _RecordsetPtr& lpRecordset)
{
    m_lock.lock();
__BEGIN_TRY
    //if(adStateOpen == lpRecordset->GetState())
    //{
    //    lpRecordset->Close();
    //}
    //lpRecordset->put_ActiveConnection((_variant_t)m_Connection.GetInterfacePtr());
    //lpRecordset->Open((_variant_t)szSQLStr, m_Connection.GetInterfacePtr(), adOpenStatic/*adOpenForwardOnly*/, adLockReadOnly, adCmdUnknown);     //  有效率问题
    lpRecordset = m_Connection->Execute((_bstr_t)szSQLStr, 0, adCmdText/*adCmdUnknown*/);
    m_lock.unlock();
    return -1;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return 0;
}


void jConnection::SetAccessFilePath(const char* szFilepath)
{
    //  Provider=Microsoft.Jet.OLEDB.4.0;Data Source={database}
    strcpy(m_ConStr, "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=");
    strcat(m_ConStr, szFilepath);
}


void jConnection::SetSQLServerInfo(const char* szServer, const char* szUsername, const char* szPassword, const char* szDatabase)
{
    //  Provider=SQLOLEDB;Server={server};UID={username};PWD={password};Database={database}
    strcpy(m_ConStr, "Provider=SQLOLEDB;Server=");
    strcat(m_ConStr, szServer);
    strcat(m_ConStr, ";UID=");
    strcat(m_ConStr, szUsername);
    strcat(m_ConStr, ";PWD=");
    strcat(m_ConStr, szPassword);
    strcat(m_ConStr, ";Database=");
    strcat(m_ConStr, szDatabase);
}


bool jConnection::IsConnect()
{
    m_lock.lock();
__BEGIN_TRY
    long ret = m_Connection->GetState();
    m_lock.unlock();
    return adStateClosed != ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


void jConnection::SetConnectionString(const char* szConStr)
{
    strncpy(m_ConStr, szConStr, 1024);
}

