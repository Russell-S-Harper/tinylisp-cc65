/*
    Contents: "tinylisp-cc65" C source code
  Repository: https://github.com/Russell-S-Harper/tinylisp-cc65
     Contact: russell.s.harper@gmail.com
*/

/* tinylisp-cc65.c to compile under cc65 and run on 8-bit platforms by Russell Harper 2026 adapted from:
 * tinylisp-float-opt.c with single float precision NaN boxing (optimized version) by Robert A. van Engelen 2022
 *
 * Given memory restrictions in 8-bit platforms, added a few features to make development easier:
 *
 *    - optional tracing for debugging purposes (compile with -DTRACE)
 *    - (print arg1 arg2 ...) print arguments
 *    - (load file) load and evaluate file.lisp, e.g. (load 'common) to load common.lisp
 *    - (incr var1 var2 ...) increment variables, mutates in place
 *    - (decr var1 var2 ...) decrement variables, mutates in place
 *    - (while (condition) (code1) (code2) ...) non-recursive loop to run code
 *    - (bye) end session gracefully, needed because EOF is used for loading files
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#define I    uint32_t
#define L    float
#define T(x) (*(I*)&x >> 20)
#define A    ((char *)cell)
#define QNAN INT32_MAX       /* Reserved "Quiet NaN" */
#define BUF  60              /* Buffer to accomodate maximum "L" in characters and a bit extra */

#if defined(__CX16__)
#define N    2000            /* CX16 can use high memory */
#define HMEM ((L *)0xA000)   /* CX16-specific memory definitions */
#define CTRL ((uint8_t *)0x00)
#define BANK 0x01
#elif defined(TRACE)
#define N    500             /* Tracing takes up a lot of memory so use a smaller N to allow room */
#else
#define N    1000            /* N * sizeof(L) should not exceed 1MB less 1 "L", i.e float: 262143 / double: 131071 */
#endif


#ifdef  TRACE

static FILE *s_trace;

static char *s_t0  = "%s (%d)\n", 
            *s_t1  = "%s (%d): (x) %lx\n", 
            *s_t1c = "%s (%d): (c) %c\n", 
            *s_t1s = "%s (%d): (s) %s\n", 
            *s_t2  = "%s (%d): (x) %lx %lx\n", 
            *s_t3  = "%s (%d): (x) %lx %lx %lx\n";

#define T0()            fprintf(s_trace, s_t0,  __func__, __LINE__)
#define T1(p1)          fprintf(s_trace, s_t1,  __func__, __LINE__, *(I*)p1)
#define T1c(c1)         fprintf(s_trace, s_t1c, __func__, __LINE__, c1)
#define T1s(s1)         fprintf(s_trace, s_t1s, __func__, __LINE__, s1)
#define T2(p1, p2)      fprintf(s_trace, s_t2,  __func__, __LINE__, *(I*)p1, *(I*)p2)
#define T3(p1, p2, p3)  fprintf(s_trace, s_t3,  __func__, __LINE__, *(I*)p1, *(I*)p2, *(I*)p3)

#else

#define T0()            /**/
#define T1(p1)          /**/
#define T1c(c1)         /**/
#define T1s(s1)         /**/
#define T2(p1, p2)      /**/
#define T3(p1, p2, p3)  /**/

#endif

L eval(L x, L e), Read(), parse();
void print(L t);

I hp = 0, sp = N, ATOM = 0x7FC, PRIM = 0x7FD, CONS = 0x7FE, CLOS = 0x7FF, NIL = 0xFFF;
L *cell, nil, tru, err, env;
static FILE *s_input;

L box(I t, I i) { L x; T2(&t, &i); *(uint32_t*)&x = (uint32_t)t<<20|i; return x; }
I ord(L x) { T1(&x); return *(uint32_t*)&x & 0xfffff; }
L num(L n) { T1(&n); return n; }
I equ(L x, L y) { T2(&x, &y); return *(uint32_t*)&x == *(uint32_t*)&y; }
L atom(const char *s) {
    I i;
    T1s(s);
    i = 0; while (i < hp && strcmp(A+i, s)) i += strlen(A + i) + 1;
    if (i == hp && (hp += strlen(strcpy(A + i, s)) + 1) > sp * sizeof(L)) abort();
    return box(ATOM, i);
}
L cons(L x, L y) { T2(&x, &y); cell[--sp] = x; cell[--sp] = y; if (hp > sp*sizeof(L)) abort(); return box(CONS, sp); }
L car(L p) { T1(&p); return (T(p)&~(CONS^CLOS)) == CONS ? cell[ord(p)+1] : err; }
L cdr(L p) { T1(&p); return (T(p)&~(CONS^CLOS)) == CONS ? cell[ord(p)] : err; }
L pair(L v, L x, L e) { T3(&v, &x, &e); return cons(cons(v, x), e); }
L closure(L v, L x, L e) { T3(&v, &x, &e); return box(CLOS, ord(pair(v, x, equ(e, env) ? nil : e))); }
L assoc(L v, L e) { T2(&v, &e); while (T(e) == CONS && !equ(v, car(car(e)))) e = cdr(e); return T(e) == CONS ? cdr(car(e)) : err; }
I not(L x) { T1(&x); return T(x) == NIL; }
I let(L x) { T1(&x); return !not(x) && !not(cdr(x)); }
L evlis(L t, L e) {
    L s, *p;
    T2(&t, &e);
    for (s = nil, p = &s; T(t) == CONS; p = cell + sp, t = cdr(t)) { *p = cons(eval(car(t), e), nil); }
    if (T(t) == ATOM) *p = assoc(t, e);
    return s;
}
L evarg(L *t, L *e, I *a) {
    L x;
    T3(t, e, a);
    if (T(*t) == ATOM && !*a) *t = assoc(*t, *e), *a = 1;
    x = car(*t); *t = cdr(*t);
    return *a ? x : eval(x, *e);
}
/* Built-ins */
L f_eval(L t, L *e) { I a; T2(&t, e); a = 0; return evarg(&t, e, &a); }
L f_quote(L t, L *e) { T1(&t); (void)e; return car(t); }
L f_cons(L t, L *e) { I a; L x; T2(&t, e); a = 0; x = evarg(&t, e, &a); return cons(x, evarg(&t, e, &a)); }
L f_car(L t, L *e) { I a; T2(&t, e); a = 0; return car(evarg(&t, e, &a)); }
L f_cdr(L t, L *e) { I a; T2(&t, e); a = 0; return cdr(evarg(&t, e, &a)); }
L f_add(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); while (!not(t)) n += evarg(&t, e, &a); return num(n); }
L f_sub(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); while (!not(t)) n -= evarg(&t, e, &a); return num(n); }
L f_mul(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); while (!not(t)) n *= evarg(&t, e, &a); return num(n); }
L f_div(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); while (!not(t)) n /= evarg(&t, e, &a); return num(n); }
L f_int(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); return trunc(n); }
L f_lt(L t, L *e) { I a; L n; T2(&t, e); a = 0; n = evarg(&t, e, &a); return n - evarg(&t, e, &a) < 0 ? tru : nil; }
L f_eq(L t, L *e) { I a; L x; T2(&t, e); a = 0; x = evarg(&t, e, &a); return equ(x, evarg(&t, e, &a)) ? tru : nil; }
L f_pair(L t, L *e) { I a; L x; T2(&t, e); a = 0; x = evarg(&t, e, &a); return T(x) == CONS ? tru : nil; }
L f_not(L t, L *e) { I a; T2(&t, e); a = 0; return not(evarg(&t, e, &a)) ? tru : nil; }
L f_or(L t, L *e) { I a; L x; T2(&t, e); a = 0; x = nil; while (!not(t) && not(x)) x = evarg(&t, e, &a); return x; }
L f_and(L t, L *e) { I a; L x; T2(&t, e); a = 0; x = tru; while (!not(t) && !not(x)) x = evarg(&t, e, &a); return x; }
L f_cond(L t, L *e) { T2(&t, e); while (not(eval(car(car(t)), *e))) t = cdr(t); return car(cdr(car(t))); }
L f_if(L t, L *e) { T2(&t, e); return car(cdr(not(eval(car(t), *e)) ? cdr(t) : t)); }
L f_leta(L t, L *e) { L u; T2(&t, e); for (; let(t); t = cdr(t)) { u = car(car(t)); *e = pair(u, eval(car(cdr(car(t))), *e), *e); } return car(t); }
L f_lambda(L t, L *e) { L u; T2(&t, e); u = car(t); return closure(u, car(cdr(t)), *e); }
L f_define(L t, L *e) { L u, v; T2(&t, e); u = car(t); v = cdr(t); env = pair(u, T(v) != NIL? eval(car(v), *e):v, env); return u; }
L f_print(L t, L *e) {
    I a;
    T2(&t, e);
    a = 0;
    putchar('(');
    while (!not(t)) {
        print(evarg(&t, e, &a));
        if (!not(t))
            putchar(' ');
    }
    putchar(')');
    /* Returning QNAN to keep the output "clean" */
    *(I*)&t = QNAN;
    return t;
}
L f_load(L t, L *e) {
    I a;
    L x;
    FILE *i;
    char f[FILENAME_MAX];
    T2(&t, e);
    a = 0;
    x = evarg(&t, e, &a);
    if (T(x) != ATOM)
        return err;
    snprintf(f, FILENAME_MAX, "%s.lisp", A + ord(x));
    /* Swap the input with the new file, easy-peasy! */
    if (i = fopen(f, "r"))
        s_input = i;
    else
        return err;
    return x;
}
L incr_or_decr(L t, L *e, L d) {
    I a;
    L x, v, f, b, r;
    T2(&t, e);
    a = 0;
    r = nil;
    while (!not(t)) {
        x = car(t);
        if (T(x) == ATOM) {
            v = eval(x, *e);
            if (isfinite(v)) {
                /* Look for the source */
                f = *e;
                while (!not(f)) {
                    b = car(f);
                    if (equ(car(b), x)) {
                        /* We have a match, mutate the value! */
                        r = cell[ord(b)] = v + d;
                        break;
                    }
                    f = cdr(f);
                }
            }
        }
        t = cdr(t);
    }
    return r;
}
L f_incr(L t, L *e) { T2(&t, e); return incr_or_decr(t, e, +1.0); }
L f_decr(L t, L *e) { T2(&t, e); return incr_or_decr(t, e, -1.0); }
L f_while(L t, L *e) {
    L c, b, w, r;
    T2(&t, e);
    c = car(t);
    b = cdr(t);
    r = nil;
    while (1) {
        if (not(eval(c, *e))) break;
        w = b;
        while (!not(w)) {
            r = eval(car(w), *e);
            w = cdr(w);
        }
    }
    return r;
}
L f_bye(L t, L *e) {
    T2(&t, e);
    (void)t;
    (void)e;
#ifdef TRACE
        fclose(s_trace);
#endif
    printf("bye!\n");
    exit(0);
}

struct { const char *s; L (*f)(L, L*); short t; } prim[] = {
    {"eval",  f_eval,  1}, {"quote", f_quote, 0}, {"cons", f_cons, 0}, {"car",    f_car,    0}, {"cdr",    f_cdr,    0},
    {"+",     f_add,   0}, {"-",     f_sub,   0}, {"*",    f_mul,  0}, {"/",      f_div,    0}, {"int",    f_int,    0},
    {"<",     f_lt,    0}, {"eq?",   f_eq,    0}, {"or",   f_or,   0}, {"and",    f_and,    0}, {"not",    f_not,    0},
    {"cond",  f_cond,  1}, {"if",    f_if,    1}, {"let*", f_leta, 1}, {"lambda", f_lambda, 0}, {"define", f_define, 0},
    {"pair?", f_pair,  0}, {"print", f_print, 0}, {"load", f_load, 0}, {"incr",   f_incr,   0}, {"decr",   f_decr,   0},
    {"while", f_while, 1}, {"bye",   f_bye,   0}, {0}
};

void assign(L v, L x, L e) { T3(&v, &x, &e); while (!equ(v, car(car(e)))) e = cdr(e); cell[ord(car(e))] = x; }
L eval(L x, L e) {
    I a; L f, v, d, g, h, t;
    T2(&x, &e);
    g = nil;
    while (1) {
        if (T(x) == ATOM) return assoc(x, e);
        if (T(x) != CONS) return x;
        f = eval(car(x), e); x = cdr(x);
        if (T(f) == PRIM) {
            x = prim[ord(f)].f(x, &e);
            if (prim[ord(f)].t) continue;
            return x;
        }
        if (T(f) != CLOS) return err;
        v = car(car(f));
        if (equ(f, g)) d = e;
        else if (not(d = cdr(f))) d = env;
        for (a = 0; T(v) == CONS; v = cdr(v)) { t = car(v); d = pair(t, evarg(&x, &e, &a), d); };
        if (T(v) == ATOM) d = pair(v, a ? x : evlis(x, e), d);
        if (equ(f, g)) {
            for (; !equ(d, e) && sp == ord(d); d = cdr(d), sp += sizeof(L)) { t = car(car(d)); assign(t, cdr(car(d)), e); }
            for (; !equ(d, h) && sp == ord(d); d = cdr(d)) sp += sizeof(L);
        }
        x = cdr(car(f)); e = d; g = f; h = e;
    }
}
char buf[BUF], see = ' ';
void look() {
    int c;
    T0(); see = c = fgetc(s_input);
    if (c == EOF) {
        /* EOF usually occurs after reading a file, so restore stdin */
        if (s_input != stdin) {
            fclose(s_input);
            s_input = stdin;
        }
        see = ' ';
    }
}
I seeing(char c) { T1c(c); return c == ' ' ? see > 0 && see <= c : see == c; }
char get() { char c; T0(); c = see; look(); return c; }
char scan() {
    int i;
    T0();
    i = 0;
    while (seeing(' ')) look();
    if (seeing('(') || seeing(')') || seeing('\'')) buf[i++] = get();
    else do buf[i++] = get(); while (i < (BUF - 1) && !seeing('(') && !seeing(')') && !seeing(' '));
    return buf[i] = 0, *buf;
}
L Read() { T0(); scan(); return parse(); }
L list() {
    L t, *p;
    T0();
    for (t = nil, p = &t; ; ) {
        if (scan() == ')') return t;
        if (*buf == '.' && !buf[1]) { Read(); scan(); return *p = t; }
        *p = cons(parse(), nil); p = cell+sp;
    }
}
L parse() {
    L n, t; int i, j;
    T0();
    if (*buf == '(') return list();
    if (*buf == '\'') { t = atom("quote"); return cons(t, cons(Read(), nil)); }
    j = sscanf(buf, "%g%n", &n, &i);
    return j > 0 && isfinite(n) && !buf[i] ? n : atom(buf);
}
void printlist(L t) {
    T1(&t);
    for (putchar('('); ; putchar(' ')) {
        print(car(t));
        if (not(t = cdr(t))) break;
        if (T(t) != CONS) { printf(" . "); print(t); break; }
    }
    putchar(')');
}
void print(L x) {
    I t;
    T1(&x);
    if (*(I *)&x == QNAN) return;
    t = T(x);
    if (t == NIL) printf("()");
    else if (t == ATOM) printf("%s", A + ord(x));
    else if (t == PRIM) printf("<%s>", prim[ord(x)].s);
    else if (t == CONS) printlist(x);
    else if (t == CLOS) printf("{%" PRIX32 "}", ord(x));
    else printf("%.3g", x);
}
void gc() { T0(); sp = ord(env); }
int main() {
    I i;
    L t;
#ifdef TRACE
    s_trace = fopen("trace.log", "w");
#endif
    T0();
    s_input = stdin;
#ifdef  __CX16__
    *CTRL = BANK;
    cell = HMEM;
#else
    cell = calloc(N, sizeof(L));
#endif
    nil = box(NIL, 0);
    err = atom("ERR");
    tru = atom("#t");
    env = pair(tru, tru, nil);
    printf("tinylisp");
    for (i = 0; prim[i].s; ++i) {
        t = atom(prim[i].s);
        env = pair(t, box(PRIM, i), env);
    }
    while (1) {
        printf("\n%" PRIu32 "> ", (I)(sp - hp / sizeof(L)));
        print(eval(Read(), env));
        gc();
    }
}
