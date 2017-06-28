
#include"simplehttp.h"
/*请求处理函数，先找出方法和是否是cgi 在获取文件信息，进入相应的处理函数 based on httpd*/
void Simhttp_accept_quest(void *arg)

  { 
   signal(SIGPIPE,SIG_IGN);//处理客户关闭的状态，而不是结束进程
   struct timeval timeout = {1,500}; 
   //int nNetTimeout=100;
//设置发送超时 
    int *pfd=(int *)arg;
       int fd=*pfd;
    setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    
       // printf("pid >>>:%d\n",pthread_self());
        
	char buf[maxb],method[minb],uri[minb],version[minb];//http request line
	char filename[minb],cgiargs[minb];
	int is_cgi;
     //

     //while(n==0)

     Simhttp_get_line(fd,buf,maxb);  
     //Simhttp_discard_header(fd);
	//printf("discard buf%s\n",buf);
      sscanf(buf,"%s %s %s",method,uri,version);//提取方法和资源地址 版本
	  if(strcasecmp(method,"GET")&&strcasecmp(method,"POST"))
	{//判断方
       printf("method error\n");
        Simhttp_discard_header(fd);
         Simhttp_method_unimplemented(fd);
	//目前只写了get post方法有时间在加上 
        close(fd);
       free(pfd);
	return ;
	}
	
	is_cgi=Simhttp_parse_uri(method,uri,filename,cgiargs);
        //printf("is cgi ?%d\n",is_cgi);
	 struct stat sbuf;
	if(stat(filename,&sbuf)<0)
	{
          Simhttp_discard_header(fd);
          Simhttp_flle_not_found(fd);
	
	}
	else
  {
	if(!is_cgi)
	{
              
		if(!(S_ISREG(sbuf.st_mode)||!(S_IRUSR&sbuf.st_mode)))//文件没有读取权限
		{
			Simhttp_flle_not_found(fd);	
		}   
           else
		Simhttp_server_static(fd,filename,sbuf.st_size);
	}
	 else 
	{
		if(!(S_ISREG(sbuf.st_mode)||!(S_IRUSR&sbuf.st_mode)))//文件没有读取权限  
		{
			Simhttp_execute_error(fd);		
		}
		else 
		Simhttp_server_cgi(fd,filename,method ,cgiargs);
	}
  }

//printf("pid2>>>>%d \n",pthread_self());
close(fd);
free(pfd);
return ;
}
/* 从套接字读取一行数据 ，以\r\n为标志提取 最后以\n作为 结尾*/
int Simhttp_get_line(int sock ,char *buf,int size)
{
  int i=0;
  char c='\0';
int n=0;
// signal(SIGPIPE,SIG_IGN);
   while((i<size-1)&&(c!='\n'))
  {
// printf(" get line \n");
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
     }else {if((n<0) &&(errno == EINTR)){ continue;} break;}
     
      
      
  }
 // if(i==0)bzero(buf,sizeof(buf));
   buf[i]='\0';
   return i;
}
//跳过头部数据
void Simhttp_discard_header(int fd)
{
       //signal(SIGPIPE,SIG_IGN);
	char buf[maxb];
	int n=1;
       
 	n=Simhttp_get_line(fd,buf,maxb);
        //n=Simhttp_get_line(fd,buf,maxb);
	while(strcmp("\n",buf)!=0&&(n>0))
	{ 
         
	 n=Simhttp_get_line(fd,buf,maxb);
          //if(n==1){printf("n>>>>>>%d\n",n);break;}
	
	}
	return ;
}
//提取filename和参数（如果有的话）
int Simhttp_parse_uri(char * method,char * uri,char *filename,char *cgiargs)
{
   
	int cgi=0;
	char *ptr;
//printf("start Simhttp_parse_uri\n");
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
           // printf("%s\n",uri);
		if(filename[strlen(filename)-1]=='/')//默认文件地址
		{
			strcat(filename,"default.html");
	        }

		if(cgi==0)strcpy(cgiargs,"");
            // printf("%s %s\n",filename,cgiargs);
		return cgi;

		
}
//http响应 头部信息 
void Simhttp_send_header(int fd,char *filename,char* filesize)
{
//signal(SIGPIPE,SIG_IGN);
char buf[minb];
char filetype[minb];
get_filetype(filename,filetype);
        sprintf(buf,"HTTP/1.0 200 OK\r\n");
	sprintf(buf,"%sServer:server test\r\n",buf);
	sprintf(buf,"%sContent-length:%d\r\n",buf,filesize);
	sprintf(buf,"%sContent-type:%s\r\n",buf,filetype);
	sprintf(buf,"%s\r\n",buf);

	send(fd,buf,strlen(buf),0);
return ;
}
//处理静态文件
void Simhttp_server_static(int fd,char *filename,int filesize )
{
	
   char * sfile,filetype[minb],buf[maxb];
   FILE * filefd;
   int n=1;

  
Simhttp_discard_header(fd);//跳过头部

        //Simhttp_send_header(fd,filename,filesize);
	//printf("static \n");
      
      filefd=fopen(filename,"r");
      if(filefd==NULL)
     {
     perror("file error;");
     Simhttp_flle_not_found(fd);
      return ;
   // exit(0);
     }
 //sleep(10);
//printf("send \n");
      Simhttp_send_header(fd,filename,filesize);


  	 fgets(buf,sizeof(buf),filefd);
         send(fd,buf,strlen(buf),0);
       while((!feof(filefd))&&n>=0)
        {
         n=send(fd,buf,strlen(buf),0);
         fgets(buf,sizeof(buf),filefd);
        }

	//printf("filefd>>>%d \n");
       // printf("do ok \n");
	fclose(filefd);
	//printf("do ok \n");

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
void Simhttp_flle_not_found(int clientfd)
{
 char buf[1024];
//signal(SIGPIPE,SIG_IGN);
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
 void Simhttp_bad_request(int clientfd)
{
char buf[1024];
//signal(SIGPIPE,SIG_IGN);
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

void Simhttp_execute_error(int clientfd)
{
char buf[minb];
//signal(SIGPIPE,SIG_IGN);
 sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "Content-type: text/html\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(clientfd, buf, strlen(buf), 0);
 sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
 send(clientfd, buf, strlen(buf), 0);
}

void Simhttp_method_unimplemented(int client)
{
 
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    sprintf(buf, "%sServer:server test\r\n",buf);
    sprintf(buf, "%Content-Type: text/html\r\n",buf);

    sprintf(buf, "%s\r\n",buf);

    sprintf(buf, "%s<HTML><HEAD><TITLE>Method Not Implemented\r\n",buf);

    sprintf(buf, "%s</TITLE></HEAD>\r\n",buf);

    sprintf(buf, "%s<BODY><P>HTTP request method not supported.\r\n",buf);

    sprintf(buf, "%s</BODY></HTML>\r\n",buf);
    send(client, buf, strlen(buf), 0);
}
//cgi 处理函数 参照httpd的思路写的
void Simhttp_server_cgi(int fd,char *filename,char * method ,char * cgiargs)
{
   printf("start Simhttp_server_cgi\n");
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
          
		Simhttp_discard_header(fd);//跳过头部
              
		
	}else if(strcasecmp(method,"POST")==0)
	{
               
		nchars=Simhttp_get_line(fd,buf,minb);
               //printf("%s\n",buf);
		while((nchars>0)&&strcmp("\n",buf))
		{ 
                     //printf("%s\n",buf);
                     buf[15]='\0';//end id
			if(strcasecmp(buf,"Content-Length:")==0)
                   {
                   				
                    content_length=atoi(&buf[16]);
                    
                    }
		    nchars=Simhttp_get_line(fd,buf,minb);
                       
		}
		if(content_length==-1)
		{
                  Simhttp_bad_request(fd);
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
               Simhttp_execute_error(fd);
			 //cgi error;
			return ;
		 }
		  if(pipe(output)<0)
		 {
			
                      perror("pipe error:"); //cgi error;
                  Simhttp_execute_error(fd);
			return ;
		 }
                  if((pid=fork())<0)
                {     
                   Simhttp_execute_error(fd);
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
		  {    //signal(SIGPIPE,SIG_IGN);
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
Pthread_pool * Tpool_craet_Pthread(int num)
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
	pl->tasksum=0;
        pl->status=isruning;
       
        
	temp=(Pthread_info*)malloc(sizeof(Pthread_info));
       
	if(temp!=NULL)
	{
	 temp->next=NULL;
	 pl->pThread_queue=temp;
	 p=temp;
        pthread_create(&(temp->tid),NULL,Tpool_excute_func,pl);
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
		pthread_create(&(temp->tid),NULL,Tpool_excute_func,pl);
		p->status=isnotruning;//
	}

	
        //pthread_create(&(pl->monitor_p),NULL,Tpool_monitor_delete_thread,pl);
        printf("create thread success\n");
	return pl;
}

/*添加任务到任务队列，兵唤醒等待的线程，如果队列满了则增加线程，线程池达到最大数不在增加线程，任务队列已满则丢弃任务*/
int  Tpool_add_Task_to_queue(Pthread_pool *thread_pl,void *(*f)(void *),void * arg)
{
	Pthread_pool * pl=thread_pl;
	Pthread_task * pt=NULL;
int * pfd=(int *)arg;
	pt=(Pthread_task *)malloc(sizeof(Pthread_task ));
	if(pt==NULL)
        { close(*pfd);
         free(pfd);
         //free(pt);
        return -1;
        }
	pt->arg=arg;
	pt->f=f;
        pt->next=NULL;
	Pthread_task* temptask; 
        
     // printf(" arg >>>%d\n",*(int *)(pt->arg));
       if(pl->uMaxoftask<pl->pTask_size)
          {
      printf("nedd add \n");
       
      printf(" arg >>>%d\n",*pfd);
      close(*pfd);
         free(pfd);
         free(pt);
        
       return 0;
          }

        pthread_mutex_lock(&(pl->pLock));
	if(pl->pTask_queue==NULL)
           pl->pTask_queue=pt;
	else 
	{
		temptask=pl->pTask_queue;
		while(temptask->next!=NULL)
			temptask=temptask->next;
	               temptask->next=pt;

	}
	pl->pTask_size++;
      	pl->tasksum++;
	pthread_mutex_unlock(&(pl->pLock)); //printf("add task \n");
	pthread_cond_signal(&(pl->tPcond));
      // printf("signal\n");
	return 1;
}

void Tpool_monitor_delete_thread(Pthread_pool *thread_pl)
{
	Pthread_pool * pl=thread_pl;
	if(pl==NULL)return ;
    int curr_num_of_doing=0;
    unsigned int lastNum;
	while(1)
	{sleep(6);
        curr_num_of_doing= Tpool_monitor_pthread(thread_pl); 
        printf("current num of thread :%d num of doing :%d\n",pl->uCurr_num,curr_num_of_doing);
		if(curr_num_of_doing==0)
		{
			sleep(15);
		   if( Tpool_monitor_pthread(thread_pl)==0&&pl->uCurr_num>min_of_thread&&pl->pTask_size==0)
		   {
			   lastNum=pl->uCurr_num-min_of_thread;
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
						//pi->next=pin->next;
                                  //pthread_cancel(pin->tid);
						 //kill(pin->tid,SIGKILL);
						free(pin);
						
					}else 
					{
						pi=pin;
					}
					
				}
                        pthread_mutex_unlock(&pl->pLock);
			    //ÈÎÎñœÏ      
		   }
		   
		}

	}

}

int Tpool_is_need_add_thread(Pthread_pool * thread_pl)
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
				pi=temp;
				pl->uCurr_num++;
				
			}
			pthread_create(&(temp->tid),NULL,Tpool_excute_func,pl);
			temp->status=isnotruning;
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

 Pthread_info  *Tpool_get_thread_by_id(Pthread_pool *thread_pl,pthread_t id){
	//thread_info *pt=NULL;
	Pthread_info *p=thread_pl->pThread_queue;
	while(p!=NULL){
		if(p->tid==id)
			return p;
		p = p->next;
	       }
	return NULL;
   }

void Tpool_excute_func(void *arg)
{
	Pthread_pool * pl=(Pthread_pool *)(arg);
	Pthread_task * pt=NULL;
	Pthread_info * pi=NULL;
	while(1)
	{ 

	pthread_mutex_lock(&(pl->pLock));
	 while(pl->pTask_size==0 )
	{ 
         
               //printf("wait for signal \n");
		pthread_cond_wait(&(pl->tPcond),&(pl->pLock));
        }
	 pthread_t tid = pthread_self();
	 pi=Tpool_get_thread_by_id(pl,tid);
         if(pi==NULL) { pthread_mutex_unlock(&(pl->pLock));pthread_cond_signal(&(pl->tPcond));break;}
 	pt=pl->pTask_queue;
       if(pt==NULL) { pthread_mutex_unlock(&(pl->pLock));continue;}
     // { printf("get the task error\n"); c;}
		pl->pTask_queue=pt->next;
		 pl->pTask_size--; 
                 pl->sum++;
               //printf("tasksum :%d\n",pl->tasksum) ;
                 
                  //pl->uWork_num++;
           //printf("sum of current :%d\n",pl->sum);
             //printf("task size %d\n",pl->pTask_size); 
        // printf("do it  \n");
                pthread_mutex_unlock(&(pl->pLock));
                
		 pi->status=isruning; 

                
              (*(pt->f))(pt->arg);
              //if( close(*(int *)pt->arg)==-1)perror("close :");
                pi->status=isnotruning;
                
            // free((int*)pt->arg);
              // pt->arg=NULL;
               free(pt);
              pt=NULL;
	}

}
//监测线程池获取当前正在处理任务的线程数目
int  Tpool_monitor_pthread(Pthread_pool * thread_pl)
{
 Pthread_pool *pl=thread_pl;
//Pthread_info * pitemp;
int num_of_running_thread=0;
 Pthread_info * pi=pl->pThread_queue;
  while(pi!=NULL)
{
if(pi->status==isruning)  
{
num_of_running_thread++;
}
pi=pi->next;
}
return num_of_running_thread;
}


