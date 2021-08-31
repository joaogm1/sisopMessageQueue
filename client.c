
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <unistd.h>


#define MQCONVSRV_QUEUE_KEY	10050
#define LOGIN_REQ		0
#define LOGOUT_REQ		20
#define SEND 11
#define MSGS 12
#define POST	13
#define SHOW 14
#define DELPOST 16
#define DELPOSTS 17
#define USERS 18
#define MYID 19
#define STOP_SERVER_REQ		99
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
	char	requestUser[MAXSIZE_USERNAME+1];
	int index;

};

struct response_msg {
	long	client_id;
	int	response;
	char	responsedata[MAXSIZE_MSGDATA+1];
};


struct user_record 
{
	long id;
	char name[MAXSIZE_USERNAME+1];
};

/******************************************************************
*
* Rotinas Utilitarias
*
*******************************************************************/

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

// rotinas do cliente

void main()
{
	int mqid;
	struct request_msg req_msg;
	struct response_msg resp_msg;
	struct user_record user_info;
	int opt; // aqui é le a opção digitada pelo usuario
	int stop_client;
	char tmp;
	char comando[1000]; // aqui le o comando que o usuario quer
	int index;
	
	mqid = msgget(MQCONVSRV_QUEUE_KEY, 0777);
	if (mqid == -1) {
   		printf("msgget() falhou, fila nao existe, servidor nao inicializado\n"); 
   		exit(1); 
	} 
	// Fazendo login n o sevvidor
	printf("Entre com o nome de usuario: \n");
	safegets(user_info.name,MAXSIZE_USERNAME);
	user_info.id = getpid();	
	// preparando requisição de login
	req_msg.request=LOGIN_REQ;
	req_msg.server_id = SERVER_ID;
	req_msg.client_id = user_info.id;
	strncpy(req_msg.requestdata,user_info.name,MAXSIZE_USERNAME);	
	// Enviando requisicao de login
	msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
	// Espera pela mensagem de resposta especifica para este cliente
	// usando o PID do processo cliente como tipo de mensagem
	if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
		printf("msgrcv() falhou\n"); 
		exit(1);
	}
	// Apresenta nossa respostas
	printf("%s\n",resp_msg.responsedata);
	// se não aceitar vmaos sair do cliente
	if (resp_msg.response!=OK_RESP)
		exit(1);
		
	// Interface de comandos 
	stop_client=0;
	while(!stop_client) {  // enquanto o usuário não quiser sair vai ficar no laço
		printf("\nIniciamos a execução, cliente speaker\n");
		printf("\nVocê tem as seguintes opções:\n");
		printf("exit: Sai do cliente;\n");
		printf("Send: Para enviarmos uma mensagem para um usuário:\n");
		printf("msgs: Para Ver o que você recebeu:\n");
		printf("post: Para postarmos em nosso forum\n");
		printf("delpost: Para deletarmos alguma mensagem\n");
		printf("delposts: Para deletarmos tudo  em nosso forum\n");
		printf("show: Para visualizarmos as mensagens em nosso forum\n");
		printf("myid: Utilize este comando para saber o seu id\n");
		printf("Users: Para mostrar a quantidade de usuários conectados :\n");

		scanf("%s",comando);
		scanf("%c",&tmp);  // 
		if (strcmp (comando,"post") == 0){
			opt=13;
		}
		else if (strcmp (comando,"show") == 0){
			opt=14;
		}else if (strcmp (comando,"send") == 0){
			opt=11;
		}
		else if (strcmp (comando,"msgs") == 0){
			opt=12;
		}else if (strcmp (comando,"myid") == 0){
			opt=19;
		}
		else if (strcmp (comando,"users") == 0){
			opt=18;
		}
		else if (strcmp (comando,"delposts") == 0){
			opt=17;
		}
		else if (strcmp (comando,"exit") == 0){
			opt=20;
		}
		else if (strcmp (comando,"delpost") == 0){
			opt=16;
		}
		// aqui faz todas as conversões, pois enviamos para o servidor o request em forma de inteiro
		
		

		
		switch(opt){ //agora testaremos enviaremos os requests para cada uma das opções
			case 20:
				stop_client = 1;				
				// Prepara requisicao de logout
				req_msg.request=LOGOUT_REQ;
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				strncpy(req_msg.requestdata,user_info.name,MAXSIZE_USERNAME);
				// Envia requisicao ao servidor
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				// Espera pela mensagem de resposta especifica para este cliente
				// usando o PID do processo cliente como tipo de mensagem
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); 
					exit(1);
				}
				// Apresenta o texto da resposta
				printf("%s\n",resp_msg.responsedata);
				break;

			case 19:
				if(opt ==19){ // o comando para identificar o id do usuario
					req_msg.request=19;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				printf("O seu id:\n");
				printf("%ld\n",resp_msg.client_id);
				break;	

				case 16:
				if(opt ==16){ //comando para deletar um post do forum publico
					req_msg.request=16;
				}
				printf("Informe o index da mensagem para ser deletada:\n");
				scanf("%d",&index);
				req_msg.index=index;
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				// Apresenta o texto convertido
				
				break;

				case 17:
				if(opt ==17){ //deletar tudo do forum
					req_msg.request=17;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				printf("Deletamos tudo do forum publico\n");
				break;	


				case 18:
				if(opt ==18){ //número de usuários logados
					req_msg.request=18;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				// Apresenta o texto convertido
				printf("Quantiade de usuários conectados:\n");
				printf("%s\n",resp_msg.responsedata);
				break;	
				
			case 14: // mostrar as mensagens do forum publico				
				if(opt ==14){
					req_msg.request=14;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				// Envia requisicao ao servidor
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				// Espera pela mensagem de resposta especifica para este cliente
				// usando o PID do processo cliente como tipo de mensagem
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				printf("%s\n",resp_msg.responsedata);
				break;

			case 13: //postar no forum
				printf("Escreva a mensagem que vamos postar:\n");
				safegets(req_msg.requestdata,MAXSIZE_MSGDATA);
				if(opt ==13){
					req_msg.request=13;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				break;


				case 12: // ver as mensagens que o usuario recebeu
				if(opt ==12){
					req_msg.request=12;
				}
				
				strncpy(req_msg.requestUser,user_info.name,MAXSIZE_USERNAME);	
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				// Envia requisicao ao servidor
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				printf("\nAs mensagens que voce recebeu:\n");
				printf("%s\n",resp_msg.responsedata);
				break;
				
				case 11: // enviar mensagens para um usuario
				// Pegar o nome do usuário que vai receber a mensagem
				printf("Escreva o nome do usuário de destino:\n");
				safegets(req_msg.requestUser,MAXSIZE_MSGDATA);

				printf("Escreva texto para ser enviado:\n");
				safegets(req_msg.requestdata,MAXSIZE_MSGDATA);
				if(opt ==11){
					req_msg.request=11;
				}
				
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				// Envia requisicao ao servidor
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				// Espera pela mensagem de resposta especifica para este cliente
				// usando o PID do processo cliente como tipo de mensagem
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}

				
				printf("Resposta do servidor:\n");
				printf("%s\n",resp_msg.responsedata);

				break;
				
			case 9:
				stop_client = 1;
				req_msg.request = STOP_SERVER_REQ;
				req_msg.server_id = SERVER_ID;
				req_msg.client_id = user_info.id;
				// Envia requisicao ao servidor
				msgsnd(mqid,&req_msg,sizeof(struct request_msg)-sizeof(long),0);
				// Espera pela mensagem de resposta especifica para este cliente
				// usando o PID do processo cliente como tipo de mensagem
				if (msgrcv(mqid,&resp_msg,sizeof(struct response_msg)-sizeof(long),user_info.id,0) < 0) {
					printf("msgrcv() falhou\n"); exit(1);
				}
				break;
					
			default:
				printf("Opcao invalida\n");
				break;
		}
	}
			
	exit(0);	
}
