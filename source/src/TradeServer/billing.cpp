
#include "Socket2.h"
#include "infotype.h"
#include "dblogic.h"
#include "billing.h"
#include "log.h"
#include <time.h>
#include <InfoNet.h>


PBILVERSION         pBilVersion;
PBILINIT            pBilInit;
PBILTERMINATE       pBilTerminate;
PBILCONNECT         pBilConnect;
PBILDISCONNECT      pBilDisconnect;
PBILTRADE           pBilTrade;
PBILREGISTERVIP     pBilRegisterVIP;
PBILQUERY           pBilQuery;
PBILRECORD          pBilRecord;


#define BILINIT             0
#define BILUNINIT           1
#define BILOFFLINE          2


BillingInterface::BillingInterface()
    :   m_hand(0), m_state(BILUNINIT)
{
}


BillingInterface::~BillingInterface()
{
}


bool BillingInterface::Init(HWND hWnd)
{
    m_hand = LoadLibrary("InfoBill.dll");

    if(!m_hand)
    {
        LGPRT("Billing", "Load InfoBill.dll Failt." __LN);
        return false;
    }

    pBilVersion = (PBILVERSION)GetProcAddress(m_hand, "bilVersion");
    if(!pBilVersion)
    {
        LGPRT("Billing", "Get Function bilVersion Failt." __LN);
        return false;
    }

    pBilInit = (PBILINIT)GetProcAddress(m_hand, "bilInit");
    if(!pBilInit)
    {
        LGPRT("Billing", "Get Function bilInit Failt." __LN);
        return false;
    }

    pBilTerminate = (PBILTERMINATE)GetProcAddress(m_hand, "bilTerminate");
    if(!pBilTerminate)
    {
        LGPRT("Billing", "Get Function bilTerminate Failt." __LN);
        return false;
    }

    pBilConnect = (PBILCONNECT)GetProcAddress(m_hand, "bilConnect");
    if(!pBilConnect)
    {
        LGPRT("Billing", "Get Function bilConnect Failt." __LN);
        return false;
    }

    pBilDisconnect = (PBILDISCONNECT)GetProcAddress(m_hand, "bilDisconnect");
    if(!pBilDisconnect)
    {
        LGPRT("Billing", "Get Function bilDisconnect Failt." __LN);
        return false;
    }

    pBilTrade = (PBILTRADE)GetProcAddress(m_hand, "bilTrade");
    if(!pBilTrade)
    {
        LGPRT("Billing", "Get Function bilTrade Failt." __LN);
        return false;
    }

    pBilRegisterVIP = (PBILREGISTERVIP)GetProcAddress(m_hand, "bilRegisterVIP");
    if(!pBilRegisterVIP)
    {
        LGPRT("Billing", "Get Function bilRegisterVIP Failt." __LN);
        return false;
    }

    pBilQuery = (PBILQUERY)GetProcAddress(m_hand, "bilQuery");
    if(!pBilQuery)
    {
        LGPRT("Billing", "Get Function bilQuery Failt." __LN);
        return false;
    }

    pBilRecord = (PBILRECORD)GetProcAddress(m_hand, "bilRecord");
    if(!pBilRecord)
    {
        LGPRT("Billing", "Get Function bilRecord Failt." __LN);
        return false;
    }

    if(1 == pBilInit(hWnd))
    {
        m_state = BILINIT;
        return true;
    }
    LGPRT("Billing", "Billing Init Failt." __LN);
    return false;
}


bool BillingInterface::Connect(const char** ip, unsigned short* port, int count)
{
    if(pBilConnect)
    {
        for(int i = 0; i < MAX_CALL; i++)
        {
            if(1 == pBilConnect(ip, port, count))
            {
            }
        }
        return true;
    }
    LGPRT("Billing", "Billing Connect Failt." __LN);
    return false;
}


bool BillingInterface::Disconnect()
{
    if(pBilDisconnect)
    {
        return 1 == pBilDisconnect();
    }
    LGPRT("Billing", "Billing Disconnect Failt." __LN);
    return false;
}


bool BillingInterface::Terminate()
{
    m_state = BILUNINIT;
    bool ret = false;
    if(pBilTerminate)
    {
        ret = (1 == pBilTerminate());
    }
    else
    {
        LGPRT("Billing", "Billing Terminate Failt." __LN);
    }
    Sleep(1000);    //  billing要求
    if(m_hand)
    {
        if(FreeLibrary(m_hand))
        {
            m_hand = 0;
        }
        else
        {
            LGPRT("Billing", "FreeLibrary Failt." __LN);
        }
    }
    return ret;
}


int BillingInterface::Trade(__int64 order, const char* account, const char* character, const char* ip, long credit, int rate, const char* server_info, const char* memo)
{
    if(pBilTrade)
    {
        char buff[32] = {0};
        char tmbuf[16] = {0};
        sprintf(buff, "%I64i", order);
        sprintf(tmbuf, "%li", time(0));
        return pBilTrade(buff, tmbuf, account, character, ip, credit, rate, server_info, memo);
    }
    LGPRT("Billing", "Billing Trade Failt." __LN);
    return 0;
}


int BillingInterface::RegisterVIP(__int64 order, const char* account, long month, long credit, long vip_type, const char* ip, const char* server_info, const char* memo)
{
    //  vip_type范围在20-50之间
    if(pBilRegisterVIP)
    {
        char buff[32] = {0};
        char tmbuf[32] = {0};
        time_t __now = time(0);
        tm* __tm = localtime(&__now);
        sprintf(buff, "%I64i", order);
        sprintf(tmbuf, "%i-%i-%i %i:%i:%i", (__tm->tm_year + 1900), (__tm->tm_mon + 1), __tm->tm_mday, __tm->tm_hour, __tm->tm_min, __tm->tm_sec);
        return pBilRegisterVIP(buff, tmbuf, account, month, credit, vip_type, ip, server_info, memo);
    }
    LGPRT("Billing", "Billing RegisterVIP Failt." __LN);
    return 0;
}


int BillingInterface::Record(__int64 order, const char* account, const char* character, const char* ip, long store_id, const char* store_name, long store_class, long credit, long number, const char* server_info, long result)
{
    if(pBilRecord)
    {
        char buff[32] = {0};
        char tmbuf[16] = {0};
        sprintf(buff, "%I64i", order);
        sprintf(tmbuf, "%li", time(0));
        return pBilRecord(buff, account, character, ip, store_id, store_name, store_class, tmbuf, credit, number, server_info, result);
    }
    LGPRT("Billing", "Billing Record Failt." __LN);
    return false;
}


int BillingInterface::Query(const char* account, int operators)
{
    if(pBilQuery)
    {
        return pBilQuery(account, operators);
    }
    LGPRT("Billing", "Billing Query Failt." __LN);
    return 0;
}


int BillingInterface::BilCallBack(long ret, long param)
{
    //printf("Ret: %li, Val: %li\n", ret, param);
    BillTask::iterator it = m_task.find(ret);
    if(it != m_task.end())
    {
        switch((*it).second->msgID)
        {
        case INFO_REQUEST_ACCOUNT:
            {
                switch((*it).second->eventID)
                {
                case QUERY_MONEY:
                    {
                        if(param >= 0)
                        {
                            RoleInfo* rol = (RoleInfo*)(*it).second->body;
                            rol->moBean = param;

                            if(rol->vip >= 0)
                            {
                                int size = sizeof(RoleInfo) + sizeof(long long);
                                BYTE* buff = new BYTE[size];
                                BYTE* ptr = buff;
                                memcpy(ptr, &(*it).second->orderID, sizeof(long long));
                                ptr += sizeof(long long);
                                memcpy(ptr, rol, sizeof(RoleInfo));

                                LG("NetMessage", "Answer => OrderID: %I64i, Section: %li, ActID: %li, ActName: %s, ChaID: %li, ChaName: %s, Bean: %li, Money: %li, VIPType: %li." __LN, (*it).second->orderID, (*it).second->section, rol->actID, rol->actName, rol->chaID, rol->chaName, rol->moBean, rol->rplMoney, rol->vip);
                                PostSendMessage((*it).second->sock, INFO_REQUEST_ACCOUNT, INFO_SUCCESS, 0, (*it).second->section, buff, size);

                                SAFE_DELETE(rol);
                                SAFE_DELETE_ARRAY(buff);
                            }
                        }
                        else
                        {
                            //  失败
                            LG("NetMessage", "Answer => OrderID: %I64i, Query Money Failt.(Param: %li)" __LN, (*it).second->orderID, param);
                            PostSendMessage((*it).second->sock, INFO_REQUEST_ACCOUNT, INFO_FAILED, 0, (*it).second->section, (BYTE*)&(*it).second->orderID, sizeof(long long));
                        }
                    }  break;

                case QUERY_KOP_ACCOUNT:
                    {
                    }  break;

                case QUERY_KOP_VIP_TYPE:
                    {
                        if(param >= 0)
                        {
                            RoleInfo* rol = (RoleInfo*)(*it).second->body;
                            rol->vip = param;

                            if(rol->moBean >= 0)
                            {
                                int size = sizeof(RoleInfo) + sizeof(long long);
                                BYTE* buff = new BYTE[size];
                                BYTE* ptr = buff;
                                memcpy(ptr, &(*it).second->orderID, sizeof(long long));
                                ptr += sizeof(long long);
                                memcpy(ptr, rol, sizeof(RoleInfo));

                                LG("NetMessage", "Answer => OrderID: %I64i, Section: %li, ActID: %li, ActName: %s, ChaID: %li, ChaName: %s, Bean: %li, Money: %li, VIPType: %li." __LN, (*it).second->orderID, (*it).second->section, rol->actID, rol->actName, rol->chaID, rol->chaName, rol->moBean, rol->rplMoney, rol->vip);
                                PostSendMessage((*it).second->sock, INFO_REQUEST_ACCOUNT, INFO_SUCCESS, 0, (*it).second->section, buff, size);

                                SAFE_DELETE(rol);
                                SAFE_DELETE_ARRAY(buff);
                            }
                        }
                        else
                        {
                            //  失败
                            LG("NetMessage", "Answer => OrderID: %I64i, Query KOP VIP Failt.(Param: %li)" __LN, (*it).second->orderID, param);
                            PostSendMessage((*it).second->sock, INFO_REQUEST_ACCOUNT, INFO_FAILED, 0, (*it).second->section, (BYTE*)&(*it).second->orderID, sizeof(long long));
                        }
                    }  break;
                }
            }  break;

        case INFO_EXCHANGE_MONEY:
            {
                RoleInfo* rol = (RoleInfo*)(*it).second->body;
                char fld1[128] = {0};
                char fld2[128] = {0};
                //char fld3[128] = {0};
                char fld4[256] = {0};
                //char fld5[256] = {0};
                sprintf(fld1, "%li", (*it).second->extend);
                sprintf(fld2, "%i", g_servInfo.serv_rate);
                sprintf(fld4, "%s, %li, %li", rol->actName, rol->rplMoney, rol->moBean);

                if(RET_SUCCEED == param)
                {
                    long coin = (*it).second->extend * g_servInfo.serv_rate;

					if((*it).second->extend < 0)
					{
						break;
					}

                    if(g_dbInfo.db_account.CountMoney(rol, coin))
                    {
                        //  兑换成功
                        int size = sizeof(RoleInfo) + sizeof(long long);
                        BYTE* buff = new BYTE[size];
                        BYTE* ptr = buff;
                        memcpy(ptr, &(*it).second->orderID, sizeof(long long));
                        ptr += sizeof(long long);
                        rol->moBean -= (*it).second->extend;
                        memcpy(ptr, rol, sizeof(RoleInfo));

                        LG("NetMessage", "Answer => OrderID: %I64i, Section: %li, ActID: %li, ActName: %s, ChaID: %li, ChaName: %s, Bean: %li, Money: %li, VIPType: %li, Credit: %li, Rate: %i." __LN, (*it).second->orderID, (*it).second->section, rol->actID, rol->actName, rol->chaID, rol->chaName, rol->moBean, rol->rplMoney, rol->vip, (*it).second->extend, g_servInfo.serv_rate);
                        g_dbInfo.db_history.LogHistory((*it).second->section, (*it).second->orderID, rol->actID, INFO_TYPE_EXCHANGE, fld1, fld2, "0", fld4, "");
                        PostSendMessage((*it).second->sock, INFO_EXCHANGE_MONEY, INFO_SUCCESS, 0, (*it).second->section, buff, size);

                        SAFE_DELETE_ARRAY(buff);
                    }
                    else
                    {
                        //  兑换失败退费
                        (*it).second->extend *= -1;
                        pBilTask money = new BilTask;
                        memcpy(money, (*it).second, sizeof(BilTask));
                        int iRet = 0;
                        iRet = g_servInfo.billing.Trade((*it).second->orderID, rol->actName, rol->chaName, (*it).second->sock->m_sock.GetIPAddress(), (*it).second->extend, g_servInfo.serv_rate, "", "");
                        if(iRet)
                        {
                            g_servInfo.billing.PushTask(iRet, money);
                        }
                        //  失败
                        LG("NetMessage", "Answer => OrderID: %I64i, Exchange Money Failt. Credit: %li, Rate: %i" __LN, (*it).second->orderID, (*it).second->extend, g_servInfo.serv_rate);
                        g_dbInfo.db_history.LogHistory((*it).second->section, (*it).second->orderID, rol->actID, INFO_TYPE_EXCHANGE, fld1, fld2, "5", fld4, "");
                        PostSendMessage((*it).second->sock, INFO_EXCHANGE_MONEY, INFO_FAILED, 0, (*it).second->section, (BYTE*)&(*it).second->orderID, sizeof(long long));
                    }
                    SAFE_DELETE(rol);
                }
                else
                {
                    //  失败
                    LG("NetMessage", "Answer => OrderID: %I64i, Exchange Money Failt.(Param: %li)" __LN, (*it).second->orderID, param);
                    g_dbInfo.db_history.LogHistory((*it).second->section, (*it).second->orderID, rol->actID, INFO_TYPE_EXCHANGE, fld1, fld2, "3", fld4, "");
                    PostSendMessage((*it).second->sock, INFO_EXCHANGE_MONEY, INFO_FAILED, 0, (*it).second->section, (BYTE*)&(*it).second->orderID, sizeof(long long));
                }
            }  break;

        case INFO_REGISTER_VIP:
            {
                RoleInfo* rol = (RoleInfo*)(*it).second->body;
                short vip = HIWORD((*it).second->extend);
                short month = LOWORD((*it).second->extend);;
                char fld1[128] = {0};
                char fld2[128] = {0};
                //char fld3[128] = {0};
                char fld4[256] = {0};
                //char fld5[256] = {0};
                sprintf(fld1, "%li", vip);
                sprintf(fld2, "%i", month);
                sprintf(fld4, "%s, %li, %li", rol->actName, rol->rplMoney, rol->moBean);

                if(RET_SUCCEED == param)
                {
                    rol->vip = vip;

                    long credit = g_dbInfo.db_vip.GetVIPCredit(vip);
                    rol->moBean -= credit;
                    int size = sizeof(RoleInfo) + sizeof(long long);
                    BYTE* buff = new BYTE[size];
                    BYTE* ptr = buff;
                    memcpy(ptr, &(*it).second->orderID, sizeof(long long));
                    ptr += sizeof(long long);
                    memcpy(ptr, rol, sizeof(RoleInfo));

                    LG("NetMessage", "Answer => OrderID: %I64i, Section: %li, ActID: %li, ActName: %s, ChaID: %li, ChaName: %s, Bean: %li, Money: %li, VIPType: %li." __LN, (*it).second->orderID, (*it).second->section, rol->actID, rol->actName, rol->chaID, rol->chaName, rol->moBean, rol->rplMoney, rol->vip);
                    g_dbInfo.db_history.LogHistory((*it).second->section, (*it).second->orderID, rol->actID, INFO_TYPE_VIP, fld1, fld2, "0", fld4, "");
                    PostSendMessage((*it).second->sock, INFO_REGISTER_VIP, INFO_SUCCESS, (*it).second->extend, (*it).second->section, buff, size);

                    SAFE_DELETE(rol);
                    SAFE_DELETE_ARRAY(buff);
                }
                else
                {
                    //  失败
                    LG("NetMessage", "Answer => OrderID: %I64i, Exchange Money Failt.(Param: %li)" __LN, (*it).second->orderID, param);
                    g_dbInfo.db_history.LogHistory((*it).second->section, (*it).second->orderID, rol->actID, INFO_TYPE_VIP, fld1, fld2, "3", fld4, "");
                    PostSendMessage((*it).second->sock, INFO_REGISTER_VIP, INFO_FAILED, 0, (*it).second->section, (BYTE*)&(*it).second->orderID, sizeof(long long));
                }
            }  break;
        }
        PopTask(ret);
    }
    return 0;
}


int BillingInterface::BilStatus(long stat, long threadid)
{
    LGPRT("Billing", "Stat: %li, ThreadID: %li\n", stat, threadid);
    //if(BILOFFLINE == m_state)
    //{
    //    Terminate();
    //}
    return 0;
}


bool BillingInterface::IsInit()
{
    return BILINIT == m_state;
}


void BillingInterface::PushTask(long id, pBilTask pTask)
{
    m_task.insert(pair<long, pBilTask>(id, pTask));
}


void BillingInterface::PopTask(long id)
{
    BillTask::iterator it = m_task.find(id);
    if(it != m_task.end())
    {
        SAFE_DELETE((*it).second);
        m_task.erase(it);
    }
}

int BillingInterface::GetVersion()
{
    return pBilVersion();
}

