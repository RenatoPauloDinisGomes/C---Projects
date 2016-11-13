#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Estruturas.h"
int main()
{
    int cond;
    head lista_verde = cria_lista();
    head lista_amarela = cria_lista();
    head lista_vermelha = cria_lista();
    head lista_global = cria_lista();
    head lista_tri=cria_lista();
    verifica_ficheiros();
    ficheiro_para_listas(lista_global,lista_tri,lista_verde,lista_amarela,lista_vermelha,"Total.txt");
    do
    {
        cond = menu(lista_tri,lista_verde,lista_amarela,lista_vermelha,lista_global);
    }
    while(cond!=8);

    descarta(lista_tri);
    descarta(lista_verde);
    descarta(lista_amarela);
    descarta(lista_vermelha);
    descarta(lista_global);
}
