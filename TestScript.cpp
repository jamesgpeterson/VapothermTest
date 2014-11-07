#include <stdio.h>
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

        if (pCommand->m_type == CCommand::CMD_END_SCRIPT)
            break;

        switch (pCommand->m_type)
        {
            case CCommand::CMD_TEST:
            {
                break;
            }

            case CCommand::CMD_VERSION:
            {
                break;
            }

            case CCommand::CMD_TYPE:
            {
                QString line = "TestType: ";
                line.append(pCommand->m_stringArg);
                logStringBlack(line.toLocal8Bit());
                break;
            }

            case CCommand::CMD_DESC:
            {
                QString line = "TestDesc: ";
                line.append(pCommand->m_stringArg);
                logStringBlack(line.toLocal8Bit());
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
                QMessageBox::StandardButton reply;
                QString title = "VapoTherm Test";
                QString prefix = "<font size=20>";
                QString suffix = "</font>";
                QString line = prefix + pCommand->m_stringArg + suffix;
                reply = QMessageBox::question(NULL, title, line, QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::No)
                {
                    m_errorEncountered = true;
                    logStringRed("User responce: NEGATIVE");
                }
                else
                {
                    logStringBlack("User responce: POSITIVE");
                }
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
                    logStringRed("FAILED to read from instrument");
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
                    logStringRed("FAILED to read from instrument");
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

            case CCommand::CMD_EXPECT:
            {
                logStringBlack(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                if (args.size() < pCommand->m_argNumber)
                {
                    logStringRed("Test FAILED - expected field not found");
                    m_errorEncountered = true;
                }
                else
                {
                    bool ok;
                    double testNumber = args[pCommand->m_argNumber-1].toDouble(&ok);
                    if (!ok)
                    {
                        logStringRed("Test FAILED - unexpected data returned from instrument");
                        m_errorEncountered = true;
                    }
                    else if ((testNumber < pCommand->m_argMin) || (testNumber > pCommand->m_argMax))
                    {
                        char msg[500];
                        sprintf(msg, "Test FAILED: %0.3lf not in expected range [%0.3lf, %0.3lf]", testNumber, pCommand->m_argMin, pCommand->m_argMax);
                        logStringRed(msg);
                        m_errorEncountered = true;
                    }
                    else
                    {
                        char msg[500];
                        sprintf(msg, "Test PASSED: %0.3lf in expected range [%0.3lf, %0.3lf]", testNumber, pCommand->m_argMin, pCommand->m_argMax);
                        logStringBlack(msg);
                    }
                }
                break;
            }

        case CCommand::CMD_EXPECT_CHAR:
            {
                logStringBlack(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                if (pCommand->m_argNumber > args.size())
                {
                    logStringRed("Test FAILED reading data from instrument");
                    m_errorEncountered = true;
                }
                else
                {
                    QString *arg = &args[pCommand->m_argNumber-1];
                    int argLength = arg->size();
                    if (pCommand->m_charNumber > argLength)
                    {
                        logStringRed("Test FAILED - argument length");
                        m_errorEncountered = true;
                    }
                    else if (arg->toLocal8Bit()[pCommand->m_charNumber-1] != pCommand->m_expectedChar)
                    {
                        char msg[200];
                        sprintf(msg, "Test FAILED - field did not match - expected \'%c\', saw \'%c\'",
                                pCommand->m_expectedChar, arg->toLocal8Bit().at(pCommand->m_charNumber-1));
                        logStringRed(msg);
                        m_errorEncountered = true;
                    }
                    else
                    {
                        logStringBlack("Test PASSED");
                    }
                }
                break;
            }

        case CCommand::CMD_EXPECT_STR:
            {
                logStringBlack(pCommand->m_line.toLocal8Bit());
                QString line = m_responseBuffer;
                line = line.trimmed();
                QStringList args = line.split(QRegExp(" "), QString::SkipEmptyParts);
                if (pCommand->m_argNumber > args.size())
                {
                    logStringRed("Test FAILED reading data from instrument");
                    m_errorEncountered = true;
                }
                else
                {
                    QString *arg = &args[pCommand->m_argNumber-1];
                    int argLength = arg->size();
                    if (pCommand->m_charNumber > argLength)
                    {
                        logStringRed("Test FAILED - argument length");
                        m_errorEncountered = true;
                    }
                    else if (arg != pCommand->m_stringArg)
                    {
                        logStringRed("Test FAILED - expected string not found");
                        m_errorEncountered = true;
                    }
                    else
                    {
                        logStringBlack("Test PASSED");
                    }
                }
                break;
            }

        case CCommand::CMD_WAITFOR:
        {
            logStringBlack(pCommand->m_line.toLocal8Bit());
            qApp->processEvents();
            m_errorEncountered = true;
            int timeout = pCommand->params_WAITFOR.m_timeoutMS;
            int channel = pCommand->params_WAITFOR.m_channelIndex;
            if ((timeout < 0) || (channel < 0))
            {
                logStringRed("Test FAILED (WAITFOR) - invalid parameters");
                break;
            }
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
                logStringRed("Test FAILED (WAITFOR) - timeout");
            }
            else
            {
                QString str = "found string: ";
                str.append(pCommand->params_WAITFOR.m_expectedString);
                logStringBlack(str.toLocal8Bit());
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

        case CCommand::CMD_END_SCRIPT:
        {
            return(false);
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


