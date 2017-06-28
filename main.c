#include"simplehttp.h"
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
/*sigset_t signal_mask;
   sigemptyset (&signal_mask);
   sigaddset (&signal_mask, SIGPIPE);
   int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
   if (rc != 0)
   {
      printf("block sigpipe error\n");
   } */
    Pthread_pool *thread_pl=NULL;

    thread_pl=Tpool_craet_Pthread(40); //
printf("process id :%d\n",getpid());
 
    int *p=NULL;
	socklen_t clilen=sizeof(cliaddr);
	while(1)
	{
	 if((connectfd=accept(listenfd,&cliaddr,&clilen))!=-1)
	 {
		//printf("has got a connect\n");
             p=(int *)malloc(sizeof(int));
              if(p==NULL)break;
             *p=connectfd;
         Tpool_add_Task_to_queue(thread_pl,Simhttp_accept_quest,(void*)p);
      //if(pthread_create(&newthreadID,NULL,Simhttp_accept_quest,(void*)p)!=0)
          //perror("pthread");
	   //Simhttp_accept_quest((void *)p);
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
