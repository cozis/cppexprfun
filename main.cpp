#include <stdio.h>
#include <stdint.h>
#include "expr.hpp"
#include "eval.hpp"
#include "parse.hpp"
#include "stringify.hpp"

uint32_t read_line(FILE *stream, char *dst, uint32_t max)
{
    uint32_t num = 0;
    for (char c; (c = getc(stream)) != '\n'; )
        if (num < max) dst[num++] = c;
    
    if (max > 0) {
        if (num == max)
            num--;
        dst[num] = '\0';
    }
    return num;
}

int main()
{
    for (;;) {

        uint32_t len;
        char text[1<<9];
        char pool[1<<12];

        /*
         * Read an expression from stdin
         */
        fprintf(stdout, "> ");
        len = read_line(stdin, text, sizeof(text));

        /*
         * Parse it
         */
        Expr *e = parse(text, len, pool, sizeof(pool));
        if (e == nullptr) {
            fprintf(stderr, "Error\n");
            continue;
        }

        /*
         * Print the expression and its result
         */
        stringify(e, text, sizeof(text));
        fprintf(stdout, "%s = %g\n", text, eval(e));
    }
    return 0;
}