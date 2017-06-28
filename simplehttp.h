#include<sys/socket.h>//socket,connect,bind,listen accept
#include<stdio.h>
#include<netinet/in.h>//sockaddr_in
#include<arpa/inet.h>//ttonl
#include<stdlib.h>//bzero();
#include<string.h>//index,bzero(),strcasecmp(),strstr
#include<sys/stat.h>//struct stat,stat;
#include<fcntl.h>//open,read,fcntl
#include<pthread.h>
#include<sys/wait.h>
#include<signal.h>
#include<sys/errno.h>
#define maxb 1024
#define minb 100
#define isruning 1
#define isnotruning 0
#define max_num_of_thread  50




#define min_of_thread 10
#define max_num_of_task 2048

//int processnum=0;
typedef struct thread_task
{
	void * arg;
	void*(*f)(void *);
	//Pthread_task * pre;
	struct thread_task* next;
}Pthread_task;
typedef struct thread_info
{
	pthread_t tid;
	int status;
	struct thread_info* next;
}Pthread_info;
typedef struct thread_pool
{       int status;
	pthread_mutex_t pLock ; //¶ÓÁÐ»¥³âËø,ŒŽÉæŒ°µœ¶ÓÁÐÐÞžÄÊ±ÐèÒªŒÓËø
	pthread_cond_t  tPcond;
        unsigned int sum;
	Pthread_task * pTask_queue;
	unsigned int pTask_size;
        unsigned int uMaxoftask;
	Pthread_info * pThread_queue;
	unsigned int uMax;
	unsigned int uCurr_num;
	//unsigned int uWaite_num;
	//unsigned int uWork_num;
	//pthread_t printf_p;
unsigned int tasksum;
	  pthread_t monitor_p;


}Pthread_pool;
 void Tpool_excute_func(void *arg);
void Tpool_monitor_delete_thread(Pthread_pool *thread_pl);
 //int Init(int &port,int &backlog);
//int start(int port);
void Simhttp_accept_quest(void *);
void Simhttp_discard_header(int fd);
int Simhttp_get_line(int sock,char *buf,int size);
int Simhttp_parse_uri(char * method,char *uri,char *filename,char * cgiargs);
void Simhttp_server_static(int fd,char *filename,int filesize);
void Simhttp_send_header(int fd,char *filename,char* filesize);
void Simhttp_server_cgi(int fd,char *filename,char * method,char * cgiargs);
void Simhttp_method_unimplemented(int client);
void Simhttp_flle_not_found(int clientfd);
void Simhttp_bad_request(int clientfd);
void Simhttp_execute_error(int clientfd);

void Simhttp_get_filetype(char * filename,char *filetype);

Pthread_pool * Tpool_craet_Pthread(int num);
int  Tpool_add_Task_to_queue(Pthread_pool *thread_pl,void *(*f)(void *),void * arg);
void Tpool_monitor_delete_thread(Pthread_pool *thread_pl);
int Tpool_is_need_add_thread(Pthread_pool * thread_pl);
 Pthread_info  *Tpool_get_thread_by_id(Pthread_pool *thread_pl,pthread_t id);
void Tpool_excute_func(void *arg);
int  Tpool_monitor_pthread(Pthread_pool * thread_pl);

