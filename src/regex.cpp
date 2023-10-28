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