#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "Estruturas.h"

head cria_lista(void)
{
    head aux;
    aux =(head)malloc(sizeof(Doente));
    if (aux != NULL)
    {
        aux->next = NULL;
    }
    return aux;
}
void insere_na_lista_tri (head lista_tri)
{
    int bi,i=0;
    head no;
    char nome_provisorio[NUM];
    no = (head)malloc(sizeof(Doente));
    while(lista_tri->next != NULL)
    {
        lista_tri=lista_tri->next;
    }
    if (no != NULL)
    {
        do
        {
            printf("Nome:\n");
            fgets(nome_provisorio,NUM,stdin);
            nome_val(nome_provisorio);
        }
        while(nome_val(nome_provisorio)!=1);
        if (nome_provisorio[0]>='a' && nome_provisorio[0]<='z')
        {
            nome_provisorio[0]=nome_provisorio[0]-'a'+'A';
        }
        do
        {
            no->nome[i]=nome_provisorio[i];
            i++;
        }
        while(nome_provisorio[i]!='\n');
        no->nome[i]='\0';
        bi=arranja_bi2();
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        no->bi = bi;
        no->data.dia = t->tm_mday;
        no->data.mes = t->tm_mon+1;
        no->data.ano = t->tm_year+1900;
        no->data.hora = t->tm_hour;
        no->data.min = t->tm_min;
        no->data.sec = t->tm_sec;
        no->pul=0;
        no->next = NULL;
        Doente_para_ficheiro(no,"Total.txt");
        lista_tri->next = no;
        printf("Doente adicionado.\n");

    }
}
int elimina_da_lista_bi2(head lista_tri, int bi)
{
    head ant;
    int pul_apagado=0;
    ant=lista_tri;
    head atual=lista_tri->next;
    for(;;)
    {
        if(atual->bi == bi)
        {
            ant->next = atual->next;
            pul_apagado=atual->pul;
            free(atual);
            break;
        }
        else if(atual->next == NULL)
        {
            printf("O paciente com o BI %d nao se encontra na lista de espera.\n",bi);
            break;
        }
        ant = atual;
        atual = atual->next;
    }
    return pul_apagado;
}
int elimina_da_lista_bi(head lista_tri, int bi)
{
    head ant;
    int pul_apagado=0;
    ant=lista_tri;
    head atual=lista_tri->next;
    for(;;)
    {
        if(atual->bi == bi)
        {
            printf("Paciente excluido!\n");
            ant->next = atual->next;
            pul_apagado=atual->pul;
            free(atual);

            break;
        }
        else if(atual->next == NULL)
        {
            printf("O paciente com o BI %d nao se encontra na lista de espera.\n",bi);
            break;
        }
        ant = atual;
        atual = atual->next;
    }
    return pul_apagado;
}
void procura_lista (head lista, int chave, head *ant, head
                    *actual)
{
    *ant = lista;
    *actual = lista->next;
    while ((*actual) != NULL && (*actual)->bi < chave)
    {
        *ant = *actual;
        *actual = (*actual)->next;
    }
    if ((*actual) != NULL && (*actual)->bi != chave)
        *actual = NULL;
}
int Imprime_lista_doente(head lista_tri)
{
    int i=0;
    if(lista_vazia(lista_tri) == 1)
    {
        printf("A Triagem encontra-se vazia!\n");
        return 0;
    }
    else
    {
        head copi = lista_tri->next;
        while(copi!=NULL)
        {
            printf("Nome: ");
            i=0;
            do
            {
                printf("%c",copi->nome[i]);
                i++;
            }
            while(copi->nome[i]!='\0');
            printf("\nBi: %d\n",copi->bi);
            printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
            printf("Hora de chegada: %d:%d:%d\n",copi->data.hora,copi->data.min,copi->data.sec);
            copi=copi->next;
        }
        return 1;
    }
}
void Imprime_lista_doente2(head lista_at)
{
    char cor1[]="Verde",cor2[]="Amarelo",cor3[]="Vermelho",*cor;
    int i=0;
    if(lista_vazia(lista_at) == 1)
    {
        printf("A Lista de Consultas encontra-se vazia!\n");
    }
    else
    {
        head copi = lista_at->next;
        while(copi!=NULL)
        {
            printf("Nome: ");
            i=0;
            do
            {
                printf("%c",copi->nome[i]);
                i++;
            }
            while(copi->nome[i]!='\0');
            if (copi->pul == 1)
                cor=cor1;
            else if (copi->pul == 2)
                cor=cor2;
            else if (copi->pul == 3)
                cor=cor3;
            printf("\nBi: %d\t\t\t\t\tPrioridade: %s\n",copi->bi,cor);
            printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
            printf("Hora de chegada: %d:%d:%d\n\n",copi->data.hora,copi->data.min,copi->data.sec);

            copi=copi->next;
        }
    }
}
void Imprime_lista_por_ordem(head lista_at)
{
    char cor1[]="Verde",cor2[]="Amarelo",cor3[]="Vermelho",*cor;
    int i=0;

    head copi = lista_at->next;
    while(copi!=NULL)
    {
        printf("Nome: ");
        i=0;
        do
        {
            printf("%c",copi->nome[i]);
            i++;
        }
        while(copi->nome[i]!='\0');
        if (copi->pul == 1)
            cor=cor1;
        else if (copi->pul == 2)
            cor=cor2;
        else if (copi->pul == 3)
            cor=cor3;
        printf("\nBi: %d\t\t\t\t\tPrioridade: %s\n",copi->bi,cor);
        printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
        printf("Hora de chegada: %d:%d:%d\n\n",copi->data.hora,copi->data.min,copi->data.sec);

        copi=copi->next;

    }
}
int Imprime_doente(head lista_tri)
{
    printf("Nome: ");
    int i=0;
    if(lista_vazia(lista_tri) == 1)
    {
        system("cls");
        printf("A Triagem encontra-se vazia!\n");
        printf("Prima qualquer tecla para voltar ao menu principal!\n");
        getch();
        system("cls");
        return 0;
    }
    else
    {
        head copi = lista_tri->next;
        do
        {
            printf("%c",copi->nome[i]);
            i++;
        }
        while(copi->nome[i]!='\0');
        printf("\nBi: %d\n",copi->bi);
        printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
        printf("Hora de chegada: %d:%d:%d\n",copi->data.hora,copi->data.min,copi->data.sec);
        return 1;
    }
}

void descarta(head list_numeros)
{
    head ant;
    while(list_numeros->next != NULL)
    {
        ant=list_numeros;
        list_numeros=list_numeros->next;
        free(ant);
    }
    free(list_numeros);
}
int escolha()
{
    int ordem;
    scanf("%d",&ordem);
    getchar();
    return ordem;
}
int bi_valido_leght(int bi)
{
    int contador=0;
    while(bi!=0)
    {
        bi=bi/10;
        contador++;
    }
    if (contador!=8)
        printf("BI invalido!\n");
    return contador;
}
int nome_val(char nome_provisorio[])
{
    int i=0,cont=0;
    do
    {
        if ((nome_provisorio[i]>='A' && nome_provisorio[i]<='Z') || (nome_provisorio[i]>='a' && nome_provisorio[i]<='z' || nome_provisorio[i]==' '))
        {
            cont++;
        }
        i++;
    }
    while(nome_provisorio[i]!='\0');
    if (nome_provisorio[0] == ' ' || nome_provisorio[0] == '\n')
        return 0;
    if(cont==i-1)
        return 1;
    else
        return 0;
}
int lista_vazia(head lista)
{
    return (lista->next == NULL ? 1 : 0);
}
int arranja_bi2()
{
    int i,numero_bi=0;
    char bi[NUM];
    printf("Insira o BI ( 8 Digitos ):\n");
    int soma = 8;
    int soma1;
    fgets(bi,sizeof(bi),stdin);
    while(strlen(bi) != 9 || soma != 8)
    {
        soma = 0;
        printf("BI Invalido. Insira de novo:\n");
        fgets(bi,sizeof(bi),stdin);
        for(i=0; i<strlen(bi); i++)
        {
            if(!isdigit(bi[i]))
            {
                soma1 += 1;
                break;
            }
            else
                soma += 1;
        }
    }
    printf("Bi Validado!\n");
    for(i=0; i<8; i++)
        numero_bi=numero_bi*10+bi[i]-'0';
    return numero_bi;
}
void retorna_menu1()
{
    printf("Prima qualquer tecla para voltar ao menu principal!\n");
    getch();
    system("cls");
}
void imprime_lista_pos_triagem(head lista)
{
    head copi = lista->next;
    int i;
    while (copi)
    {

        while(copi!=NULL)
        {
            printf("Nome: ");
            i=0;
            do
            {
                printf("%c",copi->nome[i]);
                i++;
            }
            while(copi->nome[i]!='\n');
            printf("\nBi: %d\n",copi->bi);
            printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
            printf("Hora de chegada: %d:%d\n",copi->data.hora,copi->data.min);
            copi=copi->next;
        }
        copi=copi->next;
    }
}
void imprime_contrario(head lista)
{
    head copi =lista->next;
    if(copi == NULL)
        return;
    imprime_contrario(lista->next);
    printf("Nome: ");
    int i=0;
    do
    {
        printf("%c",copi->nome[i]);
        i++;
    }
    while(copi->nome[i]!='\0');
    printf("\nBi: %d\n",copi->bi);
    printf("Data de entrada: %d/%d/%d\t\t\t",copi->data.dia,copi->data.mes,copi->data.ano);
    printf("Hora de chegada: %d:%d:%d\n",copi->data.hora,copi->data.min,copi->data.sec);
    copi=copi->next;
}
