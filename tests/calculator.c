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

int isdigit(char c123) {
    return and('/' < c123, c123 < ':');
}

int getnum(int* st, char* s) {
    int slen;
    slen = strlen(s, 100);
    int flag;
    flag = 0;
    char ch;
    int idx;

    int value;
    value = 0;

    int flagnum;

    int getv;

    flagnum = and(st[0] < slen, isdigit(s[st[0]]));
    for (flagnum = flagnum; flagnum == 1; value = value) {
        ch = s[st[0]];
        getv = ch - '0';
        value = value * 10 + getv;
        st[0] = st[0] + 1;
        flagnum = and(st[0] < slen, isdigit(s[st[0]]));
    }

    return value;
}

int init_pri(int* pri_table_init) {
    int init_lidx;
    int init_i;
    for (init_i = 0; init_i < 2; init_i = init_i + 1) {
        pri_table_init[init_i] = 2;
    }

    for (init_i = 2; init_i < 5; init_i = init_i + 1) {
        pri_table_init[init_i] = 0;
    }

    for (init_i = 5; init_i < 7; init_i = init_i + 1) {
        pri_table_init[init_i] = 2;
    }

    for (init_i = 7; init_i < 14; init_i = init_i + 1) {
        init_lidx = init_i - 7;
        pri_table_init[init_i] = pri_table_init[init_lidx];
    }

    for (init_i = 14; init_i < 21; init_i = init_i + 1) {
        pri_table_init[init_i] = 2;
    }

    pri_table_init[18] = 0;

    for (init_i = 21; init_i < 28; init_i = init_i + 1) {
        init_lidx = init_i - 7;
        pri_table_init[init_i] = pri_table_init[init_lidx];
    }

    for (init_i = 28; init_i < 33; init_i = init_i + 1) {
        pri_table_init[init_i] = 0;
    }

    pri_table_init[33] = 1;
    pri_table_init[34] = 3;

    for (init_i = 35; init_i < 42; init_i = init_i + 1) {
        pri_table_init[init_i] = 3;
    }

    for (init_i = 42; init_i < 47; init_i = init_i + 1) {
        pri_table_init[init_i] = 0;
    }

    pri_table_init[47] = 3;
    pri_table_init[48] = 1;

    return 0;
}


int op_order(char ch234) {
    int ret234;

    ret234 = 0;

    if (ch234 == '+') {
        ret234 = 0;
    }

    if (ch234 == '-') {
        ret234 = 1;
    }

    if (ch234 == '*') {
        ret234 = 2;
    }

    if (ch234 == '/') {
        ret234 = 3;
    }

    if (ch234 == '(') {
        ret234 = 4;
    }

    if (ch234 == ')') {
        ret234 = 5;
    }

    if (ch234 == '\0') {
        ret234 = 6;
    }

    return ret234;
}

int order_between(int* pri_table_order, char op_top_order, char op_cur_order) {
    int idx_order;
    idx_order = op_order(op_top_order) * 7 + op_order(op_cur_order);
    return pri_table_order[idx_order];
}

int push_stack_num(int* stack_num_push, int num_push, int* top_num_push) {
    stack_num_push[top_num_push[0]] = num_push;
    top_num_push[0] = top_num_push[0] + 1;
    return 0;
}

int pop_stack_num(int* stack_num_pop, int* top_num_pop) {
    top_num_pop[0] = top_num_pop[0] - 1;
    return stack_num_pop[top_num_pop[0]];
}

int top_stack_num(int* stack_num_top, int* top_num_top) {
    int top_idx_num;
    top_idx_num = top_num_top[0] - 1;
    return stack_num_top[top_idx_num];
}

int push_stack_op(char* stack_op_push, char op_push, int* top_op_push) {
    stack_op_push[top_op_push[0]] = op_push;
    top_op_push[0] = top_op_push[0] + 1;
    return 0;
}

char pop_stack_op(char* stack_op_pop, int* top_op_pop) {
    top_op_pop[0] = top_op_pop[0] - 1;
    return stack_op_pop[top_op_pop[0]];
}

char top_stack_op(char* stack_op_top, int* top_op_top) {
    int top_idx_op;
    top_idx_op = top_op_top[0] - 1;
    return stack_op_top[top_idx_op];
}

int calc(int c_n1, int c_n2, char c_op) {
    int c_ret;
    if (c_op == '+') {
        c_ret = c_n1 + c_n2;
    }

    if (c_op == '-') {
        c_ret = c_n1 - c_n2;
    }

    if (c_op == '*') {
        c_ret = c_n1 * c_n2;
    }

    if (c_op == '/') {
        c_ret = c_n1 / c_n2;
    }

    return c_ret;
}

int evaluate(char* e_s, int* e_pri) {
    int e_stack_num[100];
    char e_stack_op[100];

    int e_stack_num_top[1];
    int e_stack_op_top[1];
    e_stack_num_top[0] = 0;
    e_stack_op_top[0] = 0;

    push_stack_op(e_stack_op, '\0', e_stack_op_top);

    int e_tmp;
    e_tmp = 0;

    int e_st[1];
    e_st[0] = 0;

    int e_num;

    char e_top_op;
    int e_order;

    int e_n2;
    int e_n1;

    for (e_tmp = e_tmp; e_stack_op_top[0] != 0; e_tmp = e_tmp) {
        if (isdigit(e_s[e_st[0]])) {
            e_num = getnum(e_st, e_s);
            push_stack_num(e_stack_num, e_num, e_stack_num_top);
        } else {
            e_top_op = top_stack_op(e_stack_op, e_stack_op_top);
            e_order = order_between(e_pri, e_top_op, e_s[e_st[0]]);

            if (e_order == 0) {
                push_stack_op(e_stack_op, e_s[e_st[0]], e_stack_op_top);
                e_st[0] = e_st[0] + 1;
            }

            if (e_order == 1) {
                e_top_op = pop_stack_op(e_stack_op, e_stack_op_top);
                e_st[0] = e_st[0] + 1;
            }

            if (e_order == 2) {
                e_top_op = pop_stack_op(e_stack_op, e_stack_op_top);
                e_n2 = pop_stack_num(e_stack_num, e_stack_num_top);
                e_n1 = pop_stack_num(e_stack_num, e_stack_num_top);
                e_num = calc(e_n1, e_n2, e_top_op);
                push_stack_num(e_stack_num, e_num, e_stack_num_top);
            }
        }
    }

    return pop_stack_num(e_stack_num, e_stack_num_top);
}

int main() {
    char mainstr[100];
    gets(mainstr);

    int main_pri[100];
    init_pri(main_pri);

    printf("%d\n", evaluate(mainstr, main_pri));

    return 0;
}
