#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LOGIN_XOR_CONST 0x5678
#define PASSWORD_XOR_CONST 0x1234

int main() {
    char login[30];
    printf("Input login: ");
    scanf_s("%30[^\n]", login);

    int checksum = 0;
    int password = 0;
    for (int i = 0; i < strlen(login); i++) {
        if (!isupper(login[i]) || !isalpha(login[i])) {
            fprintf(stderr,"Login should contain only A-Z letters!");
            return -1;
        }
        checksum += login[i];
    }

    checksum ^= LOGIN_XOR_CONST;
    password = checksum ^ PASSWORD_XOR_CONST;
    printf("Password for login %s is %d\n", login, password);

    return 0;
}