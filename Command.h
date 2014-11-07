#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

//
// Simple Script Command
//
class CCommand
{
public:
    enum commandType_t
    {
        CMD_UNKNOWN,     // treated as a comment
        CMD_VERSION,     // version of the script file
        CMD_COMMENT,     // any line starting with either "#" or "//"
        CMD_TEST,        // test <string>
        CMD_TYPE,        // type <string>
        CMD_DESC,        // desc <string>
        CMD_PROMPT,      // prompt <string>
        CMD_PAUSE,       // pause <string>
        CMD_SLEEP,       // sleep <integer ms>
        CMD_SENDLINE_A,  // sendline_a <string>
        CMD_SENDLINE_B,  // sendline_b <string>
        CMD_READLINE_A,  // readline_a
        CMD_READLINE_B,  // readline_b
        CMD_FLUSH_A,     // flush pending input from port A
        CMD_FLUSH_B,     // flush pending input from port B
        CMD_EXPECT,      // expect <integer field> <integer min> <integer max>
        CMD_EXPECT_CHAR, // expect_char <integer field> <integer char number> <char>
        CMD_EXPECT_STR,  // expect_str <integer field> <string - pattern to match>
        CMD_WAITFOR,     // wait for specified string
        CMD_END_SCRIPT,
        CMD_END_ON_ERROR
    };

    union
    {
        struct
        {
            int      m_channelIndex;
            int      m_timeoutMS;
            QString *m_expectedString;
        } params_WAITFOR;

    };


    CCommand();
    ~CCommand();
    void parse(const char *line, int lineNumber);

public:
    commandType_t  m_type;
    QString        m_line;
    int            m_lineNumber;

    int            m_argInteger;
    int            m_argInteger2;
    QString        m_stringArg;
    int            m_argNumber;
    int            m_charNumber;
    char           m_expectedChar;
    double         m_argMin;
    double         m_argMax;
    double         m_test_hidden;
    QString        m_scriptVersion;
};



#endif // COMMAND_H
