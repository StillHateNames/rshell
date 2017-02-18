#include "BaseCommand.hh"
#include <string>  //for getline


using namespace std;
//Summary
//Will take in a command line. Input will be recieved until a set quit command is typed
//Input will be separted by the separators ||, &&, and ;
// separated input will then be run through the proper execute functions

//huh execute left to right......with the return being void, meaning the third execute function will be done with 

int main ()
{
  BaseCommand* command; // start of command "tree"
  string line;
  int val = 0;
  bool valid = 1;
  while(valid)
  {
    cout << "$ ";
    getline(cin, line);
    if(line != "")
    {
        if(line.find('#') != string::npos)
        {
          line.erase(line.find('#'));
        }
        if(line.find_first_not_of(' ') != string::npos)
        {
          line = line.substr(line.find_first_not_of(' '));
        }
    }
    if(line != "")
    {
    command = read(line); // see BaseCommand.hh
    }
    //code for separating the string
    if(command)
    {
      val = command->exec();
    }
    // executes command tree returns 1 if exit is called 
    if(val == 1)
    {
      break;
    }
  
  }
}