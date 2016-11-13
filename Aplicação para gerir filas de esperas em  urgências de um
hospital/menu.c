#include <stdio.h>
#include <stdlib.h>
#include "Estruturas.h"

int menu(head lista_tri,head lista_verde,head lista_amarela,head lista_vermelha,head lista_global)
{
    int bi,pul_apagado;
    printf(" ---------------------------------------------------------\n|                    Sistema de Urgencia                  |\n ---------------------------------------------------------\n");
    printf("1 - Admitir um novo Doente\n");
    printf("2 - Atribuir prioridade a um Doente\n");
    printf("3 - Cancelar um doente em espera para Triagem\n");
    printf("4 - Cancelar um doente em espera para Consulta\n");
    printf("5 - Lista de doentes Triagem \n");
    printf("6 - Listar os doentes Consulta\n");
    printf("7 - Atender Doente\n");
    printf("8 - Sair\n");
    int ordem = escolha();
    switch(ordem)
    {
    case 1:
        system("cls");
        insere_na_lista_tri(lista_tri);
        retorna_menu1();
        break;
    case 2:
        system("cls");
        menu_2(lista_global,lista_tri,lista_verde,lista_amarela,lista_vermelha);
        break;
    case 3:
        system("cls");
        Imprime_lista_doente(lista_tri);
        if (lista_vazia(lista_tri))
        {
            retorna_menu1();
            break;
        }
        printf("Qual o bi do Doente que pretende eliminar:\n");
        scanf("%d",&bi);
        getchar();
        elimina_da_lista_bi(lista_tri,bi);
        printf("\n");
        retorna_menu1();
        break;
    case 4:
        system("cls");
        Imprime_lista_doente2(lista_global);
        if (lista_vazia(lista_global)==1)
        {
            retorna_menu1();
            return 1;
        }
        printf("Qual o bi do Doente que pretende eliminar:\n");
        scanf("%d",&bi);
        getchar();
        pul_apagado=elimina_da_lista_bi(lista_global,bi);
        switch(pul_apagado)
        {
        case 1:
            elimina_da_lista_bi2(lista_verde,bi);
            break;
        case 2:
            elimina_da_lista_bi2(lista_amarela,bi);
            break;
        case 3:
            elimina_da_lista_bi2(lista_vermelha,bi);
            break;
        default:
            break;
        }
        printf("\n");
        retorna_menu1();
        break;
    case 5:
        system("cls");
        Imprime_lista_doente(lista_tri);
        retorna_menu1();
        break;
    case 6:
        system("cls");
        printf("Pretende imprimir que lista de Prioridade? (Os mais Antigos primeiro)\n 1-Verde\n 2-Amarela\n 3-Vermelha\n 4-Todas\n 5-Imprimir ao contrario (Os mais Recentes primeiro)\n");
        int pri = escolha();
        system("cls");
        switch(pri)
        {
        case 1:
            Imprime_lista_doente2(lista_verde);
            break;
        case 2:
            Imprime_lista_doente2(lista_amarela);
            break;
        case 3:
            Imprime_lista_doente2(lista_vermelha);
            break;
        case 4:
            if(lista_vazia(lista_global))
            {
                printf("Nao esta ninguem na Consulta!\n!");
                break;
            }
            Imprime_lista_por_ordem(lista_verde);
            Imprime_lista_por_ordem(lista_amarela);
            Imprime_lista_por_ordem(lista_vermelha);
            break;
        case 5:

            printf("Lista que pretende imprimir ao contrario?\n 1-Verde\n 2-Amarela\n 3-Vermelha\n");
            int qual =escolha();
            system("cls");
            switch(qual)
            {
            case 1:
                if (lista_vazia(lista_verde))
                {
                    printf("Nao esta ninguem na Consulta!\n");
                    break;
                }
                imprime_contrario(lista_verde);
                break;
            case 2:
                if (lista_vazia(lista_amarela))
                {
                    printf("Nao esta ninguem na Consulta!\n");
                    break;
                }
                imprime_contrario(lista_amarela);
                break;
            case 3:
                if (lista_vazia(lista_vermelha))
                {
                    printf("Nao esta ninguem na Consulta!\n");
                    break;
                }
                imprime_contrario(lista_vermelha);
                break;
            default:
                printf("Opcao invalida!\n");
            }
            break;
        default:
            printf("Opcao invalida!\n");
        }
        retorna_menu1();
        break;
    case 7:
        system("cls");
        if (lista_vazia(lista_global))
        {
            printf("Nao existe ninguem para Atendimento!\n");
            retorna_menu1();
            return ;
        }
        proximo_cliente(lista_vermelha,lista_amarela,lista_verde,lista_global);
        retorna_menu1();
        break;
    case 8:
        break;
    default:
        printf("Opcao invalida!\n");
    }
    apaga_ficheiro("Total.txt");
    lista_para_ficheiro(lista_tri,"Total.txt");
    lista_para_ficheiro(lista_global,"Total.txt");
    if (ordem == 8)
    {
        system("cls");
        printf("Tem a cereteza que pretende sair?(S/N)\n");
        char c;
        scanf("%c",&c);

        if (c == 'S' || c == 's')
            return 8;
        else
        {
            system("cls");
            return 1;
        }
    }
    else
        return 1;
}
void menu_2(head lglobal,head lista_tri,head lista_verde,head lista_amarela,head lista_vermelha)
{
    head doente_triagem = lista_tri->next;
    int atribuir;
    int cond = Imprime_doente(lista_tri);
    if (cond == 1)
    {
        apaga_ficheiro("Total.txt");
        printf("\n\t\t\t Prioridade \n\n\t1 - Verde \t2 - Amarela \t3 - Vermelha\n\nAtribuir:\n");
        scanf("%d",&atribuir);
        switch(atribuir)
        {
        case 1:
            insere_doente_fim(lglobal,doente_triagem,atribuir);
            insere_doente_fim(lista_verde,doente_triagem,atribuir);
            ajusta_primeiro(lista_tri);
            retorna_menu1();
            break;
        case 2:
            insere_doente_fim(lglobal,doente_triagem,atribuir);
            insere_doente_fim(lista_amarela,doente_triagem,atribuir);
            ajusta_primeiro(lista_tri);
            retorna_menu1();
            break;
        case 3:
            insere_doente_fim(lglobal,doente_triagem,atribuir);
            insere_doente_fim(lista_vermelha,doente_triagem,atribuir);
            ajusta_primeiro(lista_tri);
            retorna_menu1();
            break;
        default:
            printf("Atribuicao invalida");
        }
        lista_para_ficheiro(lista_tri,"Total.txt");
        lista_para_ficheiro(lglobal,"Total.txt");
    }
}
