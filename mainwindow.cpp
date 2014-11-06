#include <time.h>
#include <QLabel>
#include <QString>
#include <QColor>
#include <QMessageBox>
#include <QTimer>
#include <QCheckBox>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QDateTime>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Abort.h"


QString g_stringNotConnected  = "<html><head/><body><p><span style=\" font-size:8pt; font-weight:600; color:#F00000;\">NotConnected</span></p></body></html>";
QString g_stringConnected     = "<html><head/><body><p><span style=\" font-size:8pt; font-weight:600; color:#00F000;\">  Connected</span></p></body></html>";
QString g_stringIdle          = "<html><head/><body><p><span style=\" font-size:20pt; font-weight:600; color:#808080;\"></span></p></body></html>";
QString g_stringWorking       = "<html><head/><body><p><span style=\" font-size:20pt; font-weight:600; color:#808080;\">Testing...</span></p></body></html>";
QString g_stringPassed        = "<html><head/><body><p><span style=\" font-size:26pt; font-weight:600; color:#10D010;\">Passed</span></p></body></html>";
QString g_stringFailed        = "<html><head/><body><p><span style=\" font-size:26pt; font-weight:600; color:#F00000;\">Failed</span></p></body></html>";
QString g_stringNotRun        = "<html><head/><body><p><span style=\" font-size:26pt; font-weight:600; color:#F00000;\">Not Run</span></p></body></html>";
QString g_stringAborted       = "<html><head/><body><p><span style=\" font-size:20pt; font-weight:600; color:#F00000;\">Aborted</span></p></body></html>";

#define NOT_CONNECTED "not connected"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    CAbort::Instance()->clearRequest();
    ui->setupUi(this);
    ui->lineEditSerialNumber->setFocus();
    m_scriptFileName.clear();

    setTitle();

    m_serialPorts[0] = new QSerialPort(this);
    m_serialPorts[1] = new QSerialPort(this);
    connect(&m_script, SIGNAL(logStringBlack(const char*)), this, SLOT(logStringBlack(const char*)));
    connect(&m_script, SIGNAL(logStringGray(const char*)), this, SLOT(logStringGray(const char*)));
    connect(&m_script, SIGNAL(logStringRed(const char*)), this, SLOT(logStringRed(const char*)));
    connect(&m_script, SIGNAL(logCommand(const char *)), this, SLOT(logCommand(const char*)));
    connect(&m_script, SIGNAL(logReply(const char *)), this, SLOT(logReply(const char*)));
    connect(&m_script, SIGNAL(sendVapoThermCommand(int, const char *)), this, SLOT(sendVapoThermCommand(int, const char *)));
    connect(&m_script, SIGNAL(readVapoThermResponse(int, char *, const int , const int )), this, SLOT(readVapoThermResponse(int, char *, const int , const int )));
    connect(&m_script, SIGNAL(flushIncomingData(int)), this, SLOT(flushIncomingData(int)));

    ui->pushButtonStartTests->setEnabled(false);
    ui->pushButton_Abort->setEnabled(false);

    m_inputBufferIndex = 0;
    m_inputBufferCount = 0;


    QList<QSerialPortInfo> commPortList = QSerialPortInfo::availablePorts();
    ui->comboBox_serialPorts_A->setEnabled(false);
    ui->comboBox_serialPorts_B->setEnabled(false);
    ui->comboBox_serialPorts_A->addItem(NOT_CONNECTED);
    ui->comboBox_serialPorts_B->addItem(NOT_CONNECTED);
    for (int i=commPortList.size()-1; i>=0; i--)
    {
        QSerialPortInfo portInfo = commPortList[i];
        ui->comboBox_serialPorts_A->addItem(portInfo.portName());
        ui->comboBox_serialPorts_B->addItem(portInfo.portName());
    }
    ui->comboBox_serialPorts_A->setEnabled(true);
    ui->comboBox_serialPorts_B->setEnabled(true);

    //
    // Open ini file
    //
    m_settings = new QSettings("VapothermTest.ini", QSettings::IniFormat);
    m_settings->sync();

    //
    // Comm Ports
    //
    QString portA = m_settings->value("PortA", NOT_CONNECTED).toString();
    commPortSelected_A(portA);
    QString portB = m_settings->value("PortB", NOT_CONNECTED).toString();
    commPortSelected_B(portB);


    //
    // operator
    //
    QString operatorStr = m_settings->value("Operator", "").toString();
    ui->lineEditOperator->setText(operatorStr);

    //
    // Delay between characters on output
    // and timeout values
    //
    m_outputDelay_ms = m_settings->value("OutputDelayMS", 120).toInt();
    m_timeoutA_ms = m_settings->value("TimeoutMS_A", 100).toInt();
    m_timeoutB_ms = m_settings->value("TimeoutMS_B", 100).toInt();


    //
    // script filename
    //
    m_scriptFileName = m_settings->value("Script", "").toString();
    if (!m_scriptFileName.isEmpty())
    {
        reloadScriptButtonPress();
    }

    //
    // Report directory
    //
    m_reportDir = m_settings->value("ReportDir", "./").toString();
    ui->checkBoxGenerateReport->setChecked(m_settings->value("GenerateReport").toBool());

    //
    // Terminate on Error
    //
    ui->checkBoxTerminateOnError->setChecked(m_settings->value("TerminateOnError").toBool());

    //
    // Check serial number
    //
    m_checkSerialNumber = m_settings->value("CheckSerialNumber").toBool();



    ui->labelResults->setText(g_stringIdle);
}


MainWindow::~MainWindow()
{
    if (m_serialPorts[0]->isOpen())
    {
        m_settings->setValue("PortA", m_serialPorts[0]->portName());
        m_serialPorts[0]->clear();
        m_serialPorts[0]->close();
    }
    else
    {
        m_settings->setValue("PortA", "not connected");
    }

    if (m_serialPorts[1]->isOpen())
    {
        m_settings->setValue("PortB", m_serialPorts[1]->portName());
        m_serialPorts[1]->clear();
        m_serialPorts[1]->close();
    }
    else
    {
        m_settings->setValue("PortB", "not connected");
    }

    m_settings->setValue("Operator", ui->lineEditOperator->text());
    m_settings->setValue("Script", m_scriptFileName);
    m_settings->setValue("ReportDir", m_reportDir);
    m_settings->setValue("GenerateReport", ui->checkBoxGenerateReport->isChecked());
    m_settings->setValue("TerminateOnError", ui->checkBoxTerminateOnError->isChecked());
    m_settings->setValue("CheckSerialNumber", m_checkSerialNumber);

    m_settings->setValue("OutputDelayMS", m_outputDelay_ms);
    m_settings->setValue("TimeoutMS_A", m_timeoutA_ms);
    m_settings->setValue("TimeoutMS_B", m_timeoutB_ms);

    m_settings->sync();
    delete ui;
}


/*!
 * @brief sets the application title with the app name, version and script
 *
 * @author J. Peterson
 * @date 10/03/2014
*/
void MainWindow::setTitle()
{
    QString title = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
    title.append(" (");
    title.append(VERSION_STRING);
    title.append(")");

    if (!m_scriptFileName.isEmpty())
    {
        title.append(" - ");
        title.append(QFileInfo(m_scriptFileName).fileName());
    }
    this->setWindowTitle(title);
}




/*!
 * @brief sleeps for specified milliseconds
 *
 * @author J. Peterson
 * @date 06/11/2014
*/
void MainWindow::snooze(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}


/*!
 * @brief displays a debug messagebox
 *
 * @param[in] msg - string to display in messagebox
 *
 * @author J. Peterson
 * @date 06/11/2014
*/
void MainWindow::displayWarning(const char *msg)
{
    QString title = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
    QMessageBox::warning(this, title, msg, QMessageBox::Ok);
}


/*!
 * @brief displays a debug messagebox
 *
 * @param[in] msg - string to display in messagebox
 *
 * @author J. Peterson
 * @date 06/11/2014
*/
bool MainWindow::displayQuestion(const char *msg)
{
    QString title = QFileInfo( QCoreApplication::applicationFilePath() ).fileName();
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, title, msg, QMessageBox::Yes|QMessageBox::No);

    return(reply == QMessageBox::Yes);
}



/*!
 * @brief called when the "Run Tests" button is pressed
 *
 * @author J. Peterson
 * @date 06/01/2014
*/
void MainWindow::startTestsButtonPress()
{
    enableButtonsAfterRun(false);
    clearButtonPressed();

    CAbort::Instance()->clearRequest();
    bool terminateOnError = ui->checkBoxTerminateOnError->isChecked();

    QString line;
    m_reportStrings.clear();

    //
    // PanelBarcode
    //
    //line = "PanelBarcode:";
    //logStringBlack(line.toLocal8Bit());

    //
    // TestProgram
    //
    line = "TestProgram: ";
    line.append(QFileInfo( QCoreApplication::applicationFilePath() ).fileName());
    line.append(" ");
    line.append(QFileInfo(m_scriptFileName).fileName());
    logStringBlack(line.toLocal8Bit());

    //
    // TestProgramVer
    //
    line = "TestProgramVer: program=";
    line.append(VERSION_STRING);
    line.append(" script=");
    line.append(m_script.getScriptVersion());
    logStringBlack(line.toLocal8Bit());

    //
    // Operator
    //
    line = "Operator: " + ui->lineEditOperator->text();
    logStringBlack(line.toLocal8Bit());

    //
    // ImageBarcode
    //
    QString serialNumber = ui->lineEditSerialNumber->text().trimmed();
    if (m_checkSerialNumber && (serialNumber.isEmpty()))
    {
        logStringRed("Serial number not entered.");
        displayWarning("Serial Number must be entered before tests can be run.");
        ui->labelResults->setText(g_stringNotRun);
        enableButtonsAfterRun(true);
        ui->lineEditSerialNumber->setFocus();
        return;
    }
    if (m_checkSerialNumber && (serialNumber == m_lastSerialNumber))
    {
        QString msg = "Run again with last serial number (";
        msg.append(serialNumber);
        msg.append(")?");
        if (!displayQuestion(msg.toLocal8Bit()))
        {
            ui->labelResults->setText(g_stringNotRun);
            enableButtonsAfterRun(true);
            ui->lineEditSerialNumber->setFocus();
            return;
        }

    }
    m_lastSerialNumber = serialNumber;
    QString serialNumberStr = "ImageBarcode: " + serialNumber;
    logStringBlack(serialNumberStr.toLocal8Bit());
    logStringBlack(" ");

    //
    // Set the timeout values from the ini file
    //
    m_script.setTimeouts(m_timeoutA_ms, m_timeoutB_ms);

    bool scriptPassed = true;
    ui->labelResults->setText(g_stringWorking);
    unsigned int testCount = m_testList.size();
    for (unsigned int i=0; i<testCount; i++)
    {
        if (CAbort::Instance()->abortRequested())
        {
            ui->labelResults->setText(g_stringAborted);
            scriptPassed = false;
            enableButtonsAfterRun(true);
            break;
        }

        QListWidgetItem *item = m_testList[i];
        if (item->checkState() == Qt::Checked)
        {
            char    tmpStr[100];

            //
            // Add the test name
            //
            QString TestNameLine = "TestName: ";
            TestNameLine.append(m_script.getTestName(m_testNumbers[i]));
            logStringBlack(TestNameLine.toLocal8Bit());

            //
            // Add the Date and Time
            //
            time_t rawtime;
            struct tm *t;
            time (&rawtime);
            t = localtime (&rawtime);
            sprintf(tmpStr, "%02d/%02d/%04d", t->tm_mon+1, t->tm_mday, t->tm_year+1900);
            line = "Date: ";
            line.append(tmpStr);
            logStringBlack(line.toLocal8Bit());
            sprintf(tmpStr, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
            line = "Time: ";
            line.append(tmpStr);
            logStringBlack(line.toLocal8Bit());

            //
            // Run the test
            m_script.terminateOnError(terminateOnError);
            m_script.runTest(m_testNumbers[i]);
            if (m_script.sawError())
            {
                if (CAbort::Instance()->abortRequested())
                {
                    logStringRed("Result: FAILED - Aborted by Operator");
                    ui->labelResults->setText(g_stringAborted);
                    scriptPassed = false;
                }
                else
                {
                    logStringRed("Result: FAILED");
                    scriptPassed = false;
                }

            }
            else
            {
                logStringBlack("Result: PASSED");
            }

            //
            // Add test terminator
            //
            logStringBlack("~#~");

            if  (terminateOnError && m_script.sawError())
            {
                break;
            }

        }
    }
    if (scriptPassed)
        ui->labelResults->setText(g_stringPassed);
    else
        ui->labelResults->setText(g_stringFailed);


    if ((CAbort::Instance()->abortRequested()) && (m_indexOnAbort >= 0))
    {
        CAbort::Instance()->clearRequest();
        m_script.runTest(m_indexOnAbort);
        CAbort::Instance()->requestAbort();
    }

    if ((!CAbort::Instance()->abortRequested()) && (m_indexOnExit >= 0))
    {
        m_script.runTest(m_indexOnExit);
    }

    //
    // Write the report file
    //
    if (ui->checkBoxGenerateReport->isChecked())
    {
        generateReport();
    }

    //
    // re-enable the run button
    //
    enableButtonsAfterRun(true);
}



/*!
 * @brief called when the "Open" button is pressed
 *
 * @author J. Peterson
 * @date 06/01/2014
*/
void MainWindow::commPortSelected_A(QString deviceName)
{
    if (!ui->comboBox_serialPorts_A->isEnabled())
        return;

    if (deviceName == NOT_CONNECTED)
        return;

    ui->comboBox_serialPorts_A->setCurrentText(deviceName);

    if ( (m_serialPorts[1]->isOpen()) && (m_serialPorts[1]->portName() == deviceName) )
    {
        m_serialPorts[1]->close();
        ui->comboBox_serialPorts_B->setCurrentIndex(0);
    }

    if (m_serialPorts[0]->isOpen())
    {
        m_serialPorts[0]->close();
    }

    m_serialPorts[0]->setPortName(deviceName);

    if (m_serialPorts[0]->open(QSerialPort::ReadWrite))  //QIODevice::ReadWrite
    {
        m_serialPorts[0]->setBaudRate(QSerialPort::Baud38400);
        m_serialPorts[0]->setDataBits(QSerialPort::Data8);
        m_serialPorts[0]->setParity(QSerialPort::NoParity);
        m_serialPorts[0]->setStopBits(QSerialPort::OneStop);
        m_serialPorts[0]->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPorts[0]->setDataTerminalReady(true);

        m_serialPorts[0]->clearError();
        m_serialPorts[0]->clear();
        m_serialPorts[0]->flush();
    }
    else
    {
        ui->comboBox_serialPorts_A->setCurrentIndex(0);
        displayWarning("Could not open serial port");
        return;
    }

}


void MainWindow::commPortSelected_B(QString deviceName)
{
    if (!ui->comboBox_serialPorts_B->isEnabled())
        return;

    if (deviceName == NOT_CONNECTED)
        return;

    ui->comboBox_serialPorts_B->setCurrentText(deviceName);

    if ( (m_serialPorts[0]->isOpen()) && (m_serialPorts[0]->portName() == deviceName) )
    {
        m_serialPorts[0]->close();
        ui->comboBox_serialPorts_A->setCurrentIndex(0);
    }

    if (m_serialPorts[1]->isOpen())
    {
        m_serialPorts[1]->close();
    }

    m_serialPorts[1]->setPortName(deviceName);

    if (m_serialPorts[1]->open(QSerialPort::ReadWrite))  //QIODevice::ReadWrite
    {
        m_serialPorts[1]->setBaudRate(QSerialPort::Baud38400);
        m_serialPorts[1]->setDataBits(QSerialPort::Data8);
        m_serialPorts[1]->setParity(QSerialPort::NoParity);
        m_serialPorts[1]->setStopBits(QSerialPort::OneStop);
        m_serialPorts[1]->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPorts[1]->setDataTerminalReady(true);

        m_serialPorts[1]->clearError();
        m_serialPorts[1]->clear();
        m_serialPorts[1]->flush();
    }
    else
    {
        ui->comboBox_serialPorts_B->setCurrentIndex(0);
        displayWarning("Could not open serial port");
        return;
    }
}


void MainWindow::logCommand(const char *cmd)
{
    int index;
    QString command = cmd;

    index = command.indexOf("//");
    if (index >= 0)
        command.truncate(index);
    index = command.indexOf("#");
    if (index >= 0)
        command.truncate(index);

    QString comment = cmd;
    comment.remove(0, command.length());

    QColor saveColor = ui->textEditResults->textColor();

    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->insertPlainText (">> ");

    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(QColor( "black" ));
    ui->textEditResults->insertPlainText(command);

    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(QColor( "gray" ));
    ui->textEditResults->insertPlainText(comment);

    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(saveColor);
    ui->textEditResults->insertPlainText ("\r\n");
    qApp->processEvents();    
}


void MainWindow::logReply(const char *reply)
{
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->insertPlainText ("<< ");
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->insertPlainText (reply);
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->insertPlainText ("\r\n");
    qApp->processEvents();
}




/*!
 * @brief writes to command to the serial port
 *
 * @param[in] command - string to be sent to device
 * @return true if write is successful, false otherwise
 *
 * @author J. Peterson
 * @date 06/01/2014
*/
bool MainWindow::sendVapoThermCommand(int portIndex, const char *command)
{
    int index;
    QString cmd = command;

    index = cmd.indexOf("//");
    if (index >= 0)
        cmd.truncate(index);
    index = cmd.indexOf("#");
    if (index >= 0)
        cmd.truncate(index);
    cmd = cmd.trimmed();

    QString comment = command;
    comment.remove(0, cmd.length());


    int bytesWritten = 0;
    int commandLength = cmd.size();

    flushIncomingData(portIndex);

    //
    // Write the command, one character at a time.  The delay between characters
    // appears to be necessary for the instrument to keep up.
    //
    char smallStr[2];
    smallStr[1] = '\0';
    for (int k=0; k<commandLength; k++)
    {
        smallStr[0] = command[k];
        bytesWritten += m_serialPorts[portIndex]->write(smallStr);
        snooze(m_outputDelay_ms);
    }

    //
    // write a terminating CR-LF
    //
    bytesWritten += m_serialPorts[portIndex]->write("\r\n");


    //
    // If the bytes written differs fromm the command length plus the terminator,
    // then there must have been an error.
    //
    if (bytesWritten < commandLength+2)
    {
        return(false);
    }

    return(true);
}




void MainWindow::flushIncomingData(int portIndex)
{
    m_serialPorts[portIndex]->clear();

    m_inputBufferIndex = 0;
    m_inputBufferCount = 0;
}


/*!
 * @brief reads incoming data from the instrument
 *
 * @param[in] buffer - place to put the responce
 * @param[in] bufferSize - size of the buffer
 * @param[in] msTimeout - timeout period between successful reads
 * @return true if read is successful, false otherwise
 *
 * @author J. Peterson
 * @date 06/01/2014
*/
bool MainWindow::readVapoThermResponse(int portIndex, char *buffer, const int bufferSize, const int msTimeout)
{
    int index = 0;
    buffer[0] = '\0';

    int tick = 0;
    while (tick < msTimeout)
    {
        //
        // Read more data if our input buffer is empty
        //
        if (m_inputBufferCount == 0)
        {
            if (!m_serialPorts[portIndex]->isOpen())
            {
                m_inputBufferIndex = 0;
                m_inputBufferCount = 0;
                return(false);
            }

            m_inputBufferCount = m_serialPorts[portIndex]->read(m_inputBuffer, 1024);
            m_inputBufferIndex = 0;

            if (m_inputBufferCount == 0)
            {
                qApp->processEvents();
                snooze(1);
                tick++;
                continue;
            }
        }

        //
        // Return if our output buffer is full
        //
        if (index >= bufferSize)
        {
            return(true);
        }

        //
        // Grab the next character
        //
        buffer[index++] = m_inputBuffer[m_inputBufferIndex];
        buffer[index] = '\0';
        m_inputBufferIndex++;
        m_inputBufferCount--;

        //
        // If the last character was a line terminator then we are done
        // (provided we have at least one character
        //
        if ((buffer[index-1] == '\n') || (buffer[index-1] == '\r'))
        {
            buffer[index-1] = '\0';
            if (index > 1)
            {
                return(true);
            }
            else
            {
                index = 0;
            }
        }

    }

    return(false);
}




void MainWindow::logStringRed(const char *string)
{
    QColor saveColor = ui->textEditResults->textColor();
    qreal saveFontSize = ui->textEditResults->fontPointSize();

    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(QColor( "red" ));
    //ui->textEditResults->setFontPointSize(12.0);
    ui->textEditResults->insertPlainText(string);
    ui->textEditResults->moveCursor (QTextCursor::End);

    ui->textEditResults->setFontPointSize(saveFontSize);
    ui->textEditResults->setTextColor(saveColor);
    ui->textEditResults->insertPlainText ("\r\n");
    qApp->processEvents();

    m_reportStrings.push_back(string);
}


void MainWindow::logStringGray(const char *string)
{
    QColor saveColor = ui->textEditResults->textColor();
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(QColor( "gray" ));
    ui->textEditResults->insertPlainText(string);
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(saveColor);
    ui->textEditResults->insertPlainText ("\r\n");
    qApp->processEvents();
}



void MainWindow::logStringBlack(const char *string)
{
    QColor saveColor = ui->textEditResults->textColor();
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(QColor( "black" ));
    ui->textEditResults->insertPlainText(string);
    ui->textEditResults->moveCursor (QTextCursor::End);
    ui->textEditResults->setTextColor(saveColor);
    ui->textEditResults->insertPlainText ("\r\n");
    qApp->processEvents();

    m_reportStrings.push_back(string);
}




void MainWindow::loadScriptButtonPress()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (filename.isEmpty())
    {
        return;
    }

    m_scriptFileName = filename;
    reloadScriptButtonPress();
}


void MainWindow::reloadScriptButtonPress()
{
    if (!m_script.readScriptFile(m_scriptFileName.toLocal8Bit()))
    {
        return;
    }

    ui->listWidget->clear();
    m_testList.clear();
    m_testNumbers.clear();
    m_indexOnAbort = -1;
    m_indexOnExit = -1;

    int testCount = m_script.getTestCount();
    ui->pushButtonStartTests->setEnabled((testCount > 0));
    for (int i=0; i<testCount; i++)
    {
        QString *pStr = m_script.getTestName(i);
        if (*pStr == "OnAbort")
        {
            m_indexOnAbort = i;
            continue;
        }
        if (*pStr == "OnExit")
        {
            m_indexOnExit = i;
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(*pStr, ui->listWidget);
        m_testList.push_back(item);
        m_testNumbers.push_back(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
     }

    setTitle();
}


void MainWindow::abortButtonPress()
{
    CAbort::Instance()->requestAbort();

}


bool MainWindow::generateReport()
{
    //
    // create the filename
    //
    time_t rawtime;
    struct tm *t;
    time (&rawtime);
    t = localtime (&rawtime);
    char filename[100];
    sprintf(filename, "%s%02d%02d%04d_%02d%02d%02d.txt",
            m_reportDir.toLocal8Bit().data(),
            t->tm_mon+1, t->tm_mday, t->tm_year+1900,
            t->tm_hour, t->tm_min, t->tm_sec);

    //
    // open the file
    //
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        QMessageBox msgBox;
        QString msg = "Could not write report file: ";
        msg.append(filename);
        msgBox.setText(msg);
        msgBox.exec();
        return(false);
    }

    for (unsigned int i=0; i<m_reportStrings.size(); i++)
    {
        fprintf(fp, "%s\n", m_reportStrings[i].toLocal8Bit().data());
    }
    fclose(fp);

    return(false);
}


void MainWindow::clearButtonPressed()
{
    ui->labelResults->setText(g_stringIdle);
    ui->textEditResults->clear();
}



void MainWindow::enableButtonsAfterRun(bool enable)
{
    ui->pushButtonStartTests->setEnabled(enable);
    ui->pushButton_LoadScript->setEnabled(enable);
    ui->pushButton_ReloadScript->setEnabled(enable);
    ui->pushButton_Abort->setEnabled(!enable);
}