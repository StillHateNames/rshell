#ifndef BaseCommand_hh
#define BaseCommand_hh
#include <iostream>
#include <cstdio>        //perror()
#include <unistd.h>      //fork() and execvp() commands
#include <sys/types.h>   //wait(), waitpid(), and getpid() commands
#include <sys/wait.h>    //wait() and waitpid() commands
#include <vector>


using namespace std;

class BaseCommand
{
    public:
    BaseCommand(){};
    virtual ~BaseCommand() {};
    int virtual exec() = 0; // runs commands and frees up used commands as you go
};

BaseCommand* read(string str);

class Executable : public BaseCommand
{
    private:
    vector<char*> arg;
    public:
    Executable(vector<char*> &arg) {this->arg = arg;};
    int exec();
};

class Exit : public BaseCommand
{
  int exec() {return 1;};
};

class Connector : public BaseCommand
{
    protected:
    BaseCommand* left;
    BaseCommand* right;
};

//Connecter for ||
class parallel : public Connector 
{
  public:
  parallel(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;};
  ~parallel() {delete this->left; delete this->right;};
  int exec();
};

//connecter for &&
class series : public Connector 
{
  public:
  series(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;}
  ~series() {delete this->left; delete this->right;};
  int exec();
};

//Connecter for ;
class always : public Connector 
{ 
  public:
  always(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;}
  ~always() {delete this->left; delete this->right;};
  int exec();
};
//connecter for # comments?
#endif

