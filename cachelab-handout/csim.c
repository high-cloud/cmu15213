// writed by yyd

#include "cachelab.h"
#include<math.h>
#include<stdint.h>
#include<stdbool.h>
#include<limits.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#define bool _Bool

int g_s,g_b,g_E,g_S,g_B; // number set by user
int miss=0,hit=0,eviction=0;
int s_mask,b_mask,t_shift;


typedef struct block
{
    bool valid; // if true, was filled
    int time; // how many times havent been touch
    uint64_t tag;
}block;

int size_block=sizeof(block);

void setup(int s,int b,int E);
void load(block* cache,uint64_t address);
void checkIn(block * cache,uint64_t tag);
void save(block* cache,uint64_t address);
void modify(block* cache,uint64_t address);
void dealFile(FILE* fp);


int main(int argc,char** argv)
{
    int s,b,E;
    char filePath[30];
    for(int i=0;i<argc;++i)
    {
        if(strcmp(argv[i],"-s")==0)
        {
            s=atoi(argv[i+1]);
        }

        if(strcmp(argv[i],"-b")==0)
        {
            b=atoi(argv[i+1]);
        }
        if(strcmp(argv[i],"-E")==0)
        {
            E=atoi(argv[i+1]);
        }
        if(strcmp(argv[i],"-t")==0)
        {
            strcpy(filePath,argv[i+1]);
        }
    }

    setup(s,b,E);

    FILE *fp=fopen(filePath,"r");
    dealFile(fp);


    printSummary(hit,miss,eviction);
    return 0;
}

void dealFile(FILE* fp)
{
    block* cache=(block*) malloc(size_block*g_E*g_S);

    // initialize
    for(int i=0;i<g_E*g_S;++i)
    {
        (cache+i)->valid=0;
    }
    char str[100];
    unsigned int address;
    while(fgets(str,100,fp)!=0)
    {
        if(str[0]=='I')
            continue;
        
        if(str[1]=='L')
        {
            sscanf(strtok(str+3,","),"%x",&address);
            printf("\n %c %s",str[1],&str[3]);
            load(cache,address);
        }
        if(str[1]=='S')
        {
            sscanf(strtok(str+3,","),"%x",&address);
            printf("\n %c %s",str[1],&str[3]);
            save(cache,address);
        }
        if(str[1]=='M')
        {
            sscanf(strtok(str+3,","),"%x",&address);
            printf("\n %c %s",str[1],&str[3]);
            modify(cache,address);
        }
    }
}

void setup(int s,int b,int E)
{
    g_s=s;
    g_b=b;
    g_E=E;
    g_S=1<<s;
    g_B=1<<b;
    s_mask=g_S-1;
    t_shift=s+b;
}

void load(block* cache,uint64_t address)
{
    int index_set=s_mask&(address>>g_b);
    cache+=index_set*g_E;
    checkIn(cache,address>>t_shift);
}

void checkIn(block * cache,uint64_t tag)
{
    int max_time=0;
    int i=0,j=0;
    block* tmp=cache;
    for(;i<g_E;++i,tmp++)
    {
        if(tmp->valid)
        {
            if(tmp->tag==tag)
            {
                j=i; // a sign indicates hit
                break;
            }
            else
            {
                tmp->time++;
                if(max_time<tmp->time)
                {
                    j=i;
                    max_time=tmp->time;
                }
            }
        }
        else
        {
            max_time=INT_MAX;
            j=i;
        }
    }

    // judge whether hit
    if(j==i)
    {
        ++hit;
        printf(" hit");
    }
    else
    {
        ++miss;
        printf(" miss");
        // whether need evict
        if(max_time!=INT_MAX)
        {
            ++eviction;
            printf(" eviction");
        }
    }
    tmp=cache+j;
    tmp->time=0;
    tmp->valid=1;
    tmp->tag=tag;

    // if hit, need update times of blocks left
    for(++i;i<g_E;++i)
    {
        tmp=cache+i;
        tmp->time++;
    }
}

void save(block* cache,uint64_t address)
{
    load(cache,address);
}

void modify(block* cache,uint64_t address)
{
    load(cache,address);
    save(cache,address);
}