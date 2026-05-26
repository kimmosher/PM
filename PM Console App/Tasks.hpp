#pragma once
#include <vector>
#include <string>

class Tasks {
public:
    void run();
private:
    std::vector<std::string> list;
    void load();
    void save();
};
