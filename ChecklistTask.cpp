#include "ChecklistTask.h"
#include "Exceptions.h"
#include <iostream>

using namespace std;

ChecklistTask::ChecklistTask(int id, string title, string tag, int priority) : Task(id, title, tag), priority(priority)
{
    validatePriority(priority);
    cout << "Utworzono nową listę zadań: " << title << " (priorytet: " << priority << ")" << endl;
}

int ChecklistTask::getPriority() const
{
    return priority;
}

void ChecklistTask::setPriority(int newPriority)
{
    validatePriority(newPriority);
    priority = newPriority;
}

void ChecklistTask::addSubtask(string name, bool isDone = false)
{
    subtasks.push_back({name, isDone});
    cout << "Dodano podzadanie: " << name << endl;
}

void ChecklistTask::toggleSubtask(int index)
{
    if (index >= 0 && index < static_cast<int>(subtasks.size()))
    {
        subtasks[index].second = !subtasks[index].second;
        cout << "Zmieniono status podzadania '" << subtasks[index].first << "' na: " << (subtasks[index].second ? "Zrobione" : "Do zrobienia") << endl;
    }
    else
    {
        cout << "Nieprawidłowy numer podzadania." << endl;
    }
}

int ChecklistTask::getProgress() const
{
    if (subtasks.empty())
    {
        return 0;
    }

    int completed = 0;
    for (const auto &item : subtasks)
    {
        if (item.second)
        {
            completed++;
        }
    }

    return (completed * 100) / static_cast<int>(subtasks.size());
}

const vector<pair<string, bool>> &ChecklistTask::getSubtasks() const
{
    return subtasks;
}

string ChecklistTask::getType() const
{
    return "Checklista";
}

string ChecklistTask::getSummary() const
{
    return "Postęp: " + to_string(getProgress()) + "% (" + to_string(subtasks.size()) + " podzadań) | Priorytet: " + to_string(priority);
}
string ChecklistTask::serialize() const
{
    string subtasksString = "";
    for (size_t i = 0; i < subtasks.size(); i++)
    {
        subtasksString += subtasks[i].first + ":" + (subtasks[i].second ? "1" : "0");
        if (i + 1 < subtasks.size())
        {
            subtasksString += ";";
        }
    }

    return "Checklist|" + to_string(id) + "|" + title + "|" + tag + "|" + status + "|" + to_string(priority) + "|" + subtasksString;
}