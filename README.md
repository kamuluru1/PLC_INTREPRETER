# PLC_INTREPRETER

# KLANG Programming Language Interpreter
A basic interpreter that supports arithmetic operations, variables, conditional statements, and loops.

## Language Syntax

### Basic Operations
- Variable assignment: `x = value`
- Arithmetic: `+`, `-`, `*`, `/`
- Print function: `print(expression)`
- Multiple values: `print(x, y, z)`

### Comparison Operators
- Equal to: `==`
- Not equal to: `!=`
- Greater than: `>`
- Less than: `<`
- Greater than or equal to: `>=`
- Less than or equal to: `<=`

### Control Flow

#### If Statements
```
if condition then
    statements
end
```

#### For Loops
```
for variable = start to end
    statements
end
```

#### While Loops
```
while condition then
    statements
end
```

### Logical Operators
- AND: `and`
- OR: `or`

## Example Programs

### Basic Arithmetic
```
x = 5
y = 3
z = x + y * 2
print(z)
```

### If Statement
```
x = 10
if x > 5 then
    print(x)
end
```

### Nested If Statements
```
x = 10
y = 5
if x > 5 then
    if y < 10 then
        print(x + y)
    end
end
```

### For Loop
```
for i = 1 to 5
    print(i)
end
```

### While Loop
```
x = 1
while x <= 5 then
    print(x)
    x = x + 1
end
```

### Complex Example
```
sum = 0
for i = 1 to 10
    if i > 5 and i < 9 then
        sum = sum + i
    end
end
print(sum)
x = 1
while x <= 3 then
    y = 1
    while y <= x then
        print(x * y)
        y = y + 1
    end
    x = x + 1
end
```

## Error Handling
The interpreter will report errors for:
- Undefined variables
- Invalid syntax
- Division by zero
- Type mismatches
- Invalid operators
- Missing keywords (then, end)

## Running the Interpreter
1. Compile the interpreter code
2. Run the executable
3. Enter the path to your source file when prompted
4. The program will execute your code and show any output or errors

## Limitations
- Only supports integer values
- No string operations
- No functions or procedures
- No arrays or complex data structures

## Tips
- Each control structure (if, for, while) must end with 'end'
- Conditions in if/while must be followed by 'then'
- Variables don't need to be declared before use
- The print function requires parentheses