#include "expr.hpp"

struct EvalVisitor : Visitor {

    static const uint32_t max_depth = 32;

    bool fail;
    uint32_t depth;
    double stack[max_depth];

    EvalVisitor()
    {
        depth = 0;
        fail = false;
    }

    void visit_int(IntValue *node)
    {
        if (fail) return;

        if (depth == max_depth) {
            fail = true;
            return;
        }

        stack[depth++] = node->val;
    }

    void visit_float(FloatValue *node)
    {
        if (fail) return;

        if (depth == max_depth) {
            fail = true;
            return;
        }

        stack[depth++] = node->val;
    }

    void visit_neg(NegOperation *node)
    {
        node->op->visit(*this);
        
        if (fail) return;

        if (depth < 1) {
            fail = true;
            return;
        }
        
        stack[depth-1] = -stack[depth-1];
    }

    void visit_add(AddOperation *node)
    {
        node->lhs->visit(*this);
        node->rhs->visit(*this);

        if (fail) return;

        if (depth < 2) {
            fail = true;
            return;
        }
        
        double rhs = stack[--depth];
        double lhs = stack[--depth];
        stack[depth++] = lhs + rhs;
    }

    void visit_sub(SubOperation *node)
    {
        node->lhs->visit(*this);
        node->rhs->visit(*this);

        if (fail) return;

        if (depth < 2) {
            fail = true;
            return;
        }
        
        double rhs = stack[--depth];
        double lhs = stack[--depth];
        stack[depth++] = lhs - rhs;
    }

    void visit_mul(MulOperation *node)
    {
        node->lhs->visit(*this);
        node->rhs->visit(*this);

        if (fail) return;

        if (depth < 2) {
            fail = true;
            return;
        }
        
        double rhs = stack[--depth];
        double lhs = stack[--depth];
        stack[depth++] = lhs * rhs;
    }

    void visit_div(DivOperation *node)
    {
        node->lhs->visit(*this);
        node->rhs->visit(*this);

        if (fail) return;

        if (depth < 2) {
            fail = true;
            return;
        }
        
        double rhs = stack[--depth];
        double lhs = stack[--depth];
        stack[depth++] = lhs / rhs;
    }
};

double eval(Expr *e)
{
    EvalVisitor visitor;
    e->visit(visitor);
    return visitor.stack[visitor.depth-1];
}
