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
        bool children() { return false;};
        int exec();
};

BaseCommand* parse(string str); // correctly evaluates parentheses
