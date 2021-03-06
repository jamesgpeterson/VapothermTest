/*!
 * @file TestScript.h
 * @brief Declares the CTestScript class
 *
 * This class processes a script file that contains a list of script commands
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
#ifndef TESTSCRIPT_H
#define TESTSCRIPT_H
#include <vector>

#include <QObject>
#include "Command.h"



/*!
 * @brief This class implements the test script.
 *
 * The script is a list of commands.  This class provides methods
 * for parsing a script file as well as methods for running tests
 * in the script file.
 *
 * This class uses QT signals to communicate with the test fixture via
 * the COM ports as well as to update the user interface.
 *
 * @date 02/13/2015
 * @author J Peterson
 */
class CTestScript : public QObject
{
    Q_OBJECT

public:
    CTestScript();
    ~CTestScript();

    bool readScriptFile(const char *filename);
    int  getTestCount();
    QString *getTestName(unsigned int n);
    const QString *getScriptVersion();
    bool runTest(unsigned int n);
    bool sawError() { return(m_errorEncountered); }
    bool terminatedEarly() { return(m_terminatedEarly); }
    void terminateOnError(bool terminate) {m_terminateOnError = terminate;}
    void setTimeouts(int timeoutA_ms, int timeoutB_ms) {m_timeoutA_ms = timeoutA_ms; m_timeoutB_ms = timeoutB_ms;}

signals:
    void logStringBlack(const char *string);
    void logStringGray(const char *string);
    void logStringRed(const char *string);
    void logCommand(const char *reply);
    void logReply(const char *reply);
    bool sendVapoThermCommand(int portIndex, const char *command);
    bool readVapoThermResponse(int portIndex, char *buffer, const int bufferSize, const int msTimeout);
    void flushIncomingData(int portIndex);

private:
    int findTestByName(QString &name);
    void generateTestHeader();
    void generateTestTrailer();

private:
    std::vector<CCommand>        m_commandList;
    std::vector<unsigned int>    m_testList;
    char                         m_responseBuffer[10*1024];   // this should be way bigger than is needed
    bool                         m_errorEncountered;
    bool                         m_terminateOnError;
    bool                         m_terminatedEarly;  // this indicates that we don't want to go on with other tests
    int                          m_timeoutA_ms;
    int                          m_timeoutB_ms;
    QString                      m_version;
    QString                      m_currentTest;
    QString                      m_currentDesc;
    QString                      m_currentUnits;
};

#endif // TESTSCRIPT_H
