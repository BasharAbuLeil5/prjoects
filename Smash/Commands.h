#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

std::string createString(char **arguments);
bool isNumber(const std::string& str);
bool isRedirection(char **arguments,int argNum);
mode_t intToMode(int numMode);
void deleteCmd(char **arguments,int argNum);
std::string createStringWithOutRedirction(char ** arguments ,int argNum);
class Command {

  std::string m_cmdLine;
  

 public:
  Command(const char* cmd_line):m_cmdLine(std::string(cmd_line)){}
  virtual ~Command()=default;
  virtual void execute() = 0;
  // getfunction
  std:: string getCmd()const{ return m_cmdLine;}

  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line):Command(cmd_line){}
  virtual ~BuiltInCommand()=default;
};


class ExternalCommand : public Command {
  bool m_isComplex;
  bool m_isBackgroundComamnd;
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};
//asdasdasd
class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line):Command(cmd_line){}
  virtual ~RedirectionCommand()=default;
  void execute() override;
  bool validateCommand();
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ShowPidCommand()=default; 
  void execute() override;
};

class chprompt: public BuiltInCommand{

  public:
     chprompt(const char* cmd_line):BuiltInCommand(cmd_line){}
     void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  JobsList* m_jobsList;
public:
  QuitCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line),m_jobsList(jobs){}
  virtual ~QuitCommand() {}
  void execute() override;
};




class JobsList {
 public:
  struct JobEntry {
    std::string m_cmdLine;
    int  m_id;
    pid_t m_pid;
    bool m_stopped;
    JobEntry(const std::string& cmdLine, int jobId ,pid_t pid,bool stopped):m_cmdLine(cmdLine),m_id(jobId),m_pid(pid),m_stopped(stopped){}
  };
  std::vector<JobEntry*>m_jobsList;
  int m_maxID;
 public:
  JobsList():m_maxID(0){}
  ~JobsList();
  void addJob(const std::string&cmd,pid_t pid, bool isStopped = false);
  void printJobsList()const;
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  int getSize()const{return m_jobsList.size();}
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId); 
  void removeJobs();
  void printQuit();
  void updateMaxId();
  int getMaxID();
  // TODO: Add extra methods or modify exisitng ones as needed
  int getJIDByPID(int p)const;
  // demo refabdulla
};

class JobsCommand : public BuiltInCommand {
  JobsList* m_jobsList;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),m_jobsList(jobs){}
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  JobsList* m_jobs;
 public:
  KillCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),m_jobs(jobs){}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
  JobsList* m_jobs;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs):BuiltInCommand(cmd_line),m_jobs(jobs){}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
 public:
  ChmodCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ChmodCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  std::string m_smashName;
  std::string m_lastDir;
  JobsList* m_jobs;
  pid_t m_currProcess;
  std::string m_currCmdLine;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  void printSmashJobs();
  ~SmallShell();
  void executeCommand(const char* cmd_line);


  void setCurrProcess(int newPid){m_currProcess=newPid;}

  void setCurrCommand(const std::string cmdLine){m_currCmdLine=cmdLine;}
  void removeJob(int jobId);

  void addJob(pid_t pid ,const std::string& cmdline);

  void end();

  //void terminateFg();
  //void runFg();

  // TODO: add extra methods as needed

  //*************************getters*************************************************
  std::string getName()
  {   
    return m_smashName;
  }
  std::string getLastDir()
  {
    return m_lastDir;
  }
  

JobsList* getJobList(){
  return m_jobs;
}


pid_t getCurrentProcess(){
  return m_currProcess;
}



  //*************************setters********************************
  void updateLast(const std::string& ob1)
  {
    m_lastDir=ob1;
  }
  void setNewName(const std::string& ob="smash")// what is this ??? we have to check
  {
    m_smashName=ob;
  }


  

};

#endif //SMASH_COMMAND_H_