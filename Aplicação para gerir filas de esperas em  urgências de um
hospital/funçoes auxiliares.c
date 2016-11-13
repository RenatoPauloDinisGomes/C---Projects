#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Estruturas.c"

head cria_lista(void)
{
    head aux;
    aux =(head)malloc(sizeof(Doente));
    if (aux != NULL)
    {
        aux->nome = "";
        aux->bi = 0;
        aux->data.hora = 0;
        aux->data.min = 0;
        aux->data.sec = 0;
        aux->next = NULL;
    }
    return aux;
}
void insere_lista (head lista_tri)
{
    int bi,hora,min,sec;
    head no;
    no = (head)malloc(sizeof(Doente));
    if (no != NULL)
    {
        printf("Nome:\n");
        fgets(no->nome,NUM,stdin);
        printf("Bi:\n");
        scanf("%d",&bi);
        printf("Hora de entrada:(h m s):\n");
        scanf("%d %d %d",&hora,&min,&sec);
        no->bi = bi;
        no->data.hora = hora;
        no->data.min = min;
        no->data.sec = sec;
        lista_tri->next = no;
        no->next = NULL;
    }
}
