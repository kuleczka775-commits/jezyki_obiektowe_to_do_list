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

bool Task::operator<(const Task &other) const
{
    if (this->getPriority() != other.getPriority())
    {
        return this->getPriority() > other.getPriority();
    }
    return this->title < other.title;
}

bool Task::operator==(const Task &other) const
{
    return (this->title == other.title && this->tag == other.tag && this->getType() == other.getType());
}

ostream &operator<<(ostream &os, const Task &task)
{
    os << "[ID: " << task.id << "] [" << task.getType() << "] " << task.title << " (Tag: " << task.tag << ") | Status: " << task.status << " | " << task.getSummary();
    return os;
}