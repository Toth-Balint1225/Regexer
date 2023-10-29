#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>

/*
Length modifier transformations
(*): Kleene-star, base case
(+): At least one, (regex)+ = (regex)(regex)*
(?): Optional, (regex)? = (regex)|EMPTY
({N}): exact number, (regex){3} = (regex)(regex)(regex)
({N,}): minimum number, (regex){2,} = (regex)(regex)(regex)*
({N,M}): minimum / maximum number: (regex){2,4} = (regex)(regex)((regex)|EMPTY)((regex)|EMPTY)
*/


class Token {
public:
    struct MinMax {
        unsigned min, max;
    };

    enum TokenType {
        Empty,
        Union,
        Concat,
        Star,
        Plus,
        DupExact,
        DupMin,
        DupMinMax,
        Comma,
        Optional,
        Symbol,
        LParen,
        RParen,
        LBrace,
        RBrace,
        END,
    };

    union TokenValue {
        char symbol;
        unsigned min;
        MinMax minmax;
    };

    TokenType type;
    TokenValue value;

    explicit Token(TokenType type, char value)
    : type(type)
    , value({.symbol = value})
    { }
    
    explicit Token(TokenType type, unsigned min)
    : type(type)
    , value({.min = min})
    { }

    explicit Token(TokenType type, unsigned min, unsigned max)
    : type(type)
    , value({.minmax = {min, max}})
    { }

    explicit Token(TokenType type)
    : type(type)
    { }
};

auto operator <<(std::ostream& os, Token t) -> decltype(os);

class SyntaxError : public std::exception {
private:
    std::string message;
public:
    explicit SyntaxError(const std::string& message)
    : message(message)
    { }

    explicit SyntaxError()
    : message("Syntax Error")
    { }

    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class CompileError : public std::exception {
private:
    std::string message;
public:
    explicit CompileError(const std::string& message)
    : message(message)
    { }

    explicit CompileError()
    : message("Compile Error")
    { }

    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class Parser {
private:
    std::vector<Token> ts;
    std::vector<Token> transformed;
    unsigned min, max;
    std::stringstream current_number;
    int pos = 0;
public:
    explicit Parser(const std::vector<Token>& ts)
    : ts(ts)
    , transformed({})
    , min(0)
    , max(0)
    , current_number("")
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
    inline auto is_digit(Token t) -> bool {
        if (t.type != Token::TokenType::Symbol)
            return false;

        std::string digits = "01233456789";
        return std::find(digits.begin(), digits.end(), t.value.symbol) != digits.end();
    }

    inline void reset_current() {
        current_number.str(std::string());
    }

private:
    void Regex();
    void Union();
    void Branch();
    void Concat();
    void Expression(); 
    void DupSymbol();
    void DupCount();
    void MaxCount();
    void Number();
    void Number_I();
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

    ~RegexTransformer() {
        for (auto& regex : stack)
            delete regex;
    }

    auto to_regex() -> Regex*;
};

auto tokenize(const std::string& src) -> std::vector<Token>;
auto operator <<(std::ostream& os, CheckRes res) -> decltype(os);