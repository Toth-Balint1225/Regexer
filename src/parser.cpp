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
}
