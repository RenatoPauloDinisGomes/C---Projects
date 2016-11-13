/** \brief
 *
 * \param lista List
 * \return void
 *

void imprime_lista_pre_triagem(List lista){
	List l = lista->next;
	while (l){
		printf("Nome do paciente:%s",l->info->nome);
		printf("Bi do paciente:%d\n",l->info->bi);
		printf("Data de inscricao:%.2d/%.2d/%d\t",l->info->data.dia,l->info->data.mes,l->info->data.ano);
		printf("Hora de inscricao:%.2d:%.2d:%.2d\n\n",l->info->data.hora,l->info->data.min,l->info->data.sec);
		l=l->next;
	}
}

void imprime_lista_pos_triagem(List lista){
	List l = lista->next;
	while (l){
		printf("Nome do paciente:%s",l->info->nome);
		printf("Bi do paciente:%d\n",l->info->bi);
		printf("Data de inscricao:%.2d/%.2d/%d\t",l->info->data.dia,l->info->data.mes,l->info->data.ano);
		printf("Hora de inscricao:%.2d:%.2d:%.2d\n",l->info->data.hora,l->info->data.min,l->info->data.sec);
		int p = l->info->pul;
		switch(p){
			case 1:
				printf("Pulseira Verde.\n\n");
				break;
			case 2:
				printf("Pulseira Amarela.\n\n");
				break;
			case 3:
				printf("Pulseira Vermelha.\n\n");
				break;
		}
		l=l->next;
	}
}

int prioridade_doente(List lista, int bi){
	List atual=lista->next;
	List anterior=lista;
	while(atual!=NULL){
		if(atual->info->bi == bi){
			return atual->info->pul;
		}
		anterior=atual;
		atual=atual->next;
	}
	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include "header.h"

void triagem(List lglobal,List  lespera, List lvermelha, List lamarela, List lverde){
	int p;
	List doente_na_triagem=lespera->next;
	printf("Pulseiras:\n");
	printf("Verde=1  |  Amarela=2  |  Vermelha=3\n");
	do{
		printf("Qual a prioridade que pretende atribuir ao doente %s(%d)?",doente_na_triagem->info->nome,doente_na_triagem->info->bi);
		scanf("%d",&p);
	}while(p>3 || p<1);
	/* Inserir doente na lista global(os mais recentes primeiro)
	doente_na_triagem->info->pul=p;
	insere_doente_inicio(lglobal,doente_na_triagem->info);

	lista_para_ficheiro(lglobal,"lglobal.txt");

	/* VERDE
	if (p==1){
		insere_doente_fim(lverde,lglobal->next->info);
	}
	/* AMARELA
	else if (p==2){
		insere_doente_fim(lamarela,lglobal->next->info);
	}
	/* VERMELHA
	else{
		insere_doente_fim(lvermelha,lglobal->next->info);
	}
	cancelar_doente(lespera,doente_na_triagem->info->bi);

	lista_para_ficheiro(lespera,"lespera.txt");
	#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "header.h"

List cria_lista (void){
	List aux;
	aux = (List) malloc (sizeof (List_node));
	if (aux != NULL) {
		aux->next = NULL;
	}
	return aux;
}

void insere_doente_fim(List lista, Doente *it){
	List no, ant, atual;
	atual=lista->next;
	ant=lista;
	no = (List) malloc (sizeof (List_node));
	no->info = malloc(sizeof(Doente));
	while (atual != NULL){
		ant = atual;
		atual = atual->next;
	}
	if (no!=NULL){
		no->info=it;
		no->next = NULL;
		ant->next = no;
	}
}

void insere_doente_inicio(List lista, Doente *it){
	List no;
	List anterior, atual;
	no = (List) malloc (sizeof (List_node));
	no->info = malloc(sizeof(Doente));

	atual=lista->next;
	anterior=lista;

    if (no!=NULL){
		no->info=it;
		no->next=atual;
		anterior->next=no;
	}
}

void cancelar_doente(List lista, int bi){
	List atual=lista->next;
	List anterior=lista;
	while(atual!=NULL){
		if(atual->info->bi == bi){
			anterior->next=atual->next;
			free(atual);
			break;
		}
		anterior=atual;
		atual=atual->next;
	}
}

void imprime_lista_pre_triagem(List lista){
	List l = lista->next;
	while (l){
		printf("Nome do paciente:%s",l->info->nome);
		printf("Bi do paciente:%d\n",l->info->bi);
		printf("Data de inscricao:%.2d/%.2d/%d\t",l->info->data.dia,l->info->data.mes,l->info->data.ano);
		printf("Hora de inscricao:%.2d:%.2d:%.2d\n\n",l->info->data.hora,l->info->data.min,l->info->data.sec);
		l=l->next;
	}
}

void imprime_lista_pos_triagem(List lista){
	List l = lista->next;
	while (l){
		printf("Nome do paciente:%s",l->info->nome);
		printf("Bi do paciente:%d\n",l->info->bi);
		printf("Data de inscricao:%.2d/%.2d/%d\t",l->info->data.dia,l->info->data.mes,l->info->data.ano);
		printf("Hora de inscricao:%.2d:%.2d:%.2d\n",l->info->data.hora,l->info->data.min,l->info->data.sec);
		int p = l->info->pul;
		switch(p){
			case 1:
				printf("Pulseira Verde.\n\n");
				break;
			case 2:
				printf("Pulseira Amarela.\n\n");
				break;
			case 3:
				printf("Pulseira Vermelha.\n\n");
				break;
		}
		l=l->next;
	}
}

int prioridade_doente(List lista, int bi){
	List atual=lista->next;
	List anterior=lista;
	while(atual!=NULL){
		if(atual->info->bi == bi){
			return atual->info->pul;
		}
		anterior=atual;
		atual=atual->next;
	}
	return 0;
}
sacras:
case 4:
            if(lista_vazia(lespera) == 0){
                printf("\tAtribuir prioridade:\n(1) Verde\n(2) Amarela\n(3) Vermelha\nPrioridade: ");
                scanf("%d",&escolha);
                getchar();
                imprimir_no(lespera->next);
                atribuir2(lglobal,lespera->next);
                p = lglobal;
                while(p->next != NULL){
                    p = p->next;
                }
                switch(escolha){
                    case 1:
                        /*insere_doente(lverde,p);*/
