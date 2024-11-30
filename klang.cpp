#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <optional>
#include <unordered_map>

enum TokenType {
    INTEGER, PLUS, MINUS, MUL, DIV, LPAREN, RPAREN, EOF_TOKEN, ID, ASSIGN, COMMA, PRINT,
    EQUAL_TO, NOT_EQUAL_TO, GREATER_THAN, LESS_THAN, GREATER_THAN_OR_EQUAL_TO, LESS_THAN_OR_EQUAL_TO,
    IF, THEN, END
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
            current_char = '\0';
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

    Token get_next_token() {
        while (current_char != '\0') {
            if (std::isspace(current_char)) {
                skip_whitespace();
                continue;
            }

            if (std::isdigit(current_char)) {
                return Token(INTEGER, std::to_string(integer()));
            }

            if (std::isalpha(current_char)) {
                std::string id = "";
                while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
                    id += current_char;
                    advance();
                }
                if (id == "print") return Token(PRINT, "print");
                if (id == "if") return Token(IF, "if");
                if (id == "then") return Token(THEN, "then");
                if (id == "end") return Token(END, "end");
                return Token(ID, id);
            }

            if (current_char == '=') {
                advance();
                if (current_char == '=') {
                    advance();
                    return Token(EQUAL_TO, "==");
                }
                return Token(ASSIGN, "=");
            }

            if (current_char == '!') {
                advance();
                if (current_char == '=') {
                    advance();
                    return Token(NOT_EQUAL_TO, "!=");
                }
                throw std::runtime_error("Invalid operator: !");
            }
#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <optional>
#include <unordered_map>

enum TokenType {
    INTEGER, PLUS, MINUS, MUL, DIV, LPAREN, RPAREN, EOF_TOKEN, ID, ASSIGN, COMMA, PRINT
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

    Token get_next_token() {
        while (current_char != '\0') {
            if (std::isspace(current_char)) {
                skip_whitespace();
                continue;
            }

            if (std::isdigit(current_char)) {
                return Token(INTEGER, std::to_string(integer()));
            }

            if (std::isalpha(current_char)) {
                std::string id = "";
                while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
                    id += current_char;
                    advance();
                }
                if (id == "print") {
                    return Token(PRINT, "print");
                }
                return Token(ID, id);
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

            if (current_char == ',') {
                advance();
                return Token(COMMA, ",");
            }

            if (current_char == '=') {
                advance();
                return Token(ASSIGN, "=");
            }

            throw std::runtime_error(std::string("Invalid character: ") + current_char);
        }

        return Token(EOF_TOKEN, "");
    }
};

class SymbolTable {
public:
    struct Entry {
        std::string type;
        std::string value;
    };

private:
    std::unordered_map<std::string, Entry> table;

public:
    void addOrUpdate(const std::string& name, const std::string& type, const std::string& value) {
        if (table.find(name) != table.end() && table[name].type != type) {
            throw std::runtime_error("Type mismatch for variable: " + name);
        }
        table[name] = {type, value};
    }

    std::optional<Entry> get(const std::string& name) const {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

class Parser {
public:
    Lexer lexer;
    Token current_token;
    SymbolTable& symbolTable;

    Parser(Lexer lexer_, SymbolTable& symbolTable_) : lexer(lexer_), current_token(lexer.get_next_token()), symbolTable(symbolTable_) {}

    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
    }

    int factor() {
        Token token = current_token;
        if (token.type == INTEGER) {
            eat(INTEGER);
            return std::stoi(token.value);
        } else if (token.type == ID) {
            auto entry = symbolTable.get(token.value);
            if (!entry) {
                throw std::runtime_error("Undefined variable: " + token.value);
            }
            eat(ID);
            return std::stoi(entry->value);
        } else if (token.type == LPAREN) {
            eat(LPAREN);
            int result = expr();
            eat(RPAREN);
            return result;
        }
        throw std::runtime_error("Syntax error in factor");
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

    void assignment() {
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        int result = expr();
        symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(result));
    }

    void print_function() {
        eat(PRINT);
        eat(LPAREN);
        bool first = true;
        while (current_token.type != RPAREN) {
            if (!first) {
                std::cout << " ";
            }
            int value = expr();
            std::cout << value;
            if (current_token.type == COMMA) {
                eat(COMMA);
            }
            first = false;
        }
        eat(RPAREN);
        std::cout << std::endl;
    }

    void statement() {
        if (current_token.type == ID) {
            assignment();
        } else if (current_token.type == PRINT) {
            print_function();
        } else {
            throw std::runtime_error("Syntax error in statement");
        }
    }
};

int main() {
    std::string file_path;
    std::cin >> file_path;

    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file at " << file_path << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Lexer lexer(text);
    SymbolTable symbolTable;
    Parser parser(lexer, symbolTable);

    while (parser.current_token.type != EOF_TOKEN) {
        parser.statement();
    }

    return 0;
}();
                    return Token(LESS_THAN_OR_EQUAL_TO, "<=");
                }
                return Token(LESS_THAN, "<");
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

            if (current_char == ',') {
                advance();
                return Token(COMMA, ",");
            }

            throw std::runtime_error(std::string("Invalid character: ") + current_char);
        }

        return Token(EOF_TOKEN, "");
    }
};

class SymbolTable {
public:
    struct Entry {
        std::string type;
        std::string value;
    };

private:
    std::unordered_map<std::string, Entry> table;

public:
    void addOrUpdate(const std::string& name, const std::string& type, const std::string& value) {
        if (table.find(name) != table.end() && table[name].type != type) {
            throw std::runtime_error("Type mismatch for variable: " + name);
        }
        table[name] = {type, value};
    }

    std::optional<Entry> get(const std::string& name) const {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

class Parser {
public:
    Lexer lexer;
    Token current_token;
    SymbolTable& symbolTable;

    Parser(Lexer lexer_, SymbolTable& symbolTable_) 
        : lexer(lexer_), current_token(lexer.get_next_token()), symbolTable(symbolTable_) {}

    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
    }

    int factor() {
        Token token = current_token;
        if (token.type == INTEGER) {
            eat(INTEGER);
            return std::stoi(token.value);
        } else if (token.type == ID) {
            auto entry = symbolTable.get(token.value);
            if (!entry) {
                throw std::runtime_error("Undefined variable: " + token.value);
            }
            eat(ID);
            return std::stoi(entry->value);
        } else if (token.type == LPAREN) {
            eat(LPAREN);
            int result = expr();
            eat(RPAREN);
            return result;
        }
        throw std::runtime_error("Syntax error in factor");
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
                int divisor = factor();
                if (divisor == 0) {
                    throw std::runtime_error("Division by zero");
                }
                result /= divisor;
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

    bool condition() {
        int left = expr();
        TokenType op = current_token.type;
        
        switch(op) {
            case EQUAL_TO:
                eat(EQUAL_TO);
                return left == expr();
            case NOT_EQUAL_TO:
                eat(NOT_EQUAL_TO);
                return left != expr();
            case GREATER_THAN:
                eat(GREATER_THAN);
                return left > expr();
            case LESS_THAN:
                eat(LESS_THAN);
                return left < expr();
            case GREATER_THAN_OR_EQUAL_TO:
                eat(GREATER_THAN_OR_EQUAL_TO);
                return left >= expr();
            case LESS_THAN_OR_EQUAL_TO:
                eat(LESS_THAN_OR_EQUAL_TO);
                return left <= expr();
            default:
                throw std::runtime_error("Invalid comparison operator");
        }
    }

    void if_statement() {
        eat(IF);
        bool cond_result = condition();
        eat(THEN);
        
        if (cond_result) {
            while (current_token.type != END) {
                statement();
            }
        } else {
            // Skip statements until END
            int nesting = 1;
            while (nesting > 0) {
                if (current_token.type == IF) nesting++;
                else if (current_token.type == END) nesting--;
                current_token = lexer.get_next_token();
            }
        }
        eat(END);
    }

    void assignment() {
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        int result = expr();
        symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(result));
    }

    void print_function() {
        eat(PRINT);
        eat(LPAREN);
        bool first = true;
        while (current_token.type != RPAREN) {
            if (!first) {
                std::cout << " ";
            }
            int value = expr();
            std::cout << value;
            if (current_token.type == COMMA) {
                eat(COMMA);
            }
            first = false;
        }
        eat(RPAREN);
        std::cout << std::endl;
    }

    void statement() {
        if (current_token.type == IF) {
            if_statement();
        } else if (current_token.type == ID) {
            assignment();
        } else if (current_token.type == PRINT) {
            print_function();
        } else {
            throw std::runtime_error("Syntax error in statement");
        }
    }
};

int main() {
    std::string file_path;
    std::cout << "Enter the path to your source file: ";
    std::cin >> file_path;

    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Error: could not open file at " << file_path << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    try {
        Lexer lexer(text);
        SymbolTable symbolTable;
        Parser parser(lexer, symbolTable);

        while (parser.current_token.type != EOF_TOKEN) {
            parser.statement();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
