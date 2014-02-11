/* 
SE 3BB4, assignment 1
student name: Matthew Schweitzer
student number: 1139402
file assgn1.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/queue.h>

#include "log.h"

#define high    0
#define medium  1
#define low     2

#define hpq_times   2
#define mpq_times   4

int logindex = 0;
int *logi = &logindex;

pid_t create_job(int i);
void siga_handler();   // handler for signal SIGALRM
void sigc_handler();   // handler for signal SIGCHLD
sigset_t mymask1;      // normal signal process mask when all signals are free
                       // but SIGALRM and SIGCHLD are blocked
sigset_t mymask2;      // special signal process mask when all signals are free
sigset_t jobmask;      // job signal process mask blocking all signals, leaving
                       // only SIGUSR2 unblocked
sigset_t block_mask;   // Block all signals
struct sigaction sa_alarm;  // disposition for SIGALRM
struct sigaction sa_chld;   // disposition for SIGCHLD


int number_of_jobs;

// TO DO
// your own data structure(s) to handle jobs and the 3 queues
// your own auxialiary variables
// END TO DO

//VARIABLES
int number_of_queues = 3;
int num_runs_high = 2;
int num_runs_med = 6;
int minJobs = 3;
int maxJobs = 6;
int currentJobNumber;

//STRUCTS
struct jobs{
  int pid;
  int number_of_runs;
  int jobNumber;
};

struct jobQueue{
  int front;
  int back;
  struct jobs job[7];
};

struct jobs currentJob;
struct jobQueue queue[3];


//FUNCTIONS
void push(int priority, struct jobQueue queue[priority], struct jobs currentJob);
struct jobs pop(int priority, struct jobQueue queue[priority]);
void createQueue(int priority, struct jobQueue queue[priority]);
int isEmpty(int priority, struct jobQueue queue[priority]);


// function main ------------------------------------------------- 
int main(int argc,char** argv) {
  int i, j;
  pid_t pid;
  
  // TO DO 
 // check the number of command line arguments, if not 2, terminate
  // the program with a proper error message on the screen.
  if(argc != 2)
    {
      msg("%s","The program may only accept in 2 arguement.\n Exiting now!");
      exit(EXIT_SUCCESS);
    }

 
  // check if the single command line argument (argv[1]) has value 3 to 6,
  // if not, treminate the program with a proper error message on the
  // screen.
  // set appropriately number_of_jobs
  // END TO DO
  number_of_jobs = atoi(argv[1]);
  if((number_of_jobs < minJobs) || (number_of_jobs > maxJobs))
    {
      msg("%s","The first arguement must be between 3 and 6. \n Exiting now!\n");
      exit(EXIT_SUCCESS);
    }
 
  
  create_log("assgn1.log");
  
  // TO DO
  // prepare mymask1 -- SIGCHLD and SIGALRM blocked, all other signals free 
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset() 
  // END TO DO

  //Set SIGCHLD and SIGALRM as blocked.
  if((sigemptyset(&mymask1) == -1) || (sigaddset(&mymask1,SIGCHLD) == -1) || (sigaddset(&mymask1,SIGALRM) == -1))
    {
      msg("Failed to initialize sig_set mymask1");
    }
  if (sigprocmask(SIG_BLOCK, &mymask1, NULL) == -1)
    {   
      msg("Failed to block SIGCHLD and SIGALRM");
    }

  // TO DO
  // instal mymask1 as the process signal mask using sigrpocmask() 
  // END TO DO 
  if(sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
    {
      msg("failed to set sig_set mymask1");
    }  

  // TO DO
  // prepare mymask2 -- all signals free 
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset() 
  // END TO DO
  if(sigemptyset(&mymask2) == -1)
    {
      msg("Failed to initialize sig_set mymask2");
    }

  // TO DO
  // prepare jobmask -- all signals blocked except SIGUSR2 
  // using sigemptyset(), sigfillset(), sigaddset(), sigdelset() 
  // END TO DO
  if((sigfillset(&jobmask) == -1) || (sigdelset(&jobmask,SIGUSR2) == -1))
    {
      msg("failed to initialize jobmask or remove SIGUSR2 from it.");
    }
  if (sigprocmask(SIG_BLOCK, &jobmask, NULL) == -1)
    {
      msg("Failed to block signals in jobmask");
    }
  

  // TO DO
  // prepare SIGALRM disposition sa_alarm
  // its handler (sa_handler) is siga_handler()
  // its signal mask (sa_mask) must block all signals
  // its flags (sa_flags) must be set to SA_RESTART 
  // END TO DO
  sigfillset (&block_mask);
  sa_alarm.sa_handler = siga_handler;
  sa_alarm.sa_mask = block_mask;
  sa_alarm.sa_flags = SA_RESTART;

  
  // TO DO
  // instal SIGALRM disposition using sigaction() 
  // END TO DO
  sigaction(SIGALRM, &sa_alarm, NULL); 
  
  // TO DO
  // prepare SIGCHLD disposition sa_chld
  // its handler (sa_handler) is sigc_handler()
  // its signal mask (sa_mask) must block all signals
  // its flags (sa_flags) must be set to SA_RESTART 
  // END TO DO
  sigfillset (&block_mask);
  sa_chld.sa_handler = sigc_handler;
  sa_chld.sa_mask = block_mask;
  sa_chld.sa_flags = SA_RESTART;

  // TO DO
  // instal SIGCHLD disposition using sigaction() 
  // END TO DO
    sigaction(SIGCHLD, &sa_chld, NULL); 

  // TO DO
  // create empty high-priority queue
  // create empty medium-priority queue
  // create empty low-priority queue
  // END TO DO
    createQueue(high,queue);
    createQueue(medium,queue);
    createQueue(low, queue);
    
  // TO DO
  // create a data structure to keep information about jobs - PID, number of runs
  struct jobs job[number_of_jobs];
  for(i = 0; i < number_of_jobs; i++) {
    pid = create_job(i);
    //save pid for job i in your data structure
    job[i].pid = pid;
    job[i].jobNumber = i;
    job[i].number_of_runs = 0;
   }

  // put all jobs in the high-priority queue
  for(i = 0; i < number_of_jobs; i++)
    {
      currentJob = job[i];
      push(high, queue, currentJob);
    }
  // END TO DO

  // TO DO
  // in a loop
  //    if all queues are empty
  //       record it in the log by Msg("All jobs done\n");
  //       and display it on the screen by msg("All jobs done\n");
  //       and terminate the loop
  //    "switch on" the first job from the highest-priority non-empty queue
  //    by sending it the SIGUSR1 signal (using sigsend())
  //    Record it in the log using 
  //        Msg("Switched on high-priority job %d\n",job number);  or
  //        Msg("Switched on medium-priority job %d\n",job number); or
  //        Msg("Switched on low-priority job %d\n",job number); 
  //    announce it on the screen using corresponding msg();
  //    set alarm for 1 second using alarm()
  //    switch the current signal process mask mymask1 to mymask2 while
  //    going to suspension using sigsuspend()
  //    (thus only SIGCHLD or SIGALRM will wake it up from suspension
  //    SIGCHLD indicates that the job that is currently executing just
  //    terminated, SIGALRM indicates that the time for the job currently
  //    executing is up and it must be "switched off")
  // end loop
  // END TO DO
  while(1)
    {
      if((isEmpty(high,queue) == -1) || (isEmpty(medium,queue) == -1) || (isEmpty(low,queue) == -1))
	{
	  if(isEmpty(high,queue) == -1)
	    {
	      currentJob = pop(high, queue);
	      currentJob.number_of_runs++;
	      kill(currentJob.pid,SIGUSR1);
	      Msg("Switched on high-priority job %d\n",currentJob.jobNumber);
	      msg("Switched on high-priority job %d\n",currentJob.jobNumber);
	    }
	  else if(isEmpty(medium,queue) == -1)
	   {
	      currentJob = pop(medium, queue);
	      currentJob.number_of_runs++;
	      kill(currentJob.pid,SIGUSR1);
	      Msg("Switched on medium-priority job %d\n",currentJob.jobNumber);
	      msg("Switched on medium-priority job %d\n",currentJob.jobNumber);
	   }
	  else if(isEmpty(low, queue) == -1)
	   {
	     currentJob = pop(low, queue);
	     currentJob.number_of_runs++;
	     kill(currentJob.pid, SIGUSR1);
	     Msg("Switched on low-priority job %d\n",currentJob.jobNumber);
	     msg("Switched on low-priority job %d\n",currentJob.jobNumber);
	   }
	}
      else
	{
	  Msg("All jobs done\n");
	  msg("All jobs done\n");
	  exit(EXIT_SUCCESS);
	}

      alarm(1);
      if(sigsuspend(&mymask2) != -1)
      {
	perror("failed to suspend sig_set mymask2\n");
       }
    }

  return 0;
}// end function main
  
  
// function create_job -------------------------------------------- 
pid_t create_job(int i) {
  pid_t pid;
  char argv0[10];
  char argv1[10];
  
  // TO DO
  // switch process signal mask from mymask1 to jobmask 
  // using sigprocmask()
  // END TO DO
   if(sigprocmask(SIG_SETMASK, &jobmask, NULL) == -1)
    {
      msg("failed to set sig_set jobmask");
    } 
  
  if ((pid = fork()) < 0) 
    Sys_exit("fork error\n");
  if (pid == 0) { // child process
    strcpy(argv0,"job");
    sprintf(argv1,"%d",i);
    execl("job",argv0,argv1,NULL);
  }
  
  // parent process
  // TO DO
  // switch process signal mask from jobmask back to mymask1 
  // using sigprocmask()
  // END TO DO 
   if(sigprocmask(SIG_SETMASK, &mymask1, NULL) == -1)
    {
      msg("failed to set sig_set mymask1");
    } 
 
  return pid;
}// end function create_job
  
  
  
  
// function siga_handler ------------------------------------------ 
void siga_handler() {
  // TO DO
  // "switch of" the currently executing job by sending it SIGUSR2 signal
  // (using sigsend())
  // either put the job back to the queue it came from (you must count
  // how many times it has been through the queue) or "demote it" to the
  // lower-prority queue.
  // record this in the log using 
  //     Msg("Switched off high-priority job %d\n",job number); or
  //     Msg("Switched off medium-priority job %d\n",job number);
  //     Msg("Switched off low-priority job %d\n",job number);
  // announce it on the screen suing corresponding msg();
  // END TO DO 
  kill(currentJob.pid, SIGUSR2);
  if(currentJob.number_of_runs < num_runs_high)
    {
      push(high,queue,currentJob);
      Msg("Switched off high-priority job %d\n",currentJob.jobNumber);
      msg("Switched off high-priority job %d\n",currentJob.jobNumber);
    }
  else if(currentJob.number_of_runs == num_runs_high)
    {
      push(medium,queue,currentJob);
      Msg("Switched off high-priority job %d\n",currentJob.jobNumber);
      msg("Switched off high-priority job %d\n",currentJob.jobNumber);
    }
  else if((currentJob.number_of_runs > num_runs_high) && (currentJob.number_of_runs < num_runs_med))
    {
      push(medium,queue,currentJob);
      Msg("Switched off medium-priority job %d\n",currentJob.jobNumber);
      msg("Switched off medium-priority job %d\n",currentJob.jobNumber);
    }
  else if((currentJob.number_of_runs == num_runs_med))
    {
      push(low,queue,currentJob);
      Msg("Switched off medium-priority job %d\n",currentJob.jobNumber);
      msg("Switched off medium-priority job %d\n",currentJob.jobNumber);
    }
  else if (currentJob.number_of_runs > num_runs_med)
    {
      push(low,queue,currentJob);
      Msg("Switched off low-priority job %d\n",currentJob.jobNumber);
      msg("Switched off low-priority job %d\n",currentJob.jobNumber);
    }
  return;
}// end function siga_handler
  
  
// function sigc_handler ------------------------------------------ 
void sigc_handler() {
  // TO DO
  // disarm the alarm
  // record in the log that the currently executing job is done by Msg("job %d done\n",currentJob.pid);
  // END TO DO 
  kill(getpid(),SIGALRM);
  Msg("job %d done\n",currentJob.jobNumber);
}// end function sigc_handler

// TO DO
// functions for handling your data structures
// END TO DO

/*Push an item onto the queue
 *@inputs:queue - the queue we are operating on
 *
 */
void push(int priority, struct jobQueue queue[priority], struct jobs currentJob)
{
  queue[priority].job[queue[priority].back] = currentJob; 
  if(queue[priority].back < 6)
  {
      queue[priority].back = queue[priority].back + 1;
  }
  else if((queue[priority].back == 6))
  {
    queue[priority].back = 0;
  }

  return;
}

struct jobs pop(int priority, struct jobQueue queue[priority])
{
  struct jobs tempJob;
  tempJob = queue[priority].job[queue[priority].front];

  if(queue[priority].front < 6)
  {
    queue[priority].front = queue[priority].front + 1;
  }
  else if(queue[priority].front == 6)
  {
    queue[priority].front = 0;
  }
  return tempJob;
}


void createQueue(int priority, struct jobQueue queue[priority])
{
  queue[priority].front = 0;
  queue[priority].back = 0;
  return;
}

int isEmpty(int priority, struct jobQueue queue[priority])
{
  if(queue[priority].front == queue[priority].back)
    {
      return 0;
    }
  else
    {
      return -1;
    }
}



