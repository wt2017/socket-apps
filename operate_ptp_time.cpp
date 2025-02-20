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

#define DEVICE "/dev/ptp0"

clockid_t FileDescriptorToClockId(int file_descriptor) {
    constexpr clockid_t CLOCK_FD = 3;
    auto clock_id = reinterpret_cast<clockid_t>(file_descriptor);
    clock_id = ~clock_id << 3 | CLOCK_FD;
    return clock_id;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <get|set> [seconds nanoseconds]" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    int phc_fd_ = -1;

    if (command == "get") {
        // Open the PTP device
        phc_fd_ = open(DEVICE, O_RDONLY);
        //phc_fd_ = open(DEVICE, O_RDWR);
        if (phc_fd_ < 0) {
            std::cerr << "Error opening device: " << strerror(errno) << std::endl;
            return 1;
        }

        auto phc_clkid_ = FileDescriptorToClockId(phc_fd_);
        timespec ts{};
        if (clock_gettime(phc_clkid_, &ts) < 0) {
            std::cerr << "Error getting time: " << strerror(errno) << std::endl;
            close(phc_fd_);
            return 1;
        }

        close(phc_fd_);
    }
    else if (command == "set") {
        // Open the PTP device
        phc_fd_ = open(DEVICE, O_RDWR);
        if (phc_fd_ < 0) {
            std::cerr << "Error opening device: " << strerror(errno) << std::endl;
            return 1;
        }

        auto phc_clkid_ = FileDescriptorToClockId(phc_fd_);
        // Set the time
        struct tm tm = {};
        tm.tm_year = 2025 - 1900; // Year since 1900
        tm.tm_mon = 1;     // Month [0-11]
        tm.tm_mday = 20;        // Day of the month [1-31]
        tm.tm_hour = 22;        // Hours since midnight [0-23]
        tm.tm_min = 20;         // Minutes after the hour [0-59]
        tm.tm_sec = 0;         // Seconds after the minute [0-60]
        timespec ts{};
        ts.tv_sec = mktime(&tm);
        ts.tv_nsec = 0;
        if (clock_settime(phc_clkid_, &ts) < 0) {
            std::cerr << "Error setting time: " << strerror(errno) << std::endl;
            close(phc_fd_);
            return 1;
        }

        close(phc_fd_);
    }
    else {
        std::cerr << "Invalid command: " << command << std::endl;
        return 1;
    }

    return 0;
}
