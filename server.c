
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <ctype.h>

#define MQCONVSRV_QUEUE_KEY	10050

#define LOGIN_REQ		0
#define LOGOUT_REQ		20
#define SEND 11
#define MSGS 12
#define POST 13
#define SHOW 14
#define DELPOST 16
#define DELPOSTS 17
#define USERS 18
#define MYID 19
#define STOP_SERVER_REQ		99
#define MAX_MENSAGENS 500
#define OK_RESP			0
#define ERROR_RESP		1
#define ALREADY_LOG_RESP	2
#define CANNOT_LOG_RESP		3
#define INVALID_REQ_RESP	4
#define MAXSIZE_MSGDATA 	2000
#define SERVER_ID 		1
#define MAXSIZE_USERNAME 	20
#define MAX_USERS 		100


struct request_msg {
 	long	server_id;
 	long	client_id;
   	int	request;
	char	requestdata[MAXSIZE_MSGDATA+1];
	char	requestUser[MAXSIZE_USERNAME+1]; //esta variável leva o nome do usuário, tanto para imprimir quanto para enviar mensagens privadas
	int index; // este é o index para apagarmos alguma mensagem do forum publico 

};

struct response_msg {
	long	client_id;
	int	response;
	char	responsedata[MAXSIZE_MSGDATA+1];
};

struct send{  //struct que vai ser usada na para enviar mensagens privadas para um usuário
	char userName[MAXSIZE_USERNAME+1];
	char mensagem [MAXSIZE_MSGDATA+1];

};

struct send menesagensPrivadas[MAX_MENSAGENS]; //instanciei um vetor com as structs send 
int counSend = 0;
char case1[MAXSIZE_USERNAME+1];//		Estas variáveis são para manipulação do do comando e structs send
char case2[MAXSIZE_USERNAME+1];//
char caseResposta[MAXSIZE_MSGDATA+1];//




struct request_msg req_msg;  //botei essas structs como global para as rotinas do forum
struct response_msg resp_msg;


struct user_record 
{
	long id;
	char name[MAXSIZE_USERNAME+1];
};


struct user_record user_table[MAX_USERS];


//Rotinas Utilitarias


char *safegets(char *buffer, int buffersize)
{
	char *str;
	int slen;

	str = fgets(buffer,buffersize,stdin);

	if (str!=NULL) {
		slen = strlen(str);
		if (slen>0)
			str[slen-1] = '\0';
	}
	return str;
}



//Rotinas de Gerenciamento da Tabela de Usuarios


int initialize_user_table()
{
	int i;
	for (i=0; i<MAX_USERS; i++) {
		user_table[i].id = -1;
		strcpy(user_table[i].name,"");
	}
	return -1;
}

int add_user_to_table(char *name, long id)
{
	int i;
	for (i=0; i<MAX_USERS; i++) {
		if (user_table[i].id <= 0) {
			strncpy(user_table[i].name, name, MAXSIZE_USERNAME);
			user_table[i].id = id;
			return 0;
		}
	}
	return -1;
}

long get_user_id(char *name)
{
	int i;
	for (i=0; i<MAX_USERS; i++) {
		if (user_table[i].id > 0) {
			if (strncmp(user_table[i].name, name, MAXSIZE_USERNAME)==0) {
				return user_table[i].id;
			}
		}
	}
	return -1;
}

int del_user_from_table(char *name)
{
	int i;
	for (i=0; i<MAX_USERS; i++) {
		if (user_table[i].id > 0) {
			if (strncmp(user_table[i].name, name, MAXSIZE_USERNAME)==0) {
				user_table[i].id = -1;
				strcpy(user_table[i].name,"");
				return 0;
			}
		}
	}
	return -1;
}





//Rotinas Do forum

int indexMsgF; // variável global para atribuirmos numeração nas mensagens do forum publico
struct lista{						// esta struct lista segue os principios de uma lista simplesmente encadeada
	char mensagem[MAXSIZE_MSGDATA];
	int  id;
	struct lista *prox;
	char usuario[MAXSIZE_MSGDATA];
	int indexMsg;
	
};


struct lista *aloca(){
	return malloc (sizeof(struct lista));
};


void addItem(struct lista *cabeca){ //funcao para adicionarmos itens na lista
	struct lista *aux,*novo;
	aux = cabeca;
	int id = req_msg.client_id;
	char esteUser[MAXSIZE_MSGDATA];
	strcpy(esteUser,req_msg.requestUser);
	printf("Mensagem recebida para o usuario: %s\n",esteUser);


	while(aux->prox !=NULL){
		aux = aux-> prox;
}
	novo = aloca();
	strcpy(novo->mensagem,req_msg.requestdata);	
	indexMsgF++;
	novo->indexMsg=indexMsgF;
	strcpy(novo->usuario,req_msg.requestUser);
	novo -> prox =NULL;
	aux->prox=novo;

}

 char *imprime(struct lista *ini){ //funcao para imprimirmos itens na lista

struct lista *p;
char indexStr[MAXSIZE_MSGDATA];

char *str_to_ret= malloc (sizeof (char) * MAXSIZE_MSGDATA);
for (p = ini->prox; p != NULL; p = p->prox) {

	if(p->mensagem!=NULL){							//aqui concatenamos as mensagens e indes, quando apagamos não apagamos o index apenas a mensagem
		strcat(str_to_ret,"\n");
		strcat(str_to_ret,p->mensagem);
		strcat(str_to_ret," Index:");
		sprintf(indexStr, "%d", p->indexMsg);
		strcat(str_to_ret,indexStr);
		}
		
	}
 return str_to_ret;

}


void apagaMensagem(struct lista *ini, int index){

struct lista *p;

char *str_to_ret= malloc (sizeof (char) * MAXSIZE_MSGDATA);
for (p = ini->prox; p != NULL; p = p->prox) {

	if(index==p->indexMsg)	

	strcpy(p->mensagem,"");//aqui apagamos a mensagem, lembrando o index continua
}

}










//Rotinas Principal do Servidor



void main(){
	int mqid;
	int stop_server;
	struct lista listaForum; //esta é a lista do forum publico de mensagens
	listaForum.prox=NULL;
	char testeDeUsuario[MAXSIZE_MSGDATA];//utilizamos para testar o login do usuario
	char respostaUsuario[MAXSIZE_MSGDATA]; // utilizamos para enviar a resposta ao cliente
	int usuarios=0; // variavel global que é incrementada ou decrementada cada vez que um usuario entra ou sai
	char usuariosStr[MAXSIZE_MSGDATA]; // passamos para essa string a quantidade de usuarios
	int id;
	


	// faz algumas inicializacoes da fila e de usuarios por exemplo
	mqid = msgget(MQCONVSRV_QUEUE_KEY, IPC_CREAT | 0777);
	if (mqid == -1) {
   		printf("msgget() falhou\n"); exit(1); 
	} 
	printf("Sistema CentralSesion, Servidor Chairman INICIOU\n");
	stop_server = 0;
	initialize_user_table();
	
	while (!stop_server) {
		if (msgrcv(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),SERVER_ID,0)<0) {
			printf("msgrcv() falhou\n"); 
			exit(1);
		}
		printf("servidor: recebeu requisicao %d do cliente %ld\n",req_msg.request,req_msg.client_id);
		// Atende a requisicao
		switch(req_msg.request) {
			case LOGIN_REQ:
				if (get_user_id(req_msg.requestdata)>0) {
					printf("servidor: usuario %s ja' esta' logado\n",req_msg.requestdata);
					resp_msg.response = ALREADY_LOG_RESP;
					strcpy(resp_msg.responsedata,"Usuário ja esta logado\n");
				} else if (add_user_to_table(req_msg.requestdata,req_msg.client_id)==0) {
					printf("servidor: usuario %s aceito e logado\n",req_msg.requestdata);
					resp_msg.response = OK_RESP;
					strcpy(resp_msg.responsedata,"usuario logou\n");
					usuarios++;
				} else {
					printf("servidor: usuario %s aceito mas nao foi possivel logar\n",req_msg.requestdata);
					resp_msg.response = CANNOT_LOG_RESP;
					strcpy(resp_msg.responsedata,"Nao foi possivel logar\n");
				}	
				resp_msg.client_id = req_msg.client_id;
				break;
				
			case LOGOUT_REQ:
				if (del_user_from_table(req_msg.requestdata)==0) {
					printf("servidor: usuario %s deslogado\n",req_msg.requestdata);
					resp_msg.response = OK_RESP;
					usuarios--;
					strcpy(resp_msg.responsedata,"Usuario saiu\n");
				} else {
					printf("servidor: nao foi possivel deslogar usuario %s\n",req_msg.requestdata);
					resp_msg.response = ERROR_RESP;
					strcpy(resp_msg.responsedata,"Usuario nao conseguiu sair\n");
				}	
				resp_msg.client_id = req_msg.client_id;
				break;

				case SEND:
				// teste para saber se o usuário está logado ou não
				
				strcpy(testeDeUsuario,req_msg.requestUser);
				if( get_user_id(testeDeUsuario)==-1){ //testamos de ele está logado 
					printf("Usuário não está logado\n");
					strcpy(respostaUsuario,"Não esta logado\n");// enviamos a resposta ao cliente
				}else{
					printf("Usuário está logado\n");
						strcpy(menesagensPrivadas[counSend].mensagem,req_msg.requestdata);
						strcpy(menesagensPrivadas[counSend].userName,req_msg.requestUser);
						counSend++;
						// adicionamos na struct  a mensagem e o nome que veio pelo send, o contador global é incrementado para que da proxima vez nao utilizemos um espaço ja alocado
				
					
					strcpy(respostaUsuario,"Enviamos sua mensagem\n");
				}
				
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,respostaUsuario,MAXSIZE_MSGDATA);
				break;

				case USERS:
				
				sprintf(usuariosStr, "%d", usuarios); // fazemos aconversão de int para str
				printf("Quantidade de conectados:%d\n",usuarios);
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,usuariosStr,MAXSIZE_MSGDATA);
				break;

			


				case MYID: //comando para verificar ID-PID
				//ele só envia a  mensagem vazia já que o id já esta contido
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,req_msg.requestdata,MAXSIZE_MSGDATA);
				break;

				case DELPOSTS:
				//apagando a lista
				listaForum.prox=NULL; // para apagarmos em uma lista encadeada é só deixar como prox=null
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,req_msg.requestdata,MAXSIZE_MSGDATA);
				break;


				case POST:
				printf("servidor: Vamos postar sua mensagem\n");
				// postando
				addItem(&listaForum); //utilizamos a função já estabelecida anteriormente
				printf("Mensagem postada!\n");
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,req_msg.requestdata,MAXSIZE_MSGDATA);
				break;

				case SHOW:
				printf("servidor: Mensagens do forum\n");
				imprime(&listaForum);//imprimimos com a função ja criada 

				
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,imprime(&listaForum),MAXSIZE_MSGDATA);
				break;

				case DELPOST:
				printf("servidor: Apagaremos alguma mensagem\n");
				apagaMensagem(&listaForum,req_msg.index); //apagamos a mensagem do forum de acordo com a lista requerida

				
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strncpy(resp_msg.responsedata,imprime(&listaForum),MAXSIZE_MSGDATA);
				break;


				case MSGS:
				printf("servidor: Procurando as mensagens do cliente:%s\n",req_msg.requestUser);
			
				printf("Mensagens do cliente:\n");
				for(int i =0; i <MAX_USERS;i++){
					strcpy(case1,menesagensPrivadas[i].userName);//copia para case 1 e case 2  o destinatario da mensagem e o requisitor
					strcpy(case2,req_msg.requestUser);

					if(strcmp(case1,case2)==0){ //se destinatario e requisitor forem os mesmos  ele vai concatenar a mensagem para o usuario
						strcat(caseResposta,"\n");
						strcat(caseResposta,menesagensPrivadas[i].mensagem);
					}
				}

				strncpy(resp_msg.responsedata,caseResposta,MAXSIZE_MSGDATA);
				strcpy(caseResposta,""); // apago esta string para que o proximo não pegue as mensagem do usuario
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				break;

					
			case STOP_SERVER_REQ:
				// Para o servidor
				stop_server = 1;
				// Prepara a mensagem de resposta
				resp_msg.response = OK_RESP;
				resp_msg.client_id = req_msg.client_id;
				strcpy(resp_msg.responsedata,"Stopping server\n");
				break;

				
				
			default:
				resp_msg.response = INVALID_REQ_RESP;
				resp_msg.client_id = req_msg.client_id;
				strcpy(resp_msg.responsedata,"Invalid request\n");
				break;										
		}
		// Envia a resposta ao cliente
		msgsnd(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),0);
		printf("servidor: enviou resposta %d ao cliente %ld\n",resp_msg.response,resp_msg.client_id);
	}

	msgctl(mqid,IPC_RMID,NULL);
	
	exit(0);	
}
