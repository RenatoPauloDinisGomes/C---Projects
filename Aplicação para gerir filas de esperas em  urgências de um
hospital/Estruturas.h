#define NUM 100
/** Estruturas **/

typedef struct time
{
    int dia,mes,ano,hora,min,sec;
} Time;

typedef struct doente *head;
typedef struct doente
{
    char nome[NUM];
    int bi;
    Time data;
    int pul;  /*Pulseira 0(por atribuir), 1(verde), 2(amarela), 3(vermelha)*/
    head next;
} Doente;

head cria_lista(void);
void insere_na_lista_tri (head lista_tri);
int elimina_da_lista_bi2(head lista_tri, int bi);
int elimina_da_lista_bi(head lista_tri, int bi);
void procura_lista (head lista, int chave, head *ant, head
                    *actual);
int Imprime_lista_doente(head lista_tri);
void Imprime_lista_doente2(head lista_at);
int Imprime_doente(head lista_tri);
void liberta_tri(head lista_tri);
void descarta(head list_numeros);
int escolha();
int bi_valido_leght(int bi);
int nome_val(char nome_provisorio[]);
int lista_vazia(head lista);
int arranja_bi2();
void retorna_menu1();
void imprime_lista_pos_triagem(head lista);

void insere_doente_fim(head prioridade, head espera, int cor );
void ajusta_primeiro(head lista_tri);
void insere_doente_inicio(head lista, Doente *it);
void imprime_contrario(head lista);

void AtenderPaciente(head lglobal,head lverde,head lamarela,head lvermelha,int *cont_vermelha,int *cont_amarela,int *cont_verde);
void atender(head lglobal,head prioridade);
int eliminar_no(head prioridade);

void verifica_ficheiros();
void ficheiro_para_lista_global(head lista, char *fname);
void ficheiro_para_listas(head lglobal,head ltri, head lverde, head lamarela, head lvermelha, char *fname);
void apaga_ficheiro(char *fname);
void lista_para_ficheiro(head lista, char *fname);
void Doente_para_ficheiro(head lista,char *fname);
void proximo_cliente(head lista_vermelha, head lista_amarela, head lista_verde, head lista_global);
