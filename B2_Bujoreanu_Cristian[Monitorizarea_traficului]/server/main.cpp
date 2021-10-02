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
#include <fstream>
#include <ctime>
using namespace std;
                                                          //SERVER
#define BACKLOG 7
#define PORT1 2500
#define PORT2 3500
ofstream g("log");
time_t now=time(0);
char* dt;
//g<<"["<<dt<<"]"<<endl<<"Cerere date"<<endl<<endl;
struct 
    {
        int sd;
        char nume[10];
    }clienti[BACKLOG+1];

struct Limite
    {
        char s[30];   // numele strazii
        int v;        // viteza maxima
    };
struct
    {
        char unde[20];
        char cine[10];
    }accident;

Limite* limite=NULL;
int sd,sd1;
char vreme[6][100],e_sportive[6][100];
struct
    {
        char benzina[5];
        char motorina[5];
    }p_carburanti[6];


//=====================================

void init_file()
{
    ifstream f1("limite");
    int size;
    
    char buf[5];
    f1.getline(buf,5);
    size=atoi(buf);

    limite = new Limite[size]; 
    for(int i=0;i<5;i++)
    {
        f1.getline(limite[i].s,100);
        f1.getline(buf,100);
        limite[i].v=atoi(buf);
            
        //cout<<limite[i].s<<" cu limita: "<<limite[i].v<<endl;
    }
    f1.close();
    for(int i=0;i<5;i++)
    {
        bzero(vreme[i],100);
        bzero(e_sportive[i],100);
    }
    ifstream f2("vreme");
    for(int i=0;i<5;i++)
        f2.getline(vreme[i],100);
    f2.close();

    ifstream f3("preturi carburanti");
    for(int i=0;i<5;i++)
    { 
        f3.getline(p_carburanti[i].benzina,100);
        f3.getline(p_carburanti[i].motorina,100);
    }
    f3.close();
    
    ifstream f4("evenimente sportive");
    for(int i=0;i<5;i++)
        f4.getline(e_sportive[i],100);
    f4.close();

}

// ====================================

int init_sd(int port)
{
    struct sockaddr_in server;
    
    int sd;
    
    
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
           perror("Eroare la socket()!");
           exit(0);
    }
    int on=1;

    bzero (&server, sizeof (server));
    
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=htonl(INADDR_ANY);
    server.sin_port = htons(port);
    

    if (bind(sd,(struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("Eroare la bind()!");
        exit(0);
    }

    cout << "[server]Astept la portul " << port<<endl;

    if (listen (sd, 20) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        exit(0);
    }

    return sd;
}
//======================================

int match(char buff1[100],int end, const char* buff2)
{
    int ok=1;
    for(int i=0;i<end;i++)
        if(buff1[i]!=buff2[i])
        {
            ok=0;
            break;
        }
    return ok;
}

//======================================

void inregistreaza_accident(char msg[100],int &client)
{
   
    int ok=0;char msg1[100],rasp[100];
    bzero(rasp,100);
    strcpy(msg1,msg);
    strcpy(msg1,msg1+9);

    if(limite!=NULL)
    {    
        for(int i=0;i<5;i++)
            if(strcmp(msg1,limite[i].s)==0)
                ok=1;
    }
    
        
    if(ok)
    {   
        for(int i=0;i<9;i++)
            accident.cine[i]=msg[i];
        strcpy(msg,msg+9);
        strcpy(accident.unde,msg);
        cout<<"accident inregistrat: "<<msg1<<" de catre: "<<accident.cine<<endl;
        
        strcat(rasp, "Accident inregistrat cu succes pe strada ");
        strcat(rasp, msg);
        time_t now=time(0);
        dt = ctime(&now);
        g<<"["<<accident.cine<<"]"<<"Inregistrare accident pe strada "<<msg1<<endl<<dt<<endl<<endl;
        send(client,rasp,100,0);

    }
    else
    {
        cout<<"accident neinregistrat, strada scrisa gresit"<<endl;
        strcpy(rasp, "Strada scrisa gresit, reincercati!");
        send(client,rasp,100,0);
    }
}

//========================================

void verifica_viteza(char msg[100],int &client)
{
    int x,nr=-1;
    char rasp[100];
    if(msg[1]==' ')    //daca viteza este de o cifra
    {
        if(msg[0]>='0'&&msg[0]<='9')
        {
            char a[2];
            a[0]=msg[0];
            x=atoi(a);
            strcpy(msg,msg+2);
        }
        else
        {
            strcpy(rasp,"Viteza scrisa gresit!");
            send(client,rasp,100,0);
            return;
        }
    }
    else
        if(msg[2]==' ') //daca este de 2 cifre
        {
            if(msg[1]>='0'&&msg[1]<='9')
            {
                char a[2],b[2];
                a[0]=msg[0];
                b[0]=msg[1];
                x=atoi(a);
                x=x*10;
                x=x+atoi(b);
                strcpy(msg,msg+3);
            }
            else
            {
                strcpy(rasp,"Viteza scrisa gresit!");
                send(client,rasp,100,0);
                return;
            }        
        }
        else         //daca este de 3 cifre
        {
            if(msg[2]>='0'&&msg[2]<='9')
            {       
                char a[2],b[2],c[2];
                a[0]=msg[0];
                b[0]=msg[1];
                c[0]=msg[2];
                x=atoi(a);
                x=x*10;
                x=x+atoi(b);
                x=x*10;
                x=x+atoi(c);
                strcpy(msg,msg+4);
            }
            else
            {
                strcpy(rasp,"Viteza scrisa gresit!");
                send(client,rasp,100,0);
                return;
            }        
        }

    if(limite!=NULL)
    {    
        for(int i=0;i<5;i++)
            if(strcmp(msg,limite[i].s)==0)
                {nr=i;break;}
    }
    
    if(nr!=-1)
    {
        if(limite[nr].v>=x)
        strcpy(rasp,"Viteza regulamentara!");
        else
        strcpy(rasp,"Viteza depasita!");
        for(int i=0;i<=BACKLOG;i++)
                if(clienti[i].sd==client)
                {
                    time_t now=time(0);
                    dt = ctime(&now);
                    g<<"["<<clienti[i].nume<<"]"<<"Verificare viteza: "<<x<<"km/h pe strada "<<limite[nr].s<<endl<<"Raspuns: "<<rasp<<endl<<dt<<endl<<endl;
                    break;
                }
    }
    else
        strcpy(rasp,"Strada scrisa gresit!");
    send(client,rasp,100,0);
}

//========================================

void date_aditionale(char msg[100],int &client)
{
    int x;
    char rasp[100];
    bzero(rasp,100);

    x=rand() % 5;

    if(strcmp(msg,"vreme")==0)
        strcpy(rasp,vreme[x]);
    else
        if(strcmp(msg,"preturi")==0)           
        {
            strcpy(rasp,"benzina: ");
            strcat(rasp,p_carburanti[x].benzina);
            strcat(rasp," ; motorina: ");
            strcat(rasp,p_carburanti[x].motorina);
        }           
        else
            if(strcmp(msg,"sport")==0)         
                strcpy(rasp,e_sportive[x]);              
            else
                    strcpy(rasp,"Mesaj nerecunoscut!");
    for(int i=0;i<=BACKLOG;i++)
                if(clienti[i].sd==client)
                {
                    time_t now=time(0);
                    dt = ctime(&now);
                    g<<"["<<clienti[i].nume<<"]"<<"Cerere date despre "<<msg<<endl<<"Raspuns: "<<rasp<<endl<<dt<<endl<<endl;
                    break;
                }
    send(client,rasp,100,0);
}

//========================================

void raspunde(char msg[100],int &client)
{
    char rasp[100],msg1[100];
    int ok=1;
    bzero (rasp,100);

    if(match(msg,8,"accident")) //daca e accident
    {
        strcpy(msg,msg+9);
        for(int i=0;i<=BACKLOG;i++)
                if(clienti[i].sd==client)
                    strcpy(msg1,clienti[i].nume);
        strcat(msg1,msg);
        inregistreaza_accident(msg1,client); //msg1=[nume client][strada]
        
    } //daca nu e accident
    else
    {
        if(match(msg,8,"verifica"))
        {
            strcpy(msg,msg+9);
            verifica_viteza(msg,client);
        }
        else
            date_aditionale(msg,client);
    }

}

//============================================

void* fncthread3(void* pclient)
{
    char ultimul_accident[100];
    bzero(ultimul_accident,100);
    int client=*((int*)pclient);
    char msg[100];
    while(true)
    {
        sleep(0.2);
        bzero (msg,100);  
        recv(client,msg,100,0);
        if(strcmp(msg,"quit")==0)
            break;


        if(strcmp(accident.unde,ultimul_accident)==0)
            strcpy(msg,"nu");
        else 
        {
            strcpy(msg,"[");
            strcat(msg,accident.cine);
            strcat(msg,"] Accident: ");
            strcat(msg,accident.unde);
            strcpy(ultimul_accident,accident.unde);
        }
        send(client,msg,100,0);

    }
    close (*((int*)pclient));
    *((int*)pclient)=-1;
    return NULL;
}

//============================================

void* deschide_thread3(void*)
{
    sd1=init_sd(PORT2);
    struct sockaddr_in from;
    int length = sizeof(from);
    int client1[BACKLOG];
    for(int i=0;i<BACKLOG;i++)
        client1[i]=-1;
    bzero (&from, sizeof (from));
    while(true)
    {   
        int sk = accept(sd1,(sockaddr*) &from,(socklen_t *) &from);
        
        
        if (sk == -1)
	    {
            if(sd!=-1)
	            cout<<"[Thread3]Eroare la accept()!"<<endl;
            else
            {
                cout<<"[Thread3]Serverul s-a inchis!"<<endl;
                exit(0);
            }
            
	    }
        for(int i=0;i<BACKLOG;i++)
            if(client1[i]==-1)
            {
                client1[i]=sk;
                pthread_t thread3;
                pthread_create(&thread3,NULL,fncthread3,(void*)&client1[i]);
                break;
            }

    }
    return NULL;
}



//============================================

void* handle_connection(void* pclient)
{
    int k;
    int client=*((int*)pclient);
    char msg[100];
    recv(client,msg,9,0);
    for(int i=0;i<=BACKLOG;i++)
        if(clienti[i].sd==client)
            strcpy(clienti[i].nume,msg);
    for(int i=0;i<=BACKLOG;i++)
                if(clienti[i].sd==client)
                {
                    k=i;
                    cout<<endl<<"--Clientul "<<clienti[i].nume<<" s-a conectat!--"<<endl;
                }
    while(true)
    {
        
        bzero (msg,100);
        
        cout<<endl<<"Se asteapta mesajul.."<<endl;

        recv(client,msg,100,0);

        if(strcmp(msg,"quit")==0)
        {
            for(int i=0;i<=BACKLOG;i++)
                if(clienti[i].sd==client)
                    {cout<<"--Clientul "<<clienti[i].nume<<" s-a deconectat!--"<<endl;break;}
            break;
        }
        
        cout<<"["<<clienti[k].nume<<"] mesaj primit: "<<msg<<endl;

        raspunde(msg,client);

    }
    close (*((int*)pclient));
    *((int*)pclient)=-1;
    return NULL;
}
                                                           //SERVER

//============================================

int main()
{

    struct sockaddr_in from;
    int length = sizeof(from);
    init_file();
    sd = init_sd(PORT1);
    pthread_t t1;
    pthread_create(&t1,NULL,deschide_thread3,NULL);

    
    for(int i=0;i<BACKLOG;i++)
        {
            clienti[i].sd=-1;
            bzero(clienti[i].nume,8);
        }
    bzero (&from, sizeof (from));
    while(true)
    {   
        int sk = accept(sd,(sockaddr*) &from,(socklen_t *) &from);
        
        
        if (sk == -1)
	    {
            if(sd!=-1)
	            cout<<"Eroare la accept()!"<<endl;
            else
            {
                cout<<"Serverul s-a inchis!"<<endl;
                exit(0);
            }
            
	    }
        for(int i=0;i<BACKLOG;i++)
            {cout<<"clienti["<<i<<"]= "<<clienti[i].sd<<endl; 
            if(clienti[i].sd==-1)
            {
                clienti[i].sd=sk;
                pthread_t t;
                pthread_create(&t,NULL,handle_connection,(void*)&clienti[i].sd);
                break;
            }}

    }
    
    
}
 
            


































