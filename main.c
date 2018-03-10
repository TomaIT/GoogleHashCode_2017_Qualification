#include <stdio.h>
#include <stdlib.h>
#include "PQueue.h"

typedef struct{
    int *cache; //indicizzato con id server(dimensione nServer) contiene latenza
    int *idCache; //elenco id server collegati
    int sizeCache; //numero server collegati
    int ldc; //latenza con datacenter
    int *video; //Indicizzato con id video(dimensione nVideo) contiente numero richieste
    int *idVideo; //elenco id video richiesti
    int sizeVideo; //numero di video diversi richiesti
}Endpoint;

typedef struct{
    int *flagVideo; //indica se il video è presente (dimensione nVideo)
    int *idVideo; //elenco di idVideo presenti nel server
    int sizeIdVideo; //numero video presenti
    int cap; //capacità del server
}Server;

typedef struct{
    int *idEndp; //contiene elenco di idEndp tali da essere collegati (sia per request che per edge) con un determinato server video
    int sizeIdEndp; //numero di endpoint collegati
}Support;


int nVideo,nEndp,nReq,nServer;
int *video;
Endpoint *endp;
Server *server;
PQueue pq;
double **serverVideo_Gain; //contiene parametro guadagno (indicizzato su [idServer][idVideo])
int **serverVideoChanged; //Indica serverVideo_Gain se è cambiato
int **flagServerVideo; //Indica elemento [idServer][idVideo] già estratto
Support **serverVideoEndp; //Indica lista di endpoint collegati ad un determinato indicizzato su [idServer][idVideo]
Item **itemLink; //Contiene puntatori ad Item


void readFile(char *s);
void init_serverVideo_Gain();
void init_serverVideoEndp();
void calcola_serverVideo_Gain();
void insert_in_pqueue();
void insert_video_updating_in_server_from_pqueue();
void print_solution(FILE *f);
float get_score();
void free_all();

int main(int argc,char **argv){
    int i;
    FILE *fout;
    char nameFOut[300];
    float score,scoreTot=0;
    for(i=1;i<argc;i++){
        printf("File: %s\n",argv[i]);
        readFile(argv[i]);
        init_serverVideoEndp();
        init_serverVideo_Gain();
        pq=pqueue_init(nServer*nVideo);
        calcola_serverVideo_Gain();
        insert_in_pqueue();
        insert_video_updating_in_server_from_pqueue();
        score=get_score()*1000;
        sprintf(nameFOut,"%s_%d.sol",argv[i],(int)score);
        fout=fopen(nameFOut,"w");
        print_solution(fout);
        fclose(fout);
        printf("Score: %.1f\n",score);
        scoreTot+=score;
        free_all();
        pqueue_free(pq);
    }
    printf("\nScoreTot: %.1f\n",scoreTot);
    return 0;
}


void free_all(){
    int i,j;
    for(i=0;i<nServer;i++){
        free(serverVideo_Gain[i]);
        free(serverVideoChanged[i]);
        for(j=0;j<nVideo;j++){
            free(serverVideoEndp[i][j].idEndp);
            item_free(itemLink[i][j]);
        }
        free(itemLink[i]);
        free(serverVideoEndp[i]);
        free(flagServerVideo[i]);
    }
    free(itemLink);
    free(serverVideoChanged);
    free(serverVideoEndp);
    free(serverVideo_Gain);
    free(flagServerVideo);
    free(video);
    for(i=0;i<nEndp;i++){
        free(endp[i].cache);
        free(endp[i].idCache);
        free(endp[i].idVideo);
        free(endp[i].video);
    }
    free(endp);
    for(i=0;i<nServer;i++){
        free(server[i].flagVideo);
        free(server[i].idVideo);
    }
    free(server);
}

void init_serverVideoEndp(){
    int i,j,k;
    int idServ,idVid;
    serverVideoEndp=malloc(sizeof(Support*)*nServer);
    if(serverVideoEndp==NULL)
        exit(-50);
    for(i=0;i<nServer;i++){
        serverVideoEndp[i]=malloc((sizeof(Support)*nVideo));
        if(serverVideoEndp[i]==NULL)
            exit(-51);
        for(j=0;j<nVideo;j++)
            serverVideoEndp[i][j].sizeIdEndp=0;
    }
    for(i=0;i<nEndp;i++)
        for(j=0;j<endp[i].sizeVideo;j++)
            for(k=0;k<endp[i].sizeCache;k++){
                idServ=endp[i].idCache[k];
                idVid=endp[i].idVideo[j];
                serverVideoEndp[idServ][idVid].sizeIdEndp++;
            }
    for(i=0;i<nServer;i++)
        for(j=0;j<nVideo;j++){
            serverVideoEndp[i][j].idEndp=malloc(sizeof(int)*serverVideoEndp[i][j].sizeIdEndp);
            if(serverVideoEndp[i][j].idEndp==NULL)
                exit(-52);
            serverVideoEndp[i][j].sizeIdEndp=0;
        }
    for(i=0;i<nEndp;i++)
        for(j=0;j<endp[i].sizeVideo;j++)
            for(k=0;k<endp[i].sizeCache;k++){
                idServ=endp[i].idCache[k];
                idVid=endp[i].idVideo[j];
                serverVideoEndp[idServ][idVid].idEndp[serverVideoEndp[idServ][idVid].sizeIdEndp]=i;
                serverVideoEndp[idServ][idVid].sizeIdEndp++;
            }
}

float get_score(){
    int i,j,k,minLatency;
    uint64_t score=0,totRequest=0;
    double ret;
    for(i=0;i<nEndp;i++){
        for(j=0;j<endp[i].sizeVideo;j++){
            minLatency=endp[i].ldc;
            for(k=0;k<endp[i].sizeCache;k++){
                if((server[endp[i].idCache[k]].flagVideo[endp[i].idVideo[j]])&&(minLatency>endp[i].cache[endp[i].idCache[k]])){
                    minLatency=endp[i].cache[endp[i].idCache[k]];
                }
            }
            score+=(uint64_t)endp[i].video[endp[i].idVideo[j]]*(endp[i].ldc-minLatency);
            totRequest+=endp[i].video[endp[i].idVideo[j]];
        }
    }
    ret=(double)score/totRequest;
    return (float)ret;
}

void print_solution(FILE *f){
    int i,j;
    fprintf(f,"%d\n",nServer);
    for(i=0;i<nServer;i++){
        fprintf(f,"%d ",i);
        for(j=0;j<server[i].sizeIdVideo;j++)
            fprintf(f,"%d ",server[i].idVideo[j]);
        fprintf(f,"\n");
    }
}

void update_serverVideo_Gain(Item item){
    int i,k,x;
    for(x=0;x<serverVideoEndp[item->idServer][item->idVideo].sizeIdEndp;x++){
        i=serverVideoEndp[item->idServer][item->idVideo].idEndp[x];
        for(k=0;k<endp[i].sizeCache;k++)
            if(!flagServerVideo[endp[i].idCache[k]][item->idVideo]){
                serverVideo_Gain[endp[i].idCache[k]][item->idVideo]-=
                        (double)endp[i].video[item->idVideo]*(endp[i].ldc-endp[i].cache[endp[i].idCache[k]]);
                serverVideoChanged[endp[i].idCache[k]][item->idVideo]=1;
            }
    }
}

void insert_video_updating_in_server_from_pqueue(){
    Item item;
    int i,j;
    while(!pqueue_is_empty(pq)){
        do{
            if(pqueue_is_empty(pq))
                return;
            item=pqueue_extract(pq);
            flagServerVideo[item->idServer][item->idVideo]=1;
        }while(server[item->idServer].cap<video[item->idVideo]);
        server[item->idServer].idVideo[server[item->idServer].sizeIdVideo]=item->idVideo;
        server[item->idServer].flagVideo[item->idVideo]=1;
        server[item->idServer].sizeIdVideo++;
        server[item->idServer].cap-=video[item->idVideo];
        update_serverVideo_Gain(item);
        for(i=0;i<nServer;i++)
            for(j=0;j<nVideo;j++)
                if(serverVideoChanged[i][j]){
                    serverVideoChanged[i][j]=0;
                    pqueue_change(pq,itemLink[i][j],serverVideo_Gain[i][j]/video[j]);
                }
    }
}

void insert_in_pqueue(){
    int i,j;
    for(i=0;i<nServer;i++)
        for(j=0;j<nVideo;j++){
            itemLink[i][j]=item_init(i,j,serverVideo_Gain[i][j]/video[j]);
            if(!flagServerVideo[i][j])
                pqueue_insert(pq,itemLink[i][j]);
        }

}

void calcola_serverVideo_Gain(){
    int i,j,k;
    for(i=0;i<nEndp;i++)
        for(j=0;j<endp[i].sizeVideo;j++)
            for(k=0;k<endp[i].sizeCache;k++)
                serverVideo_Gain[endp[i].idCache[k]][endp[i].idVideo[j]]+=
                        (double)endp[i].video[endp[i].idVideo[j]]*(endp[i].ldc-endp[i].cache[endp[i].idCache[k]]);
    for(i=0;i<nServer;i++)
        for(j=0;j<nVideo;j++)
            if(serverVideo_Gain[i][j]<=0)
                flagServerVideo[i][j]=1;
}

void init_serverVideo_Gain(){
    int i;
    serverVideoChanged=malloc(sizeof(int*)*nServer);
    serverVideo_Gain=malloc(sizeof(double*)*nServer);
    flagServerVideo=malloc(sizeof(int*)*nServer);
    itemLink=malloc(sizeof(Item*)*nServer);
    if(serverVideo_Gain==NULL||flagServerVideo==NULL||serverVideoChanged==NULL||itemLink==NULL)
        exit(-11);
    for(i=0;i<nServer;i++){
        serverVideo_Gain[i]=calloc(sizeof(double),nVideo);
        serverVideoChanged[i]=calloc(sizeof(double),nVideo);
        flagServerVideo[i]=calloc(sizeof(int),nVideo);
        itemLink[i]=malloc(sizeof(Item)*nVideo);
        if(serverVideo_Gain[i]==NULL||flagServerVideo[i]==NULL||serverVideoChanged[i]==NULL||itemLink[i]==NULL)
            exit(-12);
    }
}

void readFile(char *s){
    FILE *f;
    int i,t,a,b,j;
    f=fopen(s,"r");
    if(f==NULL)
        exit(-1);
    fscanf(f,"%d %d %d %d %d",&nVideo,&nEndp,&nReq,&nServer,&a);
    video=malloc(sizeof(int)*nVideo);
    endp=malloc(sizeof(Endpoint)*nEndp);
    server=malloc(sizeof(Server)*nServer);
    if(video==NULL||endp==NULL||server==NULL)
        exit(-2);
    for(i=0;i<nServer;i++){
        server[i].idVideo=malloc(sizeof(int)*nVideo);
        server[i].flagVideo=calloc(sizeof(int),nVideo);
        if(server[i].idVideo==NULL||server[i].flagVideo==NULL)
            exit(-5);
        server[i].sizeIdVideo=0;
        server[i].cap=a;
    }
    for(i=0;i<nVideo;i++)
        fscanf(f,"%d",&video[i]);
    for(i=0;i<nEndp;i++){
        fscanf(f,"%d %d",&endp[i].ldc,&endp[i].sizeCache);
        endp[i].cache=calloc(sizeof(int),nServer);
        endp[i].idCache=malloc(sizeof(int)*endp[i].sizeCache);
        endp[i].video=calloc(sizeof(int),nVideo);
        if(endp[i].cache==NULL||endp[i].idCache==NULL||endp[i].video==NULL)
            exit(-3);
        for(j=0;j<endp[i].sizeCache;j++){
            fscanf(f,"%d %d",&endp[i].idCache[j],&t);
            endp[i].cache[endp[i].idCache[j]]=t;
        }
    }
    for(i=0;i<nReq;i++){
        fscanf(f,"%d %d %d",&t,&a,&b);
        endp[a].video[t]+=b;
    }
    for(i=0;i<nEndp;i++){
        for(j=0,endp[i].sizeVideo=0;j<nVideo;j++)
            if(endp[i].video[j]!=0)
                endp[i].sizeVideo++;
        endp[i].idVideo=malloc(sizeof(int)*endp[i].sizeVideo);
        if(endp[i].idVideo==NULL)
            exit(-4);
        for(j=0,a=0;j<nVideo;j++)
            if(endp[i].video[j]){
                endp[i].idVideo[a]=j;
                a++;
            }
    }
    fclose(f);
}
