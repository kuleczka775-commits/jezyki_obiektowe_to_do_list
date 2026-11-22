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

int Task::getPriority() const
{
    return 0;
}

string Task::getTag() const
{
    return tag;
}

string Task::getStatus() const
{
    return status;
}