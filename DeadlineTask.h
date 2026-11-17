#pragma once
#include "Task.h"

using namespace std;

class DeadlineTask : public Task
{
private:
    string dueDate;

public:
    DeadlineTask(int id, string title, string tag, string dueDate);
};
