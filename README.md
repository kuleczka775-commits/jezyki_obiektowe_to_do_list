# Osobisty Menedżer Zadań (C++17 + Raylib GUI)

Aplikacja okienkowa w języku C++17 z interfejsem graficznym w stylu Notion (Raylib + `raygui`) oraz pełnym trybem konsolowym. Projekt rozdziela logikę domenową (`TaskManager`), warstwę zapisu (`FileManager`) oraz warstwę prezentacji (`GuiApp`).

## Funkcjonalności

### Interfejs graficzny (GUI)
* **Wszystkie zadania:** przewijalne karty z etykietami typów, wyszukiwarka na żywo (tytuł, tag, opis), filtrowanie po typie zadania, sortowanie po priorytecie i szybka zmiana statusu kliknięciem.
* **Akcje specjalne bezpośrednio na kartach:**
  * `DeadlineTask` – szybkie przesunięcie terminu (`+1 dzień`),
  * `HabitTask` – podbicie serii (`+1 Streak`),
  * `ChecklistTask` – interaktywne odhaczanie podzadań oraz dopisywanie nowych (`+ Podzadanie`).
* **Formularz `+ Nowe zadanie`:** dodawanie wszystkich 4 typów zadań (`Termin`, `Nawyk`, `Wydarzenie`, `Checklista`) bezpośrednio w oknie aplikacji.
* **Widok tablicy (`BoardView`):** 3 przewijalne kolumny (*Do zrobienia*, *W trakcie*, *Wykonane*) z przenoszeniem kart (`<-` / `->`).
* **Panel statystyk:** kafelki podsumowujące, ogólny pasek postępu, wykres podziału wg typów oraz liczniki serii nawyków i podzadań checklist.
* **Zapis danych:** ręczny zapis przyciskiem w pasku bocznym oraz automatyczny zapis do `tasks.txt` przy zamykaniu okna.

### Tryb konsolowy
Zachowany w pełni jako alternatywny tryb uruchamiany flagą `--console`.

## Struktura projektu

* `main.cpp` – punkt wejścia (wybór trybu GUI / `--console`) oraz menu konsolowe
* `GuiApp.h / GuiApp.cpp` – aplikacja okienkowa w Raylib i `raygui`
* `raygui.h` – jednoplikowa biblioteka kontrolek UI dla Raylib
* `Task.h / Task.cpp` – abstrakcyjna klasa bazowa zadania
* `DeadlineTask.h / .cpp` – zadania z terminem
* `HabitTask.h / .cpp` – nawyki cykliczne
* `EventTask.h / .cpp` – wydarzenia z godziną i miejscem
* `ChecklistTask.h / .cpp` – listy z podzadaniami
* `TaskManager.h / .cpp` – logika zarządzania zadaniami, sortowanie i statystyki
* `FileManager.h / .cpp` – odczyt i zapis do pliku `tasks.txt`
* `Exceptions.h` – własne klasy wyjątków

## Schemat klas UML

```mermaid
classDiagram
    class Task {
        <<abstract>>
        #int id
        #string title
        #string tag
        #string status
        +Task(int id, string title, string tag)
        +~Task()
        +setStatus(string newStatus) void
        +getTitle() string const
        +getId() int const
        +getTag() string const
        +getStatus() string const
        +getPriority() int const
        +getType()* string
        +getSummary()* string
        +serialize()* string
        +operator<(Task other) bool const
        +operator==(Task other) bool const
        +friend operator<<(ostream os, Task task) ostream
    }

    class DeadlineTask {
        -string dueDate
        -int priority
        +DeadlineTask(int id, string title, string tag, string dueDate, int priority)
        +getDueDate() string const
        +getPriority() int const
        +getType() string const
        +getSummary() string const
        +serialize() string const
        +operator+(int days) DeadlineTask const
    }

    class HabitTask {
        -int intervalDays
        -int currentStreak
        -int priority
        +HabitTask(int id, string title, string tag, int intervalDays, int priority)
        +getCurrentStreak() int const
        +setCurrentStreak(int streak) void
        +getPriority() int const
        +getType() string const
        +getSummary() string const
        +serialize() string const
        +operator++() HabitTask
    }

    class EventTask {
        -string date
        -string startTime
        -string endTime
        -string location
        +EventTask(int id, string title, string tag, string date, string startTime, string endTime, string location)
        +getType() string const
        +getSummary() string const
        +serialize() string const
    }

    class ChecklistTask {
        -int priority
        -vector~pair~ subtasks
        +ChecklistTask(int id, string title, string tag, int priority)
        +addSubtask(string name, bool done) void
        +getSubtasks() vector~pair~ const
        +getPriority() int const
        +getType() string const
        +getSummary() string const
        +serialize() string const
    }

    class TaskStats {
        <<struct>>
        +int totalTasks
        +int todoCount
        +int inProgressCount
        +int doneCount
        +int completionPercent
        +int maxStreak
        +string bestHabitTitle
        +int totalSubtasks
        +int completedSubtasks
        +map~string, int~ countByTag
    }

    class TaskManager {
        -vector~Task*~ tasks
        -int nextId
        +TaskManager()
        +~TaskManager()
        +getNextId() int const
        +getTasks() vector~Task*~ const
        +clearTasks() void
        +addTask(Task* task) bool
        +removeTask(int id) void
        +findTaskById(int id) Task* const
        +searchByTitle(string keyword) vector~Task*~ const
        +changeStatus(int id, string newStatus) void
        +sortTasks() void
        +postponeDeadline(int id, int days) bool
        +completeHabit(int id) bool
        +displayAll() void const
        +displayTaskById(int id) void const
        +getTaskByStatus(string status) vector~Task*~ const
        +getTasksByTag(string tag) vector~Task*~ const
        +calculateStats() TaskStats const
        +displayBoardView() void const
        +displayStats() void const
    }

    class FileManager {
        -string filename
        -split(string text, char delimiter) vector~string~ const
        +FileManager(string filename)
        +saveToFile(TaskManager manager) void const
        +loadFromFile(TaskManager manager) void const
    }

    class AppView {
        <<enumeration>>
        LIST
        BOARD
        STATS
        ADD
    }

    class GuiApp {
        -TaskManager& manager
        -FileManager& fileManager
        -int screenWidth
        -int screenHeight
        -AppView currentView
        -float listScrollY
        -float boardScrollY[3]
        -char searchBuf[128]
        -int filterType
        -int activeInput
        -int addingSubtaskTaskId
        -char newSubtaskBuf[128]
        -int formType
        -int formPriority
        -char formTitle[128]
        -char formTag[64]
        -char formDate[32]
        -char formInterval[16]
        -char formStartTime[16]
        -char formEndTime[16]
        -char formLocation[128]
        -char formSubtasks[256]
        -string statusMessage
        -float statusTimer
        -bool statusIsError
        -drawSidebar() void
        -drawMainContent() void
        -drawListView(float x, float y, float w, float h) void
        -drawBoardView(float x, float y, float w, float h) void
        -drawStatsView(float x, float y, float w, float h) void
        -drawAddTaskView(float x, float y, float w, float h) void
        -resetAddForm() void
        -getNextTaskId() int const
        -showStatus(string msg, bool isError) void
        +GuiApp(TaskManager manager, FileManager fileManager, int width, int height)
        +run() void
    }

    class StdRuntimeError["std::runtime_error"] {
        <<exception>>
    }

    class FileException {
        <<exception>>
        +FileException(string msg)
    }

    class TaskNotFoundException {
        <<exception>>
        +TaskNotFoundException(string msg)
    }

    %% Dziedziczenie
    Task <|-- DeadlineTask
    Task <|-- HabitTask
    Task <|-- EventTask
    Task <|-- ChecklistTask
    StdRuntimeError <|-- FileException
    StdRuntimeError <|-- TaskNotFoundException

    %% Kompozycja (TaskManager zarzadza cyklem zycia obiektow Task)
    TaskManager "1" *-- "0..*" Task : przechowuje i zwalnia

    %% Agregacja (GuiApp korzysta z zewnetrznych obiektow przez referencje)
    GuiApp "1" o-- "1" TaskManager : referencja
    GuiApp "1" o-- "1" FileManager : referencja

    %% Asocjacje i zaleznosci
    GuiApp --> AppView : aktualny widok
    TaskManager ..> TaskStats : zwraca
    TaskManager ..> TaskNotFoundException : rzuca
    FileManager ..> TaskManager : zapisuje / wczytuje
    FileManager ..> Task : serializuje / tworzy
    FileManager ..> FileException : rzuca
```

### Relacje w projekcie
* **Dziedziczenie (`<|--`):** `DeadlineTask`, `HabitTask`, `EventTask` oraz `ChecklistTask` dziedziczą po abstrakcyjnej klasie bazowej `Task`, implementując jej metody czysto wirtualne (`getType`, `getSummary`, `serialize`). Klasy `FileException` i `TaskNotFoundException` dziedziczą po `std::runtime_error`.
* **Kompozycja (`*--`):** `TaskManager` jest właścicielem wskaźników `Task*` i odpowiada za ich cykl życia — zniszczenie `TaskManager` automatycznie zwalnia wszystkie przechowywane zadania w destruktorze (`clearTasks()`).
* **Agregacja (`o--`):** `GuiApp` przechowuje referencje (`TaskManager&`, `FileManager&`) do obiektów utworzonych w `main()`. Zamknięcie interfejsu graficznego nie niszczy tych obiektów — ich cykl życia jest niezależny od `GuiApp`.
* **Zależności (`..>`):** `FileManager` korzysta z `TaskManager` i `Task` podczas zapisu/odczytu oraz rzuca `FileException`; `TaskManager` tworzy strukturę `TaskStats` i rzuca `TaskNotFoundException`.

## Wymagania i instalacja zależności (Windows / MSYS2 UCRT64)

1. Zainstaluj bibliotekę Raylib w terminalu **MSYS2 UCRT64**:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-raylib
   ```
2. Upewnij się, że w folderze projektu znajduje się plik `raygui.h` (jeśli go brakuje, pobierz go w PowerShellu):
   ```powershell
   Invoke-WebRequest -Uri "[https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h](https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h)" -OutFile "raygui.h"
   ```

## Kompilacja

```bash
g++ -std=c++17 main.cpp Task.cpp DeadlineTask.cpp HabitTask.cpp EventTask.cpp ChecklistTask.cpp TaskManager.cpp FileManager.cpp GuiApp.cpp -o task_manager.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```

## Uruchomienie

* **Tryb okienkowy (domyślny):**
  ```bash
  ./task_manager.exe
  ```
* **Tryb konsolowy:**
  ```bash
  ./task_manager.exe --console
  ```

<!-- wersja dla wyświetlania wyłącznie w konsoli -->
<!-- # Osobisty Menedżer Zadań (C++17)

Obiektowa aplikacja konsolowa w języku C++17 do zarządzania zadaniami, inspirowana narzędziami typu Notion. Projekt wykorzystuje polimorfizm, dziedziczenie, własną hierarchię wyjątków oraz trwały zapis stanu do pliku tekstowego.

## Funkcjonalności

* **4 typy zadań (polimorfizm):**
  * `DeadlineTask` – zadania z terminem realizacji (`RRRR-MM-DD`) i priorytetem,
  * `HabitTask` – nawyki cykliczne z interwałem dni oraz licznikiem serii (*Streak*),
  * `EventTask` – wydarzenia z datą, przedziałem godzinowym i miejscem,
  * `ChecklistTask` – listy z podzadaniami i obliczaniem procentu postępu.
* **Widoki i organizacja:**
  * lista wszystkich zadań,
  * widok tablicy (`BoardView`) z podziałem na statusy (*Do zrobienia*, *W trakcie*, *Wykonane*),
  * panel statystyk postępu.
* **Zarządzanie zadaniami:** dodawanie, usuwanie, zmiana statusu, sortowanie po priorytecie i tytule, wyszukiwanie i filtrowanie oraz akcje specjalne (przesuwanie terminu, oznaczanie nawyku, odhaczanie podzadań checklisty).
* **Trwały zapis:** automatyczne wczytywanie przy starcie oraz zapis do pliku `tasks.txt`.

## Struktura projektu

* `main.cpp` – punkt wejścia i interfejs menu konsolowego
* `Task.h / Task.cpp` – abstrakcyjna klasa bazowa zadania
* `DeadlineTask.h / .cpp` – zadania z terminem
* `HabitTask.h / .cpp` – nawyki
* `EventTask.h / .cpp` – wydarzenia
* `ChecklistTask.h / .cpp` – checklisty z podzadaniami
* `TaskManager.h / .cpp` – logika zarządzania kolekcją zadań, sortowanie, filtrowanie i widoki
* `FileManager.h / .cpp` – serializacja i deserializacja pliku `tasks.txt`
* `Exceptions.h` – własne klasy wyjątków (`FileException`, `TaskNotFoundException`)

## Wymagania i kompilacja

Wymagany kompilator obsługujący **C++17** (np. `g++` w środowisku MSYS2 UCRT64):

```bash
g++ -std=c++17 main.cpp Task.cpp DeadlineTask.cpp HabitTask.cpp EventTask.cpp ChecklistTask.cpp TaskManager.cpp FileManager.cpp -o task_manager.exe
```

## Uruchomienie

```bash
./task_manager.exe
``` -->
