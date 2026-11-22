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

string DeadlineTask::getDueDate() const
{
    return dueDate;
}

void DeadlineTask::setDueDate(string newDueDate)
{
    dueDate = newDueDate;
}

string DeadlineTask::getType() const
{
    return "Termin";
}

string DeadlineTask::getSummary() const
{
    return "Termin" + dueDate + "| Priorytet: " + to_string(priority);
}
string DeadlineTask::serialize() const
{
    return "Deadline|" + to_string(id) + "|" + title + "|" + tag + "|" + status + "|" + dueDate + "|" + to_string(priority);
}