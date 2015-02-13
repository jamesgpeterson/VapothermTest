/*!
 * @file Abort.h
 * @brief Declares the CAbort class
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
#ifndef ABORT_H
#define ABORT_H

class CAbort
{
public:
    static CAbort *Instance();
    void clearRequest();
    void requestAbort();
    bool abortRequested();

private:
    CAbort();
    static CAbort *m_instance;
    bool m_abort;                //! true if an abort has been requested by the operator
};

#endif // ABORT_H
