#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QCloseEvent>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QColor>
#include <QListWidget>
#include <QString>
#include <QSettings>
#include <QtSql/QSqlDatabase>


#include "TestScript.h"

#define VERSION_STRING "1.1.003"

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
    void reloadScriptButtonPress();
    void commPortSelected_A(QString deviceName);
    void commPortSelected_B(QString deviceName);
    void serialNumberChanged(QString serialNumber);
    void abortButtonPress();
    void enableButtonsAfterRun(bool enable);
    void selectAllTests();
    void clearAllTests();
    void terminateCheckboxClicked();


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
};



#endif // MAINWINDOW_H
