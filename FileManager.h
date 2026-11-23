#pragma once
#include "TaskManager.h"
#include <string>
#include <vector>

using namespace std;

class FileManager
{
private:
    string filename;

    vector<string> split(const string &text, char delimeter) const;

public:
    FileManager(string filename = "tasks.txt");

    string getFilename() const;
    void setFilename(string newFilename);

    void saveToFile(const TaskManager &manager) const;
    void loadFromFile(TaskManager &manager) const;
};