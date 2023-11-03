#include "regexer.h"

CheckRes Symbol::check(const std::string::iterator& iter) const {
    if (*iter == sym)
        return {true, std::next(iter)};
    else 
        return {false, iter};
}


CheckRes Star::check(const std::string::iterator& iter) const {
    CheckRes next = {true, iter};
    do {
        next = value->check(next.second);
    } while (next.first);
    return {true, next.second};
}

CheckRes Union::check(const std::string::iterator& iter) const {
    auto left_res = left->check(iter);
    if (left_res.first)
        return {true, left_res.second};
    else {
        auto right_res = right->check(iter);
        if (right_res.first)
            return {true, right_res.second};
    }

    return {false, iter};
}

CheckRes Concat::check(const std::string::iterator& iter) const { 
    auto left_res = left->check(iter);
    if (left_res.first) {
        auto right_res = right->check(left_res.second);
        if (right_res.first) {
            return {true, right_res.second};
        }
    }
    return {false, iter};
}

CheckRes Empty::check(const std::string::iterator& iter) const {
    return {true, iter};
}

Symbol* Symbol::clone() const {
    return new Symbol(this->sym);
}

Star* Star::clone() const {
    return new Star(this->value->clone());
}

Union* Union::clone() const {
    return new Union(this->left->clone(), this->right->clone());
}

Concat* Concat::clone() const {
    return new Concat(this->left->clone(), this->right->clone());
}

Empty* Empty::clone() const {
    return new Empty();
}

std::vector<Token> Symbol::reverse_polish() const {
    return {Token(Token::Symbol, this->sym)};
}

std::vector<Token> Star::reverse_polish() const {
    auto res = this->value->reverse_polish();
    res.push_back(Token(Token::Star));
    return res;
}

std::vector<Token> Union::reverse_polish() const {
    auto left = this->left->reverse_polish();
    auto right = this->right->reverse_polish();
    auto res = std::vector<Token>();
    for (auto token : left) 
        res.push_back(token);
    for (auto token : right) 
        res.push_back(token);
    res.push_back(Token(Token::Union));
    return res;
}

std::vector<Token> Concat::reverse_polish() const {
    auto left = this->left->reverse_polish();
    auto right = this->right->reverse_polish();
    auto res = std::vector<Token>();
    for (auto token : left) 
        res.push_back(token);
    for (auto token : right) 
        res.push_back(token);
    res.push_back(Token(Token::Concat));
    return res;
}

std::vector<Token> Empty::reverse_polish() const {
    return {Token(Token::Empty)};
}

CheckRes Negate::check(const std::string::iterator& iter) const {
    return {false, iter};
}

Negate* Negate::clone() const {
    return new Negate(this->value->clone());
}

std::vector<Token> Negate::reverse_polish() const {
    return {};
}

auto RegexTransformer::to_regex() -> Regex* {
    for (auto token : ts) {
        switch (token.type)
        {
        case Token::Symbol: {
            stack.push_back(new Symbol(token.value.symbol));
            break;
        }
        case Token::Star: {
            auto inner = stack.back();
            stack.pop_back();
            stack.push_back(new Star(inner));
            break;
        }
        case Token::Union: {
            auto right =  stack.back();
            stack.pop_back();
            auto left = stack.back();
            stack.pop_back();
            stack.push_back(new Union(left, right));
            break;
        }
        case Token::Concat: {
            auto right =  stack.back();
            stack.pop_back();
            auto left = stack.back();
            stack.pop_back();
            stack.push_back(new Concat(left, right));
            break;
        }
        case Token::Plus: {
            auto inner = stack.back();
            stack.pop_back();
            stack.push_back(new Concat(inner, new Star(inner->clone())));
            break;
        }
        case Token::Optional: {
            auto inner = stack.back();
            stack.pop_back();
            stack.push_back(new Union(inner, new Empty()));
            break;
        }
        case Token::DupExact: {
            if (token.value.min <= 0)
                throw CompileError("Exact match duplicant count cannot be zero");
            if (token.value.min == 1)
                break;
            auto inner = stack.back();
            stack.pop_back();
            auto loop = new Concat(inner, inner->clone());
            for (unsigned i=2;i<token.value.min; i++) {
                loop = new Concat(loop, inner->clone());
            }
            stack.push_back(loop);
            break;
        }
        case Token::DupMin: {
            if (token.value.min == 0) {
                auto inner = stack.back();
                stack.pop_back();
                stack.push_back(new Star(inner));
                break;
            }
            auto inner = stack.back();
            stack.pop_back();
            auto loop = inner;
            for (unsigned i=0;i<token.value.min-1;i++) {
                loop = new Concat(loop, inner->clone());
            }
            loop = new Concat(loop, new Star(inner->clone()));
            stack.push_back(loop);
            break;
        }
        case Token::DupMinMax: {
            if (token.value.minmax.min > token.value.minmax.max)
                throw CompileError("Minimum number of matches cannot be higher than maximum");

            if (token.value.minmax.min == 0)
                throw CompileError("Minimum number of matches cannot be zero.");

            if (token.value.minmax.min == token.value.minmax.max) {
                if (token.value.minmax.min == 1)
                    break;
                auto inner = stack.back();
                stack.pop_back();
                auto loop = new Concat(inner, inner->clone());
                for (unsigned i=2;i<token.value.minmax.min; i++) {
                    loop = new Concat(loop, inner->clone());
                }
                stack.push_back(loop);
                break;
            }
            auto inner = stack.back();
            stack.pop_back();
            auto loop = inner;
            for (unsigned i=1;i<token.value.minmax.min;i++) {
                loop = new Concat(loop, inner->clone());
            }
            for (unsigned i=token.value.minmax.min;i<token.value.minmax.max;i++) {
                loop = new Concat(loop, new Union(inner->clone(), new Empty()));
            }
            stack.push_back(loop);
            break;
        }
        case Token::BracketRange: {
            auto end = stack.back();
            stack.pop_back();
            auto start = stack.back();
            stack.pop_back();
            Symbol* end_sym = (Symbol*)end;
            Symbol* start_sym = (Symbol*)start;

            if (end_sym->sym <= start_sym->sym)
                throw CompileError("Range end must be a higher character than start.");
            
            auto acc = new Union(start_sym, end_sym);
            for (char sym = start_sym->sym + 1; sym < end_sym->sym; sym++) {
                // TODO: left off here
                acc = new Union(acc, new Symbol(sym));
            }
            stack.push_back(acc);
            break;
        }
        case Token::BracketConcat: {
            auto right = stack.back();
            stack.pop_back();
            auto left = stack.back();
            stack.pop_back();
            stack.push_back(new Union(left, right));
            break;
        }
        case Token::BracketExpr: {
            break;
        }
        default:
            break;
        }
    }
    auto res = stack.back();
    stack.pop_back();
    return res;
}

auto operator <<(std::ostream& os, CheckRes res) -> decltype(os) {
    os << "{" << res.first << ", " << *res.second << "}";
    return os;
}