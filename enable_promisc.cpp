#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>      // For htons
#include <linux/if_ether.h>  // For ETH_P_ALL

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <interface_name>" << std::endl;
        return 1;
    }
    const char* interface_name = argv[1];
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        std::cerr << "Error opening socket: " << strerror(errno) << std::endl;
        return 1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ);

    // Get the current flags
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
        std::cerr << "Error getting flags: " << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }

    // Set the promiscuous mode flag
    ifr.ifr_flags |= IFF_PROMISC;

    // Set the new flags
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
        std::cerr << "Error setting flags: " << strerror(errno) << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Promiscuous mode enabled on " << interface_name << std::endl;

    close(sockfd);
    return 0;
}

