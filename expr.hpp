#pragma once

#include <cstdint>

struct IntValue;
struct FloatValue;
struct NegOperation;
struct AddOperation;
struct SubOperation;
struct MulOperation;
struct DivOperation;

struct Visitor {
    virtual void visit_int(IntValue *node) = 0;
    virtual void visit_float(FloatValue *node) = 0;
    virtual void visit_neg(NegOperation *node) = 0;
    virtual void visit_add(AddOperation *node) = 0;
    virtual void visit_sub(SubOperation *node) = 0;
    virtual void visit_mul(MulOperation *node) = 0;
    virtual void visit_div(DivOperation *node) = 0;
};

struct Expr {
    virtual void visit(Visitor &visitor) = 0;
};

struct IntValue : Expr {

    int64_t val;

    IntValue(int64_t val_)
    {
        val = val_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_int(this);
    }
};

struct FloatValue : Expr {

    double val;

    FloatValue(double val_)
    {
        val = val_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_float(this);
    }
};

struct NegOperation : Expr {

    Expr *op;

    NegOperation(Expr *op_)
    {
        op = op_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_neg(this);
    }
};

struct AddOperation : Expr {

    Expr *lhs;
    Expr *rhs;

    AddOperation(Expr *lhs_, Expr *rhs_)
    {
        lhs = lhs_;
        rhs = rhs_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_add(this);
    }
};

struct SubOperation : Expr {

    Expr *lhs;
    Expr *rhs;

    SubOperation(Expr *lhs_, Expr *rhs_)
    {
        lhs = lhs_;
        rhs = rhs_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_sub(this);
    }
};

struct MulOperation : Expr {

    Expr *lhs;
    Expr *rhs;

    MulOperation(Expr *lhs_, Expr *rhs_)
    {
        lhs = lhs_;
        rhs = rhs_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_mul(this);
    }
};

struct DivOperation : Expr {

    Expr *lhs;
    Expr *rhs;

    DivOperation(Expr *lhs_, Expr *rhs_)
    {
        lhs = lhs_;
        rhs = rhs_;
    }

    virtual void visit(Visitor &visitor)
    {
        visitor.visit_div(this);
    }
};
