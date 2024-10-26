#include <iostream>
#include <cstring>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

void PrintWiFiInfo() {
    // Use nl80211 API to get WiFi information
    // ...
}

void PrintIPInfo() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        std::cerr << "getifaddrs failed" << std::endl;
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6) {
            std::cout << "Interface: " << ifa->ifa_name << std::endl;

            int s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                          sizeof(struct sockaddr_in6),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);

            if (s != 0) {
                std::cout << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
            } else {
                std::cout << "Address: " << host << std::endl;
            }
        }
    }

    freeifaddrs(ifaddr);
}

int main() {
    std::cout << "WiFi Finder" << std::endl;
    std::cout << "===========" << std::endl;
    
    std::cout << "\nWiFi Information:" << std::endl;
    PrintWiFiInfo();
    
    std::cout << "\nIP Information:" << std::endl;
    PrintIPInfo();

    return 0;
}
