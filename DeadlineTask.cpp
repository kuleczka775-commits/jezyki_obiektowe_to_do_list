#include "DeadlineTask.h"
#include <iostream>

using namespace std;

DeadlineTask::DeadlineTask(int id, string title, string tag, string dueDate) : Task(id, title, tag), dueDate(dueDate)
{
    cout << "Utworzono nowe zadanie z terminem:" << title << endl;
}

int DeadlineTask::getPriority() const
{
    return priority;
}

void DeadlineTask::setPriority(int newPriority)
{
    priority = newPriority;
}