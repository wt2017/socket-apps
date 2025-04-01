#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int create_tap_device(const char *dev_name) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("Opening /dev/net/tun");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        return err;
    }

    return fd;
}

int main() {
    const char *tap_name = "octboot_net0";
    int tap_fd = create_tap_device(tap_name);
    if (tap_fd < 0) {
        exit(EXIT_FAILURE);
    }

    // 配置IP地址和启用设备
    system("ip address add 192.168.123.1/24 dev octboot_net0");
    system("ip link set dev octboot_net0 up");

    printf("Tap device %s created. Waiting for data...\n", tap_name);

    unsigned char buffer[1500];
    while (1) {
        int nread = read(tap_fd, buffer, sizeof(buffer));
        if (nread < 0) {
            perror("Reading from interface");
            close(tap_fd);
            exit(EXIT_FAILURE);
        }

        if (nread >= 14) {
            // print packet whose source mac address is {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}
            unsigned char source_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
            if (memcmp(buffer + 6, source_mac, 6) == 0) {
                // Print packet details
                unsigned short eth_type = (buffer[12] << 8) | buffer[13];
                printf("Ethernet Type: 0x%04x ", eth_type);
                if (eth_type == 0x0806) printf("(ARP)\n");
                else if (eth_type == 0x0800) printf("(IPv4)\n");
                else if (eth_type == 0x86dd) printf("(IPv6)\n");
                else printf("(Other)\n");
        
                // Print source and destination MAC
                printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
                printf("Dest MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            }
#if 0
            unsigned short eth_type = (buffer[12] << 8) | buffer[13];
            printf("Ethernet Type: 0x%04x ", eth_type);
            if (eth_type == 0x0806) printf("(ARP)\n");
            else if (eth_type == 0x0800) printf("(IPv4)\n");
            else if (eth_type == 0x86dd) printf("(IPv6)\n");
            else printf("(Other)\n");

            if (eth_type == 0x0800) {
                printf("IPv4 Packet\n");
                printf("Source IP: %d.%d.%d.%d\n", buffer[26], buffer[27], buffer[28], buffer[29]);
                printf("Dest IP: %d.%d.%d.%d\n", buffer[30], buffer[31], buffer[32], buffer[33]);
            } else if (eth_type == 0x86dd) {
                printf("IPv6 Packet\n");
                printf("Source IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                    buffer[22], buffer[23], buffer[24], buffer[25],
                    buffer[26], buffer[27], buffer[28], buffer[29],
                    buffer[30], buffer[31], buffer[32], buffer[33],
                    buffer[34], buffer[35], buffer[36], buffer[37]);
             
                printf("Dest IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                    buffer[38], buffer[39], buffer[40], buffer[41],
                    buffer[42], buffer[43], buffer[44], buffer[45],
                    buffer[46], buffer[47], buffer[48], buffer[49],
                    buffer[50], buffer[51], buffer[52], buffer[53]);
             
                // Show IPv6 next header (protocol)
                printf("Next Header: 0x%02x ", buffer[20]);
                switch(buffer[20]) {
                    case 0x3A: printf("(ICMPv6)\n"); break;
                    case 0x06: printf("(TCP)\n"); break;
                    case 0x11: printf("(UDP)\n"); break;
                    default: printf("(Other)\n");
                }                                              
            }
#endif
            printf("\n");
        }
#if 0
        printf("Received %d bytes:\n", nread);
        for (int i = 0; i < nread; i++) {
            printf("%02x ", buffer[i]);
            if ((i+1) % 16 == 0) printf("\n");
        }
        printf("\n");
#endif
    }

    close(tap_fd);
    return 0;
}