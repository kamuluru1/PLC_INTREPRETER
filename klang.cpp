#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <optional>
#include <unordered_map>

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
    while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
        result += current_char;
        advance();
    }
    return Token(ID, result);
}

    void handle_invalid_character() {
        std::string error_message = "Invalid character: ";
        error_message += current_char;
        throw std::runtime_error(error_message);
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
            return id();
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

        handle_invalid_character();
    }

    return Token(EOF_TOKEN, "");
}
};



class SymbolTable {
public:
    // Entry struct to hold type and value
    struct Entry {
        std::string type;
        std::string value; // For simplicity, the value is stored as a string
    };

private:
    // Stack to keep track of scopes
    std::vector<std::unordered_map<std::string, Entry>> scopes;

public:
    // Constructor initializes with a global scope
    SymbolTable() { pushScope(); }

    // Add a new scope
    void pushScope() {
        scopes.emplace_back();
    }

    // Remove the most recent scope
    void popScope() {
        if (scopes.size() <= 1) { // Prevent removing the global scope
            throw std::runtime_error("Cannot pop the global scope");
        }
        scopes.pop_back();
    }

    // Add or update a variable in the current scope
    void addOrUpdate(const std::string& name, const std::string& type, const std::string& value) {
        if (scopes.empty()) {
            throw std::runtime_error("No scope available to add variable");
        }
        auto& currentScope = scopes.back();
        // Check for type mismatch if the variable already exists
        if (currentScope.find(name) != currentScope.end() && currentScope[name].type != type) {
            throw std::runtime_error("Type mismatch: Cannot update variable with a different type");
        }
        currentScope[name] = {type, value};
    }

    // Retrieve a variable from any scope
    std::optional<Entry> get(const std::string& name) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) { 
            const auto& scope = *it;
            auto found = scope.find(name);
            if (found != scope.end()) {
                return found->second;
            }
        }
        return std::nullopt;
    }

    // Check if a variable exists in the current scope
    bool existsInCurrentScope(const std::string& name) const {
        if (scopes.empty()) {
            throw std::runtime_error("No scope available to check variable");
        }
        return scopes.back().find(name) != scopes.back().end();
    }

    // Print the symbol table for debugging
    void printTable() const {
        std::cout << "Symbol Table:\n";
        for (size_t i = 0; i < scopes.size(); ++i) {
            std::cout << "Scope " << i << ":\n";
            for (const auto& [name, entry] : scopes[i]) {
                std::cout << "  " << name << " -> { Type: " << entry.type << ", Value: " << entry.value << " }\n";
            }
        }
    }
};



class Parser {
public:
    Lexer lexer;
    Token current_token;
    SymbolTable& symbolTable;

    Parser(Lexer lexer_, SymbolTable& symbolTable_) : lexer(lexer_), current_token(lexer.get_next_token()), symbolTable(symbolTable_) {}

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

    void assignment() {
        Token token = current_token;
        if (token.type == ID) {
            std::string var_name = current_token.value;
            eat(ID);
            if (current_token.type == ASSIGN) {
                eat(ASSIGN);
                int result = expr();
                symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(result));
            } else {
                error();
            }
        }

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




int main() {
    std::string file_path;
    std::cin >> file_path;

    std::ifstream file(file_path); // use relative path from root directory
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
    SymbolTable symbolTable;
    Parser parser(lexer, symbolTable);

    // Perform assignment and print the symbol table
    parser.assignment();
    symbolTable.printTable();

    return 0;
}
