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
    bool virtual children() {return false;};
    int virtual exec(int result) = 0; // runs commands and frees up used commands as you go
};

BaseCommand* readCom(string str);

class Executable : public BaseCommand
{
    private:
    vector<char*> arg;
    public:
    bool children() {return false;};
    Executable(vector<char*> &arg) {this->arg = arg;};
    ~Executable() {};
    int exec(int result);
};

class Exit : public BaseCommand
{
  int exec(int result) {return 1;};
  ~Exit(){};
};

class Connector : public BaseCommand
{
    protected:
    BaseCommand* left;
    BaseCommand* right;
    public:
    bool children() {return true;};
};

//Connecter for ||
class parallel : public Connector 
{
  public:
  parallel(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;};
  ~parallel() {delete this->left; delete this->right;};
  int exec(int result);
};

//connecter for &&
class series : public Connector 
{
  public:
  series(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;}
  ~series() {delete this->left; delete this->right;};
  int exec(int result);
};

//Connecter for ;
class always : public Connector 
{ 
  public:
  always(BaseCommand* left, BaseCommand* right) {this->left = left; this->right = right;}
  ~always() {delete this->left; delete this->right;};
  int exec(int result);
};
//connecter for # comments?
#endif

