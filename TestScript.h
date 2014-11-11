#ifndef TESTSCRIPT_H
#define TESTSCRIPT_H
#include <vector>

#include <QObject>
#include "Command.h"



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
    void generateTestHeader(int lineNumber);
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
};

#endif // TESTSCRIPT_H
