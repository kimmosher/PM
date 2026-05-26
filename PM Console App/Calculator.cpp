#include "Calculator.hpp"
#include <iostream>
#include <stack>
#include <string>
#include <cstring>

double eval(const std::string& expr);

void Calculator::run() {
    std::cout << "\n=== Calculator ===\n";
    std::cout << "Enter expression: ";

    std::string expr;
    std::cin.ignore();
    std::getline(std::cin, expr);

    try {
        std::cout << "Result: " << eval(expr) << "\n";
    } catch (...) {
        std::cout << "Invalid expression.\n";
    }
}

// Simple expression evaluator
double eval(const std::string& expr) {
    std::stack<double> values;
    std::stack<char> ops;

    auto apply = [&](char op) {
        double b = values.top(); values.pop();
        double a = values.top(); values.pop();
        if (op == '+') values.push(a + b);
        if (op == '-') values.push(a - b);
        if (op == '*') values.push(a * b);
        if (op == '/') values.push(a / b);
    };

    for (size_t i = 0; i < expr.size(); i++) {
        if (isdigit(expr[i])) {
            double val = 0;
            while (i < expr.size() && isdigit(expr[i])) {
                val = val * 10 + (expr[i] - '0');
                i++;
            }
            i--;
            values.push(val);
        } else if (expr[i] == '(') {
            ops.push('(');
        } else if (expr[i] == ')') {
            while (ops.top() != '(') {
                apply(ops.top());
                ops.pop();
            }
            ops.pop();
        } else if (strchr("+-*/", expr[i])) {
            while (!ops.empty() && strchr("+-*/", ops.top()))
                apply(ops.top()), ops.pop();
            ops.push(expr[i]);
        }
    }

    while (!ops.empty())
        apply(ops.top()), ops.pop();

    return values.top();
}
