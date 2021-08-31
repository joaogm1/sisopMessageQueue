# sisopMessageQueue
 Sistema CentralSession
O sistema CentralSession é um sistema para comunicação e troca de mensagens entre usuários que opera de
forma centralizada e é composto de dois tipos de processos:
 Processos clientes, denominados de speaker, que implementam a interface de uso do sistema pelos usuários.
 Processo servidor, denominado de chairman, que implementa e controla os serviços de comunicação entre
os clientes.
Toda a comunicação entre estes processos deve ser implementada pelo mecanismo de IPC de la de mensagens
visto em aula.
O processo servidor chairman deve ser posto em execução antes dos processos clientes, para tanto basta
abrir uma janela de terminal e executar o chairman, deixando essa janela posteriormente minimizada com o
processo chairman rodando.
Para entrar no sistema um usuário deve abrir uma janela de terminal e executar o programa cliente speaker
e se logar no sistema. No login o usuário deve informar seu nome de usuário (não é necessário senha). O nome
do usuário é um texto sem espaço em branco de até 20 caracteres. Se não houver nenhum outro usuário já
logado com o mesmo nome, o login será aceito.
Para troca de informações entre os diversos clientes speaker (um para cada usuário distinto) e o único processo
servidor chairman utilize o mecanismo de IPC de las de mensagens. Use o campo identicador ou código da
mensagem (msgtyp) para separar as mensagens destinadas ao servidor chairman das mensagens destinadas a
um particular cliente speaker. Use o código de tipo 1 para indicar mensagens destinadas ao servidor chairman
e use o PID (Process IDentier ) dos processos cliente para indicar mensagens destinadas aos clientes.
No procedimento de login, o cliente speaker solicita que o usuário entre com seu nome e então envia ao
servidor chairman uma mensagem contendo o nome do usuário que está tentando se logar ao sistema e o PID
do cliente speaker que está interagindo com esse usuário. Depois o cliente speaker espera por uma mensagem
de tipo igual ao seu PID com a resposta do servidor chairman. Se a resposta for uma conrmação do sucesso
do login, então o cliente speaker considera que o login teve sucesso e segue com a operação normal do sistema
de comunicação. Se a resposta indicar que o processo chairman não pode fazer login então o cliente speaker
repassa essa informação ao usuário, podendo encerrar a operação ou fazer uma nova tentativa de login, se o
usuário quiser.
Na operação normal do sistema de comunicação, em todas as interações com o usuário, o cliente speaker
apenas lê os comandos do usuário e verica se estão corretamente digitados. Ele não executa estes comandos. Ao
invés, ele formata os comandos numa estrutura de dados encapsulada em uma mensagem de requisição e envia
esta mensagem para o processo chairman através da la de mensagens. Note que as mensagens de requisição
devem conter o PID do cliente speaker. Somente assim o processo chairman saberá a qual cliente speaker deve
enviar a resposta após executar o comando contido na mensagem de requisição.
Os seguintes comandos de usuário são tratados pelo processo servidor chairman:
 send <nome-usuário> <texto-mensagem>  envia uma mensagem de texto do usuário atual para o
usuário destino identicado por <nome-usuário>. A mensagem somente é enviada se o usuário de destino
estiver logado no sistema, caso contrário uma mensagem de erro deve ser mostrada na tela. Caso o usuário
de destino esteja logado no sistema, a mensagem é na verdade armazenada em uma lista mantida pelo
processo chairman, que é mostrada ao usuário de destino somente quando ele solicitar ver suas mensagens
através do comando msgs.
1
 msgs  mostra (lista) na tela todas as mensagens recebidas pelo usuário. Se não houver nenhuma
mensagem destinada ao usuário avisa que não há mensagens.
 post <texto-mensagem>  publica uma mensagem de texto no fórum público de mensagens.
 show  mostra (lista) na tela todas as mensagens que foram publicadas no fórum público de mensagens.
 del msgs  remove (deleta) todas as mensagens destinadas ao usuário.
 del post <número>  remove (deleta) a mensagem com índice <número> que foi publicada no fórum.
 del posts  remove (deleta) todas as mensagem publicadas no fórum.
 users  mostra na tela a lista de usuários correntemente logados no sistema.
 myid  mostra o nome e identicador (PID) do usuário atual.
 exit  sai do sistema CentralSession, mas antes desloga o usuário.
O servidor chairman é o processo do sistema que realmente executa os comandos dos usuários. O cliente
speaker apenas lê os comandos, separa e identica os campos do comando e envia uma mensagem de requisição
ao processo chairman para que ele execute o comando. As respostas ocasionadas pela execução do comando
são posteriormente repassadas do processo chairman para o cliente speaker através de uma ou mais mensagens
de resposta.
Toda a interação entre os clientes speaker e o processo chairman ocorre através da la de mensagens única
compartilhada por todos esses processos.
