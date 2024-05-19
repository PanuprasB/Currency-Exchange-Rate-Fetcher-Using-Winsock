#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")
using namespace std;


int main() {
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "klaida ties winsock init: " << iResult << std::endl;
        return 1;
    }

    addrinfo* address = NULL,
        * ptr = NULL,
        hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("www.lb.lt", "http", &hints, &address) != NULL) {
        cout << "klaida ties serverio adreso nuskaitymu" << endl;
        WSACleanup();
        return NULL;
    }
    SOCKET ConnectSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);


    if (ConnectSocket == INVALID_SOCKET) {
        cout << "klaida ties jungties suskurimu " << WSAGetLastError() << endl;
        freeaddrinfo(address);
        WSACleanup();
        return NULL;

    }


    iResult = connect(ConnectSocket, address->ai_addr, static_cast<int>(address->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        cout << "klaida ties prisijungimu";
        closesocket(ConnectSocket);
        freeaddrinfo(address);
        WSACleanup();
        return 1;
    }

    string getMsg = "GET /webservices/FxRates/FxRates.asmx/getCurrentFxRates?tp=eu HTTP/1.1\r\nHost: www.lb.lt\r\nConnection: close\r\n\r\n";
    iResult = send(ConnectSocket, getMsg.c_str(), getMsg.length(), 0);
    if (iResult == SOCKET_ERROR){
        cerr << "klaida ties uzklausos siuntimu" << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(address);
            WSACleanup();
            return 1;

    }
    
   

    char recvbuf[4096];
    string response;
    do {
        iResult = recv(ConnectSocket, recvbuf, 4096, 0);
        if (iResult > 0) {
            response.append(recvbuf, iResult);
        }
        else if (iResult == 0) {
            //out << "Connection closed" << endl;
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
        }
    } while (iResult > 0);
    
    string currency;
    while (true) {
        cout << "Prisijungimas sekmingas, valiutu duomenys gauti" << endl;
        cout << "Iveskite valiuta kurios kursa norite patikrinti (XXX), norint baigti darba ivsekite 0" << endl;
        cin >> currency;

        if (currency == "0") {
            break;
        }

        if (currency.length() != 3) {
            cout << "Klaida: blogas valiutos kodas (XXX)" << endl;
            continue;
        }

        size_t start_pos = response.find("<Ccy>" + currency + "</Ccy>");
        if (start_pos != string::npos) {
            size_t fx_rate_start = response.find("<Amt>", start_pos);
            if (fx_rate_start != string::npos) {
                size_t amt_end = response.find("</Amt>", fx_rate_start);
                if (amt_end != string::npos) {
                    string amt_value = response.substr(fx_rate_start + 5, amt_end - fx_rate_start - 5);

                    cout << currency << " santykis su EUR: " << amt_value << endl;
                    cout << "EUR santykis su " << currency << ": " << 1 / stod(amt_value) << endl;
                }
            }
        }
        else {
            cout << "Klaida: valiutos kodas nerastas" << endl;
        }
    }

    cout << "Programa baigta." << endl;
    
    closesocket(ConnectSocket);
    freeaddrinfo(address);
    WSACleanup();
    return 0;
       
}



