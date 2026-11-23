#pragma once
#include "Task.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

struct TaskStats
{
    int totalTasks = 0;
    int todoCount = 0;
    int inProgressCount = 0;
    int doneCount = 0;
    int completionPercent = 0;
    int maxStreak = 0;
    string bestHabitTitle = "Brak";
    int totalSubtasks = 0;
    int completedSubtasks = 0;
    map<string, int> countByTag;
};

class TaskManager
{
private:
    vector<Task *> tasks;
    int nextId;

public:
    TaskManager();
    ~TaskManager();

    TaskManager(const TaskManager &) = delete;
    TaskManager &operator=(const TaskManager &) = delete;

    int getNextId() const;
    const vector<Task *> &getTasks() const;
    void clearTasks();

    bool addTask(Task *task);
    void removeTask(int id);
    Task *findTaskById(int id) const;
    vector<Task *> searchByTitle(const string &keyword) const;
    void changeStatus(int id, string newStatus);

    void sortTasks();
    bool postponeDeadline(int id, int days);
    bool completeHabit(int id);
    void displayAll() const;
    void displayTaskById(int id) const;

    vector<Task *> getTaskByStatus(const string &status) const;
    vector<Task *> getTasksByTag(const string &tag) const;
    TaskStats calculateStats() const;
    void displayBoardView() const;
    void displayStats() const;
};