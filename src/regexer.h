#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>

/*
Length modifier transformations
(*): Kleene-star, base case
(+): At least one, (regex)+ = (regex)(regex)*
(?): Optional, (regex)? = (regex)|EMPTY
*/

class Token {
public:
    enum TokenType {
        Empty,
        Union,
        Concat,
        Star,
        Plus,
        Optional,
        Symbol,
        LParen,
        RParen,
        END,
    };

    union TokenValue {
        char symbol;
    };

    TokenType type;
    TokenValue value;

    explicit Token(TokenType type, char value)
    : type(type)
    , value({.symbol = value})
    { }

    explicit Token(TokenType type)
    : type(type)
    {}
};

auto operator <<(std::ostream& os, Token t) -> decltype(os);

class SyntaxError : public std::exception {
public:
    virtual const char* what() const noexcept override {
        return "Syntax Error";
    }
};

class Parser {
private:
    std::vector<Token> ts;
    std::vector<Token> transformed;
    int pos = 0;
public:
    explicit Parser(const std::vector<Token>& ts)
    : ts(ts)
    , transformed({})
    {} 

    void parse() {
        ts.push_back(Token(Token::END));
        pos = 0;
        Regex();
        match(Token::END);
        std::cout << std::endl;
    }

    auto reverse_polish() const -> const std::vector<Token>& {
        return transformed;
    }
private:
    inline auto lookAhead() -> Token const {
        return ts[pos];
    }

    inline void match(Token::TokenType t) {
        if (lookAhead().type  == t)
            pos++;
        else
            throw SyntaxError();
    }

    inline auto expect(Token::TokenType t) -> bool {
        if (lookAhead().type == t) {
            match(t);
            return true;
        } else {
            return false;
        }
    }
private:
    void Regex();
    void Union();
    void Branch();
    void Concat();
    void Expression(); 
    void DupSymbol();
};

using CheckRes = std::pair<bool, std::string::iterator>;

class Regex {
public:
    virtual ~Regex() = default;

    virtual CheckRes check(const std::string::iterator& iter) const = 0;
    virtual Regex* clone() const = 0;
    virtual std::vector<Token> reverse_polish() const = 0;
};


class Symbol : public Regex {
private:
    char sym;
public:
    explicit Symbol(char sym) 
    : sym(sym)
    { }

    virtual ~Symbol() override = default;
    virtual CheckRes check(const std::string::iterator& iter) const override;
    virtual Symbol* clone() const override;
    virtual std::vector<Token> reverse_polish() const override;
};

class Star : public Regex {
private:
    Regex* value;
public:
    explicit Star(Regex* value)
    : value(value)
    { }

    virtual ~Star() override {
        delete value;
    }
    virtual CheckRes check(const std::string::iterator& iter) const override;
    virtual Star* clone() const override;
    virtual std::vector<Token> reverse_polish() const override;
};

class Union : public Regex {
private:
    Regex* left;
    Regex* right;
public:
    explicit Union(Regex* left, Regex* right)
    : left(left)
    , right(right)
    { }

    virtual ~Union() override {
        delete left;
        delete right;
    }
    virtual CheckRes check(const std::string::iterator& iter) const override;
    virtual Union* clone() const override;
    virtual std::vector<Token> reverse_polish() const override;
};

class Concat : public Regex {
private:
    Regex* left;
    Regex* right;
public:
    explicit Concat(Regex* left, Regex* right)
    : left(left)
    , right(right)
    { }

    virtual ~Concat() override {
        delete left;
        delete right;
    }
    virtual CheckRes check(const std::string::iterator& iter) const override;
    virtual Concat* clone() const override;
    virtual std::vector<Token> reverse_polish() const override;
};

class Empty : public Regex {
public:
    explicit Empty()
    { }

    virtual CheckRes check(const std::string::iterator& iter) const override;
    virtual Empty* clone() const override;
    virtual std::vector<Token> reverse_polish() const override;
};


class RegexTransformer {
private:
    std::vector<Token> ts;
    std::vector<Regex*> stack;
public:
    explicit RegexTransformer(const std::vector<Token>& ts)
    : ts(ts)
    , stack({})
    { }

    auto to_regex() -> Regex*;
};

auto tokenize(const std::string& src) -> std::vector<Token>;
auto operator <<(std::ostream& os, CheckRes res) -> decltype(os);