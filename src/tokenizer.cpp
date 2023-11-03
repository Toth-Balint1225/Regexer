#include "regexer.h"

auto operator <<(std::ostream& os, Token t) -> decltype(os) {
    switch (t.type) {
        case Token::Concat:
            os << "[.]";
            break;
        case Token::Symbol:
            os << "[" << "Sym, " << t.value.symbol << "]";
            break;
        case Token::Union:
            os << "[|]";
            break;
        case Token::LParen:
            os << "[(]";
            break;
        case Token::RParen:
            os << "[)]";
            break;
        case Token::LBrace:
            os << "[{]";
            break;
        case Token::RBrace:
            os << "[}]";
            break;
        case Token::Star:
            os << "[*]";
            break;
        case Token::Plus:
            os << "[+]";
            break;
        case Token::Optional:
            os << "[?]";
            break;
        case Token::DupMin:
            os << "[Min, " << t.value.min << "]";
            break;
        case Token::DupExact:
            os << "[Exact, " << t.value.min << "]";
            break;
        case Token::DupMinMax:
            os << "[MinMax, " << t.value.minmax.min<< ", " << t.value.minmax.max<< "]";
            break;
        case Token::Comma:
            os << "[,]";
            break;
        case Token::Caret:
            os << "[^]";
            break;
        case Token::Hyphen:
            os << "[-]";
            break;
        case Token::Empty:
            os << "[Empty]";
            break;
        case Token::LBracket:
            os << "[Left Bracket]";
            break;
        case Token::RBracket:
            os << "[Right Bracket]";
            break;
        case Token::BracketRange:
            os << "[Bracket Range]";
            break;
        case Token::BracketConcat:
            os << "[Bracket Concat]";
            break;
        case Token::Negate:
            os << "[Bracket Negate]";
            break;
        case Token::BracketExpr:
            os << "[Bracket Expression]";
            break;
        default:
            break;
    }
    return os;
}



auto tokenize(const std::string& src) -> std::vector<Token> {
    auto res = std::vector<Token>();
    bool bracket = false;

    for (auto it : src) {
        switch (it) {
        case ' ':
            continue;
        case '\n':
            continue;
        case '\t':
            continue;
        default:
            break;
        }
        if (bracket) {
            switch (it) {
            case ']': {
                res.push_back(Token(Token::RBracket));
                bracket = false;
                break;
            }
            case '-': {
                res.push_back(Token(Token::Hyphen));
                break;
            }
            case '^':
                res.push_back(Token(Token::Caret));
                break;
            case '[':
            res.push_back(Token(Token::LBracket));
                break;
            default:
                res.push_back(Token(Token::Symbol, it));
                break;
            }
        } else {
            switch (it) {
            case '*':
                res.push_back(Token(Token::Star));
                break;
            case '+':
                res.push_back(Token(Token::Plus));
                break;
            case '?':
                res.push_back(Token(Token::Optional));
                break;
            case '(':
                res.push_back(Token(Token::LParen));
                break;
            case ')':
                res.push_back(Token(Token::RParen));
                break;
            case '{':
                res.push_back(Token(Token::LBrace));
                break;
            case '}':
                res.push_back(Token(Token::RBrace));
                break;
            case ',':
                res.push_back(Token(Token::Comma));
                break;
            case '|':
                res.push_back(Token(Token::Union));
                break;
            case '^':
                res.push_back(Token(Token::Caret));
                break;
            case '[': {
                res.push_back(Token(Token::LBracket));
                bracket = true;
                break;
            }
            case ']': {
                res.push_back(Token(Token::LBracket));
                break;
            }
            default:
                res.push_back(Token(Token::Symbol, it));
                break;
            }
        }
    }

    return res;
}
