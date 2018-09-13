#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "color.h"

using namespace std;


//fontes de exemplos de clients
//https : //www.ibm.com/support/knowledgecenter/ssw_ibm_i_72/rzab6/generic.htm
//https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_72/rzab6/xip6client.htm

class Client;

class Client{
    
    //sockect file descriptor
    int sockfd;

    sockaddr_in6 addr;

    public:
    Client(char server_addr[], int porta){
        //socket ipv6, do tipo stream (TCP)
        sockfd = socket(AF_INET6, SOCK_STREAM, 0 );
        if(sockfd<0){
            perror(RED("Erro ao criar socket\n"));
            exit(2);
        }
        printf(GRN("Socket criado\n"));

        //inicializando valores de addr.sin6
        memset(&addr, 0, sizeof(addr));
        addr.sin6_family = AF_INET6;
        if(strcmp(server_addr, "localhost")==0){
            addr.sin6_addr = in6addr_any;
        }
        else if (inet_pton(AF_INET6, server_addr, &addr.sin6_addr) <= 0){
            printf(RED("erro ao converter endereço do servidor"));
            exit(2);
        }

        char str[256] = {0};
        cout <<"server Addr: " << inet_ntop(AF_INET6, &(addr.sin6_addr), str, 256) << endl;

        addr.sin6_port = htons(porta);
    }

    void Connect(){
        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6))){
            perror(RED("Nao foi possivel se conectar ao servidor"));
            close(sockfd);
            exit(2);
        }
        printf(GRN("Conectado ao servidor\n"));
    }

    size_t Send(char msg[]){
        size_t len = send(sockfd, msg, strlen(msg)+1, 0);

        if(len != strlen(msg)+1){ 
            perror(YEL("Erro ao enviar mensagem"));
            exit(2);
        }

        return len;
    }

    size_t getAnswer(char *recv_buf,size_t len ){
        return recv(sockfd, recv_buf, len, 0);
    }


    void Close(){
        close(sockfd);
    }

    ~Client(){
        Close();
    }

};


int main(int argc, char *argv[]){
    if(argc < 3){
        cout << YEL("informe o end do servidor e a porta do socket ( ex: ./prog_name localhost 12345)\n");
        exit(1);
    }
    Client cliente(argv[1], atoi(argv[2]));

    cliente.Connect();


    char str[512];
    char resp[512];
    int continueLoop=true;
    while(continueLoop){
        printf(BOLD("Enviar: "));
        scanf(" %[^\n]s", str);
        cout<<"Bytes enviado: "<<cliente.Send(str)<<endl;

        switch (cliente.getAnswer(resp, 512)){
            case -1:            
                perror(RED("Erro ao receber resposta\n"));
                break;
            case 0:
                perror(RED("Servidor desligou\n"));
                continueLoop = false;
                break;
            default:
                printf(BOLD("server: ") CYN("%s\n\n"), resp);
        }

        if (!strcmp(str, "exit") || !strcmp(str, "bye"))
            break;
    }

    return 0;
}