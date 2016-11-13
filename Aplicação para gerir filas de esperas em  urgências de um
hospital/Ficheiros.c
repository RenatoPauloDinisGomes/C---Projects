#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Estruturas.h"

void verifica_ficheiros()
{
    if (access("Total.txt",F_OK)==-1)
    {
        FILE *f1;
        f1=fopen("Total.txt","w");
        fclose(f1);
    }
    if (access("Contagem.txt",F_OK)==-1)
    {
        FILE *f2;
        f2=fopen("Contagem.txt","w");
        fclose(f2);
    }
}
void ficheiro_para_listas(head lglobal,head ltri, head lverde, head lamarela, head lvermelha, char *fname)
{
    FILE *f;
    f=fopen(fname,"r");
    int pos=0;
    while(fgetc(f) != EOF)
    {
        fseek(f,pos,SEEK_SET);
        Doente *aux = malloc(sizeof(Doente));
        fscanf(f," %d %d/%d/%d %d:%d:%d %d %s",&aux->bi,&aux->data.dia,&aux->data.mes,&aux->data.ano,&aux->data.hora,&aux->data.min,&aux->data.sec,&aux->pul,&aux->nome);
        if (aux->pul==1)
        {
            insere_doente_fim(lverde,aux,aux->pul);
            insere_doente_fim(lglobal,aux,aux->pul);
        }
        else if(aux->pul==2)
        {
            insere_doente_fim(lglobal,aux,aux->pul);
            insere_doente_fim(lamarela,aux,aux->pul);
        }

        else if(aux->pul==3)
        {
            insere_doente_fim(lglobal,aux,aux->pul);
            insere_doente_fim(lvermelha,aux,aux->pul);
        }
        else if (aux->pul==0)
            insere_doente_fim(ltri,aux,aux->pul);
        pos=ftell(f);
    }
    fclose(f);
}
void apaga_ficheiro(char *fname)
{
    FILE *f;
    f=fopen(fname,"w");
    fclose(f);
}
void lista_para_ficheiro(head lista, char *fname)
{
    FILE *f;
    f=fopen(fname,"a");
    head atual;
    atual=lista->next;
    while(atual!=NULL)
    {
        fprintf(f," %d %d/%d/%d %d:%d:%d %d %s",atual->bi,atual->data.dia,atual->data.mes,atual->data.ano,atual->data.hora,atual->data.min,atual->data.sec,atual->pul,atual->nome);
        atual=atual->next;
    }
    fclose(f);
}
void Doente_para_ficheiro(head lista,char *fname)
{
    FILE *f;
    f=fopen(fname,"a");
    head atual;
    atual = lista;
    fprintf(f," %d %d/%d/%d %d:%d:%d %d %s",atual->bi,atual->data.dia,atual->data.mes,atual->data.ano,atual->data.hora,atual->data.min,atual->data.sec,atual->pul,atual->nome);
    fclose(f);
}
