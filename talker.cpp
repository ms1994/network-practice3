/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "4950"    // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_storage their_addr; 
    int numbytes, numbytes2;
    char* msg = "mensaje desde el talker, deberia aparecer en el listener asi no este corriendo el programa";
    /*if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }*/
    // Usamos ipv6 y udp protocol
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    // get address information on the rv, if error arise convert to human redable with gai_streror
    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    // Por ser udp solo necesitamos crear un socket file descriptor sin necesidad del bind (solo server)
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }
    // Corroboramos que si se creo el socket
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }
    // Enviamos el mensaje y guardamos el numero de bytes
    /*El sendto es casi lo mismo que el send pero con dos parametros adiciionales.
     * el sendto accepta el socket file, el mensaje a enviar, la longitud de ese mensaje
     * un parametro flags y adicionalmente hay que pasarle la direccion ip al cual vamos a enviar
     * la informacion (esta viene dada en la estructura sockadd) por ultimo el size de esa struct'
     *
     * int sendto(int sockfd, const void *msg, int len, unsigned int flags,
           const struct sockaddr *to, socklen_t tolen);

           As you can see, this call is basically the same as the call to send() with the addition of two other pieces of information. to is a pointer to a struct sockaddr (which will probably be another struct sockaddr_in or struct sockaddr_in6 or struct sockaddr_storage that you cast at the last minute) which contains the destination IP address and port. tolen, an int deep-down, can simply be set to sizeof *to or sizeof(struct sockaddr_storage).

To get your hands on the destination address structure, you’ll probably either get it from getaddrinfo(), or from recvfrom(), below, or you’ll fill it out by hand.
     * */
    // la informacion del address y la longituda la optuvimos del getaddre info, pero puede ser pasada
    // manualmente
    if ((numbytes = sendto(sockfd,msg, strlen(msg), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    // Liberamos la estructura
    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes\n", numbytes);
    char buffer[1024];
     socklen_t addr_len = sizeof their_addr;
    printf("talker: waiting to recvfrom...\n");
    // Poner aqui el mensaje que recibiremos del listener.
    if ( (numbytes2 = recvfrom(sockfd, buffer, 1023, 0,
                   (struct sockaddr*)&their_addr,&addr_len)) == -1)
    {
        perror("recibiendo..");
        exit(1);
    }
    printf("talker: packet contains \"%s\"\n", buffer);
    close(sockfd);

    return 0;
}
