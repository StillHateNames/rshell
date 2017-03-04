#include "Tests.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

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

BaseCommand* parse(string str)
{
    if(str != "") // not empty string
    {
        if((str.find('(') != string::npos) && (str.find('(') == 0)) // nothing before parentheses
        {
            int dCount = 0; // depth of parentheses
            unsigned i;
            for(i = 0; i < str.size(); i++)
            {
                if(str.at(i) == '(')
                {
                    dCount++;
                }
                if(str.at(i) == ')')
                {
                    dCount--;
                }
                if(dCount == 0)
                {
                    break;
                }
            }
            if(dCount != 0) // incorrect amount of parentheses
            {
                cout << "ERROR: invalid command.\n";
                return 0;
            }
            string s1 = str.substr(str.find('(') + 1, i - 1); // what's in parentheses
            string s2 = str.substr(i + 1); // what's after parentheses
            if(s2 != "") // something after parentheses
            {
                if((s2.find("||") != string::npos) || (s2.find("&&") != string::npos) || (s2.find(';') != string::npos))
                {
                    unsigned Always = s2.find(';');
                    unsigned Series = s2.find("&&");
                    unsigned Parallel = s2.find("||");
                    if((((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos))) && (Always < s2.find('(')))
                    {
                        s2 = s2.substr(s2.find_first_not_of(' ', s2.find(';') + 1));
                        return new always(parse(s1), parse(s2));
                    }
                    else if((((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos))) && (Series < s2.find('(')))
                    {
                        s2 = s2.substr(s2.find_first_not_of(' ', s2.find("&&") + 2));
                        cout << "s2 is now: " << s2 << endl;
                        return new series(parse(s1), parse(s2));
                    }
                    else if((((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Always == string::npos)))  && (Parallel < s2.find('(')))
                    {
                        s2 = s2.substr(s2.find_first_not_of(' ', s2.find("||") + 2));
                        return new parallel(parse(s1), parse(s2));
                    }
                    else
                    {
                        cout << "ERROR: invalid command.\n";
                        return 0;
                    }
                }
                else
                {
                        cout << "ERROR: invalid command.\n";
                        return 0;
                }
            }
            else
            {
                return parse(s1);
            }
        }
        else if(str.find('(') != string::npos)
        {
            if((str.rfind("||", str.find('(')) != string::npos) || (str.rfind("&&", str.find('(')) != string::npos) || (str.rfind(';', str.find('(')) != string::npos))
            {
                unsigned Always = str.rfind(';', str.find('('));
                unsigned Series = str.rfind("&&", str.find('('));
                unsigned Parallel = str.rfind("||", str.find('('));
                if(((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                {
                    string s1 = str.substr(0, Always);
                    string s2 = str.substr(str.find_first_not_of(' ', Always + 1));
                    return new always(parse(s1), parse(s2));
                }
                else if(((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos)))
                {
                    string s1 = str.substr(0, Series);
                    string s2 = str.substr(str.find_first_not_of(' ', Series + 1));
                    return new series(parse(s1), parse(s2));
                }
                else if(((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                {
                    string s1 = str.substr(0, Parallel);
                    string s2 = str.substr(str.find_first_not_of(' ', Parallel + 1));
                    return new parallel(parse(s1), parse(s2));
                }
                return 0;
            }
            else
            {
                cout << "ERROR: invalid command.\n";
                return 0;
            }
        }
        else
        {
            if((str.find("[ ") != string::npos))
            {
                if(str.at(0) == '[')
                {
                    if(str.find(']') != string::npos)
                    {
                        string s1 = str.substr(1, str.find(']') - 2);
                        test* Test;
                        if(s1.find('-') != string::npos)
                        {
                            if(s1.at(s1.find('-') + 1) == 'd')
                            {
                                unsigned first = s1.find(' ', s1.find('-') + 1);
                                unsigned last = s1.find(" ;|&", first + 1);
                                Test = new test('d', s1.substr(first + 1, last - first - 1));
                            }
                            else if(s1.at(s1.find('-') + 1) == 'f')
                            {
                                unsigned first = s1.find(' ', s1.find('-') + 1);
                                unsigned last = s1.find(" ;|&", first + 1);
                                Test = new test('f', s1.substr(first + 1, last - first - 1));
                            }
                            else
                            {
                                unsigned first = s1.find(' ', s1.find('-') + 1);
                                unsigned last = s1.find(" ;|&", first + 1);
                                Test = new test('e', s1.substr(first + 1, last  - first - 1));
                            }
                        }
                        else
                        {
                            unsigned first = s1.find_first_not_of(' ');
                            Test = new test('e', s1.substr(first));
                        }
                        if((str.find("||") != string::npos) || (str.find("&&") != string::npos) || (str.find(';') != string::npos))
                        {
                            unsigned Always = str.find(';');
                            unsigned Series = str.find("&&");
                            unsigned Parallel = str.find("||");
                            string s2;
                            if((((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos))))
                            {
                                s2 = s2.substr(s2.find(';') + 1);
                                return new always(Test, parse(s2));
                            }
                            else if((((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos))))
                            {
                                s2 = s2.substr(s2.find("&&") + 2);
                                return new series(Test, parse(s2));
                            }
                            else if((((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Always == string::npos))))
                            {
                                s2 = s2.substr(s2.find("||") + 2);
                                return new parallel(Test, parse(s2));
                            }
                            else
                            {
                                cout << "ERROR: invalid command.\n";
                                return 0;
                            }
                        }
                        else
                        {
                            return Test;
                        }
                    }
                    else
                    {
                        cout << "ERROR: invalid command.\n";
                        return 0;
                    }
                }
                else
                {
                    if((str.rfind("||", str.find('[')) != string::npos) || (str.rfind("&&", str.find('[')) != string::npos) || (str.rfind(';', str.find('[')) != string::npos))
                    {
                        unsigned Always = str.rfind(';', str.find('['));
                        unsigned Series = str.rfind("&&", str.find('['));
                        unsigned Parallel = str.rfind("||", str.find('['));
                        if(((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                        {
                            string s1 = str.substr(0, Always);
                            string s2 = str.substr(str.find_first_not_of(' ', Always + 1));
                            return new always(read(s1), parse(s2));
                        }
                        else if(((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos)))
                        {
                            string s1 = str.substr(0, Series);
                            string s2 = str.substr(str.find_first_not_of(' ', Series + 1));
                            return new series(read(s1), parse(s2));
                        }
                        else if(((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                        {
                            string s1 = str.substr(0, Parallel);
                            string s2 = str.substr(str.find_first_not_of(' ', Parallel + 1));
                            return new parallel(read(s1), parse(s2));
                        }
                        return 0;
                    }
                }
            }
            else if(str.find("test ") != string::npos)
            {
                unsigned testCheck = str.find("test ");
                if(testCheck == 0)
                {
                    test* Test;
                    if(str.find('-') != string::npos)
                    {
                        if(str.at(str.find('-') + 1) == 'd')
                        {
                            unsigned first = str.find(" ", str.find('-') + 1);
                            unsigned last = str.find_first_not_of(" ;|&", first + 1);
                            if(str.find(" ;|&", first + 1) != string::npos)
                            {
                                Test = new test('d', str.substr(first + 1, last - first - 1));
                            }
                            else
                            {
                                Test = new test('d', str.substr(first + 1));
                            };
                        }
                        else if(str.at(str.find('-') + 1) == 'f')
                        {
                            unsigned first = str.find(" ", str.find('-') + 1);
                            unsigned last = str.find(" ;|&", first + 1);
                            if(str.find(" ;|&", first + 1) != string::npos)
                            {
                                Test = new test('f', str.substr(first + 1, last - first - 1));
                            }
                            else
                            {
                                Test = new test('f', str.substr(first + 1));
                            }
                        }
                        else
                        {
                            unsigned first = str.find(" ", str.find('-') + 1);
                            unsigned last = str.find(" ;|&", first + 1);
                            cout << str.substr(first + 1) << endl;
                            if(str.find(" ;|&", first + 1) != string::npos)
                            {
                                Test = new test('e', str.substr(first + 1, last - first - 1));
                            }
                            else
                            {
                                Test = new test('e', str.substr(first + 1));
                            }
                        }
                    }
                    else
                    {
                        unsigned first = str.find(' ');
                        unsigned last = str.find(" ;|&", first + 1);
                        Test = new test('e', str.substr(first + 1, last - first - 1));
                    }
                    if((str.find("||") != string::npos) || (str.find("&&") != string::npos) || (str.find(';') != string::npos))
                    {
                        unsigned Always = str.find(';');
                        unsigned Series = str.find("&&");
                        unsigned Parallel = str.find("||");
                        string s2;
                        if((((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos))))
                        {
                            s2 = s2.substr(s2.find(';') + 1);
                            return new always(Test, parse(s2));
                        }
                        else if((((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos))))
                        {
                            s2 = s2.substr(s2.find("&&") + 2);
                            return new series(Test, parse(s2));
                        }
                        else if((((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Always == string::npos))))
                        {
                            s2 = s2.substr(s2.find("||") + 2);
                            return new parallel(Test, parse(s2));
                        }
                        else
                        {
                            cout << "ERROR: invalid command.\n";
                            return 0;
                        }
                    }
                    else
                    {
                        return Test;
                    }
                }
                else
                {
                    bool good = 0;
                    if((str.at(testCheck - 1) == ';') || (str.at(testCheck - 1) == ' '))
                    {
                        good = 1;
                    }
                    else if(testCheck > 1)
                    {
                        if(((str.at(testCheck - 1) == '&') || (str.at(testCheck - 2) == '&')) || ((str.at(testCheck - 1) == '|') || (str.at(testCheck - 2) == '|')))
                        {
                            good = 1;
                        }
                    }
                    if(good)
                    {
                        if((str.rfind("||", testCheck) != string::npos) || (str.rfind("&&", testCheck) != string::npos) || (str.rfind(';', testCheck) != string::npos))
                        {
                            unsigned Always = str.rfind(';', testCheck);
                            unsigned Series = str.rfind("&&", testCheck);
                            unsigned Parallel = str.rfind("||", testCheck);
                            if(((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                            {
                                string s1 = str.substr(0, Always);
                                string s2 = str.substr(str.find_first_not_of(' ', Always + 1));
                                return new always(read(s1), parse(s2));
                            }
                            else if(((Always > Series) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos)))
                            {
                                string s1 = str.substr(0, Series);
                                string s2 = str.substr(str.find_first_not_of(' ', Series + 1));
                                return new series(read(s1), parse(s2));
                            }
                            else if(((Parallel < Series) && (Always > Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
                            {
                                string s1 = str.substr(0, Parallel);
                                string s2 = str.substr(str.find_first_not_of(' ', Parallel + 1));
                                return new parallel(read(s1), parse(s2));
                            }
                            return 0;
                        }
                    }
                }
            }
            return read(str);
        }
    }
    else
    {
        return 0;
    }
}