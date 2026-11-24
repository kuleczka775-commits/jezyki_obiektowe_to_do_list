#include "DeadlineTask.h"
#include "Exceptions.h"
#include <iostream>
#include <ctime>

using namespace std;

DeadlineTask::DeadlineTask(int id, string title, string tag, string dueDate, int priority) : Task(id, title, tag), dueDate(dueDate), priority(priority)
{
    validateDate(dueDate);
    validatePriority(priority);
    cout << "Utworzono nowe zadanie z terminem: " << title << " (priorytet: " << priority << ")" << endl;
}

int DeadlineTask::getPriority() const
{
    return priority;
}

void DeadlineTask::setPriority(int newPriority)
{
    validatePriority(newPriority);
    priority = newPriority;
}

string DeadlineTask::getDueDate() const
{
    return dueDate;
}

void DeadlineTask::setDueDate(string newDueDate)
{
    validateDate(newDueDate);
    dueDate = newDueDate;
}

string DeadlineTask::getType() const
{
    return "Termin";
}

string DeadlineTask::getSummary() const
{
    return "Termin: " + dueDate + " | Priorytet: " + to_string(priority);
}
string DeadlineTask::serialize() const
{
    return "Deadline|" + to_string(id) + "|" + title + "|" + tag + "|" + status + "|" + dueDate + "|" + to_string(priority);
}

DeadlineTask DeadlineTask::operator+(int days) const
{
    DeadlineTask shiftedTask = *this;

    if (dueDate.length() == 10 && dueDate[4] == '-' && dueDate[7] == '-')
    {
        tm dateStruct = {};
        dateStruct.tm_year = stoi(dueDate.substr(0, 4)) - 1900;
        dateStruct.tm_mon = stoi(dueDate.substr(5, 2)) - 1;
        dateStruct.tm_mday = stoi(dueDate.substr(8, 2)) + days;
        dateStruct.tm_isdst = -1;

        mktime(&dateStruct);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &dateStruct);
        shiftedTask.dueDate = string(buffer);
    }

    return shiftedTask;
}