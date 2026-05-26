#include "Tasks.hpp"
#include <iostream>
#include <fstream>

void Tasks::load() {
    list.clear();
    std::ifstream f("tasks.txt");
    std::string line;
    while (std::getline(f, line))
        list.push_back(line);
}

void Tasks::save() {
    std::ofstream f("tasks.txt");
    for (auto& t : list)
        f << t << "\n";
}

void Tasks::run() {
    load();
    int choice;

    while (true) {
        std::cout << "\n=== Task List ===\n";
        std::cout << "1. View tasks\n";
        std::cout << "2. Add task\n";
        std::cout << "3. Remove task\n";
        std::cout << "4. Back\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        if (choice == 1) {
            for (size_t i = 0; i < list.size(); i++)
                std::cout << i+1 << ". " << list[i] << "\n";
        } else if (choice == 2) {
            std::cin.ignore();
            std::string t;
            std::cout << "Enter task: ";
            std::getline(std::cin, t);
            list.push_back(t);
            save();
        } else if (choice == 3) {
            int idx;
            std::cout << "Task number: ";
            std::cin >> idx;
            if (idx > 0 && idx <= list.size()) {
                list.erase(list.begin() + idx - 1);
                save();
            }
        } else break;
    }
}
