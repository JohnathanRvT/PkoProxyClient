#pragma once

class CGuildInviteFormMgr
{
public:
	typedef struct _sFormNode
	{
		DWORD id;
		std::string inviterName;
		CForm *pForm;
	}S_FormNode,*PS_FormNode;
	CGuildInviteFormMgr(void);
	~CGuildInviteFormMgr(void);
	static bool AddInviteForm(DWORD id,std::string inviterName);
	static bool RemoveInviteForm(DWORD id);
	static PS_FormNode GetInviteForm(DWORD id);
private:
	static std::vector<PS_FormNode> m_FormLink;
	static void _MainMousePlayerGuildEvent(CCompent* pSender, int nMsgType, int x, int y, MouseClickState dwKey);
};

extern CGuildInviteFormMgr g_stGuildInviteFormMgr;
