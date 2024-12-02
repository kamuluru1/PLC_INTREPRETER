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
    IF, THEN, END, AND, OR, FOR, TO
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
        current_char = (pos >= text.size()) ? '\0' : text[pos];
    }

    void skip_whitespace() {
        while (current_char != '\0' && std::isspace(current_char)) {
            advance();
        }
    }

    int integer() {
        std::string result;
        while (current_char != '\0' && std::isdigit(current_char)) {
            result += current_char;
            advance();
        }
        return std::stoi(result);
    }

    Token handle_identifier() {
        std::string id;
        while (current_char != '\0' && (std::isalnum(current_char) || current_char == '_')) {
            id += current_char;
            advance();
        }

        static const std::unordered_map<std::string, TokenType> keywords = {
            {"print", PRINT},
            {"if", IF},
            {"then", THEN},
            {"end", END},
            {"and", AND},
            {"or", OR},
            {"for", FOR},
            {"to", TO}
        };

        auto it = keywords.find(id);
        return it != keywords.end() ? Token(it->second, id) : Token(ID, id);
    }

    Token handle_operator() {
        char op = current_char;
        advance();
        
        switch (op) {
            case '=':
                if (current_char == '=') {
                    advance();
                    return Token(EQUAL_TO, "==");
                }
                return Token(ASSIGN, "=");

            case '!':
                if (current_char == '=') {
                    advance();
                    return Token(NOT_EQUAL_TO, "!=");
                }
                throw std::runtime_error("Invalid operator: !");

            case '>':
                if (current_char == '=') {
                    advance();
                    return Token(GREATER_THAN_OR_EQUAL_TO, ">=");
                }
                return Token(GREATER_THAN, ">");

            case '<':
                if (current_char == '=') {
                    advance();
                    return Token(LESS_THAN_OR_EQUAL_TO, "<=");
                }
                return Token(LESS_THAN, "<");
        }
        throw std::runtime_error(std::string("Invalid operator: ") + op);
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
                return handle_identifier();
            }

            switch (current_char) {
                case '=':
                case '!':
                case '>':
                case '<':
                    return handle_operator();

                case '+':
                    advance();
                    return Token(PLUS, "+");
                case '-':
                    advance();
                    return Token(MINUS, "-");
                case '*':
                    advance();
                    return Token(MUL, "*");
                case '/':
                    advance();
                    return Token(DIV, "/");
                case '(':
                    advance();
                    return Token(LPAREN, "(");
                case ')':
                    advance();
                    return Token(RPAREN, ")");
                case ',':
                    advance();
                    return Token(COMMA, ",");
                default:
                    throw std::runtime_error(std::string("Invalid character: ") + current_char);
            }
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
private:
    void skip_statement() {
        if (current_token.type == IF) {
            skip_if_statement();
        } else if (current_token.type == FOR) {
            skip_for_statement();
        } else if (current_token.type == ID) {
            eat(ID);
            eat(ASSIGN);
            expr();
        } else if (current_token.type == PRINT) {
            eat(PRINT);
            eat(LPAREN);
            while (current_token.type != RPAREN) {
                expr();
                if (current_token.type == COMMA) {
                    eat(COMMA);
                }
            }
            eat(RPAREN);
        }
    }

    void skip_if_statement() {
        eat(IF);
        condition();
        eat(THEN);
        while (current_token.type != END) {
            skip_statement();
        }
        eat(END);
    }

    void skip_for_statement() {
        eat(FOR);
        eat(ID);
        eat(ASSIGN);
        expr();
        eat(TO);
        expr();
        while (current_token.type != END) {
            skip_statement();
        }
        eat(END);
    }

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

    bool simple_condition() {
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

    bool condition() {
        bool left = simple_condition();
        while (current_token.type == AND || current_token.type == OR) {
            Token token = current_token;
            if (token.type == AND) {
                eat(AND);
                if (!left) {
                    skip_condition();
                } else {
                    left = simple_condition();
                }
            } else if (token.type == OR) {
                eat(OR);
                if (left) {
                    skip_condition();
                } else {
                    left = simple_condition();
                }
            }
        }
        return left;
    }

    void skip_condition() {
        expr();
        if (current_token.type == EQUAL_TO || current_token.type == NOT_EQUAL_TO ||
            current_token.type == GREATER_THAN || current_token.type == LESS_THAN ||
            current_token.type == GREATER_THAN_OR_EQUAL_TO || current_token.type == LESS_THAN_OR_EQUAL_TO) {
            eat(current_token.type);
            expr();
        }
    }

    void for_statement() {
        eat(FOR);
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        int start = expr();
        eat(TO);
        int end = expr();
        
        symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(start));
        
        size_t loop_start_pos = lexer.pos;
        Token loop_start_token = current_token;
        
        while (std::stoi(symbolTable.get(var_name)->value) <= end) {
            size_t current_pos = lexer.pos;
            Token current_token_save = current_token;
            
            while (current_token.type != END) {
                statement();
            }
            
            int current_val = std::stoi(symbolTable.get(var_name)->value);
            symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(current_val + 1));
            
            if (current_val < end) {
                lexer.pos = loop_start_pos;
                lexer.current_char = lexer.text[lexer.pos];
                current_token = loop_start_token;
            }
        }
        eat(END);
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
            while (current_token.type != END) {
                skip_statement();
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
        } else if (current_token.type == FOR) {
            for_statement();
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