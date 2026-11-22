#pragma once
#include <string>
#include <iostream>

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
    string getTag() const;
    string getStatus() const;

    virtual int getPriority() const;

    virtual string getType() const = 0;
    virtual string getSummary() const = 0;
    virtual string serialize() const = 0;

    bool operator<(const Task &other) const;
    bool operator==(const Task &other) const;
    friend ostream &operator<<(ostream &os, const Task &task);
};