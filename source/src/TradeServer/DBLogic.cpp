
#include "socket2.h"
#include "dblogic.h"
#include <enclib.h>



DBLogic         g_dbInfo;


DBControlBase::DBControlBase()
{
    if(!m_rs.GetInterfacePtr())
    {
        m_rs.CreateInstance("ADODB.Recordset");
    }
}


DBControlBase::~DBControlBase()
{
    if(m_rs.GetInterfacePtr())
    {
        m_rs.Release();
    }
    if(m_conn.IsConnect())
    {
        m_conn.Close();
    }
}


bool DBControlBase::CheckConnect()
{
    if(!m_conn.IsConnect())
    {
        m_conn.SetSQLServerInfo(g_servInfo.db_serv, g_servInfo.db_usr, g_servInfo.db_pwd, g_servInfo.db_name);
        m_conn.Connect();
    }
    return m_conn.IsConnect();
}


void DBControlBase::SafeCloseRecordset()
{
    if(adStateOpen == m_rs->GetState())
    {
        m_rs->Close();
    }
}



//////////////////////////////////////////////////



LoginControl::LoginControl()
{
}


LoginControl::~LoginControl()
{
}


bool LoginControl::CheckClient(IOCPSocket* client)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT Sid FROM Safety WHERE Ip = '%s' AND Type = 0", client->m_sock.GetIPAddress());
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            client->m_lServerID = (long)m_rs->GetCollect("Sid");
            ret = true;
        }
        else
        {
            client->m_lServerID = -1;
            ret = false;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool LoginControl::LoginServer(long id, const char* ip, const char* pwd)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT Passwd FROM Safety WHERE Sid = %li AND Ip = '%s' AND Type = 0", id, ip);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            char passwd[128] = {0};
            char srcpwd[33]= {0};
            strcpy(passwd, (char*)(bstr_t)m_rs->GetCollect("Passwd"));
            //Decrypt((_byte*)srcpwd, 33, (_byte*)passwd, (int)strlen(passwd));
			/*	yangyinyu	close!
            if(!strcmp(pwd, srcpwd))
			*/
            {
                ret = true;
            }
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool LoginControl::EnableLogin(LoginInfo* pli)
{
__BEGIN_TRY

    bool isNew = true;
    bool ret = false;
    LoginInfo tli;
    if(GetLoginInfo(&tli, pli->ip))
    {
        pli->id = tli.id;
        isNew = false;
    }

    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        if(isNew)
        {
            sprintf(m_sql, "INSERT INTO Safety(Sid, Ip, Passwd, Type) VALUES(%li, '%s', '%s', %i)", 
                pli->id, pli->ip, pli->pwd, pli->ban);
        }
        else
        {
            sprintf(m_sql, "UPDATE Safety SET Passwd = '%s', Type = %i WHERE Sid = %i AND Ip = '%s'", 
                pli->pwd, pli->ban, pli->id, pli->ip);
        }
        if(m_conn.Execute(m_sql))
        {
            ret = true;
        }
    }

    m_lock.unlock();
    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool LoginControl::GetLoginInfo(LoginInfo* pli, long id)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT Sid, Ip, Passwd, Type FROM Safety WHERE Sid = %i", id);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            pli->id = (_byte)m_rs->GetCollect("Sid");
            strncpy(pli->ip, (char*)(bstr_t)m_rs->GetCollect("Ip"), 16);
            strncpy(pli->pwd, (char*)(bstr_t)m_rs->GetCollect("Passwd"), 33);
            pli->ban = (int)m_rs->GetCollect("Type");
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool LoginControl::GetLoginInfo(LoginInfo* pli, const char* ip)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT Sid, Ip, Passwd, Type FROM Safety WHERE Ip = '%s'", ip);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            pli->id = (_byte)m_rs->GetCollect("Sid");
            strncpy(pli->ip, (char*)(bstr_t)m_rs->GetCollect("Ip"), 16);
            strncpy(pli->pwd, (char*)(bstr_t)m_rs->GetCollect("Passwd"), 33);
            pli->ban = (int)m_rs->GetCollect("Type");
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


//////////////////////////////////////////////////



ClassControl::ClassControl()
    : m_list(0), m_size(0)
{
}


ClassControl::~ClassControl()
{
    SAFE_DELETE_ARRAY(m_list);
    m_size = 0;
}


bool ClassControl::AddClass(ClassInfo* pci)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT clsID FROM ClassInfo WHERE clsName = '%s' AND isDel = 0", pci->clsName);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {   //  分类名重复
            ret = false;
        }
        else
        {
            sprintf(m_sql, "INSERT INTO ClassInfo(clsName, parentID, buildTime, isDel, delTime)VALUES('%s', %li, %li, 0, 0)", 
                pci->clsName, pci->parentID, time(0));
            m_conn.Execute(m_sql);
            LG("Management", "Added Class, Name: %s, ParentID: %i" __LN, pci->clsName, pci->parentID);
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool ClassControl::DelClass(long cid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE ClassInfo SET isDel = 1, delTime = %li WHERE clsID = %li  AND isDel = 0", time(0), cid);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Deled Class, ID: %li" __LN, cid);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool ClassControl::DelClass(const char* clsname)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE ClassInfo SET isDel = 1, delTime = %li WHERE clsName = '%s'  AND isDel = 0", time(0), clsname);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Deled Class, Name: %s" __LN, clsname);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool ClassControl::UpdateClass(ClassInfo* pci, long cid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE ClassInfo SET clsName = '%s', parentID = %li WHERE clsID = %li AND isDel = 0", pci->clsName, pci->parentID, cid);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据
            pci->clsID = (short)cid;
            for(int i = 0; i < m_size; i++)
            {
                if(m_list[i].clsID == cid)
                {
                    strncpy(m_list[i].clsName, pci->clsName, 9);
                    m_list[i].parentID = pci->parentID;
                    LG("Management", "Update Class, ID: %li, New Name: %s, New Parent ID: %li" __LN, cid, pci->clsName, pci->parentID);
                    break;
                }
            }
            ret = true;
        }
    }
    m_lock.unlock();

    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool ClassControl::UpdateClass(ClassInfo* pci, const char* clsname)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE ClassInfo SET clsName = '%s', parentID = %li WHERE clsName = '%s' AND isDel = 0", 
            pci->clsName, pci->parentID, clsname);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据
            for(int i = 0; i < m_size; i++)
            {
                if(!strcmp(m_list[i].clsName, clsname))
                {
                    strncpy(m_list[i].clsName, pci->clsName, 9);
                    m_list[i].parentID = pci->parentID;
                    LG("Management", "Update Class, Name: %s, New Name: %s, New Parent ID: %li" __LN, clsname, pci->clsName, pci->parentID);
                    break;
                }
            }
            ret = true;
        }
    }
    m_lock.unlock();

    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool ClassControl::ReloadToMemory()
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        SAFE_DELETE_ARRAY(m_list);
        m_size = 0;

        sprintf(m_sql, "SELECT COUNT(*) AS total FROM ClassInfo WHERE isDel = 0");
        m_conn.Execute(m_sql, m_rs);
        m_size = m_rs->GetCollect("total");
        SafeCloseRecordset();

        if(!m_size)
        {
            ret = true;
        }
        else
        {
            sprintf(m_sql, "SELECT clsID, clsName, parentID FROM ClassInfo WHERE isDel = 0 ORDER BY parentID ASC");
            m_conn.Execute(m_sql, m_rs);

            m_list = new ClassInfo[m_size];
            memset(m_list, 0, (sizeof(ClassInfo) * m_size));

            int i = 0;
            while(!m_rs->GetADOEOF())
            {
                m_list[i].clsID = (int)m_rs->GetCollect("clsID");
                strncpy(m_list[i].clsName, (char*)(bstr_t)m_rs->GetCollect("clsName"), 9);
                m_list[i].parentID = (int)m_rs->GetCollect("parentID");
                ++i;
                m_rs->MoveNext();
            }
            SafeCloseRecordset();
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


long ClassControl::GetBufferSize()
{
    return m_size * sizeof(ClassInfo);
}


bool ClassControl::GetBuffer(BYTE* buff, long size)
{
__BEGIN_TRY
    if(size < m_size)
    {
        return false;
    }

    BYTE* ptr = buff;
    for(int i = 0; i < m_size; i++)       //  取分类列表
    {
        memcpy(ptr, &m_list[i], sizeof(ClassInfo));
        ptr += sizeof(ClassInfo);
    }
    return true;
__END_TRY
    return false;
}


//////////////////////////////////////////////////



StoreControl::StoreControl()
    : m_list(0), m_size(0)
{
    if(!m_irs.GetInterfacePtr())
    {
        m_irs.CreateInstance("ADODB.Recordset");
    }
}


StoreControl::~StoreControl()
{
    if(m_irs.GetInterfacePtr())
    {
        m_irs.Release();
    }
    SAFE_DELETE_ARRAY(m_list);
    m_size = 0;
}


void StoreControl::SafeCloseItemRecordset()
{
    if(adStateOpen == m_irs->GetState())
    {
        m_irs->Close();
    }
}


bool StoreControl::AddStore(StoreStruct* pss)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT comID FROM StoreInfo WHERE comName = '%s' AND isDel = 0", pss->store_head.comName);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {   //  商品名重复
            ret = false;
        }
        else
        {
            sprintf(m_sql, "INSERT INTO StoreInfo(comName, comClass, comTime, comPrice, comRemack, comNumber, comExpire, isDel, delTime, isHot, beginTime)VALUES('%s', %li, %li, %li, '%s', %li, %li, 0, 0, %i, %li)", 
                pss->store_head.comName, pss->store_head.comClass, time(0), pss->store_head.comPrice, pss->store_head.comRemark, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot, pss->store_head.beginTime);
            m_conn.Execute(m_sql);
            LG("Management", "Added Store, Name: %s, Class: %li, Price: %li, Number: %i, Expire: %li, Hot: %i, BeginTime: %li" __LN, 
                pss->store_head.comName, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot, pss->store_head.beginTime);
            SafeCloseRecordset();

            sprintf(m_sql, "SELECT comID FROM StoreInfo WHERE comName = '%s' AND isDel = 0", pss->store_head.comName);
            m_conn.Execute(m_sql, m_rs);

            if(m_rs->GetADOEOF())
            {
                ret = false;
            }
            else
            {
                pss->store_head.comID = (long)m_rs->GetCollect("comID");

                for(int i = 0; i < pss->store_head.itemNum; i++)
                {
                    sprintf(m_sql, "INSERT INTO ItemInfo(comID, itemID, itemNum, itemFlute, itemAttrID1, itemAttrVal1, itemAttrID2, itemAttrVal2, itemAttrID3, itemAttrVal3, itemAttrID4, itemAttrVal4, itemAttrID5, itemAttrVal5) VALUES(%li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li)", 
                        pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum, pss->item_body[i].itemFlute, 
                        pss->item_body[i].itemAttrID[0], pss->item_body[i].itemAttrVal[0], 
                        pss->item_body[i].itemAttrID[1], pss->item_body[i].itemAttrVal[1], 
                        pss->item_body[i].itemAttrID[2], pss->item_body[i].itemAttrVal[2], 
                        pss->item_body[i].itemAttrID[3], pss->item_body[i].itemAttrVal[3], 
                        pss->item_body[i].itemAttrID[4], pss->item_body[i].itemAttrVal[4]);
                    m_conn.Execute(m_sql);
                    LG("Management", "Added Item, StoreID: %li, ID: %li, Number: %i" __LN, pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum);
                }
                ret = true;
            }
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::DelStore(long sid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET isDel = 1, delTime = %li WHERE comID = %li AND isDel = 0", time(0), sid);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Deled Store, ID: %li" __LN, sid);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::DelStore(const char* sname)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET isDel = 1, delTime = %li WHERE comName = '%s' AND isDel = 0", time(0), sname);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Deled Store, Name: %s" __LN, sname);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::UpdateStore(StoreStruct* pss, long sid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET comClass = %li, comPrice = %li, comRemack = '%s', comNumber = %i, comExpire = %li, isHot = %i, beginTime = %li, isDel = 0 WHERE comID = %li", 
            pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comRemark, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot, pss->store_head.beginTime, sid);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据

            LG("Management", "Update Store, ID: %li, New[Class: %li, Price: %li, Number: %i, Expire: %li, Hot: %i]" __LN, sid, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;

//__BEGIN_TRY
//    bool ret = false;
//    m_lock.lock();
//
//    if(!CheckConnect())
//    {
//        ret = false;
//    }
//    else
//    {
//        sprintf(m_sql, "UPDATE StoreInfo SET comName = '%s', comClass = %li, comPrice = %li, comRemack = '%s', comNumber = %i, comExpire = %li, isHot = %i, beginTime = %li, isDel = 0 WHERE comID = %li", 
//            pss->store_head.comName, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comRemark, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot, pss->store_head.beginTime, sid);
//        if(m_conn.Execute(m_sql))
//        {   //  成功，更新内存数据
//
//            LG("Management", "Update Store, ID: %li, New[Name: %s, Class: %li, Price: %li, Number: %i, Expire: %li, Hot: %i]" __LN, sid, pss->store_head.comName, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot);
//
//            sprintf(m_sql, "DELETE FROM ItemInfo WHERE comID = %li", sid);
//            m_conn.Execute(m_sql);
//
//            for(int i = 0; i < pss->store_head.itemNum; i++)
//            {
//                sprintf(m_sql, "INSERT INTO ItemInfo(comID, itemID, itemNum, itemFlute, itemAttrID1, itemAttrVal1, itemAttrID2, itemAttrVal2, itemAttrID3, itemAttrVal3, itemAttrID4, itemAttrVal4, itemAttrID5, itemAttrVal5) VALUES(%li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li)", 
//                    pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum, pss->item_body[i].itemFlute, 
//                    pss->item_body[i].itemAttrID[0], pss->item_body[i].itemAttrVal[0], 
//                    pss->item_body[i].itemAttrID[1], pss->item_body[i].itemAttrVal[1], 
//                    pss->item_body[i].itemAttrID[2], pss->item_body[i].itemAttrVal[2], 
//                    pss->item_body[i].itemAttrID[3], pss->item_body[i].itemAttrVal[3], 
//                    pss->item_body[i].itemAttrID[4], pss->item_body[i].itemAttrVal[4]);
//                m_conn.Execute(m_sql);
//                LG("Management", "Added Item, StoreID: %li, ID: %li, Number: %i" __LN, pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum);
//            }
//
//            pss->store_head.comID = sid;
//            for(int i = 0; i < m_size; i++)
//            {
//                if(m_list[i].store_head.comID == sid)
//                {
//                    strncpy(m_list[i].store_head.comName, pss->store_head.comName, 21);
//                    strncpy(m_list[i].store_head.comRemark, pss->store_head.comRemark, 129);
//                    m_list[i].store_head.comClass = pss->store_head.comClass;
//                    m_list[i].store_head.comPrice = pss->store_head.comPrice;
//                    m_list[i].store_head.itemNum = pss->store_head.itemNum;
//                    break;
//                }
//            }
//            ret = true;
//        }
//    }
//    m_lock.unlock();
//
//    return ret;
//__END_TRY
//    m_lock.unlock();
//    return false;
}


bool StoreControl::UpdateStore(StoreStruct* pss, const char* sname)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET comName = '%s', comClass = %li, comPrice = %li, comRemack = '%s', comNumber = %i, comExpire = %li, isHot = %i, beginTime = %li, isDel = 0 WHERE comName = '%s'", 
            pss->store_head.comName, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comRemark, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot, pss->store_head.beginTime, sname);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据

            LG("Management", "Update Store, Name: %s, New[Name: %s, Class: %li, Price: %li, Number: %i, Expire: %li, Hot: %i]" __LN, sname, pss->store_head.comName, pss->store_head.comClass, pss->store_head.comPrice, pss->store_head.comNumber, pss->store_head.comExpire, pss->store_head.isHot);

            sprintf(m_sql, "DELETE FROM ItemInfo WHERE comID IN (SELECT comID FROM StoreInfo WHERE comName = '%s' AND isDel = 0)", sname);
            if(m_conn.Execute(m_sql))
            {
                for(int i = 0; i < pss->store_head.itemNum; i++)
                {
                    sprintf(m_sql, "INSERT INTO ItemInfo(comID, itemID, itemNum, itemFlute, itemAttrID1, itemAttrVal1, itemAttrID2, itemAttrVal2, itemAttrID3, itemAttrVal3, itemAttrID4, itemAttrVal4, itemAttrID5, itemAttrVal5) VALUES(%li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li, %li)", 
                        pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum, pss->item_body[i].itemFlute, 
                        pss->item_body[i].itemAttrID[0], pss->item_body[i].itemAttrVal[0], 
                        pss->item_body[i].itemAttrID[1], pss->item_body[i].itemAttrVal[1], 
                        pss->item_body[i].itemAttrID[2], pss->item_body[i].itemAttrVal[2], 
                        pss->item_body[i].itemAttrID[3], pss->item_body[i].itemAttrVal[3], 
                        pss->item_body[i].itemAttrID[4], pss->item_body[i].itemAttrVal[4]);
                    m_conn.Execute(m_sql);
                    LG("Management", "Added Item, StoreID: %li, ID: %li, Number: %i" __LN, pss->store_head.comID, pss->item_body[i].itemID, pss->item_body[i].itemNum);
                }

                for(int i = 0; i < m_size; i++)
                {
                    if(!strcpy(m_list[i].store_head.comName, sname))
                    {
                        strncpy(m_list[i].store_head.comName, pss->store_head.comName, 21);
                        strncpy(m_list[i].store_head.comRemark, pss->store_head.comRemark, 129);
                        m_list[i].store_head.comClass = pss->store_head.comClass;
                        m_list[i].store_head.comPrice = pss->store_head.comPrice;
                        m_list[i].store_head.itemNum = pss->store_head.itemNum;
                        break;
                    }
                }
                ret = true;
            }
        }
    }
    m_lock.unlock();

    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::SetHotStore(long sid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET isHot = ((isHot + 1) %% 2) WHERE comID = %li AND isDel = 0", sid);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Set Store Hot, ID: %li" __LN, sid);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::SetHotStore(const char* sname)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE StoreInfo SET isHot = ((isHot + 1) %% 2) WHERE comName = '%s' AND isDel = 0", sname);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Set Store Hot, Name: %s" __LN, sname);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::ReloadToMemory()
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        int i = 0;
        for(i = 0; i < m_size; i++)
        {
            SAFE_DELETE_ARRAY(m_list[i].item_body);
        }
        SAFE_DELETE_ARRAY(m_list);
        m_size = 0;
        m_itemnum = 0;
        time_t tim = time(0);

        sprintf(m_sql, "SELECT COUNT(*) AS total FROM StoreInfo WHERE isDel = 0 AND comNumber <> 0 AND ((comExpire > %li) OR (comExpire <= 0)) AND beginTime <= %li", (long)tim, (long)tim);
        m_conn.Execute(m_sql, m_rs);
        m_size = m_rs->GetCollect("total");
        SafeCloseRecordset();

        if(!m_size)
        {
            ret = true;
        }
        else
        {
            sprintf(m_sql, "SELECT comID, comName, comClass, comTime, comPrice, comRemack, comNumber, comExpire, isHot, beginTime FROM StoreInfo WHERE isDel = 0 AND comNumber <> 0 AND ((comExpire > %li) OR (comExpire <= 0)) AND beginTime <= %li ORDER BY isHot DESC, comTime DESC", (long)tim, (long)tim);
            m_conn.Execute(m_sql, m_rs);

            m_list = new StoreStruct[m_size];
            memset(m_list, 0, (sizeof(StoreStruct) * m_size));

            i = 0;
            int j;
            while(!m_rs->GetADOEOF())
            {
                m_list[i].store_head.comID = (long)m_rs->GetCollect("comID");
                m_list[i].store_head.comClass = (long)m_rs->GetCollect("comClass");
                m_list[i].store_head.comTime = (time_t)m_rs->GetCollect("comTime");
                m_list[i].store_head.comPrice = (long)m_rs->GetCollect("comPrice");
                strncpy(m_list[i].store_head.comName, (char*)(bstr_t)m_rs->GetCollect("comName"), 21);
                strncpy(m_list[i].store_head.comRemark, (char*)(bstr_t)m_rs->GetCollect("comRemack"), 129);
                m_list[i].store_head.comNumber = (int)m_rs->GetCollect("comNumber");
                m_list[i].store_head.comExpire = (time_t)m_rs->GetCollect("comExpire");
                m_list[i].store_head.isHot = (_byte)(long)m_rs->GetCollect("isHot");
                m_list[i].store_head.beginTime = (time_t)m_rs->GetCollect("beginTime");

                sprintf(m_sql, "SELECT COUNT(*) AS total FROM ItemInfo WHERE comID = %li", m_list[i].store_head.comID);
                m_conn.Execute(m_sql, m_irs);
                m_list[i].store_head.itemNum = m_irs->GetCollect("total");
                m_itemnum += m_list[i].store_head.itemNum;
                SafeCloseItemRecordset();

                sprintf(m_sql, 
                    "SELECT itemID, itemNum, itemFlute, itemAttrID1, itemAttrVal1, itemAttrID2, itemAttrVal2, itemAttrID3, itemAttrVal3, itemAttrID4, itemAttrVal4, itemAttrID5, itemAttrVal5 FROM ItemInfo WHERE comID = %li", 
                    m_list[i].store_head.comID);
                m_conn.Execute(m_sql, m_irs);

                j = 0;
                m_list[i].item_body = new ItemInfo[m_list[i].store_head.itemNum];
                while(!m_irs->GetADOEOF())
                {
                    m_list[i].item_body[j].itemID = (short)m_irs->GetCollect("itemID");
                    m_list[i].item_body[j].itemNum = (_byte)m_irs->GetCollect("itemNum");
                    m_list[i].item_body[j].itemFlute = (_byte)m_irs->GetCollect("itemFlute");
                    m_list[i].item_body[j].itemAttrID[0] = (short)m_irs->GetCollect("itemAttrID1");
                    m_list[i].item_body[j].itemAttrVal[0] = (short)m_irs->GetCollect("itemAttrVal1");
                    m_list[i].item_body[j].itemAttrID[1] = (short)m_irs->GetCollect("itemAttrID2");
                    m_list[i].item_body[j].itemAttrVal[1] = (short)m_irs->GetCollect("itemAttrVal2");
                    m_list[i].item_body[j].itemAttrID[2] = (short)m_irs->GetCollect("itemAttrID3");
                    m_list[i].item_body[j].itemAttrVal[2] = (short)m_irs->GetCollect("itemAttrVal3");
                    m_list[i].item_body[j].itemAttrID[3] = (short)m_irs->GetCollect("itemAttrID4");
                    m_list[i].item_body[j].itemAttrVal[3] = (short)m_irs->GetCollect("itemAttrVal4");
                    m_list[i].item_body[j].itemAttrID[4] = (short)m_irs->GetCollect("itemAttrID5");
                    m_list[i].item_body[j].itemAttrVal[4] = (short)m_irs->GetCollect("itemAttrVal5");
                    ++j;
                    m_irs->MoveNext();
                }
                ++i;
                m_rs->MoveNext();
                SafeCloseItemRecordset();
            }

            SafeCloseRecordset();
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool StoreControl::LoadDelStore(BYTE** buff, long* len, long* scount)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    StoreStruct*    list = 0;
    long            size = 0;
    long            itemnum = 0;
    int i = 0;
    int j = 0;
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        time_t tim = time(0);

        sprintf(m_sql, "SELECT COUNT(*) AS total FROM StoreInfo WHERE isDel = 1 OR comNumber = 0 OR ((comExpire <= %li) AND (comExpire > 0)) OR beginTime > %li", (long)tim, (long)tim);
        m_conn.Execute(m_sql, m_rs);
        size = m_rs->GetCollect("total");
        SafeCloseRecordset();

        if(!size)
        {
            ret = true;
        }
        else
        {
            sprintf(m_sql, "SELECT comID, comName, comClass, comTime, comPrice, comRemack, comNumber, comExpire, isHot, beginTime FROM StoreInfo WHERE isDel = 1 OR comNumber = 0 OR ((comExpire <= %li) AND (comExpire > 0)) OR beginTime > %li ORDER BY isHot DESC, comTime DESC", (long)tim, (long)tim);
            m_conn.Execute(m_sql, m_rs);

            list = new StoreStruct[size];
            memset(list, 0, (sizeof(StoreStruct) * size));

            i = 0;
            j = 0;
            while(!m_rs->GetADOEOF())
            {
                list[i].store_head.comID = (long)m_rs->GetCollect("comID");
                list[i].store_head.comClass = (long)m_rs->GetCollect("comClass");
                list[i].store_head.comTime = (time_t)m_rs->GetCollect("comTime");
                list[i].store_head.comPrice = (long)m_rs->GetCollect("comPrice");
                strncpy(list[i].store_head.comName, (char*)(bstr_t)m_rs->GetCollect("comName"), 21);
                strncpy(list[i].store_head.comRemark, (char*)(bstr_t)m_rs->GetCollect("comRemack"), 129);
                list[i].store_head.comNumber = (int)m_rs->GetCollect("comNumber");
                list[i].store_head.comExpire = (time_t)m_rs->GetCollect("comExpire");
                list[i].store_head.isHot = (_byte)(long)m_rs->GetCollect("isHot");
                list[i].store_head.beginTime = (time_t)m_rs->GetCollect("beginTime");

                sprintf(m_sql, "SELECT COUNT(*) AS total FROM ItemInfo WHERE comID = %li", list[i].store_head.comID);
                m_conn.Execute(m_sql, m_irs);
                list[i].store_head.itemNum = m_irs->GetCollect("total");
                itemnum += list[i].store_head.itemNum;
                SafeCloseItemRecordset();

                sprintf(m_sql, 
                    "SELECT itemID, itemNum, itemFlute, itemAttrID1, itemAttrVal1, itemAttrID2, itemAttrVal2, itemAttrID3, itemAttrVal3, itemAttrID4, itemAttrVal4, itemAttrID5, itemAttrVal5 FROM ItemInfo WHERE comID = %li", 
                    list[i].store_head.comID);
                m_conn.Execute(m_sql, m_irs);

                j = 0;
                list[i].item_body = new ItemInfo[list[i].store_head.itemNum];
                while(!m_irs->GetADOEOF())
                {
                    list[i].item_body[j].itemID = (short)m_irs->GetCollect("itemID");
                    list[i].item_body[j].itemNum = (_byte)m_irs->GetCollect("itemNum");
                    list[i].item_body[j].itemFlute = (_byte)m_irs->GetCollect("itemFlute");
                    list[i].item_body[j].itemAttrID[0] = (short)m_irs->GetCollect("itemAttrID1");
                    list[i].item_body[j].itemAttrVal[0] = (short)m_irs->GetCollect("itemAttrVal1");
                    list[i].item_body[j].itemAttrID[1] = (short)m_irs->GetCollect("itemAttrID2");
                    list[i].item_body[j].itemAttrVal[1] = (short)m_irs->GetCollect("itemAttrVal2");
                    list[i].item_body[j].itemAttrID[2] = (short)m_irs->GetCollect("itemAttrID3");
                    list[i].item_body[j].itemAttrVal[2] = (short)m_irs->GetCollect("itemAttrVal3");
                    list[i].item_body[j].itemAttrID[3] = (short)m_irs->GetCollect("itemAttrID4");
                    list[i].item_body[j].itemAttrVal[3] = (short)m_irs->GetCollect("itemAttrVal4");
                    list[i].item_body[j].itemAttrID[4] = (short)m_irs->GetCollect("itemAttrID5");
                    list[i].item_body[j].itemAttrVal[4] = (short)m_irs->GetCollect("itemAttrVal5");
                    ++j;
                    m_irs->MoveNext();
                }
                ++i;
                m_rs->MoveNext();
                SafeCloseItemRecordset();
            }
            SafeCloseRecordset();
            ret = true;
        }
    }
    if(ret)
    {
        long bufsize = (size * sizeof(StoreInfo)) + (itemnum * sizeof(ItemInfo));
        *len = bufsize;
        *scount = size;
        if(bufsize)
        {
            *buff = new BYTE[bufsize];
            BYTE* tmp = *buff;
            for(i = 0; i < size; i++)       //   取商品列表
            {
                memcpy(tmp, &list[i].store_head, sizeof(StoreInfo));
                tmp += sizeof(StoreInfo);
                for(j = 0; j < list[i].store_head.itemNum; j++)
                {
                    memcpy(tmp, &list[i].item_body[j], sizeof(ItemInfo));
                    tmp += sizeof(ItemInfo);
                }
            }
        }
    }
    for(i = 0; i < size; i++)
    {
        SAFE_DELETE_ARRAY(list[i].item_body);
    }
    SAFE_DELETE_ARRAY(list);
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


long StoreControl::GetBufferSize()
{
    m_lock.lock();
    long ret = (m_size * sizeof(StoreInfo)) + (m_itemnum * sizeof(ItemInfo));
    m_lock.unlock();
    return ret;
}


bool StoreControl::GetBuffer(BYTE* buff, long size)
{
__BEGIN_TRY
    m_lock.lock();
    if(size < m_size)
    {
        m_lock.unlock();
        return false;
    }

    BYTE* ptr = buff;
    for(int i = 0; i < m_size; i++)       //   取商品列表
    {
        memcpy(ptr, &m_list[i].store_head, sizeof(StoreInfo));
        ptr += sizeof(StoreInfo);
        for(int j = 0; j < m_list[i].store_head.itemNum; j++)
        {
            memcpy(ptr, &m_list[i].item_body[j], sizeof(ItemInfo));
            ptr += sizeof(ItemInfo);
        }
    }
    m_lock.unlock();
    return true;
__END_TRY
    m_lock.unlock();
    return false;
}


long StoreControl::QueryStore(long id, char* name, long* comClass)
{
__BEGIN_TRY
    m_lock.lock();
    for(int i = 0; i < m_size; i++)
    {
        if(m_list[i].store_head.comID == id)
        {
            if(0 != m_list[i].store_head.comNumber)
            {
                if((m_list[i].store_head.comExpire > time(0)) || (m_list[i].store_head.comExpire <= 0))
                {
                    strcpy(name, m_list[i].store_head.comName);
                    *comClass = m_list[i].store_head.comClass;
                    m_lock.unlock();
                    return m_list[i].store_head.comPrice;
                }
            }
            m_lock.unlock();
            ReloadToMemory();
            return -2;
        }
    }
    m_lock.unlock();
    return -1;
__END_TRY
    m_lock.unlock();
    return -1;
//__BEGIN_TRY
//    long price;
//    m_lock.lock();
//
//    if(!CheckConnect())
//    {
//        price = -1;
//    }
//    else
//    {
//        sprintf(m_sql, "SELECT comName, comPrice FROM StoreInfo WHERE comID = %li AND isDel = 0", id);
//        m_conn.Execute(m_sql, m_rs);
//
//        if(!m_rs->GetADOEOF())
//        {
//            strcpy(name, (char*)(bstr_t)m_rs->GetCollect("comName"));
//            price = (long)m_rs->GetCollect("comPrice");
//        }
//        else
//        {
//            price = -1;
//        }
//        SafeCloseRecordset();
//    }
//    m_lock.unlock();
//
//    return price;
//__END_TRY
//    m_lock.unlock();
//    return -1;
}


int StoreControl::CountNumber(long id, long num)
{
    int ret = 1;
__BEGIN_TRY
    m_lock.lock();
    for(int i = 0; i < m_size; i++)
    {
        if(m_list[i].store_head.comID == id)
        {
            if(m_list[i].store_head.comNumber > 0)
            {
                m_list[i].store_head.comNumber += (num);
                m_lock.lock();
                if(!CheckConnect())
                {
                    ret = 1;
                }
                else
                {
                    sprintf(m_sql, "UPDATE StoreInfo SET comNumber = (%li) + comNumber WHERE comID = %li AND isDel = 0", num, id);
                    if(m_conn.Execute(m_sql))
                    {
                        ret = 0;
                    }
                }
                m_lock.unlock();

                if(m_list[i].store_head.comNumber <= 0)
                {
                    ReloadToMemory();
                    ret = 2;
                }
                break;
            }
        }
    }

    m_lock.unlock();
    return ret;
__END_TRY
    m_lock.unlock();
    return ret;
}


//////////////////////////////////////////////////



AfficheControl::AfficheControl()
    : m_list(0), m_size(0)
{
}


AfficheControl::~AfficheControl()
{
    SAFE_DELETE_ARRAY(m_list);
    m_size = 0;
}


bool AfficheControl::AddAffiche(AfficheInfo* affi)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "INSERT INTO AfficheInfo(affiTitle, comID, affiTime, isDel, delTime)VALUES('%s', '%s', %li, 0, 0)", 
            affi->affiTitle, affi->comID, time(0));
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Added Affiche, Title: %s, StoreID: %li" __LN, affi->affiTitle, affi->comID);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool AfficheControl::DelAffiche(long aid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE AfficheInfo SET isDel = 1, delTime = %li WHERE affiID = %li  AND isDel = 0", time(0), aid);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Deled Affiche, ID: %li" __LN, aid);
            ret = true;
        }
    }
    m_lock.unlock();

    if(ret)
    {   //  成功，重新从数据库读取数据
        return ReloadToMemory();
    }

    return false;
__END_TRY
    m_lock.unlock();
    return false;
}


bool AfficheControl::UpdateAffiche(AfficheInfo* affi, long aid)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE AfficheInfo SET affiTitle = '%s', comID = '%s' WHERE affiID = %li AND isDel = 0", affi->affiTitle, affi->comID, aid);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据
            affi->affiID = (long)aid;
            for(int i = 0; i < m_size; i++)
            {
                if(m_list[i].affiID == aid)
                {
                    strncpy(m_list[i].affiTitle, affi->affiTitle, 9);
                    strncpy(m_list[i].comID, affi->comID, 33);
                    LG("Management", "Update Affiche, ID: %li, NEW[Title: %s, StoreID: %li]" __LN, aid, affi->affiTitle, affi->comID);
                    break;
                }
            }
            ret = true;
        }
    }
    m_lock.unlock();

    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool AfficheControl::ReloadToMemory()
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        SAFE_DELETE_ARRAY(m_list);
        m_size = 0;

        sprintf(m_sql, "SELECT COUNT(*) AS total FROM AfficheInfo WHERE isDel = 0");
        m_conn.Execute(m_sql, m_rs);
        m_size = m_rs->GetCollect("total");
        SafeCloseRecordset();

        if(!m_size)
        {
            ret = true;
        }
        else
        {
            sprintf(m_sql, "SELECT affiID, affiTitle, comID, affiTime FROM AfficheInfo WHERE isDel = 0");
            m_conn.Execute(m_sql, m_rs);

            m_list = new AfficheInfo[m_size];
            memset(m_list, 0, (sizeof(AfficheInfo) * m_size));

            int i = 0;
            while(!m_rs->GetADOEOF())
            {
                m_list[i].affiID = (long)m_rs->GetCollect("affiID");
                strncpy(m_list[i].affiTitle, (char*)(bstr_t)m_rs->GetCollect("affiTitle"), 9);
                strncpy(m_list[i].comID, (char*)(bstr_t)m_rs->GetCollect("comID"), 33);
                m_list[i].affiTime = (time_t)m_rs->GetCollect("affiTime");
                ++i;
                m_rs->MoveNext();
            }

            SafeCloseRecordset();
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


long AfficheControl::GetBufferSize()
{
    return m_size * sizeof(AfficheInfo);
}


bool AfficheControl::GetBuffer(BYTE* buff, long size)
{
__BEGIN_TRY
    if(size < m_size)
    {
        return false;
    }

    BYTE* ptr = buff;
    for(int i = 0; i < m_size; i++)
    {
        memcpy(ptr, &m_list[i], sizeof(AfficheInfo));
        ptr += sizeof(AfficheInfo);
    }
    return true;
__END_TRY
    return false;
}



//////////////////////////////////////////////////



AccountControl::AccountControl()
{
}


AccountControl::~AccountControl()
{
}


bool AccountControl::GetRoleInfo(RoleInfo* role)
{
__BEGIN_TRY
    if((role->actID < 0) || (!strlen(role->actName)))
    {
        return false;
    }
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT [Money] FROM AccountInfo WHERE accID = %li AND accName = '%s'", 
            role->actID, role->actName);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            role->rplMoney = m_rs->GetCollect("Money");
            //role->moBean = -1;   //  魔豆
            ret = true;
        }
        else
        {
            role->rplMoney = 0;
            //role->moBean = -1;   //  魔豆
            sprintf(m_sql, "INSERT INTO AccountInfo(accID, accName, Money) VALUES(%li, '%s', %li)", 
                role->actID, role->actName, role->rplMoney);
            m_conn.Execute(m_sql);
            ret = true;
        }

        SafeCloseRecordset();
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool AccountControl::CountMoney(RoleInfo* role, long coin)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE AccountInfo SET [Money]=(%li)+[Money] WHERE accID = %li AND accName = '%s'", coin, role->actID, role->actName);
        if(m_conn.Execute(m_sql))
        {
            role->rplMoney += coin;
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool AccountControl::CountBean(RoleInfo* role, long bean)
{
__BEGIN_TRY
    g_servInfo.billing.Trade(10, role->actName, role->chaName, "", bean, 0, "", "");
    return true;
__END_TRY
    return false;
}


bool AccountControl::CountBeanAndMoney(RoleInfo* role, long coin, long bean)
{
__BEGIN_TRY
    return true;
__END_TRY
    return false;
}



//////////////////////////////////////////////////



VIPControl::VIPControl()
    : m_list(0), m_size(0)
{
}


VIPControl::~VIPControl()
{
}


bool VIPControl::AddVIP(long vipID, long vipCredit)
{
    return false;
}


bool VIPControl::UpdateVIP(long oldVipID, long vipCredit)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE VIPInfo SET vipPrice=%li WHERE vipID=%li", vipCredit, oldVipID);
        if(m_conn.Execute(m_sql))
        {
            LG("Management", "Update VIP Price, ID: %li, Price: %li" __LN, oldVipID, vipCredit);
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool VIPControl::DeleteVIP(long vipID)
{
    return false;
}


long VIPControl::GetVIPCredit(long vipID)
{
    for(int i = 0; i < m_size; i++)
    {
        if(m_list[i].vipID == vipID)
        {
            return m_list[i].vipPrice;
        }
    }
    return -1;
}


bool VIPControl::ReloadToMemory()
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        SAFE_DELETE_ARRAY(m_list);
        m_size = 0;

        sprintf(m_sql, "SELECT COUNT(*) AS total FROM VIPInfo");
        m_conn.Execute(m_sql, m_rs);
        m_size = m_rs->GetCollect("total");
        SafeCloseRecordset();

        if(!m_size)
        {
            ret = true;
        }
        else
        {
            sprintf(m_sql, "SELECT vipID, vipPrice FROM VIPInfo");
            m_conn.Execute(m_sql, m_rs);

            m_list = new VIPInfo[m_size];
            memset(m_list, 0, (sizeof(VIPInfo) * m_size));

            int i = 0;
            while(!m_rs->GetADOEOF())
            {
                m_list[i].vipID = (long)m_rs->GetCollect("vipID");
                m_list[i].vipPrice = (long)m_rs->GetCollect("vipPrice");
                ++i;
                m_rs->MoveNext();
            }

            SafeCloseRecordset();
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


//////////////////////////////////////////////////


HistoryControl::HistoryControl()
{
}


HistoryControl::~HistoryControl()
{
}


bool HistoryControl::GetBuffer(BYTE* buff, long size, int type, long section, int num, long accountID)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        switch(type)
        {
        case INFO_TYPE_HISTORY:
            {
                sprintf(m_sql, "SELECT TOP %i tradeTime, Field1, Field2, Field3 FROM OperateHistory WHERE serveID = %li AND accID = %li", num, section, accountID);
                m_conn.Execute(m_sql, m_rs);

                char tmp[64] = {0};
                char tmp2[32] = {0};
                char* ptr = 0;
                char* pos = 0;
                int ipos = 0;
                BYTE* bufptr = buff;
                HistoryInfo hi;
                while(!m_rs->GetADOEOF())
                {
                    hi.tradeTime = m_rs->GetCollect("tradeTime");
                    strcpy(tmp, (char*)(bstr_t)m_rs->GetCollect("Field1"));
                    pos = strchr(tmp, ',');
                    if(pos)
                    {
                        ipos = (int)(pos - tmp);
                        strncpy(tmp2, tmp, ipos);
                        hi.comID = atol(tmp2);  //  编号
                        ptr = pos + 1;
                        strcpy(hi.comName, ptr);    //  名称
                    }

                    hi.tradeMoney = atol((char*)(bstr_t)m_rs->GetCollect("Field2"));     //  价格
                    hi.tradeResult = atoi((char*)(bstr_t)m_rs->GetCollect("Field3"));     //  结果

                    memcpy(bufptr, &hi, sizeof(HistoryInfo));
                    bufptr += sizeof(HistoryInfo);
                }
                SafeCloseRecordset();
                ret = true;
            }  break;
        case INFO_TYPE_EXCHANGE:
            {
                ret = false;
            }  break;
        case INFO_TYPE_VIP:
            {
                ret = false;
            }  break;
        default:
            {
                ret = false;
            }  break;
        }
    }
    m_lock.unlock();
    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool HistoryControl::LogHistory(long section, long long orderID, long accountID, int type, const char* fld1, const char* fld2, const char* fld3, const char* fld4, const char* fld5)
{
    char* msgBuf = new char[SQL_BUFFER_SIZE];
    sprintf(msgBuf, 
            "INSERT INTO OperateHistory(tradeTime, serveID, orderID, accID, Type, Field1, Field2, Field3, Field4, Field5) VALUES(%li, %i, %I64i, %li, %i, '%s', '%s', '%s', '%s', '%s')", 
            time(0), section, orderID, accountID, type, fld1, fld2, fld3, fld4, fld5);
    g_listInfo.log_list.Push(msgBuf);
    strncat(msgBuf, __LN, SQL_BUFFER_SIZE);
    LG("History", msgBuf); 
    return true;
}


bool HistoryControl::ExecuteInsert(const char* sqlcmd)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        if(m_conn.Execute(sqlcmd))
        {
            ret = true;
        }
    }
    m_lock.unlock();
    return ret;
__END_TRY
    LG("BadSQL", sqlcmd);
    LG("BadSQL", __LN);
    m_lock.unlock();
    return false;
}


bool HistoryControl::CheckOrder(long long id)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT tradeID FROM OperateHistory WHERE orderID = %I64i", id);
        m_conn.Execute(m_sql, m_rs);

        if(m_rs->GetADOEOF())
        {
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();
    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


//////////////////////////////////////////////////


MailControl::MailControl()
{
}


MailControl::~MailControl()
{
}


long MailControl::AddMail(MailInfo* mail, long servID)
{
__BEGIN_TRY
    long ret = 0;
    m_lock.lock();
    if(!CheckConnect())
    {
        ret = 0;
    }
    else
    {
        sprintf(m_sql, "IF NOT EXISTS(SELECT mlID FROM MailInfo WHERE replyGM = 0 AND replyTime = 0 AND actID = %li AND chaID = %li) INSERT INTO MailInfo(mlTitle, mlInformation, actID, actName, chaID, chaName, sndTime, servID)VALUES('%s', '%s', %li, '%s', %li, '%s', %li, %li)", 
            mail->actID, mail->chaID, mail->title, mail->description, mail->actID, mail->actName, mail->chaID, mail->chaName, mail->sendTime, servID);
        if(m_conn.Execute(m_sql))
        {
            sprintf(m_sql, "SELECT @@IDENTITY AS 'Identity'");
            m_conn.Execute(m_sql, m_rs);
            ret = (long)m_rs->GetCollect("Identity");
            LG("Mail", "Added Mail, Title: %s, ActID: %li, ActName: %s, ChaID: %li, ChaName: %s, ServerID: %li" __LN, mail->title, mail->actID, mail->actName, mail->chaID, mail->chaName, servID);
            SafeCloseRecordset();
        }
    }
    m_lock.unlock();

    return ret;
__END_TRY
    m_lock.unlock();
    return 0;
}


bool MailControl::ReplyMail(MailInfo* mail, long gmID)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "UPDATE MailInfo SET replyGM = %li, replyTime = %li, replyInformation = '%s' WHERE mlID = %li AND actID = %li AND chaID = %li", gmID, time(0), mail->description, mail->id, mail->actID, mail->chaID);
        if(m_conn.Execute(m_sql))
        {   //  成功，更新内存数据
            LG("Mail", "Update Mail, gmID: %li, actID: %li, chaID: %li" __LN, gmID, mail->actID, mail->chaID);
            ret = true;
        }
    }
    m_lock.unlock();

    return ret;
__END_TRY
    m_lock.unlock();
    return false;
}


bool MailControl::ReadMail(MailInfo* mail, long* servID)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT TOP 1 mlID, mlTitle, mlInformation, actID, actName, chaID, chaName, sndTime, servID FROM MailInfo WHERE replyGM = 0 AND replyTime = 0 AND GMreaderTime < %li", (time(0) - 900/* 15分钟内不被反复读取 */));
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            sprintf(m_sql, "UPDATE MailInfo SET GMreaderTime = %li WHERE mlID = %li", time(0), (long)m_rs->GetCollect("mlID"));
            m_conn.Execute(m_sql);
            mail->id = (long)m_rs->GetCollect("mlID");
            strcpy(mail->title, (char*)(bstr_t)m_rs->GetCollect("mlTitle"));
            strcpy(mail->description, (char*)(bstr_t)m_rs->GetCollect("mlInformation"));
            mail->actID = (long)m_rs->GetCollect("actID");
            strcpy(mail->actName, (char*)(bstr_t)m_rs->GetCollect("actName"));
            mail->chaID = (long)m_rs->GetCollect("chaID");
            strcpy(mail->chaName, (char*)(bstr_t)m_rs->GetCollect("chaName"));
            mail->sendTime = (long)m_rs->GetCollect("sndTime");
            *servID = (long)m_rs->GetCollect("servID");
            LG("Mail", "Read Mail, actID: %li, chaID: %li" __LN, mail->actID, mail->chaID);
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


bool MailControl::ReadReply(MailInfo* mail, RoleInfo* role)
{
__BEGIN_TRY
    bool ret = false;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = false;
    }
    else
    {
        sprintf(m_sql, "SELECT TOP 1 mlID, replyTime, replyInformation FROM MailInfo WHERE replyGM <> 0 AND replyTime <> 0 AND actID = %li AND chaID = %li ORDER BY mlID DESC", role->actID, role->chaID);
        m_conn.Execute(m_sql, m_rs);

        if(!m_rs->GetADOEOF())
        {
            mail->id = (long)m_rs->GetCollect("mlID");
            strcpy(mail->title, "");
            mail->sendTime = (long)m_rs->GetCollect("replyTime");
            strcpy(mail->description, (char*)(bstr_t)m_rs->GetCollect("replyInformation"));
            mail->actID = role->actID;
            strcpy(mail->actName, role->actName);
            mail->chaID = role->chaID;
            strcpy(mail->chaName, role->chaName);
            LG("Mail", "Read Mail Reply, actID: %li, chaID: %li" __LN, mail->actID, mail->chaID);
            ret = true;
        }
        SafeCloseRecordset();
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return false;
}


long MailControl::GetCount()
{
__BEGIN_TRY
    long ret = 0;
    m_lock.lock();

    if(!CheckConnect())
    {
        ret = 0;
    }
    else
    {
        sprintf(m_sql, "SELECT COUNT(*) As total FROM MailInfo WHERE replyGM = 0 AND replyTime = 0 AND GMreaderTime < %li", (time(0) - 900/* 15分钟内不被反复读取 */));
        m_conn.Execute(m_sql, m_rs);
        ret = m_rs->GetCollect("total");
        SafeCloseRecordset();
    }
    m_lock.unlock();
    return ret;
__COM_TRY
__END_TRY
    m_lock.unlock();
    return 0;
}


//////////////////////////////////////////////////
