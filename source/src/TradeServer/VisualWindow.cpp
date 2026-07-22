
#include "infotype.h"
#include "resource.h"
#include "mylistview.h"
#include "billing.h"


#define CHECK_TIMER         1001
#define SYNC_STORE          1002
#define TOTAL_INFO          15
#define SYS_BASE            0
#define DB_BASE             7
#define EPT_BASE            14


long g_socket_size = 0;
const char g_threadState[][32] = {"Stopped", "Resting", "Working", "Stopped"};

#if TEST_SERVER == 1
DWORD totaltime = 0;
DWORD totalcount = 0;
DWORD nettotaltime = 0;
DWORD nettotalcount = 0;
#endif


extern void SyncStore();

INT_PTR CALLBACK InfoServerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static MyListView s_sock_list;
    static MyListView s_thread_list;
    static MyListView s_system_info;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            char buf[32] = {0};
            int i = 0;
            s_sock_list.GetListViewHandle(hWnd, IDC_CLIENT_LIST);
            s_thread_list.GetListViewHandle(hWnd, IDC_THREAD_LIST);
            s_system_info.GetListViewHandle(hWnd, IDC_SYSTEM_INFO);


            //  Socket区
            s_sock_list.SetExtended(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
            s_sock_list.InsertColumn(1, "SOCKET", 60);
            s_sock_list.InsertColumn(2, "IP", 100);
            s_sock_list.InsertColumn(3, "PORT", 70);
            s_sock_list.InsertColumn(4, "Recv Bytes", 120);
            s_sock_list.InsertColumn(5, "Send Bytes", 120);
            s_sock_list.InsertColumn(6, "Check", 50);


            //  线程区
            s_thread_list.SetExtended(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
            s_thread_list.InsertColumn(1, "Thread ID", 80);
            s_thread_list.InsertColumn(2, "Thread Type", 140);
            s_thread_list.InsertColumn(3, "Thread State", 100);

            s_thread_list.InsertItem(0);    //  监听线程
            sprintf(buf, "%li", g_servInfo.serv_apt.m_threadid);
            s_thread_list.SetItemText(0, 0, buf);
            s_thread_list.SetItemText(0, 1, "IOCP Accept Thread");
            g_servInfo.serv_apt.m_ui_idx = 0;

            //  工作线程
            for(i = 1; i <= g_servInfo.serv_numWork; i++)
            {
                s_thread_list.InsertItem(i);
                sprintf(buf, "%li", g_servInfo.serv_work[i-1].m_threadid);
                s_thread_list.SetItemText(i, 0, buf);
                s_thread_list.SetItemText(i, 1, "IOCP Work Thread");
                g_servInfo.serv_work[i-1].m_ui_idx = i;
            }

            //  商城逻辑线程
            int tmp = i + STORE_THREAD_NUMBER;
            int j = 0;
            for(; i < tmp; i++, j++)
            {
                s_thread_list.InsertItem(i);
                sprintf(buf, "%li", g_logicInfo.lgc_store[j].m_threadid);
                s_thread_list.SetItemText(i, 0, buf);
                s_thread_list.SetItemText(i, 1, "Store Logic Thread");
                g_logicInfo.lgc_store[j].m_ui_idx = i;
            }

            //  管理线程
            s_thread_list.InsertItem(i);
            sprintf(buf, "%li", g_logicInfo.man_thr.m_threadid);
            s_thread_list.SetItemText(i, 0, buf);
            s_thread_list.SetItemText(i, 1, "Manage Thread");
            g_logicInfo.man_thr.m_ui_idx = i;
            ++i;

            //  邮件线程
            s_thread_list.InsertItem(i);
            sprintf(buf, "%li", g_logicInfo.mail_thr.m_threadid);
            s_thread_list.SetItemText(i, 0, buf);
            s_thread_list.SetItemText(i, 1, "Mail Thread");
            g_logicInfo.mail_thr.m_ui_idx = i;
            ++i;

            //  日志线程
            s_thread_list.InsertItem(i);
            sprintf(buf, "%li", g_logicInfo.log_thr.m_threadid);
            s_thread_list.SetItemText(i, 0, buf);
            s_thread_list.SetItemText(i, 1, "Log Thread");
            g_logicInfo.log_thr.m_ui_idx = i;
            ++i;

			// Add by lark.li 20080819 begin
            s_thread_list.InsertItem(i);
            sprintf(buf, "%li", g_logicInfo.web_thr.m_threadid);
            s_thread_list.SetItemText(i, 0, buf);
            s_thread_list.SetItemText(i, 1, "Web Thread");
            g_logicInfo.web_thr.m_ui_idx = i;
            //++i;
			// End

            //  信息区
            s_system_info.SetExtended(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
            s_system_info.InsertColumn(1, "Name", 120);
            s_system_info.InsertColumn(2, "Value", 140);

            for(i = 0; i < TOTAL_INFO; i++)
            {
                s_system_info.InsertItem(i);
            }


            s_system_info.SetItemText(SYS_BASE+0, 0, "Server Path:");
            s_system_info.SetItemText(SYS_BASE+0, 1, g_servInfo.serv_dir);


            s_system_info.SetItemText(SYS_BASE+1, 0, "Server Name:");
            s_system_info.SetItemText(SYS_BASE+1, 1, g_servInfo.serv_name);

            s_system_info.SetItemText(SYS_BASE+2, 0, "Server Port:");
            sprintf(buf, "%i", g_servInfo.serv_port);
            s_system_info.SetItemText(SYS_BASE+2, 1, buf);

            s_system_info.SetItemText(SYS_BASE+3, 0, "Exchange Rate:");
            sprintf(buf, "1:%i", g_servInfo.serv_rate);
            s_system_info.SetItemText(SYS_BASE+3, 1, buf);

            s_system_info.SetItemText(SYS_BASE+4, 0, "Starting Time:");
            tm* nowTm = localtime(&g_servInfo.start_time);
            sprintf(buf, "%04i-%02i-%02i %02i:%02i:%02i", 
                (nowTm->tm_year + 1900), (nowTm->tm_mon + 1), nowTm->tm_mday, nowTm->tm_hour, nowTm->tm_min, nowTm->tm_sec);
            s_system_info.SetItemText(SYS_BASE+4, 1, buf);

            s_system_info.SetItemText(SYS_BASE+5, 0, "Operational Time:");


            s_system_info.SetItemText(SYS_BASE+6, 0, "");


            s_system_info.SetItemText(DB_BASE+0, 0, "Database Server:");
            s_system_info.SetItemText(DB_BASE+0, 1, g_servInfo.db_serv);

            s_system_info.SetItemText(DB_BASE+1, 0, "Database Username:");
            s_system_info.SetItemText(DB_BASE+1, 1, /*g_servInfo.db_usr*/"********");

            s_system_info.SetItemText(DB_BASE+2, 0, "Database Password:");
            s_system_info.SetItemText(DB_BASE+2, 1, /*g_servInfo.db_pwd*/"********");

            s_system_info.SetItemText(DB_BASE+3, 0, "Database Name:");
            s_system_info.SetItemText(DB_BASE+3, 1, g_servInfo.db_name);

            s_system_info.SetItemText(DB_BASE+4, 0, "Billing Server1:");
            s_system_info.SetItemText(DB_BASE+4, 1, g_servInfo.billing_serv[0]);

            s_system_info.SetItemText(DB_BASE+5, 0, "Billing Server2:");
            s_system_info.SetItemText(DB_BASE+5, 1, g_servInfo.billing_serv[1]);

            s_system_info.SetItemText(DB_BASE+6, 0, "");


            s_system_info.SetItemText(EPT_BASE+0, 0, "Exception Count:");

            SetTimer(hWnd, CHECK_TIMER, 3000, 0);
            SetTimer(hWnd, SYNC_STORE, 180000, 0);

            if(!g_servInfo.billing.Init(hWnd))
            {
                LGPRT("System", "Billing Init Failt." __LN);
            }
            LGPRT("Billing", "Billing Init Success." __LN);
            if(g_servInfo.billing.IsInit())
            {
                char* ipList[2] = {0};
                ipList[0] = g_servInfo.billing_serv[0];
                ipList[1] = g_servInfo.billing_serv[1];
                if(g_servInfo.billing.Connect((const char**)ipList, g_servInfo.billing_port, 2))
                {
                    LGPRT("Billing", "Billing Connect Succeed." __LN);
                }
            }
            LGPRT("Billing", "Billing Version: %i" __LN, g_servInfo.billing.GetVersion());
        }  break;

    case WM_TIMER:
        {
            switch(LOWORD(wParam))
            {
            case SYNC_STORE:
                {
                    SyncStore();    //  每3分钟同步一次商城物品
                }  break;
            case CHECK_TIMER:
                {
                    //  检查所有连接

                    char buf[32] = {0};
                    int i = 0;
                    sprintf(buf, "%li(S)", time(0) - g_servInfo.start_time);
                    s_system_info.SetItemText(SYS_BASE+5, 1, buf);
                    sprintf(buf, "%li", g_servInfo.excpt_count);
                    s_system_info.SetItemText(EPT_BASE+0, 1, buf);

                    s_thread_list.SetItemText(g_servInfo.serv_apt.m_ui_idx, 2, g_threadState[g_servInfo.serv_apt.m_state]);//   监听线程
                    for(i = 0; i < g_servInfo.serv_numWork; i++)    //  工作线程
                    {
                        s_thread_list.SetItemText(g_servInfo.serv_work[i].m_ui_idx, 2, g_threadState[g_servInfo.serv_work[i].m_state]);
                    }
                    for(i = 0; i < STORE_THREAD_NUMBER; i++)
                    {
                        s_thread_list.SetItemText(g_logicInfo.lgc_store[i].m_ui_idx, 2, g_threadState[g_logicInfo.lgc_store[i].m_state]); //  逻辑线程
                    }
                    s_thread_list.SetItemText(g_logicInfo.man_thr.m_ui_idx, 2, g_threadState[g_logicInfo.man_thr.m_state]);     //  管理线程
                    s_thread_list.SetItemText(g_logicInfo.mail_thr.m_ui_idx, 2, g_threadState[g_logicInfo.mail_thr.m_state]);   //  邮件线程
                    s_thread_list.SetItemText(g_logicInfo.log_thr.m_ui_idx, 2, g_threadState[g_logicInfo.log_thr.m_state]);     //  日志线程

					// Add by lark.li 20080819 begin
					s_thread_list.SetItemText(g_logicInfo.web_thr.m_ui_idx, 2, g_threadState[g_logicInfo.web_thr.m_state]);     //  Web线程
					// End


                    g_listInfo.sock_list.Check();
                    //  Socket 列表
                    if(g_socket_size != g_listInfo.sock_list.Size())
                    {
                        s_sock_list.DeleteAllItem();
                        __SockList::iterator it;
                        //g_listInfo.sock_list.m_lock.lock();
                        g_socket_size = (int)g_listInfo.sock_list.m_list.size();
                        for(it = g_listInfo.sock_list.m_list.begin(), i = 0; it != g_listInfo.sock_list.m_list.end(); it++, i++)
                        {
                            s_sock_list.InsertItem(i);
                            (*it)->m_ui_idx = i;
                            sprintf(buf, "%li", (*it)->m_sock.m_socket);
                            s_sock_list.SetItemText(i, 0, buf);
                            s_sock_list.SetItemText(i, 1, (*it)->m_sock.GetIPAddress());
                            sprintf(buf, "%i", (*it)->m_sock.GetPort());
                            s_sock_list.SetItemText(i, 2, buf);
                            sprintf(buf, "%li(B)", (*it)->m_RcvPkt);
                            s_sock_list.SetItemText(i, 3, buf);
                            sprintf(buf, "%li(B)", (*it)->m_SndPkt);
                            s_sock_list.SetItemText(i, 4, buf);
                            s_sock_list.SetItemText(i, 5, (*it)->m_check ? "Y" : "N");
                        }
                        //g_listInfo.sock_list.m_lock.unlock();
                    }
                    else
                    {
                        __SockList::iterator it;
                        //g_listInfo.sock_list.m_lock.lock();
                        for(it = g_listInfo.sock_list.m_list.begin(); it != g_listInfo.sock_list.m_list.end(); it++)
                        {
                            sprintf(buf, "%li(B)", (*it)->m_RcvPkt);
                            s_sock_list.SetItemText((*it)->m_ui_idx, 3, buf);
                            sprintf(buf, "%li(B)", (*it)->m_SndPkt);
                            s_sock_list.SetItemText((*it)->m_ui_idx, 4, buf);
                            s_sock_list.SetItemText(i, 5, (*it)->m_check ? "Y" : "N");
                        }
                        //g_listInfo.sock_list.m_lock.unlock();
                    }

#if TEST_SERVER == 1
                    printf("Total Store Times: %li, Counts: %li, Avg: %lf\n", totaltime, totalcount, ((totaltime == 0) ? 0 :((double)((double)totalcount / (double)totaltime) * 1000)));
                    printf("Total Net Times: %li, Counts: %li, Avg: %lf\n", nettotaltime, nettotalcount, ((nettotaltime == 0) ? 0 :((double)((double)nettotalcount / (double)nettotaltime) * 1000)));
#endif
                }  break;
            }
        }  break;

    case MLY_bilNotify:
        {
            g_servInfo.billing.BilCallBack((long)wParam, (long)lParam);
        }  break;

    case MLY_bilStatus:
        {
            g_servInfo.billing.BilStatus((long)wParam, (long)lParam);
        }  break;

    case WM_CLOSE:
        {
            if(g_servInfo.billing.IsInit())
            {
                g_servInfo.billing.Disconnect();
                LGPRT("Billing", "Billing Disconnect Success." __LN);
            }
            Sleep(5000);    //  billing要求
            if(!g_servInfo.billing.Terminate())
            {
                LGPRT("System", "Billing Terminate Failt." __LN);
            }
            LGPRT("Billing", "Billing Terminate Success." __LN);
            KillTimer(hWnd, CHECK_TIMER);
            KillTimer(hWnd, SYNC_STORE);
            PostQuitMessage(0);
        }  break;
    }
    return 0;
}


