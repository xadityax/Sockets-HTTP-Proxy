/* f20170452@hyderabad.bits-pilani.ac.in Rudrajit Kargupta */

/* Brief description of program...*/
/* ... */

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
#define SIZE 10000
char img_buf[MAX_B_SZ];

char* base64Encoder(char input_str[], int len_str) { 
    // Character set of base64 encoding scheme 
    char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 
    // Resultant string 
    char *res_str = (char *) malloc(SIZE * sizeof(char)); 
    int index, no_of_bits = 0, padding = 0, val = 0, count = 0, temp; 
    int i, j, k = 0; 
    // Loop takes 3 characters at a time from  
    // input_str and stores it in val 
    for (i = 0; i < len_str; i += 3){ 
            val = 0, count = 0, no_of_bits = 0; 
            for (j = i; j < len_str && j <= i + 2; j++){ 
                // binary data of input_str is stored in val 
                val = val << 8;  
                  
                // (A + 0 = A) stores character in val 
                val = val | input_str[j];  
                  
                // calculates how many time loop  
                // ran if "MEN" -> 3 otherwise "ON" -> 2 
                count++; 
              
            } 
  
            no_of_bits = count * 8;  
  
            // calculates how many "=" to append after res_str. 
            padding = no_of_bits % 3;  
  
            // extracts all bits from val (6 at a time)  
            // and find the value of each block 
            while (no_of_bits != 0)  
            { 
                // retrieve the value of each block 
                if (no_of_bits >= 6) 
                { 
                    temp = no_of_bits - 6; 
                      
                    // binary of 63 is (111111) f 
                    index = (val >> temp) & 63;  
                    no_of_bits -= 6;          
                } 
                else
                { 
                    temp = 6 - no_of_bits; 
                      
                    // append zeros to right if bits are less than 6 
                    index = (val << temp) & 63;  
                    no_of_bits = 0; 
                } 
                res_str[k++] = char_set[index]; 
            } 
    } 
  
    // padding is done here 
    for (i = 1; i <= padding; i++)  
    { 
        res_str[k++] = '='; 
    } 
  
    res_str[k] = '\0;'; 
  
    return res_str; 
  
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

int main(int argc, char *argv[]){
    int clsd,ssd,status;
    char buffer[MAX_B_SZ],buffer_img[MAX_B_SZ],username[100],pass[100],t1[100],t2[100];
    strcpy(username,argv[4]);
    printf("%s",username);
    strcpy(pass,argv[5]);
    printf("%s\n",pass);
    strcat(username,":");
    strcat(username,pass);
    printf("%s\n",username);
    char *b64 = base64Encoder(username,strlen(username));
    printf("%s\n",b64);
    sprintf(request, "GET %s/ HTTP/1.1\r\nProxy-Authorization: Basic ", argv[1]);

    // char request[]="GET http://info.in2p3.fr/ HTTP/1.1\r\nProxy-Authorization: Basic ";
    strcat(request,b64);
    strcat(request,"\r\nConnection: close\r\n\r\n");
    
    struct sockaddr_in srvr_addr;

    struct addrinfo hints,*res;

    srvr_addr.sin_family=AF_INET;
    srvr_addr.sin_addr.s_addr=inet_addr(argv[2]);// proxy 182.75.45.22
    srvr_addr.sin_port=htons(atoi(argv[3])); // port num 13128

    clsd =socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(clsd<=0){
        perror("Socket init failed..\n");return 1;
    }
    else{
        printf("Socket Initialised successfully \n");
    }
    ssd=connect(clsd,(struct sockaddr *)&srvr_addr,(socklen_t)(sizeof srvr_addr));
    if(clsd<=0){
        perror("Socket connect failed..\n");return 1;
    }
    else{
        printf("Connection established Successfully \n");
    }
    if( send(clsd , request , strlen(request) , 0) < 0){
		printf("Send failed \n");
		return 1;
	}
	puts("Data Send\n");
	// char server_reply[200000]= {};
    char img_url[10000]; int k = 0;
	if( recv(clsd, buffer , 10000000 , 0) < 0){
		printf("recv failed \n");
	}
    else{
	    printf("Reply received\n");
        printf("%s", buffer);
        FILE *fp;
        fp = fopen(argv[6],"w+");
        for(int i=0;i<strlen(buffer);i++){
            if(i+4<strlen(buffer) && buffer[i]=='<' && buffer[i+1]=='H' && buffer[i+2]=='T' && buffer[i+3]=='M'){
                char doc[MAX_B_SZ]; int l =0;
                while(i<strlen(buffer)){
                    if(buffer[i+1]=='H' && buffer[i+2]=='T' && checkHTML_endTag(buffer,i)){
                        while(buffer[i]!='>'){
                            doc[l++]=buffer[i++];
                        }
                        doc[l++]=buffer[i++];
                        break;
                    }
                    doc[l++]=buffer[i++];
                }
                fputs(doc,fp);
            }
        }
        for(int i=0;i<strlen(buffer);i++){
            if(buffer[i]=='<' && buffer[i+1]=='I'){
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
        printf("\n%s",img_url);
        //memset(buffer, 0, sizeof(buffer));
        // new request for only image
        clsd =socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(clsd<=0){
            perror("Socket init failed..\n");return 1;
        }
        else{
            printf("Socket Initialised successfully \n");
        }
        ssd=connect(clsd,(struct sockaddr *)&srvr_addr,(socklen_t)(sizeof srvr_addr));
        if(clsd<=0){
            perror("Socket connect failed..\n");return 1;
        }
        else{
            printf("Connection established Successfully \n");
        }
        char new_req[100000];
        sprintf(new_req, "GET http://info.in2p3.fr/%s HTTP/1.1\r\nProxy-Authorization: Basic %s\r\nConnection: close\r\n\r\n", img_url, b64);
        //char request[]="GET http://info.in2p3.fr/%s HTTP/1.1\r\nProxy-Authorization: Basic ";

        if( send(clsd , new_req , strlen(new_req) , 0) < 0){
            printf("Send failed \n");
            return 1;
        }
        puts("Img Data Sent\n");
        // char server_reply[200000]= {};
        if( recv(clsd, img_buf, 10000000 , 0) < 0){
            printf("Image reception failed \n");
        }
        else{
            printf("%s", img_buf);
            printf("Image received\n");
        }

    }
    return 0;
}