#include "Calendar.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>

void Calendar::run() {
    time_t t = time(nullptr);
    tm* now = localtime(&t);

    int year = now->tm_year + 1900;
    int month = now->tm_mon + 1;

    std::cout << "\n=== Calendar ===\n";
    std::cout << "Month: " << month << "  Year: " << year << "\n\n";

    tm first = {0};
    first.tm_mday = 1;
    first.tm_mon = month - 1;
    first.tm_year = year - 1900;
    mktime(&first);

    int startDay = first.tm_wday;
    int daysInMonth = 31;

    if (month == 2) daysInMonth = (year % 4 == 0) ? 29 : 28;
    else if (month == 4 || month == 6 || month == 9 || month == 11)
        daysInMonth = 30;

    std::cout << "Su Mo Tu We Th Fr Sa\n";

    for (int i = 0; i < startDay; i++)
        std::cout << "   ";

    for (int d = 1; d <= daysInMonth; d++) {
        std::cout << std::setw(2) << d << " ";
        if ((d + startDay) % 7 == 0)
            std::cout << "\n";
    }
    std::cout << "\n";
}
