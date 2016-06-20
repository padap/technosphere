#include <iostream>
#include <vector>
#include <fcntl.h>
#include <set>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

// system: Mac OS
using namespace std;

set <int> setAllProc;

 
void interruptSignalVoid(int sig)
{
    //to finish final 6th test
    signal(SIGINT, interruptSignalVoid);
    auto beg = setAllProc.begin();
    auto end = setAllProc.end();
    while (beg != end)
    {
        kill(*beg, sig);
        beg++;
    }
}


struct comandType
{
    char * commandName = nullptr;
    char ** args = nullptr;
    int  argsCount;
    bool backgroundMode;
    string in = "";
    string out = "";

    comandType(string s, vector <string> v, string sIn, string sOut, bool backgroundMode)
    {
        this->backgroundMode = backgroundMode;
        in = sIn;
        out = sOut;
        argsCount = v.size();

        commandName = (char *)malloc(sizeof(char) * (s.size() + 1));
        memcpy(commandName, s.c_str(), s.size());
        commandName[s.size()] = '\0';
        this->args = (char **)malloc(sizeof(char *) * (v.size() + 1));

        for (int i = 0; i < v.size(); i++)
        {
            args[i] = (char *)malloc(sizeof(char) * (v[i].size() + 1));
            memcpy(args[i], v[i].c_str(), v[i].size());
            args[i][v[i].size()] = '\0';
        }
        this->args[v.size()] = NULL;
    }
    comandType(const comandType & B)
    {
        commandName = (char *)malloc(sizeof(char) * (strlen(B.commandName) + 1));
        memcpy(commandName, B.commandName, strlen(B.commandName));
        commandName[strlen(B.commandName)] = '\0';

        this-> args = (char **)malloc(sizeof(char *) * (B.argsCount + 1));
        memcpy(args, B.args, B.argsCount);
        args[B.argsCount] = nullptr;
        for (int i = 0; i < B.argsCount; i++)
        {
            args[i] = (char *) malloc(sizeof(char) * (strlen(B.args[i]) + 1));
            memcpy(args[i], B.args[i], strlen(B.args[i]));
            args[i][strlen(B.args[i])] = '\0';
        }
        this-> argsCount = B.argsCount;
        this-> in        = B.in;
        this-> out       = B.out;
        this-> backgroundMode = B.backgroundMode;
    } 
    ~comandType()
    {
        free(commandName);
        for (int i = 0; i < argsCount; i++)
            free(args[i]);
        free(args);
    }
};

struct expr{
    vector <comandType> comandTypes;
    vector <string> op;
    bool backgroundMode;
    expr(vector <comandType> c, vector <string> o, bool flag)
    {
        this->backgroundMode = flag;
        this->comandTypes    = c;
        this->op             = o;
    }
};

struct procAttr{
    int PID;
    int st;

    procAttr(int p, int s)
    {
    this->PID = p;
    this->st  = s;
    }
};


comandType parseCommand(string s)
{
    bool backgroundMode = false;

    string commandName = "";
    vector <string> args;
    int pos = 0;
    string str = "";

    while (pos < s.size() && s[pos] == ' ')
        pos++;
    
    while (pos < s.size() && s[pos] != ' ' && s[pos] != '<' && s[pos] != '>')
    {
        commandName += s[pos];
        pos++;
    }

    args.push_back(commandName);

    bool inMode = false;
    bool outMode = false;

    string in = "";
    string out = "";
    str = "";

    while (pos < s.size())
    {
        if (s[pos] == '>')
        {
            if (str.size())
            {
                args.push_back(str);
                str = "";
            }
            outMode = true;
        }
        else if (s[pos] == '<')
        {
            if (str.size())
            {
                args.push_back(str);
                str = "";
            }
            inMode = true;
        }
        else if (s[pos] == ' ' || s[pos] == '\n')
        {
            if (str.size())
            {
                if (inMode)
                {
                    inMode = false;
                    in = str;
                }
                else if (outMode)
                {
                    outMode = false;
                    out = str;
                }
                else 
                    args.push_back(str);
            }
            str = "";
        }
        else
            str += s[pos];
        pos++;
    }

    if (str.size())
    {
        if (inMode)
        {
            inMode = false;
            in = str;
        }
        else if (outMode)
        {
            outMode = false;
            out = str;
        }
        else 
            args.push_back(str);
    }
    if (args[args.size() - 1] == "&")
    {
        backgroundMode = true;
        args.pop_back();
    }

    return comandType(commandName, args, in, out, backgroundMode);
}


expr parseExpr(string s)
{
    int pos = 0;
    vector <comandType> v;
    vector <string> op;
    string str = "";

    while (pos <= s.size()-1)
    {
        if ((s[pos] == '&' && s[pos + 1] == '&') || (s[pos] == '|' && s[pos + 1] == '|'))
        {
            v.push_back(parseCommand(str));
            str = "";
            string ss = "";
            ss += s[pos];
            ss += s[pos + 1];
            op.push_back(ss);
            pos++;
        }
        else if (s[pos] == '|' )
        {
            v.push_back(parseCommand(str));
            str = "";
            string ss = "";
            ss += s[pos];
            op.push_back(ss);
        }
        else
        {
            str += s[pos];
        }
        pos++;
    }

    if (str.size())
        v.push_back(parseCommand(str));
    bool flac = v[v.size() - 1].backgroundMode;
    return expr(v, op, flac);
}


procAttr executeComandType(comandType com, int fdIn, int fdOut, bool isWait, bool isIndex)
{
    int tPID = 0;
    if (!(tPID = fork()))
    {
        if (fdIn != -1)
        {
            dup2(fdIn, 0);
        }
        else if (com.in.size())
        {
            int fdIn = open(com.in.c_str(), O_RDONLY);
            dup2(fdIn, 0);
        }
        if (fdOut != -1)
        {
            dup2(fdOut, 1);
        }
        else if (com.out.size())
        {
            int fdOut = open(com.out.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, S_IREAD | S_IWRITE);
            dup2(fdOut, 1);
        }
        execvp(com.commandName, com.args);  // Here execute command
        return procAttr(0,0);
    }
    if (isWait)
    {
        int st = 0;
        int PID = 0;
        if (isIndex)
            setAllProc.insert(tPID);
        while (waitpid(tPID, &st, 0) >= 0) {}
        if (isIndex)
            setAllProc.erase(tPID);
        cerr << "Process " << tPID <<  " exited: " << WEXITSTATUS(st) << endl; //5th test
        return procAttr(PID, st);
    }
    return procAttr(-1, -1);
}


void executeExpr(expr expr, bool isIndex)
{
    int nextOper = 0;

    int fdPIPE[80] = {0};            // File decatscriptor PIPE
    int posFDPIPE = 0;               // Position of fdPIPE
    int parI = 0;                    // ParallelProcess Counter
    auto info = procAttr(-1, -1);
    if (nextOper < expr.op.size() && expr.op[nextOper] == "|")
    {
        pipe(fdPIPE + posFDPIPE);
        info = executeComandType(expr.comandTypes[0], -1, fdPIPE[posFDPIPE + 1], false, isIndex);
        close(fdPIPE[posFDPIPE + 1]);
        posFDPIPE += 2;
        parI += 1;   
    }
    else
    {
        info = executeComandType(expr.comandTypes[0], -1, -1, true, isIndex);
    }
    for (int i = 0; i < expr.op.size(); i++)
    {
        nextOper++;
        if (expr.op[i] == "&&")
        {
            if (!info.st)
            {
                if (nextOper < expr.op.size() && expr.op[nextOper] == "|")
                {
                    pipe(fdPIPE + posFDPIPE);
                    info = executeComandType(expr.comandTypes[i + 1], -1, fdPIPE[posFDPIPE + 1], false, isIndex);
                    close(fdPIPE[posFDPIPE + 1]);
                    parI++;
                    posFDPIPE += 2;
                }
                else
                {
                    parI = 0;
                    info = executeComandType(expr.comandTypes[i + 1], -1, -1, true, isIndex);
                }
            }
        }
        else if (expr.op[i] == "||")
        {
            if (info.st)
            {
                if (nextOper < expr.op.size() && expr.op[nextOper] == "|")
                {
                    pipe(fdPIPE + posFDPIPE);
                    info = executeComandType(expr.comandTypes[i + 1], -1, fdPIPE[posFDPIPE + 1], false, isIndex);
                    close(fdPIPE[posFDPIPE + 1]);
                    parI++;
                    posFDPIPE += 2;
                }
                else
                {
                    parI = 0;
                    info = executeComandType(expr.comandTypes[i + 1], -1, -1, true, isIndex);
                }
            }
        }
        else if (expr.op[i] == "|")
        {
            if (parI)
            {
                if (nextOper < expr.op.size() && expr.op[nextOper] == "|")
                {
                    pipe(fdPIPE + posFDPIPE);
                    info = executeComandType(expr.comandTypes[i + 1], fdPIPE[posFDPIPE - 2], fdPIPE[posFDPIPE + 1], false, isIndex);
                    close(fdPIPE[posFDPIPE + 1]);
                    close(fdPIPE[posFDPIPE - 2]);
                    parI++;
                    posFDPIPE += 2;
                }
                else
                {
                    info = executeComandType(expr.comandTypes[i + 1], fdPIPE[posFDPIPE - 2], -1, true, isIndex);
                    close(fdPIPE[posFDPIPE - 2]);
                    parI = 0;
                    posFDPIPE += 2;
                }
            }
        }

    }
    for (int i = 0; i < 40; i++)
        if (fdPIPE[i])
            close(fdPIPE[i]);
}

int main(int argc, char * argv[])
{   
    signal(SIGINT, interruptSignalVoid);

    string s;
    while (getline(cin, s))
    {
        if (s.size())
        {
            expr lineExpr = parseExpr(s);
            if (lineExpr.backgroundMode)
            {
                int workPID=0;
                if (!(workPID = fork()))
                {
                    executeExpr(lineExpr, false);
                    return 0;
                }
            }
            else
                executeExpr(lineExpr, true);
        }
    }

   return 0;
}