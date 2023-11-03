#include <iostream>
#include "regexer.h"

/*
".{5,32} "
"([a-fA-F0-9]{8}:){3}[a-fA-F0-9]{8}"
".{2,6}[^A-Z]{5,8}ABCD"
"AB[A-Z]{2,5}[a-z]{2,5}C"
".{0,30}ABCDE"
*/

auto main(int argc, char** argv) -> int {
    if (argc != 3) {
        std::cerr << "Usage: regexer <regex> <source>" << std::endl;
        return 1;
    }

    // Tokenize
    auto stream = tokenize(argv[1]);
    std::cout << "Token Stream:" << std::endl;
    for (auto& token : stream) {
        std::cout << token;
    }
    std::cout << std::endl;

    // Parse
    auto parser = Parser(stream);
    try {
        parser.parse();
    } catch (std::exception& e) {
        std::cerr << "[E] " << e.what() << std::endl;
        std::exit(1);
    }

    auto& rev = parser.reverse_polish();
    std::cout << "Parse Tree:" << std::endl;
    for (auto& token : rev) {
        std::cout << token;
    }
    std::cout << std::endl << std::endl;

    // Build AST
    auto trafo = RegexTransformer(rev);
    auto regex = trafo.to_regex();

    std::cout << "AST: " << std::endl;
    auto ast = regex->reverse_polish();
    for (auto& token : ast) {
        std::cout << token;
    }
    std::cout << std::endl << std::endl;

    auto src = std::string(argv[2]);
    auto check_res = regex->check(src.begin());
    
    std::cout << "Result: {" << check_res.first << ", ";
    for (auto it = check_res.second; it != src.end(); ++it) {
        std::cout << *it;
    }
    std::cout << "}" << std::endl;

    // std::cout << src << std::endl;
    std::cout << "Matched: ";
    
    for (auto it = src.begin(); it != check_res.second; ++it) {
        std::cout << *it;
    }
    std::cout << std::endl;

    delete regex;
    return 0;
}