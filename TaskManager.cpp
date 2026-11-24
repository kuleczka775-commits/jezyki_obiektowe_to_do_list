#include "TaskManager.h"
#include "DeadlineTask.h"
#include "HabitTask.h"
#include "ChecklistTask.h"
#include "Exceptions.h"
#include <iostream>
#include <algorithm>

using namespace std;

TaskManager::TaskManager() : nextId(1) {}

TaskManager::~TaskManager()
{
    clearTasks();
}

int TaskManager::getNextId() const
{
    return nextId;
}

const vector<Task *> &TaskManager::getTasks() const
{
    return tasks;
}

void TaskManager::clearTasks()
{
    for (Task *task : tasks)
    {
        delete task;
    }
    tasks.clear();
    nextId = 1;
}

bool TaskManager::addTask(Task *task)
{
    if (task == nullptr)
    {
        return false;
    }

    for (const Task *existing : tasks)
    {
        if (*existing == *task)
        {
            cout << "Zadanie '" << task->getTitle() << "' już istnieje na liście." << endl;
            delete task;
            return false;
        }
    }
    tasks.push_back(task);

    if (task->getId() >= nextId)
    {
        nextId = task->getId() + 1;
    }

    return true;
}

void TaskManager::removeTask(int id)
{
    for (size_t i = 0; i < tasks.size(); i++)
    {
        if (tasks[i]->getId() == id)
        {
            cout << "Usunięto zadanie: " << tasks[i]->getTitle() << endl;
            delete tasks[i];
            tasks.erase(tasks.begin() + i);
            return;
        }
    }

    throw TaskNotFoundException("Nie znaleziono zadania o ID: " + to_string(id));
}

Task *TaskManager::findTaskById(int id) const
{
    for (Task *task : tasks)
    {
        if (task->getId() == id)
        {
            return task;
        }
    }

    throw TaskNotFoundException("Nie znaleziono zadania o ID: " + to_string(id));
}

vector<Task *> TaskManager::searchByTitle(const string &keyword) const
{
    vector<Task *> results;
    for (Task *task : tasks)
    {
        if (task->getTitle().find(keyword) != string::npos)
        {
            results.push_back(task);
        }
    }
    return results;
}

void TaskManager::changeStatus(int id, string newStatus)
{
    Task *task = findTaskById(id);
    task->setStatus(newStatus);
    cout << "Zmieniono status zadania '" << task->getTitle() << "' na: " << newStatus << endl;
}

void TaskManager::sortTasks()
{
    sort(tasks.begin(), tasks.end(), [](const Task *a, const Task *b)
         { return *a < *b; });
    cout << "Posortowano zadania według priorytetu i tytułu." << endl;
}

bool TaskManager::postponeDeadline(int id, int days)
{
    Task *task = findTaskById(id);
    DeadlineTask *deadlineTask = dynamic_cast<DeadlineTask *>(task);

    if (deadlineTask == nullptr)
    {
        cout << "Zadanie p ID " << id << " nie jest zadaniem z terminem." << endl;
        return false;
    }

    *deadlineTask = (*deadlineTask + days);
    cout << "Przesunięto termin zadania '" << deadlineTask->getTitle() << "' na: " << deadlineTask->getDueDate() << endl;
    return true;
}

bool TaskManager::completeHabit(int id)
{
    Task *task = findTaskById(id);
    HabitTask *habitTask = dynamic_cast<HabitTask *>(task);

    if (habitTask == nullptr)
    {
        cout << "Zadanie o ID " << id << " nie jest nawykiem." << endl;
        return false;
    }

    ++(*habitTask);
    return true;
}

void TaskManager::displayAll() const
{
    if (tasks.empty())
    {
        cout << "Lista zadań jest pusta." << endl;
        return;
    }

    for (const Task *task : tasks)
    {
        cout << *task << endl;
    }
}

void TaskManager::displayTaskById(int id) const
{
    try
    {
        Task *task = findTaskById(id);
        cout << *task << endl;
    }
    catch (const TaskNotFoundException &e)
    {
        cout << "Błąd: " << e.what() << endl;
    }
}

vector<Task *> TaskManager::getTaskByStatus(const string &status) const
{
    vector<Task *> filtered;
    for (Task *task : tasks)
    {
        if (task->getStatus() == status)
        {
            filtered.push_back(task);
        }
    }
    return filtered;
}
vector<Task *> TaskManager::getTasksByTag(const string &tag) const
{
    vector<Task *> filtered;
    for (Task *task : tasks)
    {
        if (task->getTag() == tag)
        {
            filtered.push_back(task);
        }
    }
    return filtered;
}
TaskStats TaskManager::calculateStats() const
{
    TaskStats stats;
    stats.totalTasks = static_cast<int>(tasks.size());

    for (Task *task : tasks)
    {
        string status = task->getStatus();
        if (status == "Do zrobienia")
        {
            stats.todoCount++;
        }
        else if (status == "W trakcie")
        {
            stats.inProgressCount++;
        }
        else if (status == "Zrobione")
        {
            stats.doneCount++;
        }

        stats.countByTag[task->getTag()]++;

        if (HabitTask *habit = dynamic_cast<HabitTask *>(task))
        {
            if (habit->getCurrentStreak() > stats.maxStreak)
            {
                stats.maxStreak = habit->getCurrentStreak();
                stats.bestHabitTitle = habit->getTitle();
            }
        }

        if (ChecklistTask *checklist = dynamic_cast<ChecklistTask *>(task))
        {
            for (const auto &sub : checklist->getSubtasks())
            {
                stats.totalSubtasks++;
                if (sub.second)
                {
                    stats.completedSubtasks++;
                }
            }
        }
    }

    if (stats.totalTasks > 0)
    {
        stats.completionPercent = (stats.doneCount * 100) / stats.totalTasks;
    }

    return stats;
}
void TaskManager::displayBoardView() const
{
    string columns[] = {"Do zrobienia", "W trakcie", "Zrobione"};

    cout << "================ WIDOK TABLICY =================" << endl;
    for (const string &col : columns)
    {
        vector<Task *> colTasks = getTaskByStatus(col);
        cout << "\n--- " << col << " (" << colTasks.size() << ") ---" << endl;

        if (colTasks.empty())
        {
            cout << "  (brak zadań)" << endl;
        }
        else
        {
            for (const Task *task : colTasks)
            {
                cout << "  " << *task << endl;
            }
        }
    }
    cout << "================================================" << endl;
}
void TaskManager::displayStats() const
{
    TaskStats stats = calculateStats();

    cout << "============== PANEL STATYSTYK =================" << endl;
    cout << "Wszystkie zadania: " << stats.totalTasks << endl;
    cout << "  Do zrobienia: " << stats.todoCount << " | W trakcie: " << stats.inProgressCount << " | Zrobione: " << stats.doneCount << endl;

    int filled = stats.completionPercent / 5;
    cout << "Postęp ogólny: [";
    for (int i = 0; i < 20; i++)
    {
        cout << (i < filled ? '#' : '-');
    }
    cout << "] " << stats.completionPercent << "%" << endl;

    cout << "Najdłuższy streak nawyku: " << stats.maxStreak << " dni (" << stats.bestHabitTitle << ")" << endl;

    if (stats.totalSubtasks > 0)
    {
        cout << "Ukończone podzadania: "
             << stats.completedSubtasks << "/" << stats.totalSubtasks << endl;
    }

    cout << "Podział według tagów:" << endl;
    if (stats.countByTag.empty())
    {
        cout << "  (brak tagów)" << endl;
    }
    else
    {
        for (const auto &entry : stats.countByTag)
        {
            cout << "  - " << entry.first << ": " << entry.second << endl;
        }
    }
    cout << "================================================" << endl;
}