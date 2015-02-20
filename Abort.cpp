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

/*!
 * @brief CAbort constructor
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CAbort::CAbort()
{
   m_abort = false;
}

/*!
 * @brief Returns the pointer to the class object
 *
 * The first time this is called the class is intantiated.
 * There after, a pointer to the instatiated object is returned.
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CAbort *CAbort::Instance()
{
    if (m_instance == 0)
        m_instance = new CAbort();
    return(m_instance);
}

/*!
 * @brief Resets the request to abort
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
void CAbort::clearRequest()
{
    m_abort = false;
}

/*!
 * @brief Sets the state indicating that an abort is requested.
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
void CAbort::requestAbort()
{
    m_abort = true;
}

/*!
 * @brief Returns the current abort state
 *
 * @return returns true if an abort has been requested, false otherwise
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
bool CAbort::abortRequested()
{
    return(m_abort);
}
