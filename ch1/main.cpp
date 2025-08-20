#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <map>
using namespace std;

typedef enum {A_plus,A_minus,A_times,A_div} A_binop;

struct A_exp_;
struct A_stm_;
struct A_expList_;

using A_stm = std::unique_ptr<A_stm_>;
using A_exp = std::unique_ptr<A_exp_>;
using A_expList = std::unique_ptr<A_expList_>;

using Env = map<string, int>;

struct A_stm_ {
    enum {A_compoundStm, A_assignStm, A_printStm} kind;

    struct compound {A_stm stm1, stm2;};
    struct assign {string id; A_exp exp;};
    struct print {A_expList exps;};

    variant<A_stm_::compound, A_stm_::assign, A_stm_::print> u;
};

A_stm A_CompoundStm(A_stm stm1, A_stm stm2);
A_stm A_AssignStm(string id, A_exp exp);
A_stm A_PrintStm(A_expList exps);

struct A_exp_ {
    enum {A_idExp, A_numExp, A_opExp, A_eseqExp} kind;

    struct op {A_exp left; A_binop oper; A_exp right;}; // for A_OpExp
    struct eseq {A_stm stm; A_exp exp;};

    variant<string, int, A_exp_::op, A_exp_::eseq> u;
};

A_exp A_IdExp(string id);
A_exp A_NumExp(int num);
A_exp A_OpExp(A_exp left, A_binop oper, A_exp right);
A_exp A_EseqExp(A_stm stm, A_exp exp);

struct A_expList_ {
    enum {A_pairExpList, A_lastExpList} kind;

    struct pair { A_exp head; A_expList tail; };
    A_exp last;

    variant<A_expList_::pair, A_exp> u;
};

A_exp A_IdExp(string id) {
    A_exp id_exp = make_unique<A_exp_>();
    id_exp->kind = A_exp_::A_idExp;
    id_exp->u = id;

    return id_exp;
}

// A_stm_ stuff

A_stm A_CompoundStm(A_stm stm1, A_stm stm2) {
    A_stm cmp_stm = make_unique<A_stm_>();
    cmp_stm->kind = A_stm_::A_compoundStm;

    A_stm_::compound compound { move(stm1), move(stm2) };

    cmp_stm->u = move(compound);

    return cmp_stm;
}

A_stm A_AssignStm(string id, A_exp exp) {
    A_stm assign_stm = make_unique<A_stm_>();
    assign_stm->kind = A_stm_::A_assignStm;

    A_stm_::assign assign { move(id), move(exp) };

    assign_stm->u = move(assign);

    return assign_stm;
}

A_stm A_PrintStm(A_expList exps) {
    A_stm print_stm = make_unique<A_stm_>();
    print_stm->kind = A_stm_::A_printStm;

    A_stm_::print print { move(exps) };
    print_stm->u = move(print);

    return print_stm;
}


// A_exp_ stuff

A_exp A_NumExp(int num) {
    A_exp num_exp = make_unique<A_exp_>();
    num_exp->kind = A_exp_::A_numExp;
    num_exp->u = num;

    return num_exp;
}

A_exp A_OpExp(A_exp left, A_binop oper, A_exp right) {
    A_exp op_exp = make_unique<A_exp_>();
    op_exp->kind = A_exp_::A_opExp;

    A_exp_::op op { std::move(left), oper, std::move(right) };

    op_exp->u = move(op);

    return op_exp;
}

A_exp A_EseqExp(A_stm stm, A_exp exp) {
    A_exp eseq_exp = make_unique<A_exp_>();
    eseq_exp->kind = A_exp_::A_eseqExp;

    A_exp_::eseq eseq { std::move(stm), std::move(exp) };

    eseq_exp->u = move(eseq);

    return eseq_exp;
}

// A_expList stuff

A_expList A_LastExpList(A_exp exp) {
    A_expList last_exp_list = make_unique<A_expList_>();
    last_exp_list->kind = A_expList_::A_lastExpList;
    last_exp_list->u = move(exp);

    return last_exp_list;
}

A_expList A_PairExpList(A_exp head, A_expList tail) {
    A_expList pair_exp_list = make_unique<A_expList_>();
    pair_exp_list->kind = A_expList_::A_pairExpList;

    A_expList_::pair pair { move(head), move(tail) };

    pair_exp_list->u = move(pair);

    return pair_exp_list;
}

void interpret_stm(const A_stm &stm, Env &mp);
int interpret_exp(const A_exp &exp, Env &mp);
int interpret_expList(const A_expList &exps, Env &mp);

void interpret_stm(const A_stm &stm, Env &mp) {
    if (stm->kind == A_stm_::A_compoundStm) {
        const auto &c = get<A_stm_::compound>(stm->u);
        interpret_stm(c.stm1, mp);
        interpret_stm(c.stm2, mp);
    }

    if (stm->kind == A_stm_::A_assignStm) {
        const auto &c = get<A_stm_::assign>(stm->u);
        int val = interpret_exp(c.exp, mp);
        string id = c.id;
        mp[id] = val;
    }

    if (stm->kind == A_stm_::A_printStm) {
        const auto &c = get<A_stm_::print>(stm->u);
        interpret_expList(c.exps, mp);
        cout << endl;
    }
}

int interpret_exp(const A_exp &exp, Env &mp) {
    int res;
    if (exp->kind == A_exp_::A_idExp) {
        string id = get<string>(exp->u);
        res = mp[id];
    } else if (exp->kind == A_exp_::A_numExp) {
        int num = get<int>(exp->u);
        res = num;
    } else if (exp->kind == A_exp_::A_opExp) {
        const auto &c = get<A_exp_::op>(exp->u);
        int l = interpret_exp(c.left, mp);
        int r = interpret_exp(c.right, mp);
        A_binop op = c.oper;
        if (op == A_plus) {
            res = l+r;
        } else if (op == A_minus) {
            res = l-r;
        } else if (op == A_times) {
            res = l*r;
        } else if (op == A_div) {
            res = l/r;
        }
    } else { // A_eseqExp
        const auto &c = get<A_exp_::eseq>(exp->u);
        interpret_stm(c.stm, mp);
        res = interpret_exp(c.exp, mp);
    }

    //cout << "exp: " << res << endl;

    return res;
}

int interpret_expList(const A_expList &exps, Env &mp) {
    int res = 0;
    if (exps->kind == A_expList_::A_pairExpList) {
        const auto &pair = get<A_expList_::pair>(exps->u);
        res = interpret_exp(pair.head, mp);
        cout << res << " ";
        interpret_expList(pair.tail, mp);
    } else {
        const auto &exp = get<A_exp>(exps->u);
        res = interpret_exp(exp, mp);
        cout << res << " ";
    }

    return res;
}


int main() {
    A_stm prog =
        A_CompoundStm(
            A_AssignStm(
                "a",
                A_OpExp(
                    A_NumExp(5), A_plus, A_NumExp(3)
                )
            ),
            A_CompoundStm(
                A_AssignStm(
                    "b",
                    A_EseqExp(
                        A_PrintStm(
                            A_PairExpList(
                                A_IdExp("a"),
                                A_LastExpList(
                                    A_OpExp(
                                        A_IdExp("a"), A_minus, A_NumExp(1)
                                    )
                                )
                            )
                        ),
                        A_OpExp(
                            A_NumExp(10), A_times, A_IdExp("a")
                        )
                    )
                ),
                A_PrintStm(
                    A_LastExpList(
                        A_IdExp("b")
                    )
                )
            )
        );

    Env mp;
    interpret_stm(prog, mp);
    return 0;
}
