#pragma once
#include "Task.h"

using namespace std;

class DeadlineTask : public Task
{
private:
    string dueDate;
    int priority;

public:
    DeadlineTask(int id, string title, string tag, string dueDate);

    int getPriority() const override;
    void setPriority(int newPriority);
};
