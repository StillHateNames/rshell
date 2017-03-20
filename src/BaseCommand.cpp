#ifndef BaseCommand_cc
#define BaseCommand_cc
#include <vector>
#include <cstring>
#include <cstdlib>
#include <string>
#include "BaseCommand.hh"

int Executable::exec(int result)
{
  int status, finish;
  string err = arg[0];
  err += " failed";
  pid_t child = fork();
  if(child == -1)
  {
    perror("fork failed");
    return -1;
  }
  if(child == 0)
  {
    if(execvp(this->arg[0], this->arg.data()) == -1)
    {
      perror(err.c_str());
      exit(-1);
    }
  }
  else if(child > 0)
  {
    finish = waitpid(0, &status, 0);
    if(finish == -1)
    {
      perror("waitpid failed");
      return -1;
    }
    if(WIFEXITED(status))
    {
        if(WEXITSTATUS(status) == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
  }
  return 0;
}

int parallel::exec(int result) 
{
  if(result < 1)
  {
    int val = this->left->exec(result);
    if(val)
    {
        if(!this->left->children())
        {
          delete this->left; 
        }
        if(this->right) 
        {
            return this->right->exec(1);
        } 
        else
        {
            return 0;
        }
    } 
    else
    {
        if(!this->left->children())
        {
          delete this->left; 
        }
        if(this->right->children())
        {
          return this->right->exec(-1);
        }
        else
        {
          delete this->right;
        }
        return val;
    }
  }
  else
  {
    delete this->left;
    if(this->right->children())
    {
      return this->right->exec(result);
    }
    delete this->right;
    return result;
  }
}

int series::exec(int result) 
{
  if(result > -1)
  {
    int val = this->left->exec(result); 
    if(!val) 
    {
        if(!this->left->children())
        {
          delete this->left; 
        } 
        if(this->right) 
        {
          return this->right->exec(1);
        }
        else
        {
          return 0;
        }
    }
    else
    {
      if(!this->left->children())
      {
        delete this->left; 
      }
      if(this->right->children())
      {
        return this->right->exec(-1);
      }
      else
      {
        delete this->right; 
      }
      return val;
    }
  }
  else
  {
    delete this->left;
    if(this->right->children())
    {
      return this->right->exec(result);
    }
    delete this->right;
    return 0;
  }
}

int always::exec(int result)
{
    this->left->exec(result); 
    if(!this->left->children())
    {
      delete this->left; 
    }
    if(this->right) 
    {
        return this->right->exec(result);
    } 
    else
    {
        return 0;
    }
}

BaseCommand* readCom(string str) // reads string passed in by user, returns skewed BaseCommand* tree
{
  if((!str.empty()) && (str != ";") && (str != "||") && (str != "&&")) //non empty string returned.
  {
    vector<char*> arg;
    char* c1;
    char* c2;
    string s1 = "";
    string s2 = "";
    BaseCommand* exe;
    unsigned Always = str.find(';');
    unsigned Series = str.find("&&");
    unsigned Parallel = str.find("||");
    unsigned test;
    if((str.find(';') != string::npos) || (str.find("&&") != string::npos) || (str.find("||") != string::npos))
    {
      if(((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
      {
        if(Always != 0)
        {
          s1 = str.substr(0, Always);
          if(s1 == "exit")
          {
            return new Exit();
          }
        }
        test = str.find_first_not_of(' ', Always + 1);
        if(test != string::npos)
        {
            s2 = str.substr(test);
        }
        else
        {
            s2 = str.substr(Always + 1);
        }
      }
      else if(((Series < Always) && (Series < Parallel)) || ((Always == string::npos) && (Parallel == string::npos)))
      {
        if(Series)
        {
          s1 = str.substr(0, Series);
          if(s1 == "exit")
          {
            return new Exit();
          }
        }
        test = str.find_first_not_of(' ', Series + 2);
        if(test != string::npos)
        {
            s2 = str.substr(test);
        }
        else
        {
            s2 = str.substr(Series + 2);
        }
      }
      else if(((Parallel < Series) && (Parallel < Always)) || ((Series == string::npos) && (Always == string::npos)))
      {
        if(Parallel)
        {
          s1 = str.substr(0, Parallel);
          if(s1 == "exit")
          {
            return new Exit();
          }
        }
        test = str.find_first_not_of(' ', Parallel + 2);
        if(test != string::npos)
        {
            s2 = str.substr(test);
        }
        else
        {
            s2 = str.substr(Parallel + 2);
        }
      }
      if(s1 != "")
      {
        c1 = new char[s1.size() + 1];
        strcpy(c1, s1.c_str());
        c2 = strtok(c1, " ");
        while(c2 != NULL)
        {
          arg.push_back(c2);
          c2 = strtok(NULL, " ");
        }
        if(((Always < Series) && (Always < Parallel)) || ((Series == string::npos) && (Parallel == string::npos)))
        {
          exe = new always(new Executable(arg), readCom(s2));
          return exe;
        }
        else if(((Series < Always) && (Series < Parallel)) || ((Parallel == string::npos) && (Always == string::npos)))
        {
          exe = new series(new Executable(arg), readCom(s2));
          return exe;
        }
        else if(((Parallel < Series) && (Parallel < Always)) || ((Series == string::npos) && (Always == string::npos)))
        {
          exe = new parallel((new Executable(arg)), readCom(s2));
          return exe;
        }
      }
      return 0;
    }
    else
    {
      s1 = str;
      if(s1 == "exit")
      {
          return new Exit();
      }
      c1 = new char[s1.size() + 1];
      strcpy(c1, s1.c_str());
      c2 = strtok(c1, " ");
      while(c2 != NULL)
      {
        arg.push_back(c2);
        c2 = strtok(NULL, " ");
      }
      arg.push_back(NULL);
      exe = new Executable(arg);
      return exe;
    }
  }
  else
  {
    return 0;
  }
}
#endif