#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Estruturas.h"

void insere_doente_fim(head prioridade, head espera, int cor )
{
    head ptr = prioridade;
    head no = (head)malloc(sizeof(Doente));
    *no = *espera;
    if (cor ==1)
        no->pul=1;
    else if (cor == 2)
        no->pul=2;
    else if (cor == 3)
        no->pul=3;
    while(ptr->next != NULL)
    {
        ptr = ptr->next;
    }
    no->next = NULL;
    ptr->next = no;
}
void ajusta_primeiro(head lista_tri)
{
    head lembra;
    if(lista_tri->next != NULL)
    {
        lembra=lista_tri->next;
        lista_tri->next = lista_tri->next->next;
        free(lembra);
    }
    else
    {
        lista_tri->next = NULL;
    }
}
void insere_doente_inicio(head lista, Doente *it)
{
    head no;
    head anterior, atual;
    no = (head)malloc(sizeof(Doente));
    atual=lista->next;
    anterior=lista;
    if (no!=NULL)
    {
        no->next=atual;
        anterior->next=no;
    }
}
void proximo_cliente(head lista_vermelha, head lista_amarela, head lista_verde, head lista_global)
{
    int contavermelho, contaamarelo;
    FILE *f;
    f=fopen("contagem.txt", "r");
    fscanf(f,"%d %d", &contavermelho,&contaamarelo);
    fclose(f);

    if(lista_vazia(lista_verde) == 0 && ((contavermelho == 15 && lista_vazia(lista_amarela) != 0) || contaamarelo == 5 || (lista_vazia(lista_amarela) != 0 && lista_vazia(lista_vermelha)!=0)))
    {
        printf("====================================================\n");
        contaamarelo=0;
        contavermelho=0;
        atender(lista_global,lista_verde);
    }
    else if (lista_vazia(lista_amarela) == 0 && (contavermelho == 3 || contavermelho == 6 || contavermelho == 9 || contavermelho == 12 || contavermelho == 15 || lista_vazia(lista_vermelha)!=0))
    {
        printf("====================================================\n");
        contaamarelo++;
        contavermelho=0;
        atender(lista_global,lista_amarela);
    }
    else
    {
        printf("====================================================\n");
        contavermelho++;
        atender(lista_global,lista_vermelha);
    }
    f=fopen("contagem.txt","w");
    fprintf(f,"%d %d", contavermelho, contaamarelo);
    fclose(f);
}
void atender(head lglobal,head prioridade)
{
    int bi;
    bi = eliminar_no(prioridade);
    elimina_da_lista_bi(lglobal,bi);
}
int eliminar_no(head prioridade)
{
    head t = prioridade;
    head ptr = t->next;

    int bi;

    if(ptr->next != NULL)
    {
        printf("Paciente atendido:\nNome: %s\nBI: %d\n",ptr->nome,ptr->bi);
        bi = ptr->bi;
        ptr = ptr->next;
        t->next = ptr;
        return(bi);
    }
    else
    {
        printf("Paciente atendido:\nNome: %s\nBI: %d\n",ptr->nome,ptr->bi);
        bi = ptr->bi;
        ptr = NULL;
        t->next = ptr;
        return(bi);
    }
    return 1;
}
