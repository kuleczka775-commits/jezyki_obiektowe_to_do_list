#pragma once
#include "Task.h"
#include <vector>
#include <string>

using namespace std;

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
};