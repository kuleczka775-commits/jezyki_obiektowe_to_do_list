#pragma once
#include "Task.h"

using namespace std;

class DeadlineTask : public Task
{
private:
    string dueDate;
    int priority;

public:
    DeadlineTask(int id, string title, string tag, string dueDate, int priority);

    int getPriority() const override;
    void setPriority(int newPriority);

    string getDueDate() const;
    void setDueDate(string newDueDate);

    string getType() const override;
    string getSummary() const override;
    string serialize() const override;

    DeadlineTask operator+(int days) const;
};
