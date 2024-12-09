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
            {"to", TO},
            {"while", WHILE}
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

// Declarations for all possible AST node types
// Each type represents a different language construct
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

// Visitor interface for the Visitor pattern (tells the program how to handle each AST node)
class ASTVisitor {
public:
    // Defining each visit operations for the AST node type
    // 
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

// The base class for all the AST nodes 
class AST {
public:
    virtual ~AST() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Binary operations in the AST node
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

// Node type that represents actual numbers in the code
class NumberNode : public AST {
public:
    int value; // Stores actual int value for example when parsing 42 this will hold int 42

    // Constructor takes the int and stores its value
    explicit NumberNode(int value_) : value(value_) {}

    // Visitor patten tells the visitor that its a number node and this how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Used for variable references in expressions 
// For example using x in x + 1
class VariableNode : public AST {
public:
    std::string name;

    explicit VariableNode(std::string name_) : name(std::move(name_)) {}

    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Assignment operation node 
// For example x = 5
class AssignNode : public AST {
public:
    std::string name; // Target var
    std::unique_ptr<AST> value; // Expression assigned

    AssignNode(std::string name_, std::unique_ptr<AST> value_)
        : name(std::move(name_)), value(std::move(value_)) {}

     // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Represents print statements
// Used for printing 
class PrintNode : public AST {
public:
    std::vector<std::unique_ptr<AST>> expressions; // Expressions to output

    explicit PrintNode(std::vector<std::unique_ptr<AST>> expressions_)
        : expressions(std::move(expressions_)) {}

    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// Comparison operations for example x > y
class ComparisonNode : public AST {
public:
    TokenType op; // Operator
    std::unique_ptr<AST> left; // Left expression
    std::unique_ptr<AST> right; // Right expression

    ComparisonNode(TokenType op_, std::unique_ptr<AST> left_, std::unique_ptr<AST> right_)
        : op(op_), left(std::move(left_)), right(std::move(right_)) {}

    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};


// Logical operations for example  x and y, a or b
class LogicalOpNode : public AST {
public:
    TokenType op;
    std::unique_ptr<AST> left;
    std::unique_ptr<AST> right;

    LogicalOpNode(TokenType op_, std::unique_ptr<AST> left_, std::unique_ptr<AST> right_)
        : op(op_), left(std::move(left_)), right(std::move(right_)) {}

    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// If statements for example if x > 0
class IfNode : public AST {
public:
    std::unique_ptr<AST> condition; // Booleen condition
    std::vector<std::unique_ptr<AST>> body; // statements in tbe if block

    IfNode(std::unique_ptr<AST> condition_, std::vector<std::unique_ptr<AST>> body_)
        : condition(std::move(condition_)), body(std::move(body_)) {}
    
    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// While loops for exmaple while x < 10
class WhileNode : public AST {
public:
    std::unique_ptr<AST> condition; // Condtion of the loop
    std::vector<std::unique_ptr<AST>> body; // Body of the loop

    WhileNode(std::unique_ptr<AST> condition_, std::vector<std::unique_ptr<AST>> body_)
        : condition(std::move(condition_)), body(std::move(body_)) {}
    
    // Visitor patten tells the visitor what type of node and how to handle it
    void accept(ASTVisitor& visitor) override {
        visitor.visit(this);
    }
};

// For loops for example for i = 1 to 10
class ForNode : public AST {
public:
    std::string var_name; // Interator var
    std::unique_ptr<AST> start; // Intial value
    std::unique_ptr<AST> end; // End value
    std::vector<std::unique_ptr<AST>> body; // Body of the loop

    ForNode(std::string var_name_, std::unique_ptr<AST> start_, std::unique_ptr<AST> end_,
            std::vector<std::unique_ptr<AST>> body_)
        : var_name(std::move(var_name_)), start(std::move(start_)), end(std::move(end_)),
          body(std::move(body_)) {}

    // Visitor patten tells the visitor what type of node and how to handle it
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

    void visit(NumberNode* node) override {
        lastValue = node->value;
    }

    void visit(VariableNode* node) override {
        auto entry = symbolTable.get(node->name);
        if (!entry) {
            throw std::runtime_error("Undefined variable: " + node->name);
        }
        lastValue = std::stoi(entry->value);
    }

    void visit(AssignNode* node) override {
        node->value->accept(*this);
        symbolTable.addOrUpdate(node->name, "INTEGER", std::to_string(lastValue.value()));
    }

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

    void visit(IfNode* node) override {
        node->condition->accept(*this);
        if (lastValue.value()) {
            for (const auto& stmt : node->body) {
                stmt->accept(*this);
            }
        }
    }

    void visit(WhileNode* node) override {
        while (true) {
            node->condition->accept(*this);
            if (!lastValue.value()) break;
            
            for (const auto& stmt : node->body) {
                stmt->accept(*this);
            }
        }
    }

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

    void eat(TokenType token_type) {
        if (current_token.type == token_type) {
            current_token = lexer.get_next_token();
        } else {
            throw std::runtime_error("Unexpected token: " + current_token.value);
        }
    }

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

    std::unique_ptr<AST> term() {
        auto node = factor();

        while (current_token.type == MUL || current_token.type == DIV) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<BinaryOpNode>(token.type, std::move(node), factor());
        }
        return node;
    }

    std::unique_ptr<AST> expr() {
        auto node = term();

        while (current_token.type == PLUS || current_token.type == MINUS) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<BinaryOpNode>(token.type, std::move(node), term());
        }
        return node;
    }

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

    std::unique_ptr<AST> condition() {
        auto node = simple_condition();

        while (current_token.type == AND || current_token.type == OR) {
            Token token = current_token;
            eat(token.type);
            node = std::make_unique<LogicalOpNode>(token.type, std::move(node), simple_condition());
        }
        return node;
    }

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