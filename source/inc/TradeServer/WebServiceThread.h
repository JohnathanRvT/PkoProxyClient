#ifndef _WEBSERVICE_H
#define _WEBSERVICE_H

#include "thread.h"

class WebServiceThread :
	public Thread
{
public:
	WebServiceThread(void);
	virtual ~WebServiceThread(void);
    uint Run();

public:
    volatile int m_state;
};

#endif
