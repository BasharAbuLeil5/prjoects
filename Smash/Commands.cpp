#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/fcntl.h>  
#include <bitset>
#include<stack>
#include <sys/stat.h>
using namespace std;


const std::string WHITESPACE = " \n\r\t\f\v";
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell():m_smashName("smash"),m_lastDir(""),m_jobs(new JobsList()),m_currProcess(-1),m_currCmdLine(""){
// TODO: add your implementation
}

SmallShell::~SmallShell() {
  delete m_jobs;
}



JobsList::~JobsList(){
  for (JobEntry* job:m_jobsList){
    delete job;
  }
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
  
  
	// For example:

  string cmd_s = _trim(string(cmd_line));
  string commandName = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  char *arguments[COMMAND_MAX_ARGS];
  int argNum=_parseCommandLine(cmd_line,arguments);
  if(isRedirection(arguments,argNum)){
    deleteCmd(arguments,argNum);
    return new RedirectionCommand(cmd_line);
  }
  deleteCmd(arguments,argNum);
  SmallShell& smash=SmallShell::getInstance();
  if (commandName.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (commandName.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if(commandName.compare("chmod")==0){
    return new ChmodCommand(cmd_line);
  }
  else if(commandName.compare("kill")==0){
    return new KillCommand(cmd_line,smash.getJobList());
  }
  else if(commandName.compare("quit")==0){
    return new QuitCommand(cmd_line,smash.getJobList());
  }
  else if(commandName.compare("fg")==0){
    return new ForegroundCommand(cmd_line,smash.getJobList());
  }
  else if(commandName.compare("jobs")==0){
    return new JobsCommand(cmd_line,smash.getJobList());
  }
  else if(commandName.compare("cd")==0){
    return new ChangeDirCommand(cmd_line);
  }
  else if(commandName.compare("chprompt")==0){
    return new chprompt(cmd_line);
  }
  return new ExternalCommand(cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line) {
  ///////
  m_jobs->removeFinishedJobs();
  if(cmd_line||*cmd_line!='\0'){
    Command* com=CreateCommand(cmd_line);
    if (com){
      com->execute();
    }
    delete com;
  }
}



// builtIn Function




//*************************ChangeDirCommand**********************
void ChangeDirCommand::execute()
{
   std::string m_cmd = getCmd();
   char* m_args[COMMAND_MAX_ARGS];
   int m_argsCount= _parseCommandLine(m_cmd.c_str(),m_args);

    if(m_argsCount > 2)
    {
      cerr << "smash error: cd: too many arguments" << endl;//perror
      deleteCmd(m_args,m_argsCount);
      return;
    } 

    SmallShell& shell= SmallShell::getInstance();
    char m_buffer[COMMAND_ARGS_MAX_LENGTH];
    char* m_cwd = getcwd(m_buffer,COMMAND_ARGS_MAX_LENGTH);

    if(m_cwd==nullptr)
    {
      cerr << "smash error: getcwd failed" << endl;//perror
      deleteCmd(m_args,m_argsCount);
      return;
    }

    if(strcmp(m_args[1],"-")==0)
    {
          
          if(shell.getLastDir().size()==0)
          {
            cerr<<"smash error: cd: OLDPWD not set"<<endl;//perror
            deleteCmd(m_args,m_argsCount);
            return;
          }
          else
          { 
              if(chdir(shell.getLastDir().c_str())==-1)
              {
                perror("smash error: chdir failed");//perror
                deleteCmd(m_args,m_argsCount);
                return;
              }

              shell.updateLast(m_cwd);
              deleteCmd(m_args,m_argsCount);
              return;

          }
    }
    else
    {

          if(chdir(m_args[1])==-1)
          {
            perror("smash error: chdir failed");
            deleteCmd(m_args,m_argsCount);
            return;        
          }
          else
          {
            shell.updateLast(m_cwd);
            deleteCmd(m_args,m_argsCount);
          }

    }
 
}

//*******************QuitCommand*****************************

void QuitCommand::execute()
{
    char* m_args[COMMAND_MAX_ARGS];
    int m_argsCount =_parseCommandLine(getCmd().c_str(),m_args);
    if(m_argsCount==1)
    {
      deleteCmd(m_args,m_argsCount);
      exit(0);
    }
    if((strcmp(m_args[1],"kill") == 0) && m_argsCount>=2)
    {
       
       std::cout<<"smash: sending SIGKILL signal to "<<m_jobsList->getSize()<<" jobs:"<<std::endl;
        m_jobsList->printQuit();

    }

    m_jobsList->killAllJobs();
    m_jobsList->removeFinishedJobs();
    //
    //SmallShell& shell= SmallShell::getInstance();
    
    deleteCmd(m_args,m_argsCount);
    exit(0);

}


//*********************showpid*******************************
//
void ShowPidCommand::execute()
{
  cout << "smash pid is " << getpid() << endl;
}

//**********************************chprompt******************************

void chprompt::execute(){
    std::string cmd = this->getCmd().c_str();
    char* args[COMMAND_MAX_ARGS];
    int argsNum = _parseCommandLine(cmd.c_str(),args);
    SmallShell* smash = &SmallShell::getInstance();
    if(argsNum==1)
    {
      smash->setNewName("smash");
    }
    else
    {
      smash->setNewName(args[1]);
    }

    deleteCmd(args, argsNum);
}



void GetCurrDirCommand::execute(){
    char buff[COMMAND_ARGS_MAX_LENGTH];
    getcwd(buff,COMMAND_ARGS_MAX_LENGTH);
    cout<<buff<<endl;
}


void SmallShell::printSmashJobs(){
  ////it might be unnecessay 
  m_jobs->removeFinishedJobs();
  m_jobs->printJobsList();
}



void JobsList::printJobsList()const{
  // is it necessary to check if the job has stopped ??!! 
  for(JobEntry* job: m_jobsList){
    if(!(job->m_stopped))
    {
    cout<<"["<<job->m_id<<"] "<<job->m_cmdLine<<endl;
    }
  }
}


void JobsCommand::execute(){
  
  m_jobsList->removeFinishedJobs();
  m_jobsList->printJobsList();
}



//*****************ForegroundCommand******************
void ForegroundCommand::execute(){
  char *arguments[COMMAND_MAX_ARGS];
  int argNum=_parseCommandLine(getCmd().c_str(),arguments);
  int jID;
  if(argNum>=2&&isNumber(arguments[1])){
    jID=std::stoi(arguments[1]);
    if(!m_jobs->getJobById(jID)){
      std::cerr<<"smash error: fg: job-id "<<jID<<" does not exist"<<std::endl;
      deleteCmd(arguments,argNum);
      return;
    }
  }
  if(argNum==1)
    jID=m_jobs->getMaxID();
  if (argNum==1&&!m_jobs->getSize())
  {
      std::cerr<<"smash error: fg: jobs list is empty"<<std::endl;
      deleteCmd(arguments,argNum);
      return;
  }
  
  if(argNum>2 || (argNum == 2 && !isNumber(arguments[1]))){//isNUMIMPLEMNTATION
      std::cerr<<"smash error: fg: invalid arguments"<<std::endl;
      deleteCmd(arguments,argNum);
      return;
  }
  
 
  
  if(!m_jobs->getJobById(jID)){
      std::cerr<<"smash error: fg: job-id "<<jID<<" does not exist"<<std::endl;
      deleteCmd(arguments,argNum);
      return;
  }

  int pid=m_jobs->getJobById(jID)->m_pid;
  std::string cmdLine=m_jobs->getJobById(jID)->m_cmdLine;
  SmallShell& smash=SmallShell::getInstance();
  if(kill(pid,SIGCONT) == -1){
        std::cerr<<"smash error: kill failed"<<std::endl; 
        deleteCmd(arguments,argNum);
        return;
  }
  std::cout<<cmdLine<<" "<<pid<<std::endl;
  int status;
  smash.setCurrProcess(pid);//DONE
  smash.setCurrCommand(cmdLine);//DONE
  //smash.runFg()
  waitpid(pid,&status,WUNTRACED);
  if(WIFEXITED(status)){
    smash.removeJob(jID);
  }
  //smash.terminateFg();
  smash.setCurrProcess(-1);
  smash.setCurrCommand("");
  deleteCmd(arguments,argNum);
}




//************************kill**********************************
void KillCommand::execute(){
  char *arguments[COMMAND_MAX_ARGS];
  int argNum=_parseCommandLine(getCmd().c_str(),arguments);
  int jobId;
  JobsList::JobEntry* job;
  if(argNum>=3&&isNumber(arguments[2])){
    jobId=std::stoi(arguments[2]);
    job=m_jobs->getJobById(jobId);
    if(!job){
      std::cerr<<"smash error: kill: job-id "<<jobId<<" does not exist"<<std::endl;
      deleteCmd(arguments,argNum);
      return;
    }
  }
  if(argNum != 3 || !isNumber(arguments[2])||arguments[1][0] != '-' ){
        std::cerr<<"smash error: kill: invalid arguments"<<std::endl;
        deleteCmd(arguments,argNum);
        return;
  }
  std::string sigNumberStr=arguments[1];
  sigNumberStr=sigNumberStr.substr(1,sigNumberStr.size()+1);
  if(!isNumber(sigNumberStr)||!isNumber(arguments[2])){
    std::cerr<<"smash error: kill: invalid arguments"<<std::endl;
    deleteCmd(arguments,argNum);
    return;
  }
  int sigNumber=std::stoi(sigNumberStr);

  
  
  if(kill(job->m_pid,sigNumber)==-1){
    perror("smash error: kill failed");
    deleteCmd(arguments,argNum);
    return ;
  }
  if(sigNumber==SIGSTOP){
    job->m_stopped=true;
  }
  else if(sigNumber==SIGCONT){
    job->m_stopped=false;
  }
  std::cout<<"signal number "<<sigNumber<<" was sent to pid "<<job->m_pid<<std::endl;
  deleteCmd(arguments,argNum);
}

void deleteCmd(char **args, int num){
  for (int i = 0; i < num; i++)
  {
    free(args[i]);
  }
  
}



JobsList::JobEntry* JobsList::getJobById(int jobId){
  for (JobEntry* job:m_jobsList){
    if(job->m_id==jobId)
      return job;
  }
  return nullptr;
  
}
bool isComplex(const char* cmd_line)
{
    for (int i = 0; cmd_line[i] != '\0'; i++)
        if(cmd_line[i]=='*'||cmd_line[i]=='?')
            return true;
    return false;
}
//*******************external command*******************************
ExternalCommand::ExternalCommand(const char* cmd_line):Command(cmd_line)
{

 m_isBackgroundComamnd=_isBackgroundComamnd(this->getCmd().c_str());
 m_isComplex=isComplex(this->getCmd().c_str());
 

}



void ExternalCommand::execute()
{
   SmallShell& shell= SmallShell::getInstance();
   pid_t m_pid =fork();
//
   if(m_pid<0)
   {
        perror("smash error: fork failed");
        return;
   }
   if(m_pid==0)// is a child 
   {
        
          if(setpgrp()==-1){
            perror("smash error: setpgrp failed");
            return;
          }
          char m_cmdNoBg[getCmd().size()+1];
          m_cmdNoBg[getCmd().size()] = '\0';
          strcpy(m_cmdNoBg,getCmd().c_str());
          _removeBackgroundSign(m_cmdNoBg);
          char* m_args[COMMAND_MAX_ARGS];
          int m_argsNum = _parseCommandLine(m_cmdNoBg,m_args);
        if(m_isComplex) 
        {
          if(execlp("/bin/bash","/bin/bash","-c",this->getCmd().c_str(),nullptr) == -1)
          {
             perror("smash error: execlp failed");
              deleteCmd(m_args,m_argsNum);
              exit(0);
          }
        }
        else// the command is simple
        {
            if(execvp(m_args[0],m_args) == -1)
            {
                perror("smash error: execvp failed");
                deleteCmd(m_args,m_argsNum);
                exit(0);
            }
    }
   }
    if(m_pid>0)
    {
          if(m_isBackgroundComamnd)
          {
            shell.addJob(m_pid,this->getCmd());
          }
          else
          {
            shell.setCurrProcess(m_pid);
            shell.setCurrCommand(this->getCmd());
            waitpid(m_pid,nullptr,WUNTRACED);
            shell.setCurrProcess(-1);
            shell.setCurrCommand("");
          }



    }

}



void RedirectionCommand::execute(){
  char *arguments[COMMAND_MAX_ARGS];
  int argNum=_parseCommandLine(getCmd().c_str(),arguments);
  SmallShell& smash =SmallShell::getInstance();
  std::string fileName(arguments[argNum-1]);
  std::string rdType(arguments[argNum-2]);
  std::string commandLine(createStringWithOutRedirction(arguments,argNum));
  pid_t pid=fork();
  if (pid==-1){
    perror("smash error: fork failed");
    deleteCmd(arguments,argNum);
    return;
  }
  else if (pid==0){
    //sonCode
    if(setpgrp() == -1){
       perror("smash error: setpgrp failed");
       deleteCmd(arguments,argNum);
       exit(0);
    }
    if (close(STDOUT_FILENO)){
      perror("smash error: close failed");
      deleteCmd(arguments,argNum);
      exit(0);
    }
    if(rdType==">>"){
      //concatination ">>" openFile
      if (open(fileName.c_str(),O_RDWR | O_CREAT | O_APPEND, 0655)==-1){
        perror("smash error: open failed");
        deleteCmd(arguments,argNum);
        exit(0);
      }
    }
    else{
      //overRiding openFile ">" 
      if (open(fileName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0655)==-1){
        perror("smash error: open failed");
        deleteCmd(arguments,argNum);
        exit(0);
      }
    }
    // at this point the the fd[stdout_fileno]=opened file.
    smash.executeCommand(commandLine.c_str());
    deleteCmd(arguments,argNum);
    exit(0);
  }
  else{
    //parentCode
    waitpid(pid,nullptr,WNOHANG);
    deleteCmd(arguments,argNum);
    return ;//unecessaryLineOfcode it have been  added for the sake of adding.///////////////////
  }
  
}



void ChmodCommand::execute(){
  char* arguments[COMMAND_MAX_ARGS];
  int argNum=_parseCommandLine(getCmd().c_str(),arguments);
  if(argNum!=3||!isNumber(arguments[1])){
    std::cerr << "smash error: chmod: invalid arguments" << std::endl;
    deleteCmd(arguments,argNum);
    return;
  }
  int numMode=std::stoi(arguments[1]);
  int chmodResult = chmod(arguments[2],intToMode(numMode));
  deleteCmd(arguments,argNum);
  if(chmodResult == -1){
    perror("smash error: chmod failed");
    return;
  }
}
//
bool isNumber(const std::string& str)
  {
    std::string convStr(str);
    bool flag =true;
    try
    {
      std::stoi(convStr);
    }
    catch(const std::invalid_argument& excep)
    {
      flag=false;
    }
    return flag;
  }



std::string createString(char **arguments){
  std::string result("");
  int i=0;
  while(arguments[i]){
    result.append(arguments[i]);
    result.append(" ");
    i++;
  }
  return result;
}

bool RedirectionCommand::validateCommand(){
  char* m_arguments[COMMAND_MAX_ARGS];
  int m_argNum=_parseCommandLine(getCmd().c_str(),m_arguments);
  if(m_argNum!=3||isNumber(m_arguments[1])||std::stoi(m_arguments[1])){
    std::cerr << "smash error: chmod: invalid arguments" << std::endl;
    deleteCmd(m_arguments,m_argNum);
    return false;
  }
  return true;
}



bool isRedirection(char **arguments,int argNum){
  int i=0;
  while(i<argNum){
    std::string str(arguments[i]);
    if(str==">"||str==">>")
      return true;
    i++;
  }
  return false;
}


void JobsList::killAllJobs(){
  for (JobsList::JobEntry* job : m_jobsList){
    if(kill(job->m_pid,SIGKILL)== -1){
      perror("smash error: kill failed");
      return;
    }
  }
}


void JobsList::printQuit()
{
  for(JobsList:: JobEntry* args : m_jobsList)
  {
      std::cout<< args->m_pid <<": ";
      std::cout<<args->m_cmdLine<<std::endl;
  }
}


void JobsList::removeJobs(){
  for(unsigned int i=0;i<m_jobsList.size();i++){
    delete m_jobsList[i];
  }
}


void SmallShell::removeJob(int jobId){
  m_jobs->removeJobById(jobId);
}

void JobsList::removeJobById(int jobId){
  std::vector<JobEntry*>::iterator it=m_jobsList.begin();
  std::vector<JobEntry*>::iterator end=m_jobsList.end();
  for(;it!=end;it++){
    JobEntry* job=*it;
    if(job->m_id==jobId){
      delete job;
      m_jobsList.erase(it);
      return;
    }
      
  }
  
}



void JobsList::removeFinishedJobs(){
 
    std::vector<int>finishedJobs;
    int pid;
    do{
        pid = waitpid(-1,nullptr,WNOHANG);
        if(pid > 0){
            finishedJobs.push_back(getJIDByPID(pid));
        }
        else{
            break;
        }
    }while(pid !=0);
    
    for (int x:finishedJobs)
    {
      removeJobById(x);
    }
    
  
}




mode_t intToMode(int val) {
    mode_t mode = 0;
    int uga[4];

    // Extracting individual digits
    for (int i = 0; i < 4; i++) {
        uga[i] = val % 10;
        val /= 10;
    }

    // Extracting permissions
    int usr = uga[2];
    int grp = uga[1];
    int all = uga[0];
    int special = uga[3];

    // Setting special permissions
    if (special == 2)
        mode |= S_ISGID;
    else if (special == 4)
        mode |= S_ISUID;
    else if (special == 1)
        mode |= S_ISVTX;

    // Setting user permissions
    mode |= ((usr & 4) ? S_IRUSR : 0);
    mode |= ((usr & 2) ? S_IWUSR : 0);
    mode |= ((usr & 1) ? S_IXUSR : 0);

    // Setting group permissions
    mode |= ((grp & 4) ? S_IRGRP : 0);
    mode |= ((grp & 2) ? S_IWGRP : 0);
    mode |= ((grp & 1) ? S_IXGRP : 0);

    // Setting others permissions
    mode |= ((all & 4) ? S_IROTH : 0);
    mode |= ((all & 2) ? S_IWOTH : 0);
    mode |= ((all & 1) ? S_IXOTH : 0);
    return mode;
}



void SmallShell::addJob(pid_t pid, const std::string& cmdline){
  m_jobs->addJob(cmdline,pid);
}
//

//
//
void JobsList::addJob(const std::string& cmdline,pid_t pid,bool isStopped){
  this->removeFinishedJobs();
  JobEntry *newJob= new JobEntry(cmdline,getMaxID()+1,pid,isStopped);
  m_jobsList.push_back(newJob);

  //m_maxID++;
}


void JobsList::updateMaxId(){
  if(m_jobsList.empty()){
    m_maxID=0;
    return;
  }
  int max=-1;
  for(JobEntry*job:m_jobsList){
    if(job->m_id>max)
      max=job->m_id;
  }
  m_maxID=max;
}

int JobsList::getMaxID(){
  updateMaxId();
  return m_maxID;
}

int JobsList::getJIDByPID(int PID) const{
    for(auto& jobPtr : m_jobsList){
        if(jobPtr->m_pid == PID){
            return jobPtr->m_id;
        }
    }
    return -1;
}

void SmallShell::end(){
  return;

}


std::string createStringWithOutRedirction(char ** arguments ,int argNum){
  int i=0;
  std::string result="";
  while(i<argNum){
    std::string str(arguments[i]);
    if(str==">"||str==">>")
      break;
    result.append(str);
    result.append(" ");
    i++;
  }
  return result;
}

