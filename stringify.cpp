#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>
#include "expr.hpp"

struct StringifyVisitor : Visitor {

    char    *dst;
    uint32_t num;
    uint32_t max;

    void append(const char *str, uint32_t len)
    {
        if (num < max) {
            uint32_t cpy = std::min(len, max - num);
            memcpy(dst + num, str, cpy);
        }
        num += len;
    }

    StringifyVisitor(char *dst_, uint32_t max_)
    {
        dst = dst_;
        max = max_;
        num = 0;
    }

    void visit_int(IntValue *node)
    {
        std::ostringstream sstr;
        sstr << node->val;
        std::string str = sstr.str();
        append(str.c_str(), str.size());
    }

    void visit_float(FloatValue *node)
    {
        std::ostringstream sstr;
        sstr << node->val;
        std::string str = sstr.str();
        append(str.c_str(), str.size());
    }

    void visit_neg(NegOperation *node)
    {
        append("-", 1);
        node->op->visit(*this);
    }

    void visit_add(AddOperation *node)
    {
        append("(", 1);
        node->lhs->visit(*this);
        append(" + ", 3);
        node->rhs->visit(*this);
        append(")", 1);
    }

    void visit_sub(SubOperation *node)
    {
        append("(", 1);
        node->lhs->visit(*this);
        append(" - ", 3);
        node->rhs->visit(*this);
        append(")", 1);
    }

    void visit_mul(MulOperation *node)
    {
        append("(", 1);
        node->lhs->visit(*this);
        append(" * ", 3);
        node->rhs->visit(*this);
        append(")", 1);
    }

    void visit_div(DivOperation *node)
    {
        append("(", 1);
        node->lhs->visit(*this);
        append(" / ", 3);
        node->rhs->visit(*this);
        append(")", 1);
    }
};

uint32_t stringify(Expr *e, char *dst, uint32_t max)
{
    StringifyVisitor visitor(dst, max);
    e->visit(visitor);
    uint32_t num = visitor.num;

    if (max > 0) {
        if (num == max)
            num--;
        dst[num] = '\0';
    }

    return num;
}
