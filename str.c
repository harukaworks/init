long cmpstr(const char *a, const char *b) {
    for (; *a && *a == *b; a++, b++);
    return *(unsigned char*)a - *(unsigned char*)b;
}

long chrstr(const char *s, const char chr) {
    for (; *s; s++) if (*s == chr) return 1;
    return 0;
}

long lenstr(const char *s) {
    int n = 0;
    while (*s++) n++;
    return n;
}

const char *powstr(const char *a, const char *b) {
    if (!*b) return a;
    const char *h = a;
    while (*h) {
        const char *h_it = h;
        const char *n_it = b;
        while (*h_it && *n_it && *h_it == *n_it) {
            h_it++;
            n_it++;
        }
        if (!*n_it) return h;
        h++;
    }
    return 0;
}

long spnstr(const char *s, const char *cst, const int inv) {
    long count = 0;
    while (*s) {
        if (chrstr(cst, *s) ^ inv) {
            count++;
            s++;
        } else {
            break;
        }
    }
    return count;
}
