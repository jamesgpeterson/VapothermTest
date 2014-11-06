#include "Abort.h"

CAbort *CAbort::m_instance = 0;

CAbort::CAbort()
{
   m_abort = false;
}

CAbort *CAbort::Instance()
{
    if (m_instance == 0)
        m_instance = new CAbort();
    return(m_instance);
}

void CAbort::clearRequest()
{
    m_abort = false;
}

void CAbort::requestAbort()
{
    m_abort = true;
}

bool CAbort::abortRequested()
{
    return(m_abort);
}
