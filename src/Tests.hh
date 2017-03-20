#include "BaseCommand.hh"
#include <string>

using namespace std;
class test : public BaseCommand //runs test
{
    private:
        string str;
        char type;
    public:
        test(char type, string str);
        ~test() {};
        bool children() {return false;};
        int exec(int result);
};

class group : public BaseCommand
{
    private:
        BaseCommand* base;
    public:
        group(BaseCommand* base) {this->base = base;};
        ~group() {delete base;};
        bool children() {return true;};
        int exec(int result);
};

class output : public BaseCommand
{
    private:
        string str;
        BaseCommand* left;
    public:
        output(BaseCommand* left, string str) {this->left = left; this->str = str;};
        ~output() {};
        bool children() {return false;};
        int exec(int result);
};

class append : public BaseCommand
{
    private:
        string str;
        BaseCommand* left;
    public:
        append(BaseCommand* left, string str) {this->left = left; this->str = str;};
        ~append() {};
        bool children() {return false;};
        int exec(int result);
};

class input : public BaseCommand
{
    private:
        BaseCommand* left;
        string str;
    public:
        input(BaseCommand* left, string str) {this->left = left; this->str = str;};
        ~input() {};
        bool children() {return false;};
        int exec(int result);
};

class piping : public BaseCommand
{
    private:
        BaseCommand* left;
        BaseCommand* right;
    public:
        piping(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;};
        ~piping() {delete left; delete right;};
        bool children() {return true;};
        int exec(int result);
};

BaseCommand* parse(string str); // correctly evaluates parentheses