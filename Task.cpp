#include "Task.h"
#include <iostream>

Task::Task(int id, string title, string tag) : id(id), title(title), tag(tag), status("Do zrobienia") {}

void Task::setStatus(string newStatus)
{
    status = newStatus;
}

string Task::getTitle() const
{
    return title;
}

int Task::getId() const
{
    return id;
}