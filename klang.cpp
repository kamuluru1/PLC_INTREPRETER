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
    IF, THEN, END, AND, OR, FOR, TO, WHILE
};

class Token {
public:
    TokenType type;
    std::string value; // Value is used for INTEGER tokens

    Token(const TokenType type_, std::string value_) : type(type_), value(value_) {} 

    void print() const {
        std::cout << "Token(" << type << ", " << value << ")" << std::endl;
    }
};

class Lexer {
public:
    std::string text; // Input code from the test.txt file
    size_t pos; // Current position in the text
    char current_char; // Current character being analyzed

    Lexer(const std::string& text_) : text(text_), pos(0), current_char(text[pos]) {}

    // Method to advance the 'pos' pointer and set the 'current_char' variable
    void advance() {
        pos++;
        current_char = (pos >= text.size()) ? '\0' : text[pos];
    }

    void skip_whitespace() {
        while (current_char != '\0' && std::isspace(current_char)) {
            advance();
        }
    }

    // Method to create an integer from a sequence of digits. (Ex. 123 will become INTEGER(123) instead of INTEGER(1), INTEGER(2), INTEGER(3))
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

        // Check if the identifier is a keyword in the language
        static const std::unordered_map<std::string, TokenType> keywords = {
            {"print", PRINT},
            {"if", IF},
            {"then", THEN},
            {"end", END},
            {"and", AND},
            {"or", OR},
            {"for", FOR},
            {"to", TO},
            {"while", WHILE}
        };

        auto it = keywords.find(id);
        return it != keywords.end() ? Token(it->second, id) : Token(ID, id);
    }
    
    // Handles cases when an operator is found and only returns that type of token
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

    // Information about a variable to be stored in the symbol table
    struct Entry {
        std::string type;
        std::string value;
    }; 

private:
    std::unordered_map<std::string, Entry> table; // A vector made up of entries

public:

    void addOrUpdate(const std::string& name, const std::string& type, const std::string& value) {
        if (table.find(name) != table.end() && table[name].type != type) {
            throw std::runtime_error("Type mismatch for variable: " + name);
        }
        table[name] = {type, value};
    } // Checks if the variable is already in the table. If not a new entry is added, and if not the existing entry is updated.

    std::optional<Entry> get(const std::string& name) const {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second;
        }
        return std::nullopt;
    } // The get method is to retrieve the value of a variable from the table. It will return std::nullopt if the variable is not in the table.
};

class Parser {
private:

    // Skips over a statement that doesn't need to be executed(Ex. false consition)
    void skip_statement() {
        if (current_token.type == IF) {
            skip_if_statement();
        } else if (current_token.type == FOR) {
            skip_for_statement();
        } else if (current_token.type == WHILE) {
            skip_while_statement();
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


    // Skips if statement if the condition is false
    void skip_if_statement() {
        eat(IF);
        condition();
        eat(THEN);
        while (current_token.type != END) {
            skip_statement();
        }
        eat(END);
    }

    // Skips for loop if condition is not met
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

    // Skips while loop the condition is not met.
    void skip_while_statement() {
        eat(WHILE);
        condition();
        eat(THEN);
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

    // Consumes the current token if it matches the expected token type
    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
    }

    /* 
    Method to evaluate expressions and the result. 
    If the token type is INTEGER it will get the value from the token. 
    If the type is ID it will look up the variable in the symbol table and use that value in evaluating expressions.
    */  
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

    // Returns a term that can be added, subtracted, multiplied, or divided. It starts by evaluating a factor and storing the result
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

    /* 
    Evaluates the expression by adding or subtracting terms. 
    It starts by evaluating a term and storing the result. 
    It then checks if the current token is a plus or minus operator and adds or subtracts the next term accordingly. 
    */
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

    /*
    Evaluates a simple condition by comparing two expressions with a comparison operator.
    It starts by evaluating the left expression and storing the result in the 'left' variable.
    It then checks the current token type and compares the left expression with the right expression.
    If the operator is not recognized, it will throw a runtime error.
    */
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

    /*
    Evaluates conditional expressions that use more complex operators like AND and OR. 
    It builds on the simple_condition method my evaluating the left expression and storing the result in the 'left' variable. 
    It uses short circut evaluation by evaluating the left side of the expression first. 
    If the left side is false and the operator is AND, it will skip the right side by using the skip_condition method. 
    */
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

    /*
    This method is used to skip over a conditional expression without evaluating the result. 
    This is implemented in short circut evaluation 
    */
    void skip_condition() {
        expr();
        if (current_token.type == EQUAL_TO || current_token.type == NOT_EQUAL_TO ||
            current_token.type == GREATER_THAN || current_token.type == LESS_THAN ||
            current_token.type == GREATER_THAN_OR_EQUAL_TO || current_token.type == LESS_THAN_OR_EQUAL_TO) {
            eat(current_token.type);
            expr();
        }
    }

    /*
    This method begind by consuming a WHILE token and storing the position and token in the 'condition_pos' and 'condition_token' variables.
    It then evaluates the loop condition by calling the condition method, which results a boolean result. 
    It then condumes the THEN token and enters a while loop that will continue to execute the statements until the loop condition is false.
    It will excecute all the statements until it reached an END token.
    */
    void while_statement() {
        eat(WHILE);
        
        size_t condition_pos = lexer.pos;
        Token condition_token = current_token;
        
        bool cond_result = condition();
        eat(THEN);
        
        while (cond_result) {
            while (current_token.type != END) {
                statement();
            }
            
            lexer.pos = condition_pos;
            lexer.current_char = lexer.text[lexer.pos];
            current_token = condition_token;
            
            cond_result = condition();
            eat(THEN);
        }
        
        while (current_token.type != END) {
            skip_statement();
        }
        eat(END);
    }

    /*
    This method consumes a FOR token and stores the variable name in the 'var_name' variable.
    It is responsible for starting the excecution of a for loop. The method evaluates the starting expression using the expr method and stores the result in the start variable.
    It then consumes the TO token and evaluates the ending expression, storing the result in the end variable.
    The method enters a while loop that will continue to execute the statements until the loop condition is false.
    After the loop condition evaluates to false, the loop will end.
    */
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

    /*
    This method consumes an IF token once encountered and evaluates the condition using the condition method and stores the boolean value in the 'cond_result' variable.
    It then consumes the THEN token and if the cond_result is true, it will execute all the statements until it reaches the END token.
    */
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

    /* 
    If a variable is encountered, the ID token (variable) is consumed and the ASSIGN token is consumed.
    It then evaluates the expression or value using the 'expr' method and stores the variable in the symbol table via the addOrUpdate method.
    */
    void assignment() {
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        int result = expr();
        symbolTable.addOrUpdate(var_name, "INTEGER", std::to_string(result));
    }

    /* 
    If a PRINT token is encountered followed by an LPAREN token, the method will evaluate the expression using the 'expr' method until the RPAREN token is encountered.
    The method will print the value of the expression to the console.
    The 'first' variable is used to determine if a space is needed between the values.
    It can handle multiple expressions seperated by commas.
    */
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

    /*
    This method handles the execution of different statements in the source code. It determines the type of statement and calls the respective method.
    If the current token doesnt match any of these, it will throw a runtime error.
    */
    void statement() {
        if (current_token.type == IF) {
            if_statement();
        } else if (current_token.type == FOR) {
            for_statement();
        } else if (current_token.type == WHILE) {
            while_statement();
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
    std::string file_path; // ./test.txt
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
        Parser parser(lexer, symbolTable); // The parser takes the lexer and symbol table as arguments because it needs to access the lexer to get tokens and the symbol table to store variables.

        while (parser.current_token.type != EOF_TOKEN) {
            parser.statement();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
