#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h> 
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>

int main() {
    const char *if_name = "octboot_net0";
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl(SIOCGIFINDEX)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_ifindex = ifr.ifr_ifindex;
    saddr.sll_halen = ETH_ALEN;
    unsigned char dest_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    memcpy(saddr.sll_addr, dest_mac, ETH_ALEN);

    unsigned char packet[64];
    memset(packet, 0, sizeof(packet));
    unsigned char src_mac[ETH_ALEN] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    unsigned short eth_type = htons(ETH_P_IP);

    memcpy(packet, dest_mac, ETH_ALEN);
    memcpy(packet + ETH_ALEN, src_mac, ETH_ALEN);
    memcpy(packet + 2*ETH_ALEN, &eth_type, sizeof(eth_type));
    const char *message = "Hello from sender!";
    memcpy(packet + ETH_HLEN, message, strlen(message));

    int packet_len = ETH_HLEN + strlen(message);
    if (sendto(sockfd, packet, packet_len, 0, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent %d bytes to %s\n", packet_len, if_name);
    close(sockfd);
    return 0;
}