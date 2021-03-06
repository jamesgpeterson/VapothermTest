/*!
 * @file mainwindow.h
 * @brief Declares the MainWindow class
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QCloseEvent>
#include <QMainWindow>
#include <QSerialPort>
#include <QColor>
#include <QListWidget>
#include <QString>
#include <QSettings>
#include <QSqlDatabase>
#include "TestScript.h"

#define VERSION_STRING "2.5"

namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void setProgressBarValue(int n);

public slots:
    void startTestsButtonPress();
    void loadScriptButtonPress();
    void commPortSelected_A(QString deviceName);
    void commPortSelected_B(QString deviceName);
    void serialNumberChanged(QString serialNumber);
    void abortButtonPress();
    void enableButtonsAfterRun(bool enable);
    void selectAllTests();
    void clearAllTests();
    void terminateCheckboxClicked(bool checked);
    void validateSerialNumberChecked(bool checked);
    void validateSerialConnectionsChecked(bool checked);


    void logStringBlack(const char *string);
    void logStringGray(const char *string);
    void logStringRed(const char *string);
    void logStringRedToWindow(const char *string);
    void logCommand(const char *cmd);
    void logReply(const char *reply);
    bool sendVapoThermCommand(int portIndex, const char *command);
    bool readVapoThermResponse(int portIndex, char *buffer, const int bufferSize, const int msTimeout);
    bool generateReport();
    void flushIncomingData(int portIndex);

private:
    void snooze(int ms);
    void setTitle();
    void displayWarning(const char *msg);
    bool displayQuestion(const char *msg);
    void displayCommandPrompt();
    void displayReplyPrompt();
    bool connectToDatabase();
    bool serialNumberIsInDB(QString serialNumber);
    bool loadScript(const char *scriptFilename);

private:
    Ui::MainWindow *ui;
    QSerialPort    *m_serialPorts[2];
    CTestScript     m_script;
    std::vector<QListWidgetItem *> m_testList;
    std::vector<int>               m_testNumbers;
    int                            m_indexOnAbort;
    int                            m_indexOnExit;
    int                            m_outputDelay_ms;  // ms delay between output characters
    int                            m_timeoutA_ms;
    int                            m_timeoutB_ms;

    char m_inputBuffer[1024];
    int  m_inputBufferIndex;
    int  m_inputBufferCount;

    QSettings  *m_settings;
    std::vector<QString> m_reportStrings;

    QString     m_scriptFileName;
    QString     m_reportDir;
    QString     m_alternateReportDir;
    QString     m_lastSerialNumber;
    bool        m_checkSerialConnections;
    bool        m_terminateOnFirstError;

    QSqlDatabase m_database;
    bool    m_validateSerial;
    QString m_databaseServer;
    QString m_databaseName;
    QString m_databaseUser;
    QString m_databasePwd;
    QString m_databaseZNum;

    QStringList m_alternatReportFiles;
    QString m_localReportDirectory;
};



#endif // MAINWINDOW_H
