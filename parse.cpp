#include <new>
#include <cassert>
#include "parse.hpp"

struct Arena {
    char    *base;
    uint32_t size;
    uint32_t used;

    Arena()
    {
        base = nullptr;
        size = 0;
        used = 0;
    }

    void init(void *base_, uint32_t size_)
    {
        base = (char*) base_;
        size = size_;
        used = 0;
    }

    static bool is_pow2(uint32_t n)
    {
        return (n & (n-1)) == 0;
    }

    void *alloc(uint32_t num, uint32_t align)
    {
        assert(is_pow2(align));

        uint32_t pad = -(uintptr_t) (base + num) & (align-1);

        if (used + pad + num > size)
            return nullptr;

        void *ptr = base + used + pad;
        used += pad + num;

        return ptr;
    }
};

struct Scanner {
    const char *src;
    uint32_t cur;
    uint32_t len;
};

struct Token {
    enum Type {
        ADD,
        SUB,
        MUL,
        DIV,
        INT,
        FLOAT,
        OTHER,
        END,
    };
    Type type;
    union {
        double  _float;
        int64_t _int;
    } data;

    Token()
    {
        type = OTHER;
    }

    Token(Type type_)
    {
        type = type_;
        if (type == INT)
            data._int = 0;
        else
            data._float = 0;
    }

    Token(int64_t val)
    {
        type = INT;
        data._int = val;
    }

    Token(double val)
    {
        type = FLOAT;
        data._float = val;
    }
};

static bool is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static Token tokenize(Scanner &s)
{
    while (s.cur < s.len && is_space(s.src[s.cur]))
        s.cur++;
    
    if (s.cur == s.len)
        return Token::END;

    if (is_digit(s.src[s.cur])) {
        /*
         * May be either an integer or a float. Peek after the first
         * digit sequence to see if there's a dot
         */
        uint32_t peek = s.cur;
        while (peek < s.len && is_digit(s.src[peek]))
            peek++;
        bool no_dot = (peek == s.len || s.src[peek] != '.');

        if (no_dot) {
            
            /*
             * Parse integer token
             */

            int64_t buf = 0;
            do {
                int n = s.src[s.cur] - '0';

                if (buf > (INT64_MAX - n) / 10) {
                    /*
                     * Overflow. Consume all remaining digits and return
                     * the maximum representable value.
                     */
                    while (s.cur < s.len && is_digit(s.src[s.cur]))
                        s.cur++;
                    return INT64_MAX;
                }

                buf = buf * 10 + n;
                s.cur++;
            } while (s.cur < s.len && is_digit(s.src[s.cur]));
            
            return buf;

        } else {

            /*
             * Parse float token
             */

            double buf = 0;

            do {
                int n = s.src[s.cur] - '0';
                buf = buf * 10 + n;
                s.cur++;
            } while (s.src[s.cur] != '.');

            s.cur++; // Consume the dot

            /*
             * Consume the decimal part, if present
             */           
            if (s.cur < s.len && is_digit(s.src[s.cur])) {
                double q = 1;
                do {
                    q /= 10;
                    int n = s.src[s.cur] - '0';
                    buf += q * n;
                    s.cur++;
                } while (s.cur < s.len && is_digit(s.src[s.cur]));
            }

            return buf;
        }
    }

    if (s.src[s.cur] == '+') {
        s.cur++;
        return Token::ADD;
    }

    if (s.src[s.cur] == '-') {
        s.cur++;
        return Token::SUB;
    }

    if (s.src[s.cur] == '*') {
        s.cur++;
        return Token::MUL;
    }

    if (s.src[s.cur] == '/') {
        s.cur++;
        return Token::DIV;
    }

    /*
     * Unexpected character
     */
    return Token::OTHER;
}

static int precedence_of(Token token)
{
    switch (token.type) {
        case Token::ADD:
        case Token::SUB:
        return 0;

        case Token::MUL:
        case Token::DIV:
        return 1;

        default:break;
    }

    return -1;
}

static bool is_right_associative(Token op)
{
    (void) op;
    return false;
}

/*
 * Returns true if the next token is an operator
 * with a greater or equal precedence than the one
 * specified. The following operator is stored in
 * "op", but is only consimed when true is returned.
 */
static bool
follows_oper_of_higher_precedence(Scanner &s, Token &op, int min)
{
    uint32_t save = s.cur;
    op = tokenize(s);
    
    if (precedence_of(op) >= min)
        return true;

    s.cur = save;
    return false;
}

static bool should_associate_right(Scanner &s, Token op, Token &peek)
{
    uint32_t save = s.cur;
    peek = tokenize(s);
    s.cur = save;

    int p1 = precedence_of(op);
    int p2 = precedence_of(peek);

    if (p1 < p2)
        return true;
    
    if (p1 == p2 && is_right_associative(peek))
        return true;
    
    return false;
}

static Expr *make_int(Arena &a, int64_t raw)
{
    void *p = a.alloc(sizeof(IntValue), alignof(IntValue));
    if (p == nullptr)
        return nullptr;

    return new (p) IntValue(raw);
}

static Expr *make_float(Arena &a, double raw)
{
    FloatValue *n;
    
    n = (FloatValue*) a.alloc(sizeof(FloatValue), alignof(FloatValue));
    if (n == nullptr)
        return nullptr;
    
    new (n) FloatValue(raw);
    return n;
}

static Expr *make_neg(Arena &a, Expr *operand)
{
    NegOperation *n;

    n = (NegOperation*) a.alloc(sizeof(NegOperation), alignof(NegOperation));
    if (n == nullptr)
        return nullptr;
    
    new (n) NegOperation(operand);
    return n;
}

static Expr *parse_primary(Scanner &s, Arena &a)
{
    Token t;

    /*
     * Handle preceding unary operators
     */
    bool neg = false;
    for (;;) {
        t = tokenize(s);
        if (t.type == Token::SUB)
            neg = !neg;
        else {
            if (t.type != Token::ADD)
                break;
        }
    }

    Expr *e;
    switch (t.type) {

        case Token::INT   : e = make_int  (a, t.data._int);   break;
        case Token::FLOAT : e = make_float(a, t.data._float); break;

        default:
        case Token::END:
        case Token::OTHER:
        return nullptr;
    }
    if (e == nullptr)
        return nullptr;

    if (neg)
        return make_neg(a, e); // May return NULL
    else
        return e;
}

static Expr *make_add(Arena &a, Expr *lhs, Expr *rhs)
{
    AddOperation *n;

    n = (AddOperation*) a.alloc(sizeof(AddOperation), alignof(AddOperation));
    if (n == nullptr)
        return nullptr;

    new (n) AddOperation(lhs, rhs);
    return n;
}

static Expr *make_sub(Arena &a, Expr *lhs, Expr *rhs)
{
    SubOperation *n;

    n = (SubOperation*) a.alloc(sizeof(SubOperation), alignof(SubOperation));
    if (n == nullptr)
        return nullptr;

    new (n) SubOperation(lhs, rhs);
    return n;
}

static Expr *make_mul(Arena &a, Expr *lhs, Expr *rhs)
{
    MulOperation *n;

    n = (MulOperation*) a.alloc(sizeof(MulOperation), alignof(MulOperation));
    if (n == nullptr)
        return nullptr;

    new (n) MulOperation(lhs, rhs);
    return n;
}

static Expr *make_div(Arena &a, Expr *lhs, Expr *rhs)
{
    DivOperation *n;

    n = (DivOperation*) a.alloc(sizeof(DivOperation), alignof(DivOperation));
    if (n == nullptr)
        return nullptr;

    new (n) DivOperation(lhs, rhs);
    return n;
}

/*
 * Got this algorithm from:
 *     https://en.wikipedia.org/wiki/Operator-precedence_parser
 */
static Expr *parse_expression_2(Scanner &s, Arena &a,
                                Expr *lhs, int min_precedence)
{
    for (Token op; follows_oper_of_higher_precedence(s, op, min_precedence); ) {
         
         Expr *rhs = parse_primary(s, a);
         if (rhs == nullptr)
            return nullptr;
        
        for (Token peek; should_associate_right(s, op, peek); ) {
            
            int p1 = precedence_of(op);
            int p2 = precedence_of(peek);

            rhs = parse_expression_2(s, a, rhs, p1 + (p2 > p1));
            if (rhs == nullptr)
                return nullptr;
        }

        switch (op.type) {
            case Token::ADD: lhs = make_add(a, lhs, rhs); break;
            case Token::SUB: lhs = make_sub(a, lhs, rhs); break;
            case Token::MUL: lhs = make_mul(a, lhs, rhs); break;
            case Token::DIV: lhs = make_div(a, lhs, rhs); break;
            default:break;
        }
        if (lhs == nullptr)
            return nullptr;
    }

    return lhs;
}

Expr *parse(const char *src, uint32_t len,
            void *mem, uint32_t memlen)
{
    Arena a;
    a.init(mem, memlen);

    Scanner s;
    s.src = src;
    s.len = len;
    s.cur = 0;

    Expr *e;
    
    e = parse_primary(s, a);
    if (e == nullptr)
        return nullptr;
    
    return parse_expression_2(s, a, e, 0);
}
