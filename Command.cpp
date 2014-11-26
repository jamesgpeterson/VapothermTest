
#include <vector>
#include <QStringList>
#include <QMessageBox>
#include "Command.h"

/*!
 * @brief CCommand constructor
 *
 * All member variables are initialized.  No allocation or
 * complex tasks are performed.
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CCommand::CCommand()
{
    m_type = CMD_UNKNOWN;
    m_line.clear();
    m_lineNumber = -1;

    m_argInteger = 0;
    m_stringArg.clear();
    m_argNumber = 0;
    m_charNumber = 0;
    m_expectedChar = '\0';
    m_argMin = 0.0;
    m_argMax = 0.0;
}


/*!
 * @brief CCommand destructor
 *
 * The QStrings that hold the script file line and the string argument
 * are explicitly cleared.
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
CCommand::~CCommand()
{
    m_line.clear();
    m_stringArg.clear();
    switch (m_type)
    {
    case CMD_WAITFOR:
        params_WAITFOR.m_expectedString->clear();
        delete params_WAITFOR.m_expectedString;
        break;
    case CMD_VERSION:
        //params_VERSION.m_scriptVersion->clear();
        //delete params_VERSION.m_scriptVersion;
        break;
    default:
        break;
    }
}


/*!
 * @brief parse the script file line and extract the command parameters
 *
 * The following is a list of the commands:
 *
 *     # <string - comment>                                  - single line comment
 *     test <string - test name>                             - marks the begining of a test section
 *     type <string - test type>                             - test type
 *     desc <string - test description>                      - description of the test
 *     sendline_a <string - command to send to instrument>   - sends a string to the instrument serial interface
 *     sendline_b <string - command to send to instrument>   - sends a string to the instrument serial interface
 *     readline_a                                            - reads a NL terminated line from instrument
 *     readline_b                                            - reads a NL terminated line from instrument
 *     flush_a                                               - flushes all pending input from port a
 *     flush_b                                               - flushes all pending input from port b
 *     expect <int field> <int min> <int max>                - tests the range of the integer field
 *     units <string>                                        - provides a description of the units used by the "expect" command
 *     expect_char <int - field> <int - char number> <char>  - tests if specified  character matches
 *     expect_str <int - field> <string>                     - tests the specified field for the string
 *     sleep <int - ms>                                      - sleeps the specified number of milliseconds
 *     prompt <string - question>                            - asks the user a yes/no question
 *     pause <string - comment>                              - pause till the user resumes
 *     waitfor <a|b> <int-ms> <string>                       - read from specified channel until string is seen or timeout
 *     end_script                                            - terminate the script
 *     end_on_error                                          - terminate on error on previous command
 *
 * @author J. Peterson
 * @date 06/22/2014
*/
void CCommand::parse(const char *line, int lineNumber)
{
    m_lineNumber = lineNumber;
    QString str = line;
    m_line = str.trimmed();

    //
    // comment line
    //
    if (  (m_line.startsWith("#"))
       || (m_line.startsWith("//"))
       || (m_line.length() == 0) )
    {
        m_type = CMD_COMMENT;
        return;
    }

    QStringList args = m_line.split(QRegExp("[ \t]"), QString::SkipEmptyParts);

    //
    // version
    //
    if (args[0] == "scriptVersion")
    {
        m_type = CMD_VERSION;

        m_scriptVersion =  m_line.right(m_line.size()-13).trimmed();
        return;
    }

    //
    // test
    //
    if (args[0] == "test")
    {
        m_type = CMD_TEST;
        m_stringArg = m_line.right(m_line.size()-4);
        m_stringArg = m_stringArg.trimmed();
        return;
    }


#if 0
    //
    // type
    //
    if (args[0] == "type")
    {
        m_type = CMD_TYPE;
        m_stringArg = m_line.right(m_line.size()-4);
        m_stringArg = m_stringArg.trimmed();
        return;
    }
#endif

    //
    // description
    //
    if (args[0] == "desc")
    {
        m_type = CMD_DESC;
        m_stringArg = m_line.right(m_line.size()-4);
        m_stringArg = m_stringArg.trimmed();
        return;
    }

    //
    // sendline_a
    //
    if (args[0] == "sendline_a")
    {
        m_type = CMD_SENDLINE_A;
        m_stringArg = m_line.right(m_line.size()-10);
        m_stringArg = m_stringArg.trimmed();
        return;
    }

    //
    // sendline_b
    //
    if (args[0] == "sendline_b")
    {
        m_type = CMD_SENDLINE_B;
        m_stringArg = m_line.right(m_line.size()-10);
        m_stringArg = m_stringArg.trimmed();
        return;
    }
    //
    // readline_a
    //
    if (args[0] == "readline_a")
    {
        m_type = CMD_READLINE_A;
        return;
    }

    //
    // readline_b
    //
    if (args[0] == "readline_b")
    {
        m_type = CMD_READLINE_B;
        return;
    }

    //
    // flush_a
    //
    if (args[0] == "flush_a")
    {
        m_type = CMD_FLUSH_A;
        return;
    }

    //
    // flush_b
    //
    if (args[0] == "flush_b")
    {
        m_type = CMD_FLUSH_B;
        return;
    }

    //
    // units
    //
    if (args[0] == "units")
    {
        m_type = CMD_UNITS;
        m_stringArg = m_line.right(m_line.size()-5);
        m_stringArg = m_stringArg.trimmed();
        return;
    }

    //
    // expect
    //
    if (args[0] == "expect")
    {
        m_type = CMD_EXPECT;
        bool b1=true, b2=true, b3=true;
        m_argNumber = args[1].toInt(&b1);
        m_argMin = args[2].toDouble(&b1);
        m_argMax = args[3].toDouble(&b1);
        if (!b1 || !b2 || !b3 || (m_argNumber < 1))
        {
            m_type = CMD_UNKNOWN;
        }
        return;
    }

    //
    // expect_char
    //
    if (args[0] == "expect_char")
    {
        m_type = CMD_EXPECT_CHAR;
        bool b1=true, b2=true;
        m_argNumber = args[1].toInt(&b1);  // argument number
        m_charNumber = args[2].toInt(&b2);     //
        m_expectedChar = args[3].toLocal8Bit()[0];
        if (!b1 || !b2 || (m_argNumber < 1))
        {
            m_type = CMD_UNKNOWN;
        }
        return;
    }

    //
    // expect_str
    //
    if (args[0] == "expect_str")
    {
        m_type = CMD_EXPECT_STR;
        bool b1=true;
        m_argNumber = args[1].toInt(&b1);
        m_stringArg = args[2];
        if (!b1 || (m_argNumber < 1))
        {
            m_type = CMD_UNKNOWN;
        }
        return;
    }

    //
    // waitfor
    //
    if (args[0] == "waitfor")
    {
        m_type = CMD_WAITFOR;
        params_WAITFOR.m_channelIndex = -1;
        params_WAITFOR.m_timeoutMS = -1;
        params_WAITFOR.m_expectedString = NULL;
        if (args.size() >= 2)
        {
            if ((args[1] == "a") || (args[1] == "A") || (args[1] == "0"))
            {
                params_WAITFOR.m_channelIndex = 0;
            }
            else if ((args[1] == "b") || (args[1] == "B") || (args[1] == "1"))
            {
                params_WAITFOR.m_channelIndex = 1;
            }
        }
        if (args.size() >= 3)
        {
            params_WAITFOR.m_timeoutMS = args[2].toInt();
        }
        if (args.size() >= 4)
        {
            params_WAITFOR.m_expectedString =  new QString(args[3]);
        }
        if ((params_WAITFOR.m_timeoutMS <= 0) || (params_WAITFOR.m_channelIndex < 0) || (params_WAITFOR.m_expectedString==NULL) )
        {
            char msg[500];
            sprintf(msg, "malformed command on line %d: %s", lineNumber,  line+1);
            QMessageBox msgBox;
            msgBox.setText(msg);
            msgBox.exec();
            m_type = CMD_UNKNOWN;
        }
        return;
    }

    //
    // sleep
    //
    if (args[0] == "sleep")
    {
        m_type = CMD_SLEEP;
        m_argInteger = args[1].toInt();
        return;
    }

    //
    // prompt
    //
    if (args[0] == "prompt")
    {
        m_type = CMD_PROMPT;
        m_stringArg = m_line.right(m_line.size()-6);
        m_stringArg = m_stringArg.trimmed();
        return;
    }

    //
    // pause
    //
    if (args[0] == "pause")
    {
        m_type = CMD_PAUSE;
        m_stringArg = m_line.right(m_line.size()-5);
        m_stringArg = m_stringArg.trimmed();
        return;
    }

    //
    // end_on_error
    //
    if (args[0] == "end_on_error")
    {
        m_type = CMD_END_ON_ERROR;
        return;
    }

    //
    // unknown
    //
    m_type = CMD_UNKNOWN;
}


