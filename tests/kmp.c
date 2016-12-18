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

int and(int b1, int b2) {
    int andtmp135;
    andtmp135 = (b1 + b2 == 2);
    return andtmp135;
}

int or(int b1, int b2) {
    int ortmp145;
    ortmp145 = (b1 + b2 != 0);
    return ortmp145;
}

int buildnext(char* bup, int* bunext) {
    int buplen;
    buplen = strlen(bup, 100);

    bunext[0] = (0 - 1);

    int buk;
    int buj;
    buk = (0 - 1);
    buj = 0;

    int tmp;
    tmp = 0;

    for (buj = 0; buj < buplen - 1; tmp = tmp + 1) {
        int cmp12345;
        cmp12345 = or(buk == 0 - 1, bup[buj] == bup[buk]);

        if (cmp12345 == 1) {
            buk = buk + 1;
            buj = buj + 1;
            bunext[buj] = buk;
        } else {
            buk = bunext[buk];
        }

        if (buk == (0 - 1)) {
            buk = buk + 1;
            buj = buj + 1;
            bunext[buj] = buk;
        }
    }

    return 0;
}

int kmpsearch(char* s, char* p, int* next, int* kmp_start) {
    int kmpj;
    int kmpslen;
    int kmpplen;

    kmpj = 0;
    kmpslen = strlen(s, 100);
    kmpplen = strlen(p, 100);

    int tmp2;
    tmp2 = 0;

    int cmp12;
    cmp12 = and(kmp_start[0] < kmpslen, kmpj < kmpplen);

    for (kmp_start[0] = kmp_start[0]; cmp12 != 0; tmp2 = tmp2) {
        int cmp4242;
        cmp4242 = or(kmpj == 0 - 1, s[kmp_start[0]] == p[kmpj]);

        if (cmp4242) {
            kmp_start[0] = kmp_start[0] + 1;
            kmpj = kmpj + 1;
        } else {
            kmpj = next[kmpj];
        }

        cmp12 = and(kmp_start[0] < kmpslen, kmpj < kmpplen);
    }

    int kmpretval;

    if (kmpj == kmpplen) {
        kmpretval = kmp_start[0] - kmpj;
    } else {
        kmpretval = (0 - 1);
    }

    return kmpretval;
}

int main() {
    char mains[100];
    char mainp[100];
    int mainnext[100];

    gets(mains);

    gets(mainp);

    buildnext(mainp, mainnext);
    int start[1];
    start[0] = 0;

    int tmp3;
    int main_n;
    main_n = strlen(mains, 100);
    int result;
    int found;
    found = 0;
    result = 0 - 1;
    int maincond;

    for (tmp3 = 0; start[0] < main_n; tmp3 = tmp3) {
        result = kmpsearch(mains, mainp, mainnext, start);
        maincond = and(result != 0 - 1, tmp3 != 0);
        if (maincond) {
            printf(",");
        }

        if (result != 0 - 1) {
            found = 1;
            printf("%d", result);
        }

        tmp3 = 1;
    }

    if (found == 0) {
        printf("False\n");
    }

    return 0;
}
