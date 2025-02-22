#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/ptp_clock.h>
#include <time.h>
#include <string.h>
#include <cerrno>
#include <iostream>

#define DEVICE_TEMPLATE "/dev/ptp%d"

clockid_t FileDescriptorToClockId(int file_descriptor) {
    constexpr clockid_t CLOCK_FD = 3;
    auto clock_id = reinterpret_cast<clockid_t>(file_descriptor);
    clock_id = ~clock_id << 3 | CLOCK_FD;
    return clock_id;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <device_number>" << std::endl;
        return 1;
    }

    int phc_fd_ = -1;
    int device_number = std::stoi(argv[1]);
    char device_path[20];
    snprintf(device_path, sizeof(device_path), DEVICE_TEMPLATE, device_number);

    // Open the PTP device
    phc_fd_ = open(device_path, O_RDWR);
    if (phc_fd_ < 0) {
        std::cerr << "Error opening device: " << strerror(errno) << std::endl;
        return 1;
    }

    auto phc_clkid_ = FileDescriptorToClockId(phc_fd_);
    // Adjust the frequency
    struct timex tx = {};
    tx.modes = ADJ_FREQUENCY;
    tx.freq = 1000;

    if (clock_adjtime(phc_clkid_, &tx) < 0) {
        std::cerr << "Error adjusting frequency: " << strerror(errno) << std::endl;
        close(phc_fd_);
        return 1;
    }

    close(phc_fd_);
    return 0;
}
