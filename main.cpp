#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "GuiApp.h"
#include "TaskManager.h"
#include "FileManager.h"
#include "DeadlineTask.h"
#include "HabitTask.h"
#include "EventTask.h"
#include "ChecklistTask.h"
#include "Exceptions.h"
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

int readInt(const string &prompt)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Błąd: podaj poprawną liczbę całkowitą." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readLine(const string &prompt)
{
    string text;
    cout << prompt;
    getline(cin, text);
    return text;
}

void printMenu()
{
    cout << "\n\n========== OSOBISTY MENEDŻER ZADAŃ ==========" << endl;
    cout << "1. Wyświetl wszystkie zadania" << endl;
    cout << "2. Widok tablicy (BoardView)" << endl;
    cout << "3. Panel statystyk" << endl;
    cout << "4. Dodaj nowe zadanie" << endl;
    cout << "5. Zmień status zadania" << endl;
    cout << "6. Akcje specjalne (termin / nawyk / checklista)" << endl;
    cout << "7. Sortuj zadania (priorytet i tytuł)" << endl;
    cout << "8. Szukaj lub filtruj zadania" << endl;
    cout << "9. Usuń zadanie" << endl;
    cout << "10. Zapisz stan do pliku" << endl;
    cout << "0. Zapisz i zakończ" << endl;
    cout << "=============================================" << endl;
}

void handleAddTask(TaskManager &manager)
{
    cout << "\nWybierz typ zadania:" << endl;
    cout << "1. Zadanie z terminem (DeadlineTask)" << endl;
    cout << "2. Nawyk (HabitTask)" << endl;
    cout << "3. Wydarzenie (EventTask)" << endl;
    cout << "4. Lista podzadań (ChecklistTask)" << endl;

    int type = readInt("Typ (1-4): ");
    if (type < 1 || type > 4)
    {
        cout << "Nieprawidłowy wybór typu zadania." << endl;
        return;
    }

    int id = manager.getNextId();
    string title = readLine("Tytuł: ");
    string tag = readLine("Tag (np. studia, wizyty, dom): ");

    try
    {
        Task *newTask = nullptr;
        if (type == 1)
        {
            string dueDate = readLine("Termin (RRRR-MM-DD): ");
            int priority = readInt("Priorytet (1-niski, 2-średni, 3-wysoki): ");
            cout << "\n=============================================";
            newTask = new DeadlineTask(id, title, tag, dueDate, priority);
        }
        else if (type == 2)
        {
            int interval = readInt("Co ile dni chcesz wykonywać nawyk: ");
            int priority = readInt("Priorytet (1-niski, 2-średni, 3-wysoki): ");
            cout << "\n=============================================";
            newTask = new HabitTask(id, title, tag, interval, priority);
        }
        else if (type == 3)
        {
            string date = readLine("Data wydarzenia (RRRR-MM-DD): ");
            string startTime = readLine("Godzina rozpoczęcia (GG:MM): ");
            string endTime = readLine("Godzina zakończenia (GG:MM): ");
            string location = readLine("Miejsce: ");
            cout << "\n=============================================";
            newTask = new EventTask(id, title, tag, date, startTime, endTime, location);
        }
        else if (type == 4)
        {
            int priority = readInt("Priorytet (1-niski, 2-średni, 3-wysoki): ");
            cout << "\n=============================================";
            ChecklistTask *checklist = new ChecklistTask(id, title, tag, priority);

            int count = readInt("Ile podzadań chesz dodać? ");
            for (int i = 0; i < count; i++)
            {
                string subName = readLine("  Podzadanie " + to_string(i + 1) + ": ");
                if (!subName.empty())
                {
                    checklist->addSubtask(subName);
                }
            }
            newTask = checklist;
        }
        manager.addTask(newTask);
    }
    catch (const exception &e)
    {
        cout << "Błąd walidacji: " << e.what() << endl;
    }
}

void handleChangeStatus(TaskManager &manager)
{
    int id = readInt("Podaj ID zadania: ");
    cout << "Nowy status: 1. Do zrobienia | 2. W trakcie | 3. Zrobione" << endl;
    int statusChoice = readInt("Wybór (1-3): ");

    string newStatus;
    if (statusChoice == 1)
    {
        newStatus = "Do zrobienia";
    }
    else if (statusChoice == 2)
    {
        newStatus = "W trakcie";
    }
    else if (statusChoice == 3)
    {
        newStatus = "Zrobione";
    }
    else
    {
        cout << "Nieprawidłowy kod statusu." << endl;
        return;
    }

    cout << endl;

    try
    {
        manager.changeStatus(id, newStatus);
    }
    catch (const TaskNotFoundException &e)
    {
        cout << "Błąd: " << e.what() << endl;
    }
}

void handleSpecialActions(TaskManager &manager)
{
    cout << "\n--- AKCJE SPECJALNE ---" << endl;
    cout << "1. Przesuń termin zadania o X dni" << endl;
    cout << "2. Zalicz nawyk i podbij streak" << endl;
    cout << "3. Odhacz podzadanie w checkliscie" << endl;
    cout << "4. Dodaj nowe podzadanie do istniejącej checklisty" << endl;

    int action = readInt("Wybor (1-4): ");
    int id = readInt("Podaj Id zadania: ");

    try
    {
        if (action == 1)
        {
            int days = readInt("O ile dni przesunąć termin? ");
            cout << endl;
            manager.postponeDeadline(id, days);
        }
        else if (action == 2)
        {
            cout << endl;
            manager.completeHabit(id);
        }
        else if (action == 3 || action == 4)
        {
            Task *task = manager.findTaskById(id);
            ChecklistTask *checklist = dynamic_cast<ChecklistTask *>(task);

            if (checklist == nullptr)
            {
                cout << "Zadanie o ID " << id << " nie jest checklistą." << endl;
                return;
            }

            if (action == 3)
            {
                const auto &subs = checklist->getSubtasks();
                if (subs.empty())
                {
                    cout << "Ta checklista nie ma jeszcze żadnych podzadań." << endl;
                    return;
                }
                for (size_t i = 0; i < subs.size(); i++)
                {
                    cout << " " << (i + 1) << ". [" << (subs[i].second ? "x" : " ") << "] " << subs[i].first << endl;
                }
                int idx = readInt("Który numer podzadania przełączyć? ");
                checklist->toggleSubtask(idx - 1);
            }
            else
            {
                string subName = readLine("Nazwa nowego podzadania: ");
                checklist->addSubtask(subName);
            }
        }
        else
        {
            cout << "Nieprawidłowa opcja." << endl;
        }
    }
    catch (const exception &e)
    {
        cout << "Błąd: " << e.what() << endl;
    }
}

void handleSearchAndFilter(const TaskManager &manager)
{
    cout << "\n1. Szukaj po fragmencie tytułu" << endl;
    cout << "2. Filtruj po dokładnym tagu" << endl;
    int mode = readInt("Wybór (1-2): ");

    vector<Task *> results;
    if (mode == 1)
    {
        string keyword = readLine("Wpisz szukaną frazę: ");
        results = manager.searchByTitle(keyword);
    }
    else if (mode == 2)
    {
        string tag = readLine("Wpisz nazwę tagu: ");
        results = manager.getTasksByTag(tag);
    }
    else
    {
        cout << "Nieprawidłowy wybór." << endl;
        return;
    }

    if (results.empty())
    {
        cout << "Brak pasujących zadań." << endl;
    }
    else
    {
        cout << "\nZnalezione zadania (" << results.size() << "):" << endl;
        for (const Task *task : results)
        {
            cout << " " << *task << endl;
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    TaskManager manager;
    FileManager fileManager("tasks.txt");

    try
    {
        fileManager.loadFromFile(manager);
    }
    catch (const FileException &e)
    {
        cout << "Informacja: " << e.what() << " (start z pustą listą zadań)." << endl;
    }

    bool consoleMode = (argc > 1 && string(argv[1]) == "--console");

    if (!consoleMode)
    {
        GuiApp app(manager, fileManager);
        app.run();
        return 0;
    }

    int choice = -1;
    while (choice != 0)
    {
        printMenu();
        choice = readInt("Wybierz opcje: ");
        cout << endl;

        switch (choice)
        {
        case 1:
            manager.displayAll();
            break;
        case 2:
            manager.displayBoardView();
            break;
        case 3:
            manager.displayStats();
            break;
        case 4:
            handleAddTask(manager);
            break;
        case 5:
            handleChangeStatus(manager);
            break;
        case 6:
            handleSpecialActions(manager);
            break;
        case 7:
            manager.sortTasks();
            manager.displayAll();
            break;
        case 8:
            handleSearchAndFilter(manager);
            break;
        case 9:
        {
            int id = readInt("Podaj ID zadania do usuniecia: ");
            cout << endl;
            try
            {
                manager.removeTask(id);
            }
            catch (const TaskNotFoundException &e)
            {
                cout << "Błąd: " << e.what() << endl;
            }
            break;
        }
        case 10:
            try
            {
                fileManager.saveToFile(manager);
            }
            catch (const FileException &e)
            {
                cout << "Błąd zapisu: " << e.what() << endl;
            }
            break;
        case 0:
            try
            {
                fileManager.saveToFile(manager);
            }
            catch (const FileException &e)
            {
                cout << "Błąd zapisu przy zamykaniu: " << e.what() << endl;
            }
            cout << "Do zobaczenia!" << endl;
            break;
        default:
            cout << "Nieznana opcja. Wybierz numer od 0 do 10." << endl;
            break;
        }
    }

    return 0;
}