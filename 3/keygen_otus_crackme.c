#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#define LOGIN_XOR_CONST 0xedb88320
#define PWD_LEN 8 // affects on speed
#define XOR_CONSTANT 0x99

static char *generate_random_string(char *str, size_t size);


int main() {

    char login[30];
    printf("Input login: ");
    scanf_s("%30[^\n]", login);

    unsigned int login_checksum = 0xffffffff;

    for (int i = 0; i < strlen(login); i++) {
        if (!isalpha(login[i]) && !isdigit(login[i])) {
            fprintf(stderr, "Login should contain only letters (A-Z/a-z) and digits (0-9)");
            return -1;
        }
        login_checksum ^= login[i];
        for (int j = 0; j < 8; j++){
            login_checksum = (-(login_checksum & 1) & LOGIN_XOR_CONST) ^ (login_checksum >> 1);
		}
    }
    login_checksum = ~login_checksum;

    char password[8];
    while (true) {
        int password_checksum = 0;
        generate_random_string(password, PWD_LEN);
        for (int i = 0; i < strlen(password); i++) {
            password_checksum += password[i] ^ XOR_CONSTANT;
        }
		
        if ((login_checksum & 0xff) == (password_checksum & 0xff)) {
            printf("For login %s password is: %s", login, password);
            break;
        }
        memset(password,'0',PWD_LEN);
		
    }

    return 0;
}

static char *generate_random_string(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}