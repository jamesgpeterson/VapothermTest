/*!
 * @file TestScript.cpp
 * @brief Implements the CTestScript class
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
#include <stdio.h>
#include <time.h>
#include <QMessageBox>
#include <QtCore/QtGlobal>
#include <QTimer>
#include "TestScript.h"
#include <QCloseEvent>
#include <QMainWindow>
#include <QApplication>
#include <QTime>
#include "Abort.h"
#include <QFileInfo>
#include <QMessageBox>



/*!
 * @brief CTestScript constructor
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CTestScript::CTestScript()
{
    m_commandList.clear();
    m_testList.clear();
    m_responseBuffer[0] = '\0';
    m_terminateOnError = false;
    m_errorEncountered = false;
    m_terminatedEarly = false;
}


/*!
 * @brief CTestScript destructor
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CTestScript::~CTestScript()
{
    m_commandList.clear();
    m_testList.clear();
}

const QString *CTestScript::getScriptVersion()
{
    return(&m_version);
}




/*!
 * @brief Reads a script file parsing the commands as it goes
 *
 * @param[in] filename - name of the script file
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
bool CTestScript::readScriptFile(const char *filename)
{
    //
    // Clear out the current list of tests and commands
    //
    m_testList.clear();
    m_commandList.clear();
    //m_version = "none";
    m_version.clear();

    //
    // If no file name was given then we are done
    //
    if ((filename == NULL) || (filename[0] == '\0'))
    {
        return(false);
    }

    //
    // Open the file
    //
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        QMessageBox msgBox;
        msgBox.setText("Could not open script file.");
        msgBox.exec();
        return(false);
    }


    //
    // Read each line of the script file and parse it.
    while (!feof(fp))
    {
        char lineBuffer[1024];
        if (fgets(lineBuffer, sizeof(lineBuffer), fp))
        {
            //
            // create the new command
            //
            m_commandList.push_back(CCommand());
            int i = m_commandList.size() - 1;
            CCommand *pCommand = &m_commandList[i];
            pCommand->parse(lineBuffer, i);
            if (pCommand->m_type == CCommand::CMD_UNKNOWN)
            {
                QString title = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
                QString msg = "<html><head/><body><p><span style=\" font-size:10pt; font-weight:600; color:#F00000;\"><pre>Poorly formed command on line ";
                QString lineNum;
                lineNum.setNum(i+1, 10);
                msg.append(lineNum);
                msg.append(":\n\n    ");
                msg.append(lineBuffer);
                msg.append("\n</pre></span></p></body></html>");
                QMessageBox::warning(NULL, title, msg);
            }
            if (pCommand->m_type == CCommand::CMD_VERSION)
            {
                if (!m_version.isEmpty())
                {
                    QString title = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
                    QString msg = "<html><head/><body><p><span style=\" font-size:10pt; font-weight:600; color:#F00000;\"><pre>Re-declaration of Version on line: ";
                    QString lineNum;
                    lineNum.setNum(i, 10);
                    msg.append(lineNum);
                    msg.append(":\n\n    ");
                    msg.append(lineBuffer);
                    msg.append("\n</pre></span></p></body></html>");
                    QMessageBox::warning(NULL, title, msg);
                }
                m_version = pCommand->m_scriptVersion;
            }
            if (pCommand->m_type == CCommand::CMD_TEST)
            {
                m_testList.push_back(i);
            }
        }
    }

    fclose(fp);

    return(true);
}


int CTestScript::findTestByName(QString &name)
{
    int testCount = getTestCount();
    for (int i=0; i<testCount; i++)
    {
        QString *pStr = getTestName(i);
        if (name == *pStr)
            return(i);
    }
    return(-1);
}



/*!
 * @brief Runs the specified test from the currently loaded test script
 *
 * @param[in] n - number of test to run
 * @return Returns a bool indicating if the tests should be aborted
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
bool CTestScript::runTest(unsigned int n)
{
    m_errorEncountered = false;
    m_terminatedEarly = false;

    if (n >= m_testList.size())
    {
        return(true);
    }

    int firstCommand = m_testList[n];
    int lastCommand = (n < m_testList.size()-1) ? m_testList[n+1] : m_commandList.size();

    for (int i=firstCommand; i<lastCommand; i++)
    {
        if (CAbort::Instance()->abortRequested())
        {
            m_errorEncountered = true;
            return(true);
        }

        CCommand *pCommand = &m_commandList[i];

        switch (pCommand->m_type)
        {
            case CCommand::CMD_TEST:
            {
                m_currentTest = pCommand->m_stringArg;
                m_currentDesc.clear();
                m_currentUnits.clear();
                break;
            }

            case CCommand::CMD_VERSION:
            {
                break;
            }
#if 0
            case CCommand::CMD_TYPE:
            {
                QString line = "TestType: ";
                line.append(pCommand->m_stringArg);
                logStringBlack(line.toLocal8Bit());
                break;
            }
#endif

            case CCommand::CMD_DESC:
            {
                m_currentDesc = pCommand->m_stringArg;
                break;
            }

            case CCommand::CMD_SLEEP:
            {
                logStringGray(pCommand->m_line.toLocal8Bit());
                QEventLoop loop;
                QTimer::singleShot(pCommand->m_argInteger, &loop, SLOT(quit()));
                loop.exec();
                break;
            }

            case CCommand::CMD_PROMPT:
            {
                generateTestHeader();
                logStringBlack("Nominal: \"YES\"");
                QMessageBox::StandardButton reply;
                QString title = "VapoTherm Test";
                QString prefix = "<font size=20>";
                QString suffix = "</font>";
                QString line = prefix + pCommand->m_stringArg + suffix;
                reply = QMessageBox::question(NULL, title, line, QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::No)
                {
                    m_errorEncountered = true;
                    logStringRed("Value: \"NO\"");
                    logStringRed("Result: FAIL");
                    logStringRed("FailDesc: failed operator inspection");
                }
                else
                {
                    logStringBlack("Value: \"YES\"");
                    logStringBlack("Result: PASS");
                }
                generateTestTrailer();
                break;
            }

            case CCommand::CMD_PAUSE:
            {
                QString title = "VapoTherm Test";
                QString prefix = "<font size=20>";
                QString suffix = "</font>";
                QString line = prefix + pCommand->m_stringArg + suffix;
                QMessageBox::information(NULL, title, line, QMessageBox::Ok);
                break;
            }

            case CCommand::CMD_SENDLINE_A:
            {
                if (!sendVapoThermCommand(0, pCommand->m_stringArg.toLocal8Bit()))
                {
                    m_errorEncountered = true;
                }
                logCommand(pCommand->m_stringArg.toLocal8Bit());
                readVapoThermResponse(0, m_responseBuffer, sizeof(m_responseBuffer), m_timeoutA_ms);
                logReply(m_responseBuffer);
                break;
            }

            case CCommand::CMD_READLINE_A:
            {
                qApp->processEvents();
                m_responseBuffer[0] = '\0';
                if (!readVapoThermResponse(0, m_responseBuffer, sizeof(m_responseBuffer), m_timeoutA_ms))
                {
                    logStringGray("Failed to read from device or fixture");
                    m_errorEncountered = true;
                }
                else
                {
                    logReply(m_responseBuffer);
                }
                break;
            }

            case CCommand::CMD_SENDLINE_B:
            {
                if (!sendVapoThermCommand(1, pCommand->m_stringArg.toLocal8Bit()))
                {
                    m_errorEncountered = true;
                }
                logCommand(pCommand->m_stringArg.toLocal8Bit());
                readVapoThermResponse(1, m_responseBuffer, sizeof(m_responseBuffer), m_timeoutB_ms);
                logReply(m_responseBuffer);
                break;
            }

            case CCommand::CMD_READLINE_B:
            {
                qApp->processEvents();
                m_responseBuffer[0] = '\0';
                if (!readVapoThermResponse(1, m_responseBuffer, sizeof(m_responseBuffer), m_timeoutB_ms))
                {
                    logStringGray("Failed to read from device or fixture");
                    m_errorEncountered = true;
                }
                else
                {
                    logReply(m_responseBuffer);
                }
                break;
            }

            case CCommand::CMD_FLUSH_A:
            {
                flushIncomingData(0);
                break;
            }

            case CCommand::CMD_FLUSH_B:
            {
                flushIncomingData(1);
                break;
            }

            case CCommand::CMD_UNITS:
            {
                m_currentUnits = pCommand->m_stringArg;
                //QString line = "Units: ";
                //line.append(pCommand->m_stringArg);
                //logStringGray(line);
                break;
            }

            case CCommand::CMD_EXPECT:
            {
                generateTestHeader();
                logStringGray(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                char msg[500];
                sprintf(msg, "LowerLimit: %0.3lf", pCommand->m_argMin);
                logStringBlack(msg);
                sprintf(msg, "UpperLimit: %0.3lf", pCommand->m_argMax);
                logStringBlack(msg);

                if (args.size() < pCommand->m_argNumber)
                {
                    logStringBlack("Value: none");
                    logStringRed("Result: FAIL");
                    logStringRed("FailDesc: expected field not found");
                    m_errorEncountered = true;
                }
                else
                {
                    bool ok;
                    double testNumber = args[pCommand->m_argNumber-1].toDouble(&ok);
                    if (!ok)
                    {
                        logStringBlack("Value: none");
                        logStringRed("Result: FAIL");
                        logStringRed("FailDesc: unexpected data returned from device or fixture");
                        m_errorEncountered = true;
                    }
                    else if ((testNumber < pCommand->m_argMin) || (testNumber > pCommand->m_argMax))
                    {
                        sprintf(msg, "Value: %0.3lf", testNumber);
                        logStringBlack(msg);
                        logStringRed("Result: FAIL");
                        m_errorEncountered = true;
                    }
                    else
                    {
                        sprintf(msg, "Value: %0.3lf", testNumber);
                        logStringBlack(msg);
                        logStringBlack("Result: PASS");
                    }
                }
                generateTestTrailer();
                break;
            }

        case CCommand::CMD_EXPECT_CHAR:
            {
                generateTestHeader();
                logStringGray(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                char msg[500];
                sprintf(msg, "Nominal: \'%c\'", pCommand->m_expectedChar);
                logStringBlack(msg);

                if (pCommand->m_argNumber > args.size())
                {
                    logStringBlack("Value: none");
                    logStringRed("Result: FAIL");
                    logStringRed("FailDesc: failed reading data from device or fixture");
                    m_errorEncountered = true;
                }
                else
                {
                    QString *arg = &args[pCommand->m_argNumber-1];
                    int argLength = arg->size();
                    if (pCommand->m_charNumber > argLength)
                    {
                        logStringBlack("Value: none");
                        logStringRed("Result: FAIL");
                        logStringRed("FailDesc: argument length is too short");
                        m_errorEncountered = true;
                    }
                    else if (arg->toLocal8Bit()[pCommand->m_charNumber-1] != pCommand->m_expectedChar)
                    {
                        char msg[200];

                        char c = arg->toLocal8Bit().at(pCommand->m_charNumber-1);
                        sprintf(msg, "Value: \'%c\'", c);
                        logStringRed(msg);
                        logStringRed("Result: FAIL");
                        m_errorEncountered = true;
                    }
                    else
                    {
                        char c = arg->toLocal8Bit().at(pCommand->m_charNumber-1);
                        sprintf(msg, "Value: \'%c\'", c);
                        logStringBlack(msg);
                        logStringBlack("Result: PASS");
                    }
                }
                generateTestTrailer();
                break;
            }

        case CCommand::CMD_EXPECT_STR:
            {
                generateTestHeader();
                logStringGray(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                char msg[500];
                sprintf(msg, "Nominal: \"%s\"", pCommand->m_stringArg.toLocal8Bit().data());
                logStringBlack(msg);
                if (pCommand->m_argNumber > args.size())
                {
                    logStringBlack("Value: none");
                    logStringRed("Result: FAIL");
                    logStringRed("FailDesc: failed to read data from device or fixture");
                    m_errorEncountered = true;
                }
                else
                {
                    sprintf(msg, "Value: \"%s\"", args[pCommand->m_argNumber-1].toLocal8Bit().data());
                    logStringBlack(msg);
                    QString *arg = &args[pCommand->m_argNumber-1];
                    if (arg != pCommand->m_stringArg)
                    {
                        logStringRed("Result: FAIL");
                        logStringRed("FailDesc: expected string not found");
                        m_errorEncountered = true;
                    }
                    else
                    {
                        logStringBlack("Result: PASS");
                    }
                }
                generateTestTrailer();
                break;
            }

        case CCommand::CMD_WAITFOR:
        {
            logStringGray(pCommand->m_line.toLocal8Bit());
            qApp->processEvents();
            m_errorEncountered = true;
            int timeout = pCommand->params_WAITFOR.m_timeoutMS;
            int channel = pCommand->params_WAITFOR.m_channelIndex;
            QTime t;
            t.start();
            while (t.elapsed() < timeout)
            {
                qApp->processEvents();
                m_responseBuffer[0] = '\0';
                if (readVapoThermResponse(channel, m_responseBuffer, sizeof(m_responseBuffer), timeout))
                {
                    logReply(m_responseBuffer);
                    QString str = m_responseBuffer;
                    if (str.contains(*pCommand->params_WAITFOR.m_expectedString))
                    {
                        m_errorEncountered = false;
                        break;
                    }
                }
            }
            if (m_errorEncountered)
            {
                logStringGray("WAITFOR: command timed out");
            }
            else
            {
                QString str = "WAITFOR: found expected string (";
                str.append(pCommand->params_WAITFOR.m_expectedString);
                str.append(")");
                logStringGray(str.toLocal8Bit());
            }
            break;
        }

        case CCommand::CMD_COMMENT:
            {
                if (pCommand->m_line.length() > 0)
                {
                    logStringGray(pCommand->m_line.toLocal8Bit());
                }
                break;
            }

        case CCommand::CMD_END_ON_ERROR:
        {
            if (m_errorEncountered)
            {
                m_terminatedEarly = true;
                return(false);
            }
            break;
        }

        case CCommand::CMD_UNKNOWN:
        default:
            {
                QString msg = "Unknown directive: " + pCommand->m_line;
                logStringRed(msg.toLocal8Bit());
                m_errorEncountered = true;
                break;
            }
        }

        if (m_errorEncountered && m_terminateOnError)
        {
            break;
        }
    }

    return(true);
}



/*!
 * @brief Reports the number of test in the current script
 *
 * @return number of tests in script
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
int CTestScript::getTestCount()
{
    return(m_testList.size());
}


/*!
 * @brief returns the command description/name
 *
 * @return returns a pointer to the QString that is the command description
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
QString *CTestScript::getTestName(unsigned int n)
{
    if (n >= m_testList.size())
    {
        return(NULL);
    }

    int commandIndex = m_testList[n];
    return(&m_commandList[commandIndex].m_stringArg);
}

void CTestScript::generateTestHeader()
{
    //
    // Add the test name
    //
    logStringBlack(" ");
    QString TestNameLine = "TestName: ";
    TestNameLine.append(m_currentTest);
#if 0
    TestNameLine.append("  [line=");
    QString numStr;
    numStr.setNum(lineNumber);
    TestNameLine.append(numStr);
    TestNameLine.append("]");
#endif
    logStringBlack(TestNameLine.toLocal8Bit());

    //
    // Add the test description
    //
    if (!m_currentDesc.isEmpty())
    {
        QString TestDescLine = "TestDesc: ";
        TestDescLine.append(m_currentDesc);
        logStringBlack(TestDescLine.toLocal8Bit());
    }

    //
    // Add the test type
    //
    QString TestTypeLine = "TestType: ";
    TestTypeLine.append(m_currentTest);
    if (!m_currentDesc.isEmpty())
    {
        TestTypeLine.append(" [");
        TestTypeLine.append(m_currentDesc);
        TestTypeLine.append("]");
    }
    else
    {
#if 0
        TestTypeLine.append(" [line=");
        QString numStr;
        numStr.setNum(lineNumber);
        TestTypeLine.append(numStr);
        TestTypeLine.append("]");
#endif
    }
    logStringBlack(TestTypeLine.toLocal8Bit());

    //
    // Add the Date and Time
    //
    time_t rawtime;
    struct tm *t;
    time (&rawtime);
    t = localtime (&rawtime);
    char    tmpStr[100];
    sprintf(tmpStr, "%02d/%02d/%04d", t->tm_mon+1, t->tm_mday, t->tm_year+1900);
    QString line = "Date: ";
    line.append(tmpStr);
    logStringBlack(line.toLocal8Bit());
    sprintf(tmpStr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
    line = "Time: ";
    line.append(tmpStr);
    logStringBlack(line.toLocal8Bit());

    //
    // Add Units
    //
    if (!m_currentUnits.isEmpty())
    {
        QString unitsLine = "Units: ";
        unitsLine.append(m_currentUnits);
        logStringBlack(unitsLine.toLocal8Bit());
    }
}


void CTestScript::generateTestTrailer()
{
    //
    // Add test terminator
    //
    logStringBlack("~#~");
}

