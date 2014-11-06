#ifndef ABORT_H
#define ABORT_H

class CAbort
{
public:
    static CAbort *Instance();
    void clearRequest();
    void requestAbort();
    bool abortRequested();

private:
    CAbort();
    static CAbort *m_instance;
    bool m_abort;
};

#endif // ABORT_H
