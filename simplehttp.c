
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
#define maxb 1024
#define minb 100
#define isruning 1
#define isnotruning 0
#define max_num_of_thread  50
#define min_of_thread 10
#define max_num_of_task 1000
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
	  pthread_t monitor_p;


}Pthread_pool;
 void excute_func(void *arg);
void is_need_delete_thread(Pthread_pool *thread_pl);
 // int Init(int &port,int &backlog);
//int start(int port);
void accept_quest(void *);
void read_header(int fd);
int get_line(int sock,char *buf,int size);
int parse_uri(char * method,char *uri,char *filename,char * cgiargs);
void server_static(int fd,char *filename,int filesize);
void sendheader(int fd,char *filename,char* filesize);
void server_cgi(int fd,char *filename,char * method,char * cgiargs);
void unimplemented(int client);
void not_found(int clientfd);
void bad_request(int clientfd);
void execute_error(int clientfd);

void getfiletype(char * filename,char *filetype);

Pthread_pool * craet_Pthread(int num);
int  add_Task_to_queue(Pthread_pool *thread_pl,void *(*f)(void *),void * arg);
void is_need_delete_thread(Pthread_pool *thread_pl);
int is_need_add_thread(Pthread_pool * thread_pl);
 Pthread_info  *get_thread_by_id(Pthread_pool *thread_pl,pthread_t id);
void excute_func(void *arg);
int  monitor_pthread(Pthread_pool * thread_pl);

int main(int argc ,char *argv[])
{
	int listenfd,connectfd,backlog;
	int port;
//printf("%d\n",MSG_DONTWAIT);
if(argc>=2){
port=atoi(argv[1]);
}
else {
printf("init port error\n");
return 0;
}
char buf[1024];

backlog=5;  
 pthread_t newthreadID;
    
  // if(Init(port,backlog)==0)return ;
	struct sockaddr_in cliaddr;
	struct sockaddr_in serveraddr;
	;// 加上错误处理
if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
{
perror("creat socket error");
exit(-1);
}
	printf("Init ok.... port:%d \n",port);
	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons(port);
	if(bind(listenfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
        {
     perror("bind error");
     exit(-1);
}
	printf("web test\n");
	if(listen(listenfd,backlog)<0)
{
 perror("listen error ");
 exit(-1);
}

 
    Pthread_pool *thread_pl=NULL;

    thread_pl=craet_Pthread(10); 
 
    int *p=NULL;
	socklen_t clilen=sizeof(cliaddr);
	while(1)
	{
	 if((connectfd=accept(listenfd,&cliaddr,&clilen))!=-1)
	 {
		 printf("has got a connect\n");
             p=(int *)malloc(sizeof(int));
             *p=connectfd;
          add_Task_to_queue(thread_pl,accept_quest,(void*)p);
      //if(pthread_create(&newthreadID,NULL,accept_quest,(void*)p)!=0)
          //perror("pthread");
	   //accept_quest((void *)p);
                 //int n= recv(connectfd,buf,1024,0);
              //if(n<0)perror("recv error:");
                  // buf[n]='\0';
                 //printf("message:%d %s\n", n,buf);
            
			// close(connectfd);
	 }			
	}
	close (listenfd);
	return 0;

}
/*请求处理函数，先找出方法和是否是cgi 在获取文件信息，进入相应的处理函数 based on httpd*/
void accept_quest(void *arg)

  {   int *pfd=(int *)arg;
       int fd=*pfd;
        printf("fd:%d\n",fd);
        printf("start accept_quest\n");
	char buf[maxb],method[minb],uri[minb],version[minb];//http request line
	char filename[minb],cgiargs[minb];
	int is_cgi;
     //
int n=0;
    while(n==0)
	{n=get_line(fd,buf,maxb);}
        
      // read_header(fd);
	 
      sscanf(buf,"%s %s %s ",method,uri,version);//提取方法和资源地址 版本
	 if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
	{//判断方
       // read_header(fd);
        //unimplemented(fd);
	//目前只写了get post方法有时间在加上
       free(pfd); 
        close(fd);
	return ;
	}
	
	is_cgi=parse_uri(method,uri,filename,cgiargs);
        printf("is cgi ?%d\n",is_cgi);
	struct stat sbuf;
	if(stat(filename,&sbuf)<0)
	{
         //read_header(fd);
        //not_found(fd);
	
	}
	else
  {
	if(!is_cgi)
	{
              
		if(!(S_ISREG(sbuf.st_mode)||!(S_IRUSR&sbuf.st_mode)))//文件没有读取权限
		{
			not_found(fd);	
		}else 
		server_static(fd,filename,sbuf.st_size);
	}
	else 
	{
		if(!(S_ISREG(sbuf.st_mode)||!(S_IRUSR&sbuf.st_mode)))//文件没有读取权限  
		{
			execute_error(fd);		
		}
		else 
		server_cgi(fd,filename,method ,cgiargs);
	}
  }
//free(pfd);
close(fd);
}
/* 从套接字读取一行数据 ，以\r\n为标志提取 最后以\n作为 结尾*/
int get_line(int sock ,char *buf,int size)
{

  int i=0;
  char c='\0';
int n=0;
   while((i<size-1)&&(c!='\n'))
  {
 //printf(" get line \n");
    n=recv(sock,&c,1,0);
	  if(n>0)
         {

	    if(c=='\r')
		{
		n=recv(sock,&c,1,MSG_PEEK);
            if((n>0)&&(c=='\n')) recv(sock,&c,1,0);
           else c='\n';
           }
	   buf[i]=c;
           //printf("%c",c);
	   i++;
     }else
       {c='\n';}
  }

   buf[i]='\0';
   return i;
}
//跳过头部数据
void read_header(int fd)
{
       
	char buf[maxb];
	int n=1;
 	n=get_line(fd,buf,maxb);
	while(strcmp(buf,"\n")&&n>0)
	{ 
          //printf("%s \n","header discard");
	  n=get_line(fd,buf,maxb);
         
	  printf("%s",buf);
	}
	return ;
}
//提取filename和参数（如果有的话）
int parse_uri(char * method,char * uri,char *filename,char *cgiargs)
{
   
	int cgi=0;
	char *ptr;
printf("start parse_uri\n");
strcpy(cgiargs,"");
	if(strcasecmp(method, "GET") == 0)  
       {  
      ptr=index(uri,'?');//找出？后面的参数
	  if(ptr)
	  {
		  strcpy(cgiargs,ptr+1);
		  *ptr='\0';
                 cgi=1;
	  }
	  else strcpy(cgiargs,"");
	  
       
	}
       else if(strcasecmp(method, "POST") == 0) 
	{
		cgi=1;

	}
	//default dir;
                sprintf(filename,"./testfile%s",uri);
            printf("%s\n",uri);
		if(filename[strlen(filename)-1]=='/')//默认文件地址
		{
			strcat(filename,"default.html");
	        }

		if(cgi==0)strcpy(cgiargs,"");
             printf("%s %s\n",filename,cgiargs);
		return cgi;

		
}
//http响应 头部信息 
void sendheader(int fd,char *filename,char* filesize)
{
char buf[minb];
char filetype[minb];
get_filetype(filename,filetype);
        sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer:server test\r\n",buf);
	sprintf(buf,"%sContent-length:%d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type:%s\r\n",buf,filetype);
	sprintf(buf,"%s\r\n",buf);

	send(fd,buf,strlen(buf),0);
}
//处理静态文件
void server_static(int fd,char *filename,int filesize )
{
	
     char * sfile,filetype[maxb],buf[maxb];
    FILE * filefd;
      signal(SIGPIPE,SIG_IGN);
	read_header(fd);//跳过头部
	printf("static \n");
       
      filefd=fopen(filename,"r");
      if(filefd==NULL)
     {
     perror("file error;");
      not_found(fd);
      // return ;
    exit(0);
     }
       sendheader(fd,filename,filesize);
  	 fgets(buf,sizeof(buf),filefd);
         //send(fd,buf,strlen(buf),0);
       while(!feof(filefd))
        {
         send(fd,buf,strlen(buf),0);
         fgets(buf,sizeof(buf),filefd);
        }

	printf("filefd>>>%d \n");

	fclose(filefd);
	


        //printf("%s\n",buf);


}
void get_filetype(char * filename,char *filetype)
{
	if(strstr(filename,".html"))
		strcpy(filetype,".html");
	else if(strstr(filename,".gif"))
		strcpy(filetype,".gif");
	else if(strstr(filename,".jpg"))
		strcpy(filetype,".jpg");
	else strcpy(filetype,".text/plain");
}
/*based on tinyhttpd */
void not_found(int clientfd)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf,"%sServer:server test\r\n",buf);
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>not found 404 \r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(clientfd, buf, strlen(buf), 0);
}


/*based on tinyhttpd*/
 void bad_request(int clientfd)
{
char buf[1024];

 sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
 send(clientfd, buf, sizeof(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(clientfd, buf, sizeof(buf), 0);
 sprintf(buf, "\r\n");
 send(clientfd, buf, sizeof(buf), 0);
 sprintf(buf, "<P>Your browser sent a bad request, ");
 send(clientfd, buf, sizeof(buf), 0);
 sprintf(buf, "such as a POST without a Content-Length \r\n");
 send(clientfd, buf, sizeof(buf), 0);
}

void execute_error(int clientfd)
{
char buf[minb];
 sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
 send(clientfd, buf, strlen(buf), 0);
}

void unimplemented(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "%sServer:server test\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</TITLE></HEAD>\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(client, buf, strlen(buf), 0);
}
//cgi 处理函数 参照httpd的思路写的
void server_cgi(int fd,char *filename,char * method ,char * cgiargs)
{
   printf("start server_cgi\n");
	char buf[minb];
	int input[2];
	int output[2];
	pid_t pid;
	int content_length=-1;
	int nchars;
	int status;
	char c;
	//buf[0]='a';buf[1]='\0';
	
	if (strcasecmp(method,"GET") == 0)
	{
          
		read_header(fd);//跳过头部
              
		
	}else if(strcasecmp(method,"POST")==0)
	{
               
		nchars=get_line(fd,buf,minb);
               //printf("%s\n",buf);
		while((nchars>0)&&strcmp(buf,"\n"))
		{ 
                     //printf("%s\n",buf);
                     buf[15]='\0';//end id
			if(strcasecmp(buf,"Content-Length:")==0)
                   {
                   				
                    content_length=atoi(&buf[16]);
                    
                    }
		    nchars=get_line(fd,buf,minb);
                       
		}
		if(content_length==-1)
		{
                  bad_request(fd);
				return ;
		}
		//post extract the information
	}
            printf("%s\n",filename);
	    sprintf(buf,"HTTP/1.0 200 OK\r\n");
            sprintf(buf,"%sServer:My Web Server\r\n",buf);
            printf("%s.....%d\n",buf,content_length);
            send(fd,buf,strlen(buf),0);
		 if(pipe(input)<0)
		 {
                 perror("pipe error:");
               execute_error(fd);
			 //cgi error;
			return ;
		 }
		  if(pipe(output)<0)
		 {
			
                      perror("pipe error:"); //cgi error;
                  execute_error(fd);
			return ;
		 }
                  if((pid=fork())<0)
                {     
                   execute_error(fd);
                   return ;
                } 
 		  if(pid==0)
		  { 
			  dup2(input[0],0);
			  dup2(output[1],1);
			  close(input[1]);
			  close(output[0]); 
			  char method_env[minb];
	                char query_env[minb];
		       char contlength_env[minb];
			sprintf(method_env, "REQUEST_METHOD=%s", method);
                        putenv(method_env);
                        
		 	  if(strcasecmp(method,"GET")==0)
			  {
				 
		         sprintf(query_env, "QUERY_STRING=%s",cgiargs);
                            putenv(query_env);
                             
			  }else  if(strcasecmp(method,"POST")==0)
			  {
			      sprintf(contlength_env,"Content-Length=%d",content_length);
				 putenv(contlength_env);
				

			  }
                      
			execl(filename,filename,NULL);
                


		  }else 
		  {    signal(SIGPIPE,SIG_IGN);
			  close(input[0]); 
			  close(output[1]);
	           if(strcasecmp(method,"POST")==0)
		     {
                       for(int i=0;i<content_length;i++)
		         {
			       recv(fd,&c,1,0);  
                        write(input[1],&c,1);
			}
		   }
         while (read(output[0],&c,1) > 0)
         send(fd,&c,1,0);
         //printf("do all \n");//debug
         close(output[0]);
         close(input[1]);
         waitpid(pid,&status, 0);
 
		  }
	
}

/*******************下面 部分是动态线程池部分 ，参考了其他 作者的写法 自己也根据自己的理解修改了，只为学习使用 ******************/
/* 创建线程池 */
Pthread_pool * craet_Pthread(int num)
{
        printf("create start\n");
	if(num<1)return NULL;
        if(num>=max_num_of_thread)num=max_num_of_thread;
	Pthread_pool * pl;
	pl=(Pthread_pool *)malloc(sizeof(Pthread_pool));
	if(pl==NULL)return NULL;
      int g=pthread_mutex_init(&(pl->pLock),NULL);
       
	int m=pthread_cond_init(&(pl->tPcond),NULL);
        pl->pTask_queue=NULL;
	pl->pThread_queue=NULL;
	Pthread_info * temp=NULL;
	Pthread_info * p;
        pl->sum=0;
	pl->pTask_size=0;
	pl->uMax=max_num_of_thread;
        pl->uMaxoftask=max_num_of_task;
	pl->uCurr_num=num;
	//pl->uWork_num=0;
        pl->status=isruning;
       
        
	temp=(Pthread_info*)malloc(sizeof(Pthread_info));
       
	if(temp!=NULL)
	{
	 temp->next=NULL;
	 pl->pThread_queue=temp;
	 p=temp;
        pthread_create(&(temp->tid),NULL,excute_func,pl);
        p->status=isnotruning;
	}else {
        free(pl);
        return NULL;
           }
   
   
	for(int i=1;i<num;i++)
	{
		
		temp=(Pthread_info*)malloc(sizeof(Pthread_info));
		if(temp!=NULL)
		{
		  
		  temp->next=NULL;
		  p->next=temp;
		  p=temp;

		}
		else 
		{
			free(pl);
			return NULL;
		}
		pthread_create(&(p->tid),NULL,excute_func,pl);
		p->status=isnotruning;//
	}

	
        //pthread_create(&(pl->monitor_p),NULL,is_need_delete_thread,pl);
        printf("create thread success\n");
	return pl;
}

/*添加任务到任务队列，兵唤醒等待的线程，如果队列满了则增加线程，线程池达到最大数不在增加线程，任务队列已满则丢弃任务*/
int  add_Task_to_queue(Pthread_pool *thread_pl,void *(*f)(void *),void * arg)
{
	Pthread_pool * pl=thread_pl;
	Pthread_task * pt=NULL;
	pt=(Pthread_task *)malloc(sizeof(Pthread_task ));
	if(pt==NULL)return -1;
	pt->arg=arg;
	pt->f=f;
        pt->next=NULL;
	Pthread_task* temptask; 
        
	
        if(pl->uMaxoftask<=pl->pTask_size&&pl->uCurr_num<=max_num_of_thread)
          {
      printf("nedd add \n");
         if( is_need_add_thread( thread_pl)==0) return 0;
     // 
          }
        pthread_mutex_lock(&(pl->pLock));
	if(pl->pTask_queue==NULL)pl->pTask_queue=pt;
	else 
	{
		temptask=pl->pTask_queue;
		while(temptask->next!=NULL)
			temptask=temptask->next;
	               temptask->next=pt;

	}
	pl->pTask_size++;
      	
	pthread_mutex_unlock(&(pl->pLock)); //printf("add task \n");
	pthread_cond_signal(&(pl->tPcond));
       printf("signal\n");
	return 1;
}

void is_need_delete_thread(Pthread_pool *thread_pl)
{
	Pthread_pool * pl=thread_pl;
	if(pl==NULL)return ;
    int curr_num_of_doing=0;
    unsigned int lastNum;
	while(1)
	{sleep(6);
        curr_num_of_doing= monitor_pthread(thread_pl); 
        printf("current num of thread :%d\n",pl->uCurr_num);
		if(curr_num_of_doing==0)
		{
			sleep(5);
		   if( monitor_pthread(thread_pl)==0&&pl->uCurr_num>min_of_thread)
		   {
			   lastNum=pl->uCurr_num/2;
			   pthread_mutex_lock(&pl->pLock);
			   Pthread_info * pi=pl->pThread_queue;
			    Pthread_info * pin;
				while(lastNum>0&&pi->next!=NULL)
				{     pin=pi->next;
					if(pin->status==isnotruning)
					{
						pi->next=pin->next;
						pl->uCurr_num--;

						
						lastNum--;
						pi=pi->next;
						kill(pin->tid,SIGKILL);
						free(pin);
						
					}else 
					{
						pi=pin;
					}
					
				}
		   }
		   pthread_mutex_unlock(&pl->pLock);
			    //ÈÎÎñœÏ
		}

	}

}

int is_need_add_thread(Pthread_pool * thread_pl)
{
	Pthread_pool * pl=thread_pl;
	Pthread_info * pi;
	Pthread_info * temp;
	 pthread_mutex_lock(&pl->pLock);
	int curr_num=pl->uCurr_num;
        int num;
    
    
    
         if(pl->uMax>=(2*curr_num))
	{
       num=curr_num;
        }else if(pl->uMax>curr_num)
        {
         num=pl->uMax-curr_num;
        }
         else if(pl->uMax==curr_num) 
         {
          pthread_mutex_unlock(&pl->pLock);  
          return 0;
         }
		
		pi=pl->pThread_queue;
		while(pi->next!=NULL)
			pi=pi->next;
		for(int i=0;i<num;i++)
		{
			temp=(Pthread_info*)malloc(sizeof(Pthread_info));
			temp->next=NULL;
			if(temp!=NULL)
			{
				pi->next=temp;
				pi=pi->next;
				pl->uCurr_num++;
				
			}
			pthread_create(&(temp->tid),NULL,excute_func,pl);
			//temp->status=isruning;
			//continue;
		}

	
	pthread_mutex_unlock(&pl->pLock);
return num;
}

/*
void destroy_Pthread(Pthread_pool * thread_pl)
{
	Pthread_pool   *pl=thread_pl;;
	Pthread_task * pt = NULL;
	Pthread_task * pt_temp;
	if(!p->status)
		return ;
	p->status=isnotruning;
	pthread_cond_broadcast(&(pl->tPcond));
	pt=pl->pTask_queue;
	if(pt!=NULL)
	while(pt->next!=NULL){
		pt_temp=pt->next;
		pt=pt->next;
		free(pt_temp);
	}
	
	Pthread_info *pi_tmp=NULL;
	Pthread_info * pi= pl->pThread_queue;
	while(pi->next!=NULL){
		pi_temp= pi->next;
		pi=pi->next;
		free(pi->next);
	}

	pthread_mutex_destroy(&(pl->pLock));
	pthread_cond_destroy(&(p->tPcond));
	return ;
}*/

 Pthread_info  *get_thread_by_id(Pthread_pool *thread_pl,pthread_t id){
	//thread_info *pt=NULL;
	Pthread_info *p=thread_pl->pThread_queue;
	while(p!=NULL){
		if(p->tid==id)
			return p;
		p = p->next;
	       }
	return NULL;
   }

void excute_func(void *arg)
{
	Pthread_pool * pl=(Pthread_pool *)(arg);
	Pthread_task * pt=NULL;
	Pthread_info * pi=NULL;
	while(1)
	{ 
//printf("thread %d\n",pthread_self());
	pthread_mutex_lock(&(pl->pLock));
	 while(pl->pTask_size==0 )
	{ 
              
		pthread_cond_wait(&(pl->tPcond),&(pl->pLock));
        }
		pt=pl->pTask_queue;
		pl->pTask_queue=pt->next;
		 pthread_t tid = pthread_self();
		pi=get_thread_by_id(pl,tid);
		 pi->status=isruning;
                 pl->sum++;
              printf("queue_ tasksize :%d\n",pl->pTask_size);
                  pl->pTask_size--;
                  //pl->uWork_num++;
            printf("sum of current :%d\n",pl->uCurr_num);
                
                pthread_mutex_unlock(&(pl->pLock));

                (*(pt->f))(pt->arg);
                 pi->status=isnotruning;
              // free(pt->arg);
               free(pt);
              pt=NULL;
	}

}
//监测线程池获取当前正在处理任务的线程数目
int  monitor_pthread(Pthread_pool * thread_pl)
{
 Pthread_pool *pl=thread_pl;
//Pthread_info * pitemp;
int num_of_running_thread=0;
 Pthread_info * pi=pl->pThread_queue;
  while(pi!=NULL)
{
if(pi->status==isruning)
num_of_running_thread++;
pi=pi->next;
}
return num_of_running_thread;
}


