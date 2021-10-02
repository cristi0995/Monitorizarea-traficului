#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
using namespace std;
                                          //CLIENT
#define PORT1 2500
#define PORT2 3500

char nr_inmatriculare[11];
pthread_mutex_t mutex_comunicare;
bool must_quit=false;
char* ip;
char* masina;
ofstream *g=NULL;
void* fncthread1(void* psd)
{
    must_quit=false;
    char msg[100],rasp[100],viteza[10][4],strada[5][25];
    int x,y;
    int sd = (intptr_t) psd;
    
    ifstream f("verificari_in");

    

    for(int i=0;i<5;i++)
        f.getline(strada[i],100);
    for(int i=0;i<10;i++)
        f.getline(viteza[i],100);

    while(true)
    {
        
        for (int i = 0; i<40 ; i++)
        {
            sleep(1);
            if(must_quit==true)
                return NULL;
        }

        bzero(msg,100);
        bzero(rasp,100);
        strcpy(msg,"verifica ");
        x=rand() % 10;
        strcat(msg,viteza[x]);
        strcat(msg," ");
        y=rand() % 5;
        strcat(msg,strada[y]);
        pthread_mutex_lock(&mutex_comunicare);
        send(sd,msg,100,0);
        recv(sd,rasp,100,0);
        *g<<viteza[x]<<"km/h pe strada "<<strada[y]<<endl<<"Raspuns: "<<rasp<<endl<<endl;
        pthread_mutex_unlock(&mutex_comunicare);
    }
    return NULL;
}

void* fncthread2(void* psd)
{
    int sd = (intptr_t) psd;
    char msg[100],rasp[100];
    while(true)
    {
        bzero (rasp,100);
        bzero (msg,100);
        cout<<"Introduceti mesajul: ";
        cin.getline(msg,200);
        
        pthread_mutex_lock(&mutex_comunicare);
        send(sd,msg,100,0);

        if(strcmp(msg,"quit")==0)
        {           
            pthread_mutex_unlock(&mutex_comunicare);
            must_quit=true;
            return NULL;
        }

        recv(sd,rasp,100,0);
        pthread_mutex_unlock(&mutex_comunicare);
        
        cout<<rasp<<endl<<endl;
    }
    return NULL;
    
}

void* fncthread3(void*)
{
    must_quit=false;
     int sd1;
    struct sockaddr_in server1;

    
    if ((sd1 = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      cout<<"Eroare la socket()."<<endl;
      exit(1);
    }
    
    server1.sin_family = AF_INET;
    server1.sin_addr.s_addr = inet_addr(ip);
    server1.sin_port = htons (PORT2);
    if (connect (sd1, (struct sockaddr *) &server1,sizeof (struct sockaddr)) == -1)
    {
      cout<<"[PORT 3500]Eroare la connect()."<<endl;
      exit(1);
    }
    char msg[100],rasp[100];

    while(true)
    {
        
        bzero(msg,100);
        bzero(rasp,100);
        if(must_quit==true)
        {
            strcpy(msg,"quit");
            send(sd1,msg,100,0);
            return NULL;
        }
        else
        {    
            strcpy(msg,"este accident?");
            send(sd1,msg,100,0);
        }

        recv(sd1,rasp,100,0);
        if(strcmp(rasp,"nu")!=0)
            *g<<"[Accident raportat]: "<<rasp<<endl<<endl;
    }
}
int main(int argc, char *argv[])
{
    int sd;
    ip=argv[1];
       masina=argv[2];
    g = new ofstream(masina);
    
    struct sockaddr_in server;

    
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      cout<<"Eroare la socket()."<<endl;
      exit(1);
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (PORT1);

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      cout<<"[PORT 2500]Eroare la connect()."<<endl;
      exit(1);
    }
    //cin>>nr_inmatriculare;
    strncpy(nr_inmatriculare,argv[2],9);
    nr_inmatriculare[9]='\0';
    send(sd,nr_inmatriculare,9,0);
    must_quit=false;
    cout<<"--Comenzi disponibile--"<<endl;
    cout<<"verifica \"viteza\" \"strada\" - pentru a verifica daca viteza data este regulamentara"<<endl;
    cout<<"accident \"strada\"          - pentru a inregistra un accident"<<endl;
    cout<<"vreme                      - pentru a afla detalii despre vreme"<<endl;
    cout<<"sport                      - pentru a afla detalii despre evenimente sportive"<<endl;
    cout<<"preturi                    - pentru a afla preturile combustibililor din apropiere"<<endl;
    cout<<"quit                       - pentru a va deconecta"<<endl<<endl; 
    pthread_mutex_init(&mutex_comunicare, 0);
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    pthread_create(&thread1,NULL,fncthread1,(void*)(intptr_t)sd);
    pthread_create(&thread2,NULL,fncthread2,(void*)(intptr_t)sd);
    pthread_create(&thread3,NULL,fncthread3,NULL);
    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    pthread_join(thread3,NULL);
    
    delete g;
  
    close(sd);
}
