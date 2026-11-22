#pragma once
#include "Task.h"
#include <vector>
#include <utility>

using namespace std;

class ChecklistTask : public Task
{
private:
    int priority;
    vector<pair<string, bool>> subtasks;

public:
    ChecklistTask(int id, string title, string tag, int priority);

    int getPriority() const override;
    void setPriority(int newPriority);

    void addSubtask(string name, bool isDone = false);
    void toggleSubtask(int index);
    int getProgress() const;
    const vector<pair<string, bool>> &getSubtasks() const;

    string getType() const override;
    string getSummary() const override;
    string serialize() const override;
};