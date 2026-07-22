


#if !defined __DBLOGIC_H__
#define __DBLOGIC_H__


#include "ado.h"
#include "infotype.h"


#define SQL_BUFFER_SIZE                 4096

class IOCPSocket;

class DBControlBase
{
public:
    DBControlBase();
    virtual ~DBControlBase();

    virtual bool CheckConnect();
    virtual void SafeCloseRecordset();

protected:
    jConnection     m_conn;
    _RecordsetPtr   m_rs;
    Lock            m_lock;
    char            m_sql[SQL_BUFFER_SIZE];
};


//  登陆信息
class LoginControl : public DBControlBase
{
public:
    LoginControl();
    ~LoginControl();

    bool CheckClient(IOCPSocket* client);
    bool LoginServer(long id, const char* ip, const char* pwd);
    bool EnableLogin(LoginInfo* pli);
    bool GetLoginInfo(LoginInfo* pli, long id);
    bool GetLoginInfo(LoginInfo* pli, const char* ip);
};


//  分类信息
class ClassControl : public DBControlBase
{
public:
    ClassControl();
    ~ClassControl();

    bool AddClass(ClassInfo* pci);
    bool DelClass(long cid);
    bool DelClass(const char* clsname);
    bool UpdateClass(ClassInfo* pci, long cid);
    bool UpdateClass(ClassInfo* pci, const char* clsname);
    bool ReloadToMemory();
    long GetBufferSize();
    bool GetBuffer(BYTE* buff, long size);
public:
    ClassInfo*      m_list;
    long            m_size;
};


//  商城信息
class StoreControl : public DBControlBase
{
public:
    StoreControl();
    ~StoreControl();

public:
    void SafeCloseItemRecordset();
    bool AddStore(StoreStruct* pss);
    bool DelStore(long sid);
    bool DelStore(const char* sname);
    bool UpdateStore(StoreStruct* pss, long sid);
    bool UpdateStore(StoreStruct* pss, const char* sname);
    //  1失败，2下架，0成功
	int  CountNumber(long id, long num);
    bool SetHotStore(long sid);
    bool SetHotStore(const char* sname);
    bool ReloadToMemory();
    bool LoadDelStore(BYTE** buff, long* len, long* scount);
    long GetBufferSize();
    bool GetBuffer(BYTE* buff, long size);
    //  -1 错误的价格，-2已下架
    long QueryStore(long id, char* name, long* comClass);

public:
    StoreStruct*    m_list;
    _RecordsetPtr   m_irs;
    long            m_size;
    long            m_itemnum;
};


//  公告信息
class AfficheControl : public DBControlBase
{
public:
    AfficheControl();
    ~AfficheControl();

    bool AddAffiche(AfficheInfo* affi);
    bool DelAffiche(long aid);
    bool UpdateAffiche(AfficheInfo* affi, long aid);
    bool ReloadToMemory();
    long GetBufferSize();
    bool GetBuffer(BYTE* buff, long size);

public:
    AfficheInfo*    m_list;
    long            m_size;
};


//  帐号信息
class AccountControl : public DBControlBase
{
public:
    AccountControl();
    ~AccountControl();

    bool GetRoleInfo(RoleInfo* role);
    bool CountMoney(RoleInfo* role, long coin);
    bool CountBean(RoleInfo* role, long bean);
    bool CountBeanAndMoney(RoleInfo* role, long coin, long bean);

};


//  VIP信息
class VIPControl : public DBControlBase
{
public:
    VIPControl();
    virtual ~VIPControl();

    bool AddVIP(long vipID, long vipCredit);
    bool UpdateVIP(long oldVipID, long vipCredit);
    bool DeleteVIP(long vipID);
    long GetVIPCredit(long vipID);
    bool ReloadToMemory();
public:
    long        m_size;
    VIPInfo*    m_list;
};


//  记录信息
class HistoryControl : public DBControlBase
{
public:
    HistoryControl();
    ~HistoryControl();

    bool GetBuffer(BYTE* buff, long size, int type, long section, int num, long accountID);
    bool LogHistory(long section, long long orderID, long accountID, int type, const char* fld1, const char* fld2, const char* fld3, const char* fld4, const char* fld5);
    bool ExecuteInsert(const char* sqlcmd);
    bool CheckOrder(long long id);
};


//  邮件记录
class MailControl : public DBControlBase
{
public:
    MailControl();
    ~MailControl();
    long AddMail(MailInfo* mail, long servID);
    bool ReplyMail(MailInfo* mail, long gmID);
    bool ReadMail(MailInfo* mail, long* servID);
    bool ReadReply(MailInfo* mail, RoleInfo* role);
    long GetCount();
};



typedef struct _DBLogic_
{
    LoginControl        db_login;
    ClassControl        db_class;
    StoreControl        db_store;
    AfficheControl      db_affiche;
    AccountControl      db_account;
    HistoryControl      db_history;
    VIPControl          db_vip;
    MailControl         db_mail;
}DBLogic, *pDBLogic;

extern DBLogic             g_dbInfo;

#endif
