#include "FileManager.h"
#include "DeadlineTask.h"
#include "HabitTask.h"
#include "EventTask.h"
#include "ChecklistTask.h"
#include "Exceptions.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

FileManager::FileManager(string filename) : filename(filename) {}

string FileManager::getFilename() const
{
    return filename;
}

void FileManager::setFilename(string newFilename)
{
    filename = newFilename;
}

vector<string> FileManager::split(const string &text, char delimiter) const
{
    vector<string> tokens;
    stringstream ss(text);
    string item;

    while (getline(ss, item, delimiter))
    {
        tokens.push_back(item);
    }

    return tokens;
}

void FileManager::saveToFile(const TaskManager &manager) const
{
    ofstream file(filename);
    if (!file.is_open())
    {
        throw FileException("Nie udało się otworzyć pliku do zapisu: " + filename);
    }

    for (const Task *task : manager.getTasks())
    {
        file << task->serialize() << "\n";
    }

    if (file.fail())
    {
        throw FileException("Wystapił bląd podczas zapisywania danych do pliku: " + filename);
    }

    file.close();
    cout << "Zapisano " << manager.getTasks().size() << " zadań do pliku: '" << filename << "'" << endl;
}

void FileManager::loadFromFile(TaskManager &manager) const
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw FileException("Nie udało sie otworzyć pliku do odczytu: '" + filename + "'");
    }

    manager.clearTasks();

    string line;
    int lineNumber = 0;
    int loadedCount = 0;

    while (getline(file, line))
    {
        lineNumber++;

        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (line.empty())
        {
            continue;
        }

        vector<string> fields = split(line, '|');
        if (fields.size() < 5)
        {
            cout << "Ostrzeżenie: Pominięto uszkodzoną linie nr " << lineNumber << "." << endl;
            continue;
        }

        try
        {
            string type = fields[0];
            int id = stoi(fields[1]);
            string title = fields[2];
            string tag = fields[3];
            string status = fields[4];

            Task *task = nullptr;

            if (type == "DEADLINE" && fields.size() >= 7)
            {
                string dueDate = fields[5];
                int priority = stoi(fields[6]);
                task = new DeadlineTask(id, title, tag, dueDate, priority);
            }
            else if (type == "HABIT" && fields.size() >= 8)
            {
                int intervalDays = stoi(fields[5]);
                int currentStreak = stoi(fields[6]);
                int priority = stoi(fields[7]);

                HabitTask *habit = new HabitTask(id, title, tag, intervalDays, priority);
                habit->setCurrentStreak(currentStreak);
                task = habit;
            }
            else if (type == "EVENT" && fields.size() >= 9)
            {
                string date = fields[5];
                string startTime = fields[6];
                string endTime = fields[7];
                string location = fields[8];
                task = new EventTask(id, title, tag, date, startTime, endTime, location);
            }
            else if (type == "CHECKLIST" && fields.size() >= 6)
            {
                int priority = stoi(fields[5]);
                ChecklistTask *checklist = new ChecklistTask(id, title, tag, priority);

                if (fields.size() >= 7 && !fields[6].empty())
                {
                    vector<string> rawSubtasks = split(fields[6], ';');
                    for (const string &sub : rawSubtasks)
                    {
                        size_t colonPos = sub.rfind(':');
                        if (colonPos != string::npos)
                        {
                            string subName = sub.substr(0, colonPos);
                            bool isDone = (sub.substr(colonPos + 1) == "1");
                            checklist->addSubtask(subName, isDone);
                        }
                    }
                }

                task = checklist;
            }
            else
            {
                cout << "Ostrzeżenie: Nieznany typ lub niepełne dane w linii nr " << lineNumber << "." << endl;
                continue;
            }

            task->setStatus(status);
            if (manager.addTask(task))
            {
                loadedCount++;
            }
        }
        catch (const exception &e)
        {
            cout << "Ostrzeżenie: Błąd odczytu linii nr " << lineNumber << " (" << e.what() << ")." << endl;
        }
    }

    file.close();
    cout << "Wczytano " << loadedCount << " zadań z pliku: '" << filename << "'" << endl;
}