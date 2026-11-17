#pragma once
#include <string>

using namespace std;

class Task
{
protected:
    int id;
    string title;
    string tag;
    string status;

public:
    Task(int id, string title, string tag);
    virtual ~Task() = default;

    void setStatus(string newStatus);
    string getTitle() const;
    int getId() const;
};