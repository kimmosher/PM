#include "Notepad.hpp"
#include <iostream>
#include <fstream>
#include <string>

void Notepad::run() {
    std::cout << "\n=== Notepad ===\n";
    std::cout << "Filename: ";

    std::string file;
    std::cin >> file;

    std::cin.ignore();
    std::cout << "Enter text (end with a single '.' on a line):\n";

    std::ofstream f(file);
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line == ".") break;
        f << line << "\n";
    }
}
