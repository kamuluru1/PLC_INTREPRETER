#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <memory>

enum TokenType {
    INTEGER, PLUS, MINUS, MUL, DIV, LPAREN, RPAREN, EOF_TOKEN, ID, ASSIGN, COMMA, PRINT,
    EQUAL_TO, NOT_EQUAL_TO, GREATER_THAN, LESS_THAN, GREATER_THAN_OR_EQUAL_TO, LESS_THAN_OR_EQUAL_TO,
    IF, THEN, END, AND, OR, FOR, TO, WHILE
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


/* Converts string input into a stream of tokens. */
class Lexer {
public:
    std::string text;
    size_t pos;
    char current_char;

    Lexer(const std::string& text_) : text(text_), pos(0), current_char(text[pos]) {}

    // Advance the 'pos' pointer and set the 'current_char' variable
    void advance() {
        pos++;
        current_char = (pos >= text.size()) ? '\0' : text[pos];
    }

    // Skip whitespace characters in the text
    void skip_whitespace() {
        while (current_char != '\0' && std::isspace(current_char)) {
            advance();
        }
    }

    //Returns an integer from the input. Can be multiple digits. For example 123 is just 1 INTEGER token with value 123 instead of 3 INTEGER tokens with values 1, 2, 3.
    int integer() {
        std::string result;
        while (current_char != '\0' && std::isdigit(current_char)) {
            result += current_char;
            advance();
        }
        return std::stoi(result);
    }

    //Handles variable declarations. Variables can only contain letters and underscores. For example, a is an ID token with value a, but a1 is two ID tokens with values a and 1.
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
            {"to", TO},
            {"while", WHILE}
        };

        auto it = keywords.find(id);
        return it != keywords.end() ? Token(it->second, id) : Token(ID, id);
    }

    //Returns tokens for the respective operators. For example, == is an EQUAL_TO token, but = is an ASSIGN token.
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

    //Method to get the next token from the input. It will return an EOF (end of file) token when the input has been fully processed. 
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

//
class SymbolTable {
public:

    //Entry struct to store the type and value of a variable.
    struct Entry {
        std::string type;
        std::string value;
    };

private:
    std::unordered_map<std::string, Entry> table;

public:

    //When it encounters a new variable, it will check if it already exists in the symbol table. If it does, it will update the existing one. If not then it will add a new entry. 
    void addOrUpdate(const std::string& name, const std::string& type, const std::string& value) {
        if (table.find(name) != table.end() && table[name].type != type) {
            throw std::runtime_error("Type mismatch for variable: " + name);
        }
        table[name] = {type, value};
    }

    //Retrieves a variable from the table. If it doesn't exist, it will return an empty optional.
    std::optional<Entry> get(const std::string& name) const {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

// Different types of AST nodes
class AST;
class BinaryOpNode;
class NumberNode;
class VariableNode;
class AssignNode;
class PrintNode;
class IfNode;
class WhileNode;
class ForNode;
class ComparisonNode;
class LogicalOpNode;

// Visitor interface
class ASTVisitor {
public:
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(AssignNode* node) = 0;
    virtual void visit(PrintNode* node) = 0;
    virtual void visit(IfNode* node) = 0;
    virtual void visit(WhileNode* node) = 0;
    virtual void visit(ForNode* node) = 0;
    virtual void visit(ComparisonNode* node) = 0;
    virtual void visit(LogicalOpNode* node) = 0;
    virtual ~ASTVisitor() = default;
};

// Base AST node class
class AST {
public:
    virtual ~AST() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Node implementations
class BinaryOpNode : public AST {
public:
    TokenType op;
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;

    BinaryOpNode(TokenType op_, std::unique_ptr<AST> left_, std::unique_ptr<AST> right_)
        : op(op_), left(std::move(left_)), right(std::move(right_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for integer values
class NumberNode : public AST {
public:
    int value;

    explicit NumberNode(int value_) : value(value_) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for variables (identifiers)
class VariableNode : public AST {
public:
    std::string name;

    explicit VariableNode(std::string name_) : name(std::move(name_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for assignment statements (variable = expression)
class AssignNode : public AST {
public:
    std::string name;
    std::unique_ptr<AST> value;

    AssignNode(std::string name_, std::unique_ptr<AST> value_)
        : name(std::move(name_)), value(std::move(value_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for print statements
class PrintNode : public AST {
public:
    std::vector<std::unique_ptr<AST>> expressions;

    explicit PrintNode(std::vector<std::unique_ptr<AST>> expressions_)
        : expressions(std::move(expressions_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for comparison operations (==, !=, >, <, >=, <=)
class ComparisonNode : public AST {
public:
    TokenType op;
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;

    ComparisonNode(TokenType op_, std::unique_ptr<AST> left_, std::unique_ptr<AST> right_)
        : op(op_), left(std::move(left_)), right(std::move(right_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for logical operations (AND, OR)
class LogicalOpNode : public AST {
public:
    TokenType op;
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;

    LogicalOpNode(TokenType op_, std::unique_ptr<AST> left_, std::unique_ptr<AST> right_)
        : op(op_), left(std::move(left_)), right(std::move(right_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for if statements
class IfNode : public AST {
public:
    std::unique_ptr<AST> condition;
    std::vector<std::unique_ptr<AST>> body;

    IfNode(std::unique_ptr<AST> condition_, std::vector<std::unique_ptr<AST>> body_)
        : condition(std::move(condition_)), body(std::move(body_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for while loops
class WhileNode : public AST {
public:
    std::unique_ptr<AST> condition;
    std::vector<std::unique_ptr<AST>> body;

    WhileNode(std::unique_ptr<AST> condition_, std::vector<std::unique_ptr<AST>> body_)
        : condition(std::move(condition_)), body(std::move(body_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Node for for loops
class ForNode : public AST {
public:
    std::string var_name;
    std::unique_ptr<AST> start;
    std::unique_ptr<AST> end;
    std::vector<std::unique_ptr<AST>> body;

    ForNode(std::string var_name_, std::unique_ptr<AST> start_, std::unique_ptr<AST> end_,
            std::vector<std::unique_ptr<AST>> body_)
        : var_name(std::move(var_name_)), start(std::move(start_)), end(std::move(end_)),
          body(std::move(body_)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Interpreter class
class Interpreter : public ASTVisitor {
private:
    SymbolTable& symbolTable;
    std::optional<int> lastValue;

public:
    explicit Interpreter(SymbolTable& symbolTable_) : symbolTable(symbolTable_) {}

    void visit(BinaryOpNode* node) override {
        node->left->accept(*this);
        int left = lastValue.value();
        node->right->accept(*this);
        int right = lastValue.value();

        switch (node->op) {
            case PLUS:
                lastValue = left + right;
                break;
            case MINUS:
                lastValue = left - right;
                break;
            case MUL:
                lastValue = left * right;
                break;
            case DIV:
                if (right == 0) throw std::runtime_error("Division by zero");
                lastValue = left / right;
                break;
            default:
                throw std::runtime_error("Invalid binary operator");
        }
    }

    //Visits a NumberNode and stores its value in the lastValue variable.
    void visit(NumberNode* node) override {
        lastValue = node->value;
    }

    //Visits a VariableNode and retrieves its value from the symbol table. If the variable is not found, it will throw a runtime error.
    void visit(VariableNode* node) override {
        auto entry = symbolTable.get(node->name);
        if (!entry) {
            throw std::runtime_error("Undefined variable: " + node->name);
        }
        lastValue = std::stoi(entry->value);
    }

    //Visits an AssignNode, evaluates the expression on the right side of the assignment, and stores the result in the symbol table.
    void visit(AssignNode* node) override {
        node->value->accept(*this);
        symbolTable.addOrUpdate(node->name, "INTEGER", std::to_string(lastValue.value()));
    }

    //Visits a PrintNode, evaluates each expression in the print statement, and prints the result to the console.
    void visit(PrintNode* node) override {
        bool first = true;
        for (const auto& expr : node->expressions) {
            if (!first) std::cout << " ";
            expr->accept(*this);
            std::cout << lastValue.value();
            first = false;
        }
        std::cout << std::endl;
    }

    //Visits a ComparisonNode, evaluates the left and right expressions, and stores the result of the comparison in the lastValue variable.
    void visit(ComparisonNode* node) override {
        node->left->accept(*this);
        int left = lastValue.value();
        node->right->accept(*this);
        int right = lastValue.value();

        switch (node->op) {
            case EQUAL_TO:
                lastValue = left == right;
                break;
            case NOT_EQUAL_TO:
                lastValue = left != right;
                break;
            case GREATER_THAN:
                lastValue = left > right;
                break;
            case LESS_THAN:
                lastValue = left < right;
                break;
            case GREATER_THAN_OR_EQUAL_TO:
                lastValue = left >= right;
                break;
            case LESS_THAN_OR_EQUAL_TO:
                lastValue = left <= right;
                break;
            default:
                throw std::runtime_error("Invalid comparison operator");
        }
    }

    //Visits a LogicalOpNode, evaluates the left and right expressions, and stores the result of the logical operation in the lastValue variable.
    void visit(LogicalOpNode* node) override {
        node->left->accept(*this);
        bool left = lastValue.value();

        if (node->op == AND && !left) {
            lastValue = false;
            return;
        }
        if (node->op == OR && left) {
            lastValue = true;
            return;
        }

        node->right->accept(*this);
        bool right = lastValue.value();

        lastValue = (node->op == AND) ? (left && right) : (left || right);
    }

    //Visits an IfNode, evaluates the condition
    void visit(IfNode* node) override {
        node->condition->accept(*this);
        if (lastValue.value()) {
            for (const auto& stmt : node->body) {
                stmt->accept(*this);
            }
        }
    }
    //Visits a WhileNode, evaluates the condition
    void visit(WhileNode* node) override {
        while (true) {
            node->condition->accept(*this);
            if (!lastValue.value()) break;
            
            for (const auto& stmt : node->body) {
                stmt->accept(*this);
            }
        }
    }

    //Visits a ForNode, evaluates the start and end expressions, and iterates over the body of the for loop.
    void visit(ForNode* node) override {
        node->start->accept(*this);
        int start = lastValue.value();
        node->end->accept(*this);
        int end = lastValue.value();

        for (int i = start; i <= end; i++) {
            symbolTable.addOrUpdate(node->var_name, "INTEGER", std::to_string(i));
            for (const auto& stmt : node->body) {
                stmt->accept(*this);
            }
        }
    }
};

class Parser {
private:
    Lexer lexer;
    Token current_token;
    SymbolTable& symbolTable;

    //Consumes the current token if it matches the expected token_type. If not, it will throw a runtime error. 
    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
    }

    /*
    This method parses a factor, which can be an integer, variable, or an expression in parentheses and returns a unique pointer to the AST node that represents the factor. 
    If the current token is an INTEGER, it consumes the token and returns a NumberNode. 
    If the token is an identifier, it consumes it and returns a VariableNode. If the token is LPAREN, it consumes the token, parses the expression and expects a RPAREN to follow.
    */
    std::unique_ptr<AST> factor() {
        Token token = current_token;
        if (token.type == INTEGER) {
            eat(INTEGER);
            return std::make_unique<NumberNode>(std::stoi(token.value));
        } else if (token.type == ID) {
            eat(ID);
            return std::make_unique<VariableNode>(token.value);
        } else if (token.type == LPAREN) {
            eat(LPAREN);
            auto node = expr();
            eat(RPAREN);
            return node;
        }
        throw std::runtime_error("Syntax error in factor");
    }

    /*
    This method parses a term, which is a factor followed by multiplication or division operations. 
    While the current token is a multiplication or division operator, it consumes the operator and the next factor, creating a BinaryOpNode for each operation. 
    It returns a unique pointer to the AST node that represents that term. 
    */ 
    std::unique_ptr<AST> term() {
        auto node = factor();

        while (current_token.type == MUL || current_token.type == DIV) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<BinaryOpNode>(token.type, std::move(node), factor());
        }
        return node;
    }

    /*
    This method parses an expression, which is a term followed by addition or subtraction operations. 
    While the current token is an addition or subtraction operator, it consumes the operator and the next term, creating a BinaryOpNode for each operation. 
    It then returns a unique pointer to the AST node representing the expression. 
    */
    std::unique_ptr<AST> expr() {
        auto node = term();

        while (current_token.type == PLUS || current_token.type == MINUS) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<BinaryOpNode>(token.type, std::move(node), term());
        }
        return node;
    }

    /*
    This method parses a simple condition, which is an expression followed by a comparison operator and another expression. 
    It checks if the current token is a comparison operator. If it is, it consumes the operator and the next expression, creating a ComparisonNode. 
    It returns a unique pointer to the AST node that represents the condition.
    */
    std::unique_ptr<AST> simple_condition() {
        auto left = expr();
        TokenType op = current_token.type;
        
        switch(op) {
            case EQUAL_TO:
            case NOT_EQUAL_TO:
            case GREATER_THAN:
            case LESS_THAN:
            case GREATER_THAN_OR_EQUAL_TO:
            case LESS_THAN_OR_EQUAL_TO:
                eat(op);
                return std::make_unique<ComparisonNode>(op, std::move(left), expr());
            default:
                throw std::runtime_error("Invalid comparison operator");
        }
    }

    /*
    This method parses a condition in an abstract syntax tree (AST), which can be a simple condition potentially followed by logical AND or OR operations. 
    It starts by parsing a simple condition and assigns it to the node variable. 
    Then, while the current token represents a logical AND or OR operator, the method consumes the operator, 
    creates a new logical operation node LogicalOpNode using std::make_unique, and updates the node by linking it to the newly parsed condition.
    Finally, it returns a unique pointer to the resulting AST node.
    */
    std::unique_ptr<AST> condition() {
        auto node = simple_condition();

        while (current_token.type == AND || current_token.type == OR) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<LogicalOpNode>(token.type, std::move(node), simple_condition());
        }
        return node;
    }

    /*
     This method parses an if statement by first consuming the IF token and parsing the associated condition. 
     After consuming the THEN token, it processes the body of the if statement by repeatedly parsing individual statements and adding them to a vector until the END token is encountered. 
     Finally, it consumes the END token and returns a unique pointer to an IfNode representing the entire if statement, comprising the condition and the body.
    */
    std::unique_ptr<AST> if_statement() {
        eat(IF);
        auto cond = condition();
        eat(THEN);
        
        std::vector<std::unique_ptr<AST>> body;
        while (current_token.type != END) {
            body.push_back(statement());
        }
        eat(END);
        
        return std::make_unique<IfNode>(std::move(cond), std::move(body));
    }

    /*
    This method parses a while statement by consuming the WHILE token and parsing the associated condition using the condition() method. 
    It then consumes the THEN token and parses the body of the while loop by repeatedly processing statements and adding them to a vector until the END token is encountered. 
    Finally, it consumes the END token and returns a unique pointer to a WhileNode representing the complete while statement, which includes the condition and the loop body.
    */
    std::unique_ptr<AST> while_statement() {
        eat(WHILE);
        auto cond = condition();
        eat(THEN);
        
        std::vector<std::unique_ptr<AST>> body;
        while (current_token.type != END) {
            body.push_back(statement());
        }
        eat(END);
        
        return std::make_unique<WhileNode>(std::move(cond), std::move(body));
    }

    /*
    This method parses a for statement by consuming the FOR token and retrieving the variable name from the current token. 
    It then consumes the ASSIGN token, parses the start expression, and consumes the TO token to parse the end expression. 
    Afterward, it processes the body of the for loop by parsing individual statements and adding them to a vector until the END token is encountered. 
    Finally, it consumes the END token and returns a unique pointer to a ForNode representing the complete for statement, including the variable, start expression, end expression, and body.
    */
    std::unique_ptr<AST> for_statement() {
        eat(FOR);
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        auto start = expr();
        eat(TO);
        auto end = expr();
        
        std::vector<std::unique_ptr<AST>> body;
        while (current_token.type != END) {
            body.push_back(statement());
        }
        eat(END);
        
        return std::make_unique<ForNode>(std::move(var_name), std::move(start), std::move(end), std::move(body));
    }

    /*
    This method parses a print statement by consuming the PRINT token and the left parenthesis, followed by parsing the first expression and adding it to a vector of expressions. 
    While the current token is a comma, it consumes the comma and parses subsequent expressions, adding each to the vector. 
    After all expressions are processed, it consumes the right parenthesis and returns a unique pointer to a PrintNode representing the print statement with the list of parsed expressions.
    */
    std::unique_ptr<AST> print_statement() {
        eat(PRINT);
        eat(LPAREN);
        std::vector<std::unique_ptr<AST>> expressions;
        
        expressions.push_back(expr());
        while (current_token.type == COMMA) {
            eat(COMMA);
            expressions.push_back(expr());
        }
        
        eat(RPAREN);
        return std::make_unique<PrintNode>(std::move(expressions));
    }

    /*
    This method parses an assignment statement in an abstract syntax tree (AST) by retrieving the variable name from the current token and then consuming the ID and ASSIGN tokens. 
    It subsequently parses the assigned expression and returns a unique pointer to an AssignNode that represents the assignment statement, 
    linking the variable name to the parsed expression.
    */
    std::unique_ptr<AST> assignment_statement() {
        std::string var_name = current_token.value;
        eat(ID);
        eat(ASSIGN);
        return std::make_unique<AssignNode>(var_name, expr());
    }

public:
    Parser(Lexer lexer_, SymbolTable& symbolTable_) 
        : lexer(lexer_), current_token(lexer.get_next_token()), symbolTable(symbolTable_) {}
    
    TokenType current_token_type() const {
        return current_token.type;
    }

    /*Parses a statement, which can be if, for, while, assign or print.*/
    std::unique_ptr<AST> statement() {
        switch (current_token.type) {
            case IF:
                return if_statement();
            case FOR:
                return for_statement();
            case WHILE:
                return while_statement();
            case ID:
                return assignment_statement();
            case PRINT:
                return print_statement();
            default:
                throw std::runtime_error("Invalid statement");
        }
    }
};

int main() {

    // Once the interpreter code is run, type ./filename.txt in the terminal to run the code in the external file. This interface is intended to mimic a simple command line. 
    std::string file_path;
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
        Interpreter interpreter(symbolTable);

        while (parser.current_token_type() != EOF_TOKEN) {
            auto ast = parser.statement();
            ast->accept(interpreter);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
