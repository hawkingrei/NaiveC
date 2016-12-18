int strlen(char* str, int maxlen) {
    int i;
    int flag;
    int len;

    flag = 1;
    len = 0;
    for (i = 0; i < maxlen; i = i + 1) {
        if (str[i] == '\0') {
            flag = 0;
        }
        if (flag == 1) {
            len = len + 1;
        }
    }
    return len;
}

int main() {
    char mainstr[100];
    int mainlen;
    int mainmaxlen;
    int mainflag;
    int maini;
    int mainj;

    gets(mainstr);
    mainlen = strlen(mainstr, 100);
    mainflag = 1;
    for (maini = 0; maini < mainlen; maini = maini + 1) {
        mainj = mainlen - maini - 1;
        if (mainstr[maini] != mainstr[mainj]) {
            mainflag = 0;
        }
    }
    if (mainflag == 1) {
        printf("True\n");
    } else {
        printf("False\n");
    }
    return 0;
}
