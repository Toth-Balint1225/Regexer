#include "regexer.h"

void Parser::Regex() {
    Branch();
    Union();
}

void Parser::Union() {
    if (expect(Token::Union)) {
        Branch();
        // std::cout << "| ";
        transformed.push_back(Token(Token::Union));
        Union();
    }
}

void Parser::Branch() {
    Expression();
    Concat();
}

void Parser::Concat() {
    if (expect(Token::LParen)) {
        Regex();
        match(Token::RParen);
        DupSymbol();
        transformed.push_back(Token(Token::Concat));
        Concat();
    }
    else if (expect(Token::Symbol)) {
        // std::cout << ts[pos-1].value.symbol << " ";
        transformed.push_back(Token(Token::Symbol, ts[pos-1].value.symbol));
        DupSymbol();
        transformed.push_back(Token(Token::Concat));
        Concat();
    }
    //std::cout << ". ";
}

void Parser::Expression() {
    if (expect(Token::LParen)) {
        Regex();
        match(Token::RParen);
        DupSymbol();
    }
    else if (expect(Token::Symbol)) {
        // std::cout << ts[pos-1].value.symbol << " ";
        transformed.push_back(Token(Token::Symbol, ts[pos-1].value.symbol));
        DupSymbol();
    }
}

void Parser::DupSymbol() {
    if (expect(Token::Star)) {
        transformed.push_back(Token(Token::Star));
    }
    else if (expect(Token::Plus)) {
        transformed.push_back(Token(Token::Plus));
    }
    else if (expect(Token::Optional)) {
        transformed.push_back(Token(Token::Optional));
    }
    else if (expect(Token::LBrace)) {
        Number();
        min = std::atoi(current_number.str().c_str());
        reset_current();
        DupCount();
    }
}

void Parser::DupCount() {
    if (expect(Token::RBrace)) {
        transformed.push_back(Token(Token::DupExact, min));
    }
    else if (expect(Token::Comma)) {
        MaxCount();
    }
}

void Parser::MaxCount() {
    if (expect(Token::RBrace)) {
        transformed.push_back(Token(Token::DupMin, min));
    }
    else {
        Number();
        max = std::atoi(current_number.str().c_str());
        reset_current();
        match(Token::RBrace);
        transformed.push_back(Token(Token::DupMinMax, min, max));
    }
}

void Parser::Number() {
    auto matched = ts[pos];
    match(Token::Symbol);
    if (!is_digit(matched))
        throw SyntaxError("Syntax Error: Expected numerical value in quantifier");
    current_number << matched.value.symbol;
    Number_I();
}

void Parser::Number_I() {
    auto matched = ts[pos];
    if (expect(Token::Symbol)) {
        if (!is_digit(matched))
            throw SyntaxError("Syntax Error: Expected numerical value in quantifier");
        current_number << matched.value.symbol;
        Number_I();
    }
}