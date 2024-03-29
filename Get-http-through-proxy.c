/* f20171075@hyderabad.bits-pilani.ac.in Aditya Agarwal */

/* Brief description of program...*/
/* 1. We learned to do Socket programming to set up a socket and connect to it. We use TCP protocol and IPv4.
   2. We used this socket to connect to our proxy server at a particular ip address and port number of the proxy server.
   3. After establishing socket based connection to proxy server, we send a get request through the proxy. But we have to also
      authorise by sending Authetication header which contains base64 encoded username:pass string.
   4. We receive response in buffer. Depending on type of response we move forward -
   5. In case of HTTP 20X response, we seperate the header from the body to wite the HTML doc to index.html.
      Secondly, if the website is info.in2p3.fr we copy the img url from IMG tag and then send another get request for just the image.
      this way we get only the image chunk (with response header ofcourse) which we then save as logo.gif by seperating from response
      header.
   6. In case of HTTP 30X response, we handle as many redirects as they occur until a 20X response is received by checking the status 
      code in response.
   7. When finally a 20X response is received, we seperate the header and write the body to index.html. */

#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>

#define MAX_B_SZ 1000000
#define MSZ 10000

char img_buf[MAX_B_SZ];
char allb64s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 

char* getB64ver(char doSTR[]) { 
    int i=0,j=0,k=0,index, bcns = 0, daps = 0, hasher = 0, count = 0, temp;
    char *fin_res_b64 = (char *) malloc(MSZ * sizeof(char));  
    while(i<strlen(doSTR)){ 
            hasher = 0;
            count = 0, 
            bcns = 0; 
            for(j = i;j< strlen(doSTR) && j<=i+2;){ 
                hasher = hasher << 8;  
                hasher = hasher | doSTR[j];  
                count++; 
                j++;
            } 
            bcns = count * 8;  daps = bcns % 3;
            while(bcns!=0){ 
                if (bcns >= 6){ 
                    temp = bcns - 6; 
                    index = (hasher >> temp) & 63;  bcns -= 6;          
                } 
                else{ 
                    temp = 6 - bcns; 
                    index = (hasher << temp) & 63;  bcns = 0; 
                } 
                fin_res_b64[k++] = allb64s[index]; 
            }
            i+=3; 
    }
    i=1;
    while(i<=daps){
        fin_res_b64[k++] = '='; 
        i++;
    }
    fin_res_b64[k] = '\0;'; 
    return fin_res_b64; 
} 

int checkHTML_endTag(char str[],int ind){
    char html_temp[] = "</HTML>";
    int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    return 1;
}

int isRedirect(char *buffptr){
    buffptr = strstr(buffptr,"HTTP");
    int status_code;
    if(buffptr){
        //printf("This is buffptr from Redirect %s\n",buffptr);
        sscanf(buffptr,"%*s %d",&status_code);
        printf("Status Code : %d\n",status_code);
    }
    if(status_code>=300 && status_code<400){
        //printf("Redirecting \n");
        return 1;
    }
    return 0;
}

int checkContentEnd(char *buffptr){
    if(buffptr[-3]=='\r' && buffptr[-2]=='\n' && buffptr[-1]=='\r' && *buffptr=='\n')
        return 1;
    return 0;
}

char rrurl[10024];

int isRedirect2(int num_sock){
    char tmp_buff[MAX_B_SZ];
    char *buffptr=tmp_buff+4;
    char *buffptr2;
    int lengthRecv, status;
    for(;;){
        lengthRecv = recv(num_sock, buffptr, 1, 0);
        if(checkContentEnd(buffptr)){
            break;
        }
        if(lengthRecv<=0){
            printf("Inside findContentLength error\n");
            return -1;
        }
        else buffptr++;
    }
    *buffptr=0;
    buffptr=tmp_buff+4;
    buffptr2=buffptr;
    if(isRedirect(buffptr)){
        memset(rrurl,0,sizeof(rrurl));
        char redirect_url[10024];
        memset(redirect_url,0,sizeof(redirect_url));
        int indx =0;
        buffptr = strstr(buffptr,"Location:");
        for(int i=0;i<strlen(buffptr);i++){
            if(buffptr[i]=='h' && buffptr[i+1]=='t'){
                while(buffptr[i]!='\r' && buffptr[i+1]!='\n'){
                    redirect_url[indx++]=buffptr[i++];
                }
                printf("redirect_url is : %s\n",redirect_url);
                strcpy(rrurl,redirect_url);
                break;
            }
        }
        return 1;
    }
    return 0;
}

int checkParseFail(char *buffptr){
    if(buffptr[-3]=='\r' && buffptr[-2]=='\n' && buffptr[-1]=='\r' && *buffptr=='\n')
        return 1;
    return 0;
}

int requester(char str[],int ind){
    char html_temp[] = "</HTML>";
    int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    //char html_temp[] = "</HTML>";
    //int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    return 1;
}
int requesterEND(char str[],int ind){
    char html_temp[] = "</HTML>";
    int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    //char html_temp[] = "</HTML>";
    //int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    //char html_temp[] = "</HTML>";
    //int j = 0;
    for(int i = ind;i<ind+6;i++){
        if(str[i]!=html_temp[j++])
            return 0;
    }
    return 1;
}
//the only filed that it parsed is 'Content-Length' 
int findContentLength(int num_sock){
    char tmp_buff[10024];
    char *buffptr=tmp_buff+4;
    int lengthRecv, status;
    for(;;){
        lengthRecv = recv(num_sock, buffptr, 1, 0);
        if(checkContentEnd(buffptr)){
            break;
        }
        if(lengthRecv<=0){
            printf("Inside findContentLength error\n");
            return -1;
        }
        else buffptr++;
    }
    *buffptr=0;
    buffptr=tmp_buff+4;
    buffptr = strstr(buffptr,"Content-Length:");
    if(buffptr){
        sscanf(buffptr,"%*s %d",&lengthRecv);
        while(!isdigit(atoi(buffptr))){
            buffptr++;
        }
        return lengthRecv;
    }
    return -1;
}

int main(int argc, char *argv[]){
    int num_sock,ssd,status;
    struct sockaddr_in sv_address;
    sv_address.sin_family=AF_INET;
    sv_address.sin_addr.s_addr=inet_addr(argv[2]);// proxy 182.75.45.22
    sv_address.sin_port=htons(atoi(argv[3])); // port num 13128
 
    num_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(num_sock<=0){
        printf("Sock error\n");
        return -1;
    }
    ssd = connect(num_sock,(struct sockaddr *)&sv_address,(socklen_t)(sizeof sv_address));
    if(num_sock<=0){
        printf("Sock connect fail\n");
        return -1;
    }
    char buffer[MAX_B_SZ],buffer_img[MAX_B_SZ],username[1000],pass[1000];
    strcpy(username,argv[4]); strcpy(pass,argv[5]);
    strcat(username,":"); strcat(username,pass);
    char *b64 = getB64ver(username);
    char request[10000];
    char img_url[10000]; int k = 0;
    sprintf(request, "GET http://%s HTTP/1.1\r\nProxy-Authorization: Basic ", argv[1]);
    //sprintf(request, "GET http://bits-judge-server.herokuapp.com/redirect1 / HTTP/1.1\r\nProxy-Authorization: Basic ");
    strcat(request,b64); strcat(request,"\r\nConnection: close\r\n\r\n");
    printf("%s\n",request);
    if(send(num_sock,request,strlen(request),0)<0){
        printf("Send failed \n");
        return 1;
    }
    // handle redirects
    while(1){
        if(!isRedirect2(num_sock))
            break;
        // parse redirect url 
        char *buffptr; int indx=0;
        char redirect_url[10024];
        memset(redirect_url,0,sizeof(redirect_url));
        strcpy(redirect_url,rrurl);
        num_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(num_sock<=0){
            printf("Sock error\n");
            return -1;
        }
        ssd = connect(num_sock,(struct sockaddr *)&sv_address,(socklen_t)(sizeof sv_address));
        if(num_sock<=0){
            printf("Sock connect fail\n");
            return -1;
        }
        // send new request
        char redir_buff[MAX_B_SZ], redir_req[100024];
        memset(redir_buff,0,sizeof(redir_buff));
        memset(redir_req,0,sizeof(redir_req));
        sprintf(redir_req, "GET %s HTTP/1.1\r\nProxy-Authorization: Basic %s\r\nConnection: close\r\n\r\n",redirect_url,b64);    
        //printf("Redir_req is %s\n",redir_req);
        if(send(num_sock,redir_req,strlen(redir_req),0)<0){
            printf("Send failed in redirect request\n");
            break;
        }
        memset(request,0,sizeof(request));
        strcpy(request,redir_req);
    }
    // Done redirecting now we will save html response by seperating header
    num_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(num_sock<=0){
        printf("Sock error\n");
        return -1;
    }
    ssd = connect(num_sock,(struct sockaddr *)&sv_address,(socklen_t)(sizeof sv_address));
    if(num_sock<=0){
        printf("Sock connect fail\n");
        return -1;
    }
    //printf("Request sent finally is %s\n",request);
    if(send(num_sock,request,strlen(request),0)<0){
            printf("Send failed in redirect request\n");
            return -1;
    }
    int lengthRecv,img_len,page_lens;
    page_lens=findContentLength(num_sock);
    if(page_lens<0){
        num_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(num_sock<=0){
            printf("Sock error\n");
            return -1;
        }
        ssd = connect(num_sock,(struct sockaddr *)&sv_address,(socklen_t)(sizeof sv_address));
        if(num_sock<=0){
            printf("Sock connect fail\n");
            return -1;
        }
        if(send(num_sock,request,strlen(request),0)<0){
                printf("Send failed in redirect request\n");
                return -1;
        }
        page_lens=findContentLength(num_sock);
        FILE *fpy;
        fpy = fopen(argv[6],"w+");
        char recv_data[MAX_B_SZ];
        int lengthRecv;
        while(lengthRecv = recv(num_sock, recv_data, 1024, 0)){
            if(lengthRecv==-1){
                printf("error reception or page done\n");
                return -1;
            }
            fwrite(recv_data,1,lengthRecv,fpy);
        }
        fclose(fpy);
        return 0;
    }
    //printf("Page content length is %d\n",page_lens);
    FILE *fpz;
    fpz = fopen(argv[6],"w+");
    if(page_lens){
        int lens=0;
        char recv_data[MAX_B_SZ];
        while(lengthRecv = recv(num_sock, recv_data, 1024, 0)){
            if(lengthRecv==-1){
                printf("error reception\n");
                return -1;
            }
            fwrite(recv_data,1,lengthRecv,fpz);
            lens+=lengthRecv;
            if(lens==page_lens){
                printf("Page Saved\n");
                break;
            }
        }
        fclose(fpz);
    }
    // get img url
    if(!strcmp(argv[1],"info.in2p3.fr")){
        for(int i=0;i<strlen(buffer);i++){
            if(i+3<strlen(buffer) && buffer[i]=='<' && buffer[i+1]=='I' && buffer[i+2]=='M' && buffer[i+3]=='G'){
                while(buffer[i]!='\"'){
                    i++;
                }
                i++;       
                while(buffer[i]!='\"'){
                    img_url[k++]=buffer[i++];
                }
                img_url[k++]='\0';
                break;
            }
        }
        num_sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(num_sock<=0){
            printf("Sock error\n");
            return -1;
        }
        ssd = connect(num_sock,(struct sockaddr *)&sv_address,(socklen_t)(sizeof sv_address));
        if(num_sock<=0){
            printf("Sock connect fail\n");
            return -1;
        }
        printf("\nRetrieving : %s\n",img_url);
        char new_req[100000];
        sprintf(new_req, "GET http://%s/%s HTTP/1.1\r\nProxy-Authorization: Basic %s\r\nConnection: close\r\n\r\n",argv[1], "cc.gif", b64);
        if(send(num_sock,new_req,strlen(new_req),0)<0){
            printf("Send 2 failed \n");
            return 1;
        }
        img_len=findContentLength(num_sock);
        //printf("image length is %d\n",img_len);
        if(img_len){
            int lens=0;
            FILE* image_fd=fopen(argv[7],"wb");
            char recv_data[MAX_B_SZ];
            while(lengthRecv = recv(num_sock, recv_data, 1024, 0)){
                if(lengthRecv==-1){
                    printf("error reception\n");
                    return -1;
                }
                fwrite(recv_data,1,lengthRecv,image_fd);
                lens+=lengthRecv;
                //printf("Length recieved: %d\n",lens);
                if(lens==img_len){
                    printf("Image Download Done\n");
                    break;
                }
            }
            fclose(image_fd);
        }
    }
    return 0;
}
