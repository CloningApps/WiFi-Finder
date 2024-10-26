#include <iostream>
#include <windows.h>
#include <wlanapi.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "iphlpapi.lib")

void PrintWiFiInfo() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        std::cout << "WlanOpenHandle failed with error: " << dwResult << std::endl;
        return;
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        std::cout << "WlanEnumInterfaces failed with error: " << dwResult << std::endl;
        WlanCloseHandle(hClient, NULL);
        return;
    }

    for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
        pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];
        wprintf(L"Interface name: %ws\n", pIfInfo->strInterfaceDescription);

        PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;
        DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
        WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;

        dwResult = WlanQueryInterface(hClient,
                                      &pIfInfo->InterfaceGuid,
                                      wlan_intf_opcode_current_connection,
                                      NULL,
                                      &connectInfoSize,
                                      (PVOID*)&pConnectInfo,
                                      &opCode);

        if (dwResult != ERROR_SUCCESS) {
            wprintf(L"WlanQueryInterface failed with error: %u\n", dwResult);
            continue;
        }

        if (pConnectInfo->isState == wlan_interface_state_connected) {
            wprintf(L"Connected to: %ws\n", pConnectInfo->strProfileName);
        } else {
            wprintf(L"Not connected\n");
        }

        WlanFreeMemory(pConnectInfo);
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
    }
    WlanCloseHandle(hClient, NULL);
}

void PrintIPInfo() {
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);
        if (pAddresses == NULL) {
            std::cout << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
            return;
        }

        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    }

    if (dwRetVal == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            if (pCurrAddresses->OperStatus == IfOperStatusUp && 
                (pCurrAddresses->IfType == IF_TYPE_IEEE80211 || pCurrAddresses->IfType == IF_TYPE_ETHERNET_CSMACD)) {
                
                std::cout << "Adapter name: " << pCurrAddresses->AdapterName << std::endl;
                
                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
                while (pUnicast != NULL) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(sa_in->sin_addr), ip, INET_ADDRSTRLEN);
                        std::cout << "IPv4 Address: " << ip << std::endl;
                    } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                        sockaddr_in6* sa_in6 = (sockaddr_in6*)pUnicast->Address.lpSockaddr;
                        char ip[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, &(sa_in6->sin6_addr), ip, INET6_ADDRSTRLEN);
                        std::cout << "IPv6 Address: " << ip << std::endl;
                    }
                    pUnicast = pUnicast->Next;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    } else {
        std::cout << "GetAdaptersAddresses failed with error: " << dwRetVal << std::endl;
    }

    if (pAddresses) {
        free(pAddresses);
    }
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
