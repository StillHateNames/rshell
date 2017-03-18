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

BaseCommand* parse(string str); // correctly evaluates parentheses