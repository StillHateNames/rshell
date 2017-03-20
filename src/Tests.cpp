#include "Tests.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <climits>
#include <fcntl.h>
#include <sys/wait.h>
#include <fstream>
#include <cstdlib>

using namespace std;

test::test(char type, string str)
{
    this->type = type;
    this->str = str;
}

int test::exec(int result)
{
    struct stat status;
    int finish;
    finish = stat(this->str.c_str(), &status);
    if(finish == -1)
    {
        int data = errno;
        if(data == ENOENT)
        {
            cout << "(False)\n";
            return -1;
        }
        perror("stat failed");
        return -1;
    }
    finish = status.st_mode;
    if(this->type == 'e')
    {
        if(S_ISREG(finish) || S_ISDIR(finish))
        {
            cout << "(True)\n";
            return 0;
        }
        cout << "(False)\n";
        return -1;
    }
    if(this->type == 'd')
    {
        if(S_ISDIR(finish))
        {
            cout << "(True)\n";
            return 0;
        }
        cout << "(False)\n";
        return -1;
    }
    if(this->type == 'f')
    {
        if(S_ISREG(finish))
        {
            cout << "(True)\n";
            return 0;
        }
        cout << "(False)\n";
        return -1;
    }
    return 0;
}

unsigned find(string str, string look)
{
    if(!str.empty() && !look.empty())
    {
        for(unsigned i = 0; i < str.length() - look.length() + 1; i++)
        {
            if(str.at(i) == '(')
            {
                int count = 1;
                bool end = 0;
                for(i = i + 1; i < str.length(); i++)
                {
                    if(str.at(i) == '(')
                    {
                        count++;
                    }
                    else if(str.at(i) == ')')
                    {
                        count--;
                    }
                    else if(count == 0)
                    {
                        end = 1;
                        i++;
                        break;
                    }
                }
                if(!end)
                {
                    return UINT_MAX;
                }
            }
            if(str.at(i) == look.at(0))
            {
                bool found = 1;
                for(unsigned j = 1; j < look.length(); j++)
                {
                    if(str.at(i + j) != look.at(j))
                    {
                        found = 0;
                        break;
                    }
                }
                if(found)
                {
                    return i;
                }
            }
        }
        return UINT_MAX;
    }
    return UINT_MAX;
}

int group::exec(int result)
{
    if(result > -1)
    {
        return base->exec(0);
    }
    else
    {
        delete base;
        return result;
    }
}

BaseCommand* parse(string str)
{
    if((!str.empty()) && (str != ";") && (str != "||") && (str != "&&"))
    {
        unsigned Always = find(str, ";");
        unsigned Series = find(str, "&&");
        unsigned Parallel = find(str, "||");
        unsigned Pipe = find(str, "|");
        string s1, s2;
        int branch = 0;
        BaseCommand *left, *right;
        if((find(str, "||") != UINT_MAX) || (find(str, "&&") != UINT_MAX) || (find(str, ";") != UINT_MAX) || (find(str, "|") != UINT_MAX))
        {
            unsigned space, end;
            if(((Always < Series) && (Always < Parallel) && (Always < Pipe))) 
            {
                space = str.find_first_not_of(' ');
                end = str.find_last_not_of(' ', Always - 1);
                s1 = str.substr(space, end - space + 1);
                space = str.find_first_not_of(' ', Always + 1);
                s2 = str.substr(space);
                branch = 1;
            }
            if(((Always > Series) && (Series < Parallel) && (Series < Pipe)))
            {
                space = str.find_first_not_of(' ');
                end = str.find_last_not_of(' ', Series - 1);
                s1 = str.substr(space, end - space + 1);
                space = str.find_first_not_of(' ', Series + 2);
                s2 = str.substr(space);
                branch = 2;
            }
            if(((Parallel < Series) && (Always > Parallel) && (Parallel < Pipe)))
            {
                space = str.find_first_not_of(' ');
                end = str.find_last_not_of(' ', Parallel - 1);
                s1 = str.substr(space, end - space + 1);
                space = str.find_first_not_of(' ', Parallel + 2);
                s2 = str.substr(space);
                branch = 3;
            }
            if((Pipe < Always) && (Pipe < Series) && (Pipe < Parallel))
            {
                space = str.find_first_not_of(' ');
                end = str.find_last_not_of(' ', Pipe - 1);
                s1 = str.substr(space, end - space + 1);
                space = str.find_first_not_of(' ', Pipe + 1);
                s2 = str.substr(space);
                branch = 4;
            }
        }
        else
        {
            s1 = str;
        }
        if(s1.at(0) == '(')
        {
            int count = 1;
            unsigned i;
            for(i = 1; i < s1.length(); i++)
            {
                if(s1.at(i) == '(')
                {
                    count++;
                }
                if(s1.at(i) == ')')
                {
                    count--;
                }
                if(!count)
                {
                    break;
                }
            }
            s1 = s1.substr(1, i - 1);
            left = new group(parse(s1));
            if(count)
            {
                cout << "ERROR: invalid command.\n";
                return 0;
            }
        }
        else if((s1.at(0) == '[') || (s1.find("test") == 0))
        {
            if(s1.at(0) == '[')
            {
                if(s1.find(']') != string::npos)
                {
                    s1 = s1.substr(1, s1.find(']') - 1);
                }
                else
                {
                    cout << "ERROR: invalid command.\n";
                    return 0;
                }
                if(s1.find('-') != string::npos)
                {
                    unsigned start;
                    if(s1.at(s1.find('-') + 1) == 'd')
                    {
                        start = s1.find_first_not_of(' ', s1.find(' ', s1.find('-')));
                        left = new test('d', s1.substr(start, s1.find_last_not_of(' ') - start + 1));
                    }
                    else if(s1.at(s1.find('-') + 1) == 'f')
                    {
                        start = s1.find_first_not_of(' ', s1.find(' ', s1.find('-')));
                        left = new test('f', s1.substr(start, s1.find_last_not_of(' ') - start + 1));
                    }
                    else
                    {
                        start = s1.find_first_not_of(' ', s1.find(' ', s1.find('-')));
                        left = new test('e', s1.substr(start, s1.find_last_not_of(' ') - start + 1));
                    }
                }
                else
                {
                    left = new test('e', s1.substr(s1.find_first_not_of(' '), s1.find_last_not_of(' ') - s1.find_first_not_of(' ') + 1));
                }
            }
            else
            {
                s1 = s1.substr(4);
                if(s1.find('-') != string::npos)
                {
                    if(s1.at(s1.find('-') + 1) == 'd')
                    {
                        left = new test('d', s1.substr(s1.find(' ', s1.find('-')) + 1, s1.find_last_not_of(' ') - s1.find(' ', s1.find('-')) + 2));
                    }
                    else if(s1.at(s1.find('-') + 1) == 'f')
                    {
                        left = new test('f', s1.substr(s1.find(' ', s1.find('-')) + 1, s1.find_last_not_of(' ') - s1.find(' ', s1.find('-')) + 2));
                    }
                    else
                    {
                        left = new test('e', s1.substr(s1.find(' ', s1.find('-')) + 1, s1.find_last_not_of(' ') - s1.find(' ', s1.find('-')) + 2));
                    }
                }
                else
                {
                    left = new test('e', s1.substr(s1.find_first_not_of(' '), s1.find_last_not_of(' ') - s1.find_first_not_of(' ') + 1));
                }
            }
        }
        else if((find(s1, "<") != UINT_MAX) || (find(s1, ">") != UINT_MAX) || (find(s1, ">>") != UINT_MAX))
        {
            unsigned Input = find(s1, "<");
            unsigned Append = find(s1, ">>");
            unsigned Output = find(s1, ">");
            unsigned begin, end;
            string temp;
            if(Input != UINT_MAX)
            {
                begin = s1.find_first_not_of(' ');
                end = s1.find_last_not_of(' ', Input - 1);
                temp = s1.substr(begin, end - begin + 1);
                left = readCom(temp);
                begin = s1.find_first_not_of(' ', Input + 1);
                end = s1.find_last_not_of(' ');
                temp = s1.substr(begin, end - begin + 1);
                left = new input(left, temp);
            }
            else if(Append != UINT_MAX)
            {
                begin = s1.find_first_not_of(' ');
                end = s1.find_last_not_of(' ', Append - 1);
                temp = s1.substr(begin, end - begin + 1);
                left = readCom(temp);
                begin = s1.find_first_not_of(' ', Append + 2);
                end = s1.find_last_not_of(' ');
                temp = s1.substr(begin, end - begin + 1);
                left = new append(left, temp);
            }
            else
            {
                begin = s1.find_first_not_of(' ');
                end = s1.find_last_not_of(' ', Output - 1);
                temp = s1.substr(begin, end - begin + 1);
                left = readCom(temp);
                begin = s1.find_first_not_of(' ', Output + 1);
                end = s1.find_last_not_of(' ');
                temp = s1.substr(begin, end - begin + 1);
                left = new output(left, temp);
            }
        } 
        else
        {
            left = readCom(s1);
        }
        if(branch)
        {
            right = parse(s2);
            if(branch == 1)
            {
                return new always(left, right);
            }
            else if(branch == 2)
            {
                return new series(left, right);
            }
            else if(branch == 3)
            {
                return new parallel(left, right);
            }
            else if(branch == 4)
            {
                return new piping(left, right);
            } 
        }
        else
        {
            return left;
        }
        return 0;
    }
    else
    {
        return 0;
    }
}

int output::exec(int result)
{
    int p[2];
    if(pipe(p) == -1)
    {
        perror("pipe failed");
        return -1;
    }
    pid_t child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(1);
        close(p[0]);
        if(dup(p[1]) == -1)
        {
            perror("dup failed");
            exit(-1);
        }
        int val = this->left->exec(0);
        close(p[1]);
        exit(val);
    }
    child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(0);
        close(p[1]);
        dup(p[0]);
        int file = open(str.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00700);
        if(file == -1)
        {
            perror("open failed");
            exit(-1);
        }
        char buf;
        while(read(p[0], &buf, 1))
        {
            write(file, &buf, 1);
        }
        close(p[0]);
        exit(0);
    }
    close(p[0]);
    close(p[1]);
    int status;
    int finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int piping::exec(int result)
{
    int p[2];
    if(pipe(p) == -1)
    {
        perror("pipe failed");
        return -1;
    }
    pid_t child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(1);
        close(p[0]);
        if(dup(p[1]) == -1)
        {
            perror("dup failed");
            exit(-1);
        }
        int val = this->left->exec(0);
        close(p[1]);
        exit(val);
        
    }
    child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(0);
        close(p[1]);
        if(dup(p[0]) == -1)
        {
            perror("dup failed");
            exit(-1);
        }
        int val = this->right->exec(0);
        close(p[0]);
        exit(val);
    }
    close(p[0]);
    close(p[1]);
    int status;
    int finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int append::exec(int result)
{
    int p[2];
    if(pipe(p) == -1)
    {
        perror("pipe failed");
        return -1;
    }
    pid_t child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(1);
        close(p[0]);
        if(dup(p[1]) == -1)
        {
            perror("dup failed");
            exit(-1);
        }
        close(p[1]);
        exit(this->left->exec(0));
        
    }
    child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(0);
        close(p[1]);
        dup(p[0]);
        int file = open(str.c_str(), O_WRONLY | O_CREAT | O_APPEND, 00700);
        if(file == -1)
        {
            perror("open failed");
            exit(-1);
        }
        char buf;
        while(read(p[0], &buf, 1))
        {
            write(file, &buf, 1);
        }
        close(p[0]);
        exit(0);
    }
    close(p[0]);
    close(p[1]);
    int status;
    int finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int input::exec(int result)
{
    int p[2];
    if(pipe(p) == -1)
    {
        perror("pipe failed");
        return -1;
    }
    pid_t child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(1);
        close(p[0]);
        dup(p[1]);
        int file = open(str.c_str(), O_RDONLY);
        if(file == -1)
        {
            perror("open failed");
            exit(-1);
        }
        char buf;
        while(read(file, &buf, 1))
        {
            write(p[1], &buf, 1);
        }
        close(p[1]);
        exit(0);
    }
    child = fork();
    if(child == -1)
    {
        perror("fork failed");
        return -1;
    }
    if(!child)
    {
        close(0);
        close(p[1]);
        if(dup(p[0]) == -1)
        {
            perror("dup failed");
            exit(-1);
        }
        int val = this->left->exec(0);
        close(p[0]);
        exit(val);
    }
    close(p[0]);
    close(p[1]);
    int status;
    int finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == -1)
        {
            return -1;
        }
    }
    return 0;
}
