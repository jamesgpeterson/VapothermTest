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

#include "TestScript.h"

#define VERSION_STRING "1.0.010"

namespace Ui
{
    class MainWindow;
}


class CSettings
{
     QString Script;
     QString Jim;
     QString PortA;
     QString PortB;
     bool    TerminateOnError;

     QString ReportDir;
     int     OutputDelayMS;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void startTestsButtonPress();
    void loadScriptButtonPress();
    void reloadScriptButtonPress();
    void commPortSelected_A(QString deviceName);
    void commPortSelected_B(QString deviceName);
    void abortButtonPress();
    void clearButtonPressed();
    void enableButtonsAfterRun(bool enable);

    void logStringBlack(const char *string);
    void logStringGray(const char *string);
    void logStringRed(const char *string);
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
    bool        m_checkSerialNumber;
};



#endif // MAINWINDOW_H