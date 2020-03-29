#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>

#define REQUEST_SIZE 2048
#define HOST_SIZE 64
char request[REQUEST_SIZE];
char host[HOST_SIZE];
int mypipe[2];

int port=80;
int client=10;
int time=10;

int speed=0;
int failed=0;
int bytes=0;

volatile int timerexpired=0;

#define errpintf(format, args...) \
        {fprintf(stderr, "[%s][%s][%d]\n"format, __FILE__, \
        __FUNCTION__, __LINE__, ##args); \
        exit(-1);}

void help();
void build_request(const char *url);
void print_info();
void benchmark();
int connect2socket();
void benchcore();

int main(int argc, char *argv[]){

    if(argc==1){
        help();
        return 0;
    }
    if(argc>=3){
        time=atoi(argv[2]);
    }
    if(argc==4){
        client=atoi(argv[3]);
    }
    if(argc>4){
        errpintf("too many argv\n");
    }

    // printf("argv[1]:%s",argv[1]);
    build_request(argv[1]);

    print_info();

    benchmark();
    
    return 0;
}

void help(){
    printf("myworkbench url time client\n");
    printf("time: defult 10s\n");
    printf("client: defult 10\n");
}

void build_request(const char *url)
{
    char tmp[10];
    int i;

    memset(host,0,HOST_SIZE);
    memset(request,0,REQUEST_SIZE);

    strcpy(request,"GET");
    strcat(request," ");

    if(NULL==strstr(url,"://"))
    {
        fprintf(stderr, "\n%s: is not a valid URL.\n",url);
        exit(2);
    }
    if(strlen(url)>1500)
    {
        fprintf(stderr,"URL is too long.\n");
        exit(2);
    }
    if (0!=strncasecmp("http://",url,7)) 
    { 
        fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
        exit(2);
    }
    
    /* protocol/host delimiter */
    i=strstr(url,"://")-url+3;

    if(strchr(url+i,'/')==NULL) {
        fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
        exit(2);
    }
    
    /* get port from hostname */
    if(index(url+i,':')!=NULL && index(url+i,':')<index(url+i,'/'))
    {
        strncpy(host,url+i,strchr(url+i,':')-url-i);
        memset(tmp,0,10);
        strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);
        /* printf("tmp=%s\n",tmp); */
        port=atoi(tmp);
        if(port==0) port=80;
    } 
    else
    {
        strncpy(host,url+i,strcspn(url+i,"/"));
    }
    // printf("Host=%s\n",host);
    strcat(request+strlen(request),url+i+strcspn(url+i,"/"));

    strcat(request," HTTP/1.0\n");
    // strcat(request," HTTP/1.1");
  
    strcat(request,"zzzzzzzzzzzzzzzzzzzzzzzzzzzzz\r\n");

    strcat(request,"\r\n");
    strcat(request,"Host: ");
    strcat(request,host);
    strcat(request,"\r\n");
}

void print_info()
{
    printf("\nRequest:\n%s\n",request);
    printf("Runing info: ");
    printf("%d clients", client);
    printf(", running %d sec\n", time);
}

void benchmark()
{   
    int sock,pid;
    int i,j,k;
    FILE *f;

    if( (sock = connect2socket())<0 ){
        errpintf("connect to server failed");
    }
    close(sock);

    if(pipe(mypipe)){
        errpintf("pipe failed");    
    }

    fflush(NULL);
    for(int i=0;i<client;i++){
        if( (pid = fork())<=0 ){
            sleep(1);
            break;
        }
    }
    if(pid < 0){
        errpintf("fork failed");
    }
    
    if(pid == 0){
        benchcore();

        f=fdopen(mypipe[1],"w");
        if(f==NULL){
            errpintf("open pipe for writing failed.");
        }
        // printf("%d %d %d\n",speed,failed,bytes);
        fprintf(f,"%d %d %d\n",speed,failed,bytes);

        fclose(f);

        return ;
    }else{
        f=fdopen(mypipe[0],"r");
        if(f==NULL) 
        {
            errpintf("open pipe for reading failed.");
        }
        
        setvbuf(f,NULL,_IONBF,0);
        speed=0;
        failed=0;
        bytes=0;

        while(1)
        {
            pid=fscanf(f,"%d %d %d",&i,&j,&k);
            if(pid<2)
            {
                fprintf(stderr,"Some of our childrens died.\n");
                break;
            }
            
            speed+=i;
            failed+=j;
            bytes+=k;
        
            if(--client==0) break;
        }
    
        fclose(f);

        printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
            (int)((speed+failed)/(time/60.0f)),
            (int)(bytes/(float)time),
            speed,
            failed);
    }
    
    return ;
    
}

int connect2socket()
{
    int sock;
    struct sockaddr_in sa;
    struct hostent *ht;
    unsigned long saaddr = 0;

    memset(&sa,0,sizeof(sa));

    sa.sin_family = AF_INET;

    saaddr = inet_addr(host);
    if(saaddr != INADDR_NONE){
        memcpy(&sa.sin_addr,&saaddr,sizeof(saaddr));
    }else{
        ht = gethostbyname(host);
        if(ht == NULL){
            return -1;
        }
        memcpy(&sa.sin_addr, ht->h_addr, ht->h_length);
    }

    sa.sin_port=htons(port);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){
        return sock;
    }

    if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        return -1;
    return sock;
}

void alarm_handler(int signal)
{
    timerexpired = 1;
}

void benchcore()
{
    int rlen;
    char buf[1500];
    int s,i;
    struct sigaction sa;

    /* setup alarm signal handler */
    sa.sa_handler=alarm_handler;
    sa.sa_flags=0;
    if(sigaction(SIGALRM,&sa,NULL))
        exit(3);
    
    alarm(time); // after benchtime,then exit

    rlen=strlen(request);
    nexttry:while(1)
    {
        if(timerexpired)
        {
            if(failed>0)
            {
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                failed--;
            }
            return;
        }
        
        s=connect2socket(); 
        if(s<0){ 
            failed++;
            continue;
        } 
        if(rlen!=write(s,request,rlen)){
            failed++;
            close(s);
            continue;
        }
        /* read all available data from socket */
        while(1)
        {
            if(timerexpired) break; 
            i=read(s,buf,1500);
            /* fprintf(stderr,"%d\n",i); */
            if(i<0) 
            { 
                failed++;
                close(s);
                goto nexttry;
            }
            else
            if(i==0) break;
            else
            bytes+=i;
        }
        if(close(s)) {failed++;continue;}
        speed++;
    }
}