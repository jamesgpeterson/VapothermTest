/*!
 * @file Abort.cpp
 * @brief Implements the CAbort class
 *
 * This singleton class holds the state of abort requests from the operator.
 *
 * @author    	J. Peterson
 * @date        02/13/2015
 * @copyright	(C) Copyright Enercon Technologies 2015, All rights reserved.
 *
 * Revision History
 * ----------------
 *  Version | Author       | Date        | Description
 *  :--:    | :-----       | :--:        | :----------
 *   1      | J. Peterson  | 02/13/2015  | initial version
 *
*/
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
