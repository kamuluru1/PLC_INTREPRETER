#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <vector>

enum TokenType {
    INTEGER, PLUS, MINUS, MUL, DIV, LPAREN, RPAREN, EOF_TOKEN, ID, ASSIGN
};

class Token {
public:
    TokenType type;
    std::string value;

    Token(const TokenType type_, std::string value_) : type(type_), value(value_) {}

    void print() const {
        std::cout << "Token(" << type << ", " << value << ")" << std::endl;
    }
};

class Lexer {
public:
    std::string text;
    size_t pos;
    char current_char;

    Lexer(const std::string& text_) : text(text_), pos(0), current_char(text[pos]) {}

    void advance() {
        pos++;
        if (pos >= text.size()) {
            current_char = '\0'; // Null character to indicate end
        } else {
            current_char = text[pos];
        }
    }

    void skip_whitespace() {
        while (current_char != '\0' && std::isspace(current_char)) {
            advance();
        }
    }

    int integer() {
        std::string result = "";
        while (current_char != '\0' && std::isdigit(current_char)) {
            result += current_char;
            advance();
        }
        return std::stoi(result);
    }

    Token id() {
        std::string result = "";
        while (current_char != '\0' && std::isalnum(current_char) || current_char == '_') {
            result += current_char;
            advance();
        }
        return Token(ID, result);
    }

    Token get_next_token() {
        while (current_char != '\0') {
            if (std::isspace(current_char)) {
                skip_whitespace();
                continue;
            }

            if (std::isdigit(current_char)) {
                return Token(INTEGER, std::to_string(integer()));
            }

            if (current_char == '+') {
                advance();
                return Token(PLUS, "+");
            }

            if (current_char == '-') {
                advance();
                return Token(MINUS, "-");
            }

            if (current_char == '*') {
                advance();
                return Token(MUL, "*");
            }

            if (current_char == '/') {
                advance();
                return Token(DIV, "/");
            }

            if (current_char == '(') {
                advance();
                return Token(LPAREN, "(");
            }

            if (current_char == ')') {
                advance();
                return Token(RPAREN, ")");
            }

            if (current_char == '=') {
                advance();
                return Token(ASSIGN, "=");
            }

            std::string error_message = "Invalid character: ";
            error_message += current_char;
            throw std::runtime_error(error_message);
        }

        return Token(EOF_TOKEN, "");
    }
};

class Parser {
public:
    Lexer lexer;
    Token current_token;

    Parser(Lexer lexer_) : lexer(lexer_), current_token(lexer.get_next_token()) {}

    void error() {
        std::string error_message = "Syntax error: unexpected token type ";
        error_message += std::to_string(current_token.type);
        throw std::runtime_error(error_message);
    }

    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            error();
        }
    }

    int factor() {
        Token token = current_token;
        if (token.type == INTEGER) {
            eat(INTEGER);
            return std::stoi(token.value);
        } else if (token.type == LPAREN) {
            eat(LPAREN);
            int result = expr();
            eat(RPAREN);
            return result;
        }
        error();
        return 0; // Should never reach here
    }

    int term() {
        int result = factor();
        while (current_token.type == MUL || current_token.type == DIV) {
            Token token = current_token;
            if (token.type == MUL) {
                eat(MUL);
                result *= factor();
            } else if (token.type == DIV) {
                eat(DIV);
                result /= factor();
            }
        }
        return result;
    }

    int expr() {
        int result = term();
        while (current_token.type == PLUS || current_token.type == MINUS) {
            Token token = current_token;
            if (token.type == PLUS) {
                eat(PLUS);
                result += term();
            } else if (token.type == MINUS) {
                eat(MINUS);
                result -= term();
            }
        }
        return result;
    }
};

class Interpreter {
public:
    Parser parser;

    Interpreter(Parser parser_) : parser(parser_) {}

    int interpret() {
        return parser.expr();
    }
};

class SymbolTable {
public:
    struct Row {
        std::string type;
        std::string name;
        std::string scope;
        std::string address;
        std::string value;
    };

    std::vector<Row> table;

    void add(const std::string& type, const std::string& name, const std::string& scope,
         const std::string& address, const std::string& value) {
        Row new_row{type, name, scope, address, value}; // Aggregate initialization for Row
        table.push_back(new_row); // Add the row to the table vector
    }



};

int main() {
    // Prompt the user to enter the relative path to the file
    std::string file_path;
    std::cin >> file_path;

    // Try to open the file using the provided relative path
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file at " << file_path << std::endl;
        return 1;
    }

    // Read the file contents
    std::string text;
    std::string line;
    while (std::getline(file, line)) {
        text += line;
    }
    file.close();

    // Create lexer, parser, and interpreter
    Lexer lexer(text);
    Parser parser(lexer);
    Interpreter interpreter(parser);
    int result = interpreter.interpret();

    // Output the result
    std::cout << result << std::endl;

    return 0;
}