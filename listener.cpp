/*
** listener.c -- a datagram sockets "server" demo
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

#define MYPORT "4950"    // the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6: transformar el address a forma leible 
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM; // Para usar UDP
    hints.ai_flags = AI_PASSIVE; // use my IP
    // Obtenemos le addres y lo guadamos en hints y ademos un link list de address en servinfo
    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {// null usa mi ip
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Creamos el socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }
        // le hacemos el bind del socket al ip address y al puerto
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }
    // si no se puedo hacer el bind
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    // liberamos el linklist, siempre hacerlo
    //freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    /*
     *  Como es udp no necesitamos estar en modo listen, si no que simplemente podemos recibir data con 
     *  la funcion especial para udp
     *
     *  El recvfrom es casi lo mismo que el read solo que tenenmos dos parametros mas.
     *
     *  socket file, el buffer, el size del buffer, flags (por lo general es 0), y la sockaddr struct del
     *  ip de que envia la informacion con el size de esta
     *
     *  int recvfrom(int sockfd, void *buf, int len, unsigned int flags,
             struct sockaddr *from, int *fromlen);

             Again, this is just like recv() with the addition of a couple fields. from is a pointer to a local struct sockaddr_storage that will be filled with the IP address and port of the originating machine. fromlen is a pointer to a local int that should be initialized to sizeof *from or sizeof(struct sockaddr_storage). When the function returns, fromlen will contain the length of the address actually stored in from.

recvfrom() returns the number of bytes received, or -1 on error (with errno set accordingly).

So, here’s a question: why do we use struct sockaddr_storage as the socket type? Why not struct sockaddr_in? Because, you see, we want to not tie ourselves down to IPv4 or IPv6. So we use the generic struct sockaddr_storage which we know will be big enough for either.

     * */
    // Aqui se uso otra struct para el parametro from para guardar cualquier tipo de ip, sea ipv4 o ipv6
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    /*
     * Remember, if you connect() a datagram socket, you can then simply use send() and recv() for all your
     * transactions. The socket itself is still a datagram socket and the packets still use UDP, but the socket 
     * interface will automatically add the destination and source information for you.
     *
     * Es decir el recvfrom y el sendto se usa en udp cuando no haces el connect pero si lo haces puedes usar
     * el send y el recv como si fuera tcp
     * */


    // print the ip address of the sender
    printf("listener: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    // print size of the packet received
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    // muestra el mensaje recibido, poder aqui hacer 
    printf("listener: packet contains \"%s\"\n", buf);
    // Probar aqui enviar un mensaje.
    char* msg2 = "Mensaje enviado desde el listener para ser recibido por el talker";
    int numbytes2;
    if ( (numbytes2=sendto(sockfd, msg2, strlen(msg2), 0, (struct sockaddr *)&their_addr,addr_len)) == -1)
    {
        perror("error enviando al talker");
        exit(1);
    }
    freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}
