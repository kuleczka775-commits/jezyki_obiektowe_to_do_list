#include "GuiApp.h"
#include "DeadlineTask.h"
#include "HabitTask.h"
#include "EventTask.h"
#include "ChecklistTask.h"
#include "Exceptions.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

using namespace std;

namespace
{
    Font uiFont = {0};
    bool customFontLoaded = false;

    void loadUiFont()
    {
        int codepoints[352];
        for (int i = 0; i < 352; i++)
        {
            codepoints[i] = 32 + i;
        }

        const char *fontPath = nullptr;
        if (FileExists("C:/Windows/Fonts/segoeui.ttf"))
        {
            fontPath = "C:/Windows/Fonts/segoeui.ttf";
        }
        else if (FileExists("C:/Windows/Fonts/arial.ttf"))
        {
            fontPath = "C:/Windows/Fonts/arial.ttf";
        }

        if (fontPath != nullptr)
        {
            uiFont = LoadFontEx(fontPath, 72, codepoints, 352);
            SetTextureFilter(uiFont.texture, TEXTURE_FILTER_BILINEAR);
            customFontLoaded = true;
        }
        else
        {
            uiFont = GetFontDefault();
            customFontLoaded = false;
        }
    }

    void drawUiText(const char *text, float x, float y, float size, Color color)
    {
        DrawTextEx(uiFont, text, Vector2{x, y}, size, 1.0f, color);
    }

    string fitText(const string &text, float fontSize, float maxWidth)
    {
        if (maxWidth <= 20.0f)
        {
            return "";
        }
        if (MeasureTextEx(uiFont, text.c_str(), fontSize, 1.0f).x <= maxWidth)
        {
            return text;
        }
        string clipped = text;
        while (clipped.size() > 3)
        {
            clipped.pop_back();
            // Nie ucinaj w polowie znaku UTF-8
            while (!clipped.empty() && ((unsigned char)clipped.back() & 0xC0) == 0x80)
            {
                clipped.pop_back();
            }
            string candidate = clipped + "...";
            if (MeasureTextEx(uiFont, candidate.c_str(), fontSize, 1.0f).x <= maxWidth)
            {
                return candidate;
            }
        }
        return "...";
    }

    void drawCardBox(Rectangle rec, float roundness, Color fill, Color border)
    {
        DrawRectangleRounded(rec, roundness, 8, border);
        Rectangle inner = {rec.x + 1.5f, rec.y + 1.5f, rec.width - 3.0f, rec.height - 3.0f};
        DrawRectangleRounded(inner, roundness, 8, fill);
    }

    bool drawPillButton(Rectangle rec, const char *text, float fontSize,
                        Color bg, Color hoverBg, Color border, Color textColor, bool interactive)
    {
        Vector2 mouse = GetMousePosition();
        bool hovered = interactive && CheckCollisionPointRec(mouse, rec);

        drawCardBox(rec, 0.25f, hovered ? hoverBg : bg, border);

        Vector2 textSize = MeasureTextEx(uiFont, text, fontSize, 1.0f);
        float tx = rec.x + (rec.width - textSize.x) * 0.5f;
        float ty = rec.y + (rec.height - textSize.y) * 0.5f;
        drawUiText(text, tx, ty, fontSize, textColor);

        return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    string toLowerAscii(const string &s)
    {
        string out = s;
        for (char &c : out)
        {
            c = (char)tolower((unsigned char)c);
        }
        return out;
    }

    string trimStr(const string &s)
    {
        size_t first = s.find_first_not_of(" \t\r\n");
        if (first == string::npos)
        {
            return "";
        }
        size_t last = s.find_last_not_of(" \t\r\n");
        return s.substr(first, last - first + 1);
    }

    vector<string> splitStr(const string &text, char delimiter)
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

    string addDaysToDate(const string &dateStr, int daysToAdd)
    {
        int y = 0, m = 0, d = 0;
        if (sscanf(dateStr.c_str(), "%d-%d-%d", &y, &m, &d) != 3)
        {
            return dateStr;
        }
        if (m < 1 || m > 12 || d < 1 || d > 31)
        {
            return dateStr;
        }

        auto daysInMonth = [](int year, int month)
        {
            static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            if (month == 2)
            {
                bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                return leap ? 29 : 28;
            }
            return mdays[month - 1];
        };

        d += daysToAdd;
        while (d > daysInMonth(y, m))
        {
            d -= daysInMonth(y, m);
            m++;
            if (m > 12)
            {
                m = 1;
                y++;
            }
        }

        char buf[32];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
        return string(buf);
    }

    vector<pair<string, bool>> parseSubtasksFromTask(const Task *task)
    {
        vector<pair<string, bool>> result;
        vector<string> fields = splitStr(task->serialize(), '|');
        if (fields.size() >= 7 && !fields[6].empty())
        {
            vector<string> raw = splitStr(fields[6], ';');
            for (const string &entry : raw)
            {
                size_t pos = entry.rfind(':');
                if (pos != string::npos)
                {
                    string name = entry.substr(0, pos);
                    bool done = (entry.substr(pos + 1) == "1");
                    result.push_back({name, done});
                }
            }
        }
        return result;
    }

    bool isDoneStatus(const string &status)
    {
        return status == "Wykonane" || status == "Zrobione";
    }

    string getNextStatus(const string &current)
    {
        if (current == "Do zrobienia")
        {
            return "W trakcie";
        }
        if (current == "W trakcie")
        {
            return "Wykonane";
        }
        return "Do zrobienia";
    }

    struct BadgeStyle
    {
        const char *label;
        Color bg;
        Color text;
    };

    BadgeStyle getTaskTypeStyle(const Task *task)
    {
        if (dynamic_cast<const DeadlineTask *>(task) != nullptr)
        {
            return {"Termin", Color{254, 232, 232, 255}, Color{180, 45, 45, 255}};
        }
        if (dynamic_cast<const HabitTask *>(task) != nullptr)
        {
            return {"Nawyk", Color{240, 232, 255, 255}, Color{110, 55, 180, 255}};
        }
        if (dynamic_cast<const EventTask *>(task) != nullptr)
        {
            return {"Wydarzenie", Color{226, 240, 255, 255}, Color{30, 95, 180, 255}};
        }
        if (dynamic_cast<const ChecklistTask *>(task) != nullptr)
        {
            return {"Checklista", Color{255, 243, 222, 255}, Color{175, 100, 15, 255}};
        }
        return {"Zadanie", Color{238, 238, 238, 255}, Color{70, 70, 70, 255}};
    }

    int getTaskTypeIndex(const Task *task)
    {
        if (dynamic_cast<const DeadlineTask *>(task) != nullptr)
            return 1;
        if (dynamic_cast<const HabitTask *>(task) != nullptr)
            return 2;
        if (dynamic_cast<const EventTask *>(task) != nullptr)
            return 3;
        if (dynamic_cast<const ChecklistTask *>(task) != nullptr)
            return 4;
        return 0;
    }

    void getStatusColors(const string &status, Color &bg, Color &hoverBg, Color &border, Color &text)
    {
        if (isDoneStatus(status))
        {
            bg = Color{226, 245, 233, 255};
            hoverBg = Color{208, 238, 218, 255};
            border = Color{170, 220, 186, 255};
            text = Color{28, 115, 58, 255};
        }
        else if (status == "W trakcie")
        {
            bg = Color{228, 241, 255, 255};
            hoverBg = Color{210, 232, 255, 255};
            border = Color{174, 208, 245, 255};
            text = Color{25, 92, 175, 255};
        }
        else
        {
            bg = Color{242, 242, 240, 255};
            hoverBg = Color{230, 230, 226, 255};
            border = Color{212, 212, 208, 255};
            text = Color{75, 75, 75, 255};
        }
    }

    void drawInputField(Rectangle rec, char *buffer, int bufferSize, int fieldId,
                        int &activeInput, const char *placeholder)
    {
        bool editing = (activeInput == fieldId);
        if (GuiTextBox(rec, buffer, bufferSize, editing))
        {
            activeInput = editing ? 0 : fieldId;
        }
        if (buffer[0] == '\0' && activeInput != fieldId)
        {
            drawUiText(placeholder, rec.x + 12.0f, rec.y + (rec.height - 24.0f) * 0.5f, 24.0f, Color{165, 165, 165, 255});
        }
    }
}

GuiApp::GuiApp(TaskManager &manager, FileManager &fileManager, int width, int height)
    : manager(manager),
      fileManager(fileManager),
      screenWidth(width),
      screenHeight(height),
      currentView(AppView::LIST),
      listScrollY(0.0f),
      filterType(0),
      activeInput(0),
      addingSubtaskTaskId(-1),
      formType(0),
      formPriority(2),
      statusMessage(""),
      statusTimer(0.0f),
      statusIsError(false)
{
    boardScrollY[0] = boardScrollY[1] = boardScrollY[2] = 0.0f;
    memset(searchBuf, 0, sizeof(searchBuf));
    memset(newSubtaskBuf, 0, sizeof(newSubtaskBuf));
    resetAddForm();
}

void GuiApp::resetAddForm()
{
    formPriority = 2;
    memset(formTitle, 0, sizeof(formTitle));
    memset(formTag, 0, sizeof(formTag));
    snprintf(formDate, sizeof(formDate), "2026-11-30");
    snprintf(formInterval, sizeof(formInterval), "1");
    snprintf(formStartTime, sizeof(formStartTime), "10:00");
    snprintf(formEndTime, sizeof(formEndTime), "11:00");
    memset(formLocation, 0, sizeof(formLocation));
    memset(formSubtasks, 0, sizeof(formSubtasks));
    activeInput = 0;
}

int GuiApp::getNextTaskId() const
{
    int maxId = 0;
    for (const Task *t : manager.getTasks())
    {
        if (t->getId() > maxId)
        {
            maxId = t->getId();
        }
    }
    return maxId + 1;
}

void GuiApp::showStatus(const string &msg, bool isError)
{
    statusMessage = msg;
    statusTimer = 3.5f;
    statusIsError = isError;
}

void GuiApp::run()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Osobisty Menedżer Zadań");
    SetWindowMinSize(1024, 680);
    SetExitKey(KEY_NULL); // ESC nie zamyka okna podczas edycji pola tekstowego
    SetTargetFPS(60);

    loadUiFont();

    GuiSetFont(uiFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x262626ff);

    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xeeeeecff);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xd8d8d4ff);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xe2e2deff);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xb8b8b2ff);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0x111111ff);

    GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL, 0xffffffff);
    GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL, 0xd0d0ccff);
    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL, 0x222222ff);
    GuiSetStyle(TEXTBOX, BASE_COLOR_FOCUSED, 0xffffffff);
    GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, 0x6b9ef8ff);
    GuiSetStyle(TEXTBOX, TEXT_COLOR_FOCUSED, 0x111111ff);
    GuiSetStyle(TEXTBOX, BASE_COLOR_PRESSED, 0xffffffff);
    GuiSetStyle(TEXTBOX, BORDER_COLOR_PRESSED, 0x2563ebff);
    GuiSetStyle(TEXTBOX, TEXT_COLOR_PRESSED, 0x111111ff);

    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        if (IsKeyPressed(KEY_ESCAPE))
        {
            activeInput = 0;
            addingSubtaskTaskId = -1;
        }

        if (statusTimer > 0.0f)
        {
            statusTimer -= GetFrameTime();
        }

        BeginDrawing();
        ClearBackground(Color{255, 255, 255, 255});

        drawSidebar();
        drawMainContent();

        EndDrawing();
    }

    // Automatyczny zapis zmian przy zamykaniu okna
    try
    {
        fileManager.saveToFile(manager);
    }
    catch (const exception &)
    {
    }

    if (customFontLoaded)
    {
        UnloadFont(uiFont);
    }
    CloseWindow();
}

void GuiApp::drawSidebar()
{
    const float sidebarWidth = 340.0f;

    DrawRectangle(0, 0, (int)sidebarWidth, screenHeight, Color{247, 247, 245, 255});
    DrawLine((int)sidebarWidth, 0, (int)sidebarWidth, screenHeight, Color{225, 225, 222, 255});

    drawUiText("Menedżer Zadań", 24, 26, 36, Color{32, 32, 32, 255});
    drawUiText("Workspace", 24, 70, 22, Color{135, 135, 135, 255});

    // Glowny przycisk dodawania zadania
    Rectangle addBtn = {20.0f, 118.0f, sidebarWidth - 40.0f, 56.0f};
    if (drawPillButton(addBtn, "+ Nowe zadanie", 26.0f,
                       Color{42, 42, 42, 255}, Color{65, 65, 65, 255},
                       Color{30, 30, 30, 255}, Color{255, 255, 255, 255}, true))
    {
        resetAddForm();
        addingSubtaskTaskId = -1;
        currentView = AppView::ADD;
    }

    auto drawNavButton = [&](float y, const char *label, AppView view)
    {
        Rectangle btnRect = {20.0f, y, sidebarWidth - 40.0f, 56.0f};
        bool active = (currentView == view);

        Color bg = active ? Color{228, 228, 224, 255} : Color{241, 241, 238, 255};
        Color hoverBg = Color{222, 222, 218, 255};
        Color border = active ? Color{190, 190, 184, 255} : Color{226, 226, 222, 255};

        if (drawPillButton(btnRect, label, 25.0f, bg, hoverBg, border, Color{30, 30, 30, 255}, true))
        {
            activeInput = 0;
            addingSubtaskTaskId = -1;
            currentView = view;
        }
        if (active)
        {
            DrawRectangleRounded(Rectangle{20.0f, y + 10.0f, 5.0f, 36.0f}, 0.5f, 4, Color{48, 48, 48, 255});
        }
    };

    drawNavButton(196.0f, "Wszystkie zadania", AppView::LIST);
    drawNavButton(264.0f, "Widok tablicy", AppView::BOARD);
    drawNavButton(332.0f, "Statystyki", AppView::STATS);

    // Krotkie podsumowanie na dole paska
    int doneCount = 0;
    for (const Task *t : manager.getTasks())
    {
        if (isDoneStatus(t->getStatus()))
        {
            doneCount++;
        }
    }
    string miniInfo = "Ukończono: " + to_string(doneCount) + " / " + to_string(manager.getTasks().size());
    drawUiText(miniInfo.c_str(), 24.0f, (float)screenHeight - 122.0f, 22.0f, Color{115, 115, 115, 255});

    float saveBtnY = (float)screenHeight - 82.0f;
    Rectangle saveBtn = {20.0f, saveBtnY, sidebarWidth - 40.0f, 56.0f};
    if (drawPillButton(saveBtn, "Zapisz do pliku", 25.0f,
                       Color{238, 238, 235, 255}, Color{224, 224, 220, 255},
                       Color{205, 205, 200, 255}, Color{35, 35, 35, 255}, true))
    {
        try
        {
            fileManager.saveToFile(manager);
            showStatus("Zapisano zadania do pliku.");
        }
        catch (const exception &e)
        {
            showStatus(string("Błąd zapisu: ") + e.what(), true);
        }
    }
}

void GuiApp::drawMainContent()
{
    const float startX = 384.0f;
    const float y = 26.0f;
    const float contentWidth = max(400.0f, (float)screenWidth - startX - 40.0f);

    const char *title = "";
    if (currentView == AppView::LIST)
    {
        title = "Wszystkie zadania";
    }
    else if (currentView == AppView::BOARD)
    {
        title = "Widok tablicy (BoardView)";
    }
    else if (currentView == AppView::STATS)
    {
        title = "Panel statystyk";
    }
    else if (currentView == AppView::ADD)
    {
        title = "Nowe zadanie";
    }

    drawUiText(title, startX, y, 44, Color{30, 30, 30, 255});
    DrawLine((int)startX, (int)y + 60, screenWidth - 40, (int)y + 60, Color{230, 230, 230, 255});

    float bodyY = y + 76.0f;
    float bodyHeight = max(200.0f, (float)screenHeight - bodyY - 68.0f);

    if (currentView == AppView::LIST)
    {
        drawListView(startX, bodyY, contentWidth, bodyHeight);
    }
    else if (currentView == AppView::BOARD)
    {
        drawBoardView(startX, bodyY, contentWidth, bodyHeight);
    }
    else if (currentView == AppView::STATS)
    {
        drawStatsView(startX, bodyY, contentWidth, bodyHeight);
    }
    else if (currentView == AppView::ADD)
    {
        drawAddTaskView(startX, bodyY, contentWidth, bodyHeight);
    }

    if (statusTimer > 0.0f)
    {
        Rectangle toast = {startX, (float)screenHeight - 56.0f, contentWidth, 42.0f};
        Color bg = statusIsError ? Color{254, 236, 236, 255} : Color{236, 248, 239, 255};
        Color border = statusIsError ? Color{240, 185, 185, 255} : Color{185, 228, 196, 255};
        Color txt = statusIsError ? Color{175, 40, 40, 255} : Color{35, 110, 50, 255};

        drawCardBox(toast, 0.25f, bg, border);
        drawUiText(statusMessage.c_str(), startX + 16.0f, (float)screenHeight - 48.0f, 23, txt);
    }
}

void GuiApp::drawListView(float startX, float startY, float width, float height)
{
    // 1. Pasek wyszukiwania + sortowanie
    float sortW = 240.0f;
    float searchW = max(200.0f, width - sortW - 16.0f);
    drawInputField(Rectangle{startX, startY, searchW, 46.0f}, searchBuf, sizeof(searchBuf),
                   1, activeInput, "Szukaj po tytule, tagu lub opisie...");

    Rectangle sortBtn = {startX + width - sortW, startY, sortW, 46.0f};
    if (drawPillButton(sortBtn, "Sortuj (priorytet)", 23.0f,
                       Color{244, 244, 241, 255}, Color{232, 232, 228, 255},
                       Color{212, 212, 206, 255}, Color{40, 40, 40, 255}, true))
    {
        manager.sortTasks();
        showStatus("Posortowano zadania według priorytetu i tytułu.");
    }

    // 2. Pasek filtrow po typie zadania
    const char *filterLabels[5] = {"Wszystkie", "Termin", "Nawyk", "Wydarzenie", "Checklista"};
    float fx = startX;
    float filterY = startY + 58.0f;
    for (int i = 0; i < 5; i++)
    {
        Vector2 sz = MeasureTextEx(uiFont, filterLabels[i], 22.0f, 1.0f);
        float bw = sz.x + 28.0f;
        bool selected = (filterType == i);

        Color bg = selected ? Color{45, 45, 45, 255} : Color{243, 243, 240, 255};
        Color hoverBg = selected ? Color{60, 60, 60, 255} : Color{232, 232, 228, 255};
        Color border = selected ? Color{35, 35, 35, 255} : Color{216, 216, 212, 255};
        Color txt = selected ? Color{255, 255, 255, 255} : Color{60, 60, 60, 255};

        if (drawPillButton(Rectangle{fx, filterY, bw, 38.0f}, filterLabels[i], 22.0f,
                           bg, hoverBg, border, txt, true))
        {
            filterType = i;
            listScrollY = 0.0f;
        }
        fx += bw + 10.0f;
    }

    // Filtrowanie listy zadan
    string query = toLowerAscii(trimStr(searchBuf));
    vector<Task *> visibleTasks;
    for (Task *t : manager.getTasks())
    {
        if (filterType != 0 && getTaskTypeIndex(t) != filterType)
        {
            continue;
        }
        if (!query.empty())
        {
            string hay = toLowerAscii(t->getTitle() + " " + t->getTag() + " " + t->getSummary());
            if (hay.find(query) == string::npos)
            {
                continue;
            }
        }
        visibleTasks.push_back(t);
    }

    string countLabel = "Widoczne: " + to_string(visibleTasks.size()) + " / " + to_string(manager.getTasks().size());
    Vector2 cntSize = MeasureTextEx(uiFont, countLabel.c_str(), 22.0f, 1.0f);
    if (startX + width - cntSize.x > fx + 10.0f)
    {
        drawUiText(countLabel.c_str(), startX + width - cntSize.x, filterY + 8.0f, 22.0f, Color{115, 115, 115, 255});
    }

    float listTop = filterY + 52.0f;
    float listHeight = max(100.0f, height - (listTop - startY));
    Rectangle listArea = {startX, listTop, width, listHeight};

    if (visibleTasks.empty())
    {
        drawCardBox(Rectangle{startX, listTop, width, 110.0f}, 0.15f,
                    Color{250, 250, 249, 255}, Color{230, 230, 226, 255});
        const char *emptyMsg = manager.getTasks().empty()
                                   ? "Brak zadań. Kliknij '+ Nowe zadanie' po lewej, aby coś dodać."
                                   : "Brak zadań pasujących do wybranego filtra.";
        drawUiText(emptyMsg, startX + 26.0f, listTop + 40.0f, 25.0f, Color{130, 130, 130, 255});
        return;
    }

    // Wyliczenie wysokosci kazdej karty (z zawijaniem podzadan i polem dodawania)
    const float cardGap = 14.0f;
    const float availSubW = max(120.0f, width - 40.0f);
    vector<float> cardHeights(visibleTasks.size(), 124.0f);
    vector<vector<pair<string, bool>>> subtasksCache(visibleTasks.size());
    float totalHeight = 0.0f;

    for (size_t i = 0; i < visibleTasks.size(); i++)
    {
        float h = 124.0f;
        if (dynamic_cast<ChecklistTask *>(visibleTasks[i]) != nullptr)
        {
            subtasksCache[i] = parseSubtasksFromTask(visibleTasks[i]);
            if (!subtasksCache[i].empty())
            {
                int rows = 1;
                float rowX = 0.0f;
                for (const auto &sub : subtasksCache[i])
                {
                    string pillText = string(sub.second ? "[x] " : "[ ] ") + sub.first;
                    float pw = min(MeasureTextEx(uiFont, pillText.c_str(), 20.0f, 1.0f).x + 22.0f, 260.0f);
                    if (rowX > 0.0f && rowX + pw > availSubW)
                    {
                        rows++;
                        rowX = 0.0f;
                    }
                    rowX += pw + 10.0f;
                }
                h += (float)rows * 44.0f + 4.0f;
            }
            if (addingSubtaskTaskId == visibleTasks[i]->getId())
            {
                h += 56.0f;
            }
        }
        cardHeights[i] = h;
        totalHeight += h + cardGap;
    }

    float maxScroll = max(0.0f, totalHeight - listHeight + 10.0f);
    Vector2 mouse = GetMousePosition();
    bool mouseInList = CheckCollisionPointRec(mouse, listArea);
    if (mouseInList)
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
        {
            listScrollY -= wheel * 48.0f;
        }
    }
    listScrollY = max(0.0f, min(listScrollY, maxScroll));

    // Odlozone akcje po zakonczeniu iteracji
    int statusTaskId = -1;
    string statusNextVal = "";
    int deleteTaskId = -1;
    int postponeTaskId = -1;
    int habitTaskId = -1;
    int toggleChecklistTaskId = -1;
    int toggleSubtaskIdx = -1;
    int openSubtaskEditorForId = -1;
    bool closeSubtaskEditor = false;
    int addSubtaskTargetId = -1;

    BeginScissorMode((int)listArea.x, (int)listArea.y, (int)max(1.0f, listArea.width), (int)max(1.0f, listArea.height));

    float currentY = listTop - listScrollY;
    for (size_t i = 0; i < visibleTasks.size(); i++)
    {
        Task *task = visibleTasks[i];
        float cardH = cardHeights[i];

        if (currentY + cardH >= listTop && currentY <= listTop + listHeight)
        {
            Rectangle cardRect = {startX, currentY, width, cardH};
            bool hovered = mouseInList && CheckCollisionPointRec(mouse, cardRect);

            drawCardBox(cardRect, 0.12f,
                        hovered ? Color{252, 252, 250, 255} : Color{255, 255, 255, 255},
                        hovered ? Color{198, 198, 192, 255} : Color{226, 226, 222, 255});

            // Przyciski po prawej stronie
            bool isDeadline = (dynamic_cast<DeadlineTask *>(task) != nullptr);
            bool isHabit = (dynamic_cast<HabitTask *>(task) != nullptr);
            bool isChecklist = (dynamic_cast<ChecklistTask *>(task) != nullptr);

            float rightCursor = startX + width - 18.0f;

            Rectangle delBtn = {rightCursor - 90.0f, currentY + 34.0f, 90.0f, 46.0f};
            if (drawPillButton(delBtn, "Usuń", 22.0f,
                               Color{254, 240, 240, 255}, Color{252, 220, 220, 255},
                               Color{240, 195, 195, 255}, Color{185, 45, 45, 255}, mouseInList))
            {
                deleteTaskId = task->getId();
            }
            rightCursor = delBtn.x - 10.0f;

            Color stBg, stHover, stBorder, stText;
            getStatusColors(task->getStatus(), stBg, stHover, stBorder, stText);
            Rectangle statusBtn = {rightCursor - 185.0f, currentY + 34.0f, 185.0f, 46.0f};
            if (drawPillButton(statusBtn, task->getStatus().c_str(), 22.0f,
                               stBg, stHover, stBorder, stText, mouseInList))
            {
                statusTaskId = task->getId();
                statusNextVal = getNextStatus(task->getStatus());
            }
            rightCursor = statusBtn.x - 10.0f;

            if (isDeadline)
            {
                Rectangle postBtn = {rightCursor - 122.0f, currentY + 34.0f, 122.0f, 46.0f};
                if (drawPillButton(postBtn, "+1 dzień", 21.0f,
                                   Color{255, 245, 235, 255}, Color{252, 232, 212, 255},
                                   Color{238, 200, 165, 255}, Color{170, 90, 20, 255}, mouseInList))
                {
                    postponeTaskId = task->getId();
                }
                rightCursor = postBtn.x - 12.0f;
            }
            else if (isHabit)
            {
                Rectangle habitBtn = {rightCursor - 128.0f, currentY + 34.0f, 128.0f, 46.0f};
                if (drawPillButton(habitBtn, "+1 Streak", 21.0f,
                                   Color{242, 235, 255, 255}, Color{228, 214, 252, 255},
                                   Color{202, 180, 242, 255}, Color{105, 50, 175, 255}, mouseInList))
                {
                    habitTaskId = task->getId();
                }
                rightCursor = habitBtn.x - 12.0f;
            }
            else if (isChecklist)
            {
                bool isOpen = (addingSubtaskTaskId == task->getId());
                Rectangle subBtn = {rightCursor - 152.0f, currentY + 34.0f, 152.0f, 46.0f};
                if (drawPillButton(subBtn, isOpen ? "Zamknij" : "+ Podzadanie", 21.0f,
                                   Color{255, 243, 222, 255}, Color{252, 230, 194, 255},
                                   Color{235, 198, 138, 255}, Color{165, 92, 12, 255}, mouseInList))
                {
                    if (isOpen)
                    {
                        closeSubtaskEditor = true;
                    }
                    else
                    {
                        openSubtaskEditorForId = task->getId();
                    }
                }
                rightCursor = subBtn.x - 12.0f;
            }

            // Lewa strona karty: badge typu + tytul + tag + opis
            BadgeStyle style = getTaskTypeStyle(task);
            Vector2 badgeSz = MeasureTextEx(uiFont, style.label, 20.0f, 1.0f);
            Rectangle badgeRec = {startX + 18.0f, currentY + 18.0f, badgeSz.x + 22.0f, 32.0f};
            DrawRectangleRounded(badgeRec, 0.35f, 6, style.bg);
            drawUiText(style.label, badgeRec.x + 11.0f, badgeRec.y + 6.0f, 20.0f, style.text);

            float titleX = badgeRec.x + badgeRec.width + 14.0f;
            string rawTitle = "#" + to_string(task->getId()) + "  " + task->getTitle();
            string tagText = "#" + task->getTag();
            Vector2 tagSz = MeasureTextEx(uiFont, tagText.c_str(), 20.0f, 1.0f);
            float maxTitleW = max(80.0f, rightCursor - titleX - tagSz.x - 36.0f);
            string shownTitle = fitText(rawTitle, 28.0f, maxTitleW);
            drawUiText(shownTitle.c_str(), titleX, currentY + 18.0f, 28.0f, Color{28, 28, 28, 255});

            Vector2 shownTitleSz = MeasureTextEx(uiFont, shownTitle.c_str(), 28.0f, 1.0f);
            float tagX = titleX + shownTitleSz.x + 14.0f;
            if (tagX + tagSz.x + 20.0f < rightCursor)
            {
                Rectangle tagRec = {tagX, currentY + 19.0f, tagSz.x + 18.0f, 30.0f};
                DrawRectangleRounded(tagRec, 0.35f, 6, Color{240, 240, 238, 255});
                drawUiText(tagText.c_str(), tagRec.x + 9.0f, tagRec.y + 5.0f, 20.0f, Color{105, 105, 105, 255});
            }

            string shownSummary = fitText(task->getSummary(), 22.0f, max(100.0f, rightCursor - (startX + 20.0f)));
            drawUiText(shownSummary.c_str(), startX + 20.0f, currentY + 70.0f, 22.0f, Color{105, 105, 105, 255});

            // Interaktywne podzadania dla checklisty (z zawijaniem wierszy)
            float nextBlockY = currentY + 118.0f;
            if (!subtasksCache[i].empty())
            {
                float sx = startX + 20.0f;
                float sy = nextBlockY;
                for (size_t s = 0; s < subtasksCache[i].size(); s++)
                {
                    bool done = subtasksCache[i][s].second;
                    string pillText = string(done ? "[x] " : "[ ] ") + subtasksCache[i][s].first;
                    Vector2 psz = MeasureTextEx(uiFont, pillText.c_str(), 20.0f, 1.0f);
                    float pw = min(psz.x + 22.0f, 260.0f);
                    if (sx > startX + 20.0f && sx + pw > startX + width - 20.0f)
                    {
                        sx = startX + 20.0f;
                        sy += 44.0f;
                    }
                    string shownPill = fitText(pillText, 20.0f, pw - 16.0f);

                    Color pBg = done ? Color{230, 246, 236, 255} : Color{245, 245, 242, 255};
                    Color pHover = done ? Color{214, 238, 222, 255} : Color{234, 234, 230, 255};
                    Color pBorder = done ? Color{176, 222, 192, 255} : Color{216, 216, 212, 255};
                    Color pTxt = done ? Color{32, 115, 60, 255} : Color{75, 75, 75, 255};

                    if (drawPillButton(Rectangle{sx, sy, pw, 36.0f}, shownPill.c_str(), 20.0f,
                                       pBg, pHover, pBorder, pTxt, mouseInList))
                    {
                        toggleChecklistTaskId = task->getId();
                        toggleSubtaskIdx = (int)s;
                    }
                    sx += pw + 10.0f;
                }
                nextBlockY = sy + 44.0f;
            }

            // Rozwijane pole dodawania nowego podzadania
            if (isChecklist && addingSubtaskTaskId == task->getId())
            {
                float inputY = nextBlockY;
                float addBtnW = 110.0f;
                float inputW = max(140.0f, width - 40.0f - addBtnW - 10.0f);

                bool enterPressed = (activeInput == 20 && IsKeyPressed(KEY_ENTER));
                drawInputField(Rectangle{startX + 20.0f, inputY, inputW, 42.0f},
                               newSubtaskBuf, sizeof(newSubtaskBuf), 20, activeInput,
                               "Wpisz nazwę nowego podzadania i wciśnij Enter...");

                bool addClicked = drawPillButton(Rectangle{startX + 20.0f + inputW + 10.0f, inputY, addBtnW, 42.0f},
                                                 "Dodaj", 21.0f,
                                                 Color{42, 42, 42, 255}, Color{65, 65, 65, 255},
                                                 Color{30, 30, 30, 255}, Color{255, 255, 255, 255}, mouseInList);
                if (enterPressed || addClicked)
                {
                    addSubtaskTargetId = task->getId();
                }
            }
        }

        currentY += cardH + cardGap;
    }

    EndScissorMode();

    // Wykonanie odlozonych akcji
    if (closeSubtaskEditor)
    {
        addingSubtaskTaskId = -1;
        if (activeInput == 20)
            activeInput = 0;
    }

    if (openSubtaskEditorForId != -1)
    {
        addingSubtaskTaskId = openSubtaskEditorForId;
        memset(newSubtaskBuf, 0, sizeof(newSubtaskBuf));
        activeInput = 20;
    }

    if (addSubtaskTargetId != -1)
    {
        string cleanName = trimStr(newSubtaskBuf);
        if (cleanName.empty())
        {
            showStatus("Wpisz nazwę podzadania.", true);
            activeInput = 20;
        }
        else
        {
            for (Task *t : manager.getTasks())
            {
                if (t->getId() == addSubtaskTargetId)
                {
                    ChecklistTask *cl = dynamic_cast<ChecklistTask *>(t);
                    if (cl != nullptr)
                    {
                        cl->addSubtask(cleanName, false);
                        if (isDoneStatus(cl->getStatus()))
                        {
                            cl->setStatus("W trakcie");
                        }
                        showStatus("Dodano podzadanie '" + cleanName + "' do checklisty #" + to_string(addSubtaskTargetId) + ".");
                        memset(newSubtaskBuf, 0, sizeof(newSubtaskBuf));
                        activeInput = 20;
                    }
                    break;
                }
            }
        }
    }

    if (statusTaskId != -1)
    {
        try
        {
            manager.changeStatus(statusTaskId, statusNextVal);
            showStatus("Zmieniono status zadania #" + to_string(statusTaskId) + " na: " + statusNextVal);
        }
        catch (const exception &e)
        {
            showStatus(string("Błąd: ") + e.what(), true);
        }
    }

    if (habitTaskId != -1)
    {
        for (Task *t : manager.getTasks())
        {
            if (t->getId() == habitTaskId)
            {
                HabitTask *habit = dynamic_cast<HabitTask *>(t);
                if (habit != nullptr)
                {
                    vector<string> fields = splitStr(habit->serialize(), '|');
                    int streak = (fields.size() >= 7) ? stoi(fields[6]) : 0;
                    habit->setCurrentStreak(streak + 1);
                    habit->setStatus("Wykonane");
                    showStatus("Nawyk #" + to_string(habitTaskId) + " wykonany! Nowy streak: " + to_string(streak + 1));
                }
                break;
            }
        }
    }

    if (postponeTaskId != -1)
    {
        for (Task *t : manager.getTasks())
        {
            if (t->getId() == postponeTaskId)
            {
                vector<string> fields = splitStr(t->serialize(), '|');
                if (fields.size() >= 7)
                {
                    int id = t->getId();
                    string title = t->getTitle();
                    string tag = t->getTag();
                    string status = t->getStatus();
                    string newDate = addDaysToDate(fields[5], 1);
                    int priority = stoi(fields[6]);

                    try
                    {
                        manager.removeTask(id);
                        DeadlineTask *updated = new DeadlineTask(id, title, tag, newDate, priority);
                        updated->setStatus(status);
                        manager.addTask(updated);
                        showStatus("Przesunięto termin zadania #" + to_string(id) + " na: " + newDate);
                    }
                    catch (const exception &e)
                    {
                        showStatus(string("Błąd przesuwania terminu: ") + e.what(), true);
                    }
                }
                break;
            }
        }
    }

    if (toggleChecklistTaskId != -1 && toggleSubtaskIdx != -1)
    {
        for (Task *t : manager.getTasks())
        {
            if (t->getId() == toggleChecklistTaskId)
            {
                vector<string> fields = splitStr(t->serialize(), '|');
                vector<pair<string, bool>> subs = parseSubtasksFromTask(t);
                if (fields.size() >= 6 && toggleSubtaskIdx < (int)subs.size())
                {
                    subs[toggleSubtaskIdx].second = !subs[toggleSubtaskIdx].second;

                    int id = t->getId();
                    string title = t->getTitle();
                    string tag = t->getTag();
                    int priority = stoi(fields[5]);

                    int doneCnt = 0;
                    for (const auto &s : subs)
                    {
                        if (s.second)
                            doneCnt++;
                    }
                    string newStatus = t->getStatus();
                    if (doneCnt == (int)subs.size() && !subs.empty())
                    {
                        newStatus = "Wykonane";
                    }
                    else if (doneCnt > 0)
                    {
                        newStatus = "W trakcie";
                    }
                    else
                    {
                        newStatus = "Do zrobienia";
                    }

                    try
                    {
                        manager.removeTask(id);
                        ChecklistTask *updated = new ChecklistTask(id, title, tag, priority);
                        for (const auto &s : subs)
                        {
                            updated->addSubtask(s.first, s.second);
                        }
                        updated->setStatus(newStatus);
                        manager.addTask(updated);
                        showStatus("Zaktualizowano podzadanie w checkliście #" + to_string(id) + ".");
                    }
                    catch (const exception &e)
                    {
                        showStatus(string("Błąd checklisty: ") + e.what(), true);
                    }
                }
                break;
            }
        }
    }

    if (deleteTaskId != -1)
    {
        if (deleteTaskId == addingSubtaskTaskId)
        {
            addingSubtaskTaskId = -1;
            activeInput = 0;
        }
        try
        {
            manager.removeTask(deleteTaskId);
            showStatus("Usunięto zadanie #" + to_string(deleteTaskId) + ".");
        }
        catch (const exception &e)
        {
            showStatus(string("Błąd: ") + e.what(), true);
        }
    }
}

void GuiApp::drawBoardView(float startX, float startY, float width, float height)
{
    const float colGap = 20.0f;
    const float colW = (width - 2.0f * colGap) / 3.0f;

    const char *colTitles[3] = {"Do zrobienia", "W trakcie", "Wykonane"};
    Color colHeaderBg[3] = {
        Color{241, 241, 238, 255},
        Color{230, 241, 255, 255},
        Color{228, 245, 234, 255}};
    Color colHeaderText[3] = {
        Color{65, 65, 65, 255},
        Color{28, 92, 175, 255},
        Color{30, 115, 60, 255}};

    vector<Task *> columns[3];
    for (Task *t : manager.getTasks())
    {
        if (isDoneStatus(t->getStatus()))
        {
            columns[2].push_back(t);
        }
        else if (t->getStatus() == "W trakcie")
        {
            columns[1].push_back(t);
        }
        else
        {
            columns[0].push_back(t);
        }
    }

    int moveTaskId = -1;
    string moveNewStatus = "";
    int deleteTaskId = -1;
    Vector2 mouse = GetMousePosition();

    for (int col = 0; col < 3; col++)
    {
        float colX = startX + (float)col * (colW + colGap);
        Rectangle colRect = {colX, startY, colW, height};

        drawCardBox(colRect, 0.04f, Color{249, 249, 247, 255}, Color{230, 230, 226, 255});

        // Naglowek kolumny
        Rectangle headerRect = {colX + 12.0f, startY + 12.0f, colW - 24.0f, 48.0f};
        DrawRectangleRounded(headerRect, 0.25f, 6, colHeaderBg[col]);
        string hdrText = string(colTitles[col]) + " (" + to_string(columns[col].size()) + ")";
        drawUiText(hdrText.c_str(), headerRect.x + 14.0f, headerRect.y + 11.0f, 24.0f, colHeaderText[col]);

        // Obszar kart kolumny
        float cardsTop = startY + 72.0f;
        float cardsH = max(80.0f, height - 84.0f);
        Rectangle scrollArea = {colX + 10.0f, cardsTop, colW - 20.0f, cardsH};

        const float cardH = 166.0f;
        const float cardGap = 12.0f;
        float totalH = (float)columns[col].size() * (cardH + cardGap);
        float maxScroll = max(0.0f, totalH - cardsH + 8.0f);

        bool mouseInCol = CheckCollisionPointRec(mouse, scrollArea);
        if (mouseInCol)
        {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f)
            {
                boardScrollY[col] -= wheel * 44.0f;
            }
        }
        boardScrollY[col] = max(0.0f, min(boardScrollY[col], maxScroll));

        BeginScissorMode((int)scrollArea.x, (int)scrollArea.y,
                         (int)max(1.0f, scrollArea.width), (int)max(1.0f, scrollArea.height));

        if (columns[col].empty())
        {
            drawUiText("Brak zadań", colX + 24.0f, cardsTop + 20.0f, 22.0f, Color{155, 155, 155, 255});
        }

        for (size_t i = 0; i < columns[col].size(); i++)
        {
            Task *task = columns[col][i];
            float cy = cardsTop + (float)i * (cardH + cardGap) - boardScrollY[col];

            if (cy + cardH < cardsTop || cy > cardsTop + cardsH)
            {
                continue;
            }

            Rectangle cardRect = {colX + 12.0f, cy, colW - 24.0f, cardH};
            drawCardBox(cardRect, 0.12f, Color{255, 255, 255, 255}, Color{224, 224, 220, 255});

            // Typ i tag
            BadgeStyle style = getTaskTypeStyle(task);
            Vector2 bSz = MeasureTextEx(uiFont, style.label, 18.0f, 1.0f);
            Rectangle bRec = {cardRect.x + 12.0f, cardRect.y + 12.0f, bSz.x + 18.0f, 28.0f};
            DrawRectangleRounded(bRec, 0.35f, 6, style.bg);
            drawUiText(style.label, bRec.x + 9.0f, bRec.y + 5.0f, 18.0f, style.text);

            string tagTxt = fitText("#" + task->getTag(), 18.0f, cardRect.width - bRec.width - 36.0f);
            drawUiText(tagTxt.c_str(), bRec.x + bRec.width + 10.0f, cardRect.y + 16.0f, 18.0f, Color{120, 120, 120, 255});

            // Tytul i skrot
            string titleTxt = fitText("#" + to_string(task->getId()) + " " + task->getTitle(),
                                      24.0f, cardRect.width - 24.0f);
            drawUiText(titleTxt.c_str(), cardRect.x + 12.0f, cardRect.y + 48.0f, 24.0f, Color{30, 30, 30, 255});

            string sumTxt = fitText(task->getSummary(), 19.0f, cardRect.width - 24.0f);
            drawUiText(sumTxt.c_str(), cardRect.x + 12.0f, cardRect.y + 82.0f, 19.0f, Color{110, 110, 110, 255});

            // Przyciski przenoszenia i usuwania na dole karty
            float btnY = cardRect.y + cardH - 46.0f;
            float bx = cardRect.x + 12.0f;

            if (col > 0)
            {
                if (drawPillButton(Rectangle{bx, btnY, 54.0f, 34.0f}, "<-", 20.0f,
                                   Color{242, 242, 239, 255}, Color{226, 226, 222, 255},
                                   Color{208, 208, 204, 255}, Color{45, 45, 45, 255}, mouseInCol))
                {
                    moveTaskId = task->getId();
                    moveNewStatus = colTitles[col - 1];
                }
                bx += 62.0f;
            }
            if (col < 2)
            {
                if (drawPillButton(Rectangle{bx, btnY, 54.0f, 34.0f}, "->", 20.0f,
                                   Color{242, 242, 239, 255}, Color{226, 226, 222, 255},
                                   Color{208, 208, 204, 255}, Color{45, 45, 45, 255}, mouseInCol))
                {
                    moveTaskId = task->getId();
                    moveNewStatus = colTitles[col + 1];
                }
            }

            Rectangle delBtn = {cardRect.x + cardRect.width - 82.0f, btnY, 70.0f, 34.0f};
            if (drawPillButton(delBtn, "Usuń", 19.0f,
                               Color{254, 240, 240, 255}, Color{252, 220, 220, 255},
                               Color{240, 195, 195, 255}, Color{185, 45, 45, 255}, mouseInCol))
            {
                deleteTaskId = task->getId();
            }
        }

        EndScissorMode();
    }

    if (moveTaskId != -1)
    {
        try
        {
            manager.changeStatus(moveTaskId, moveNewStatus);
            showStatus("Przeniesiono zadanie #" + to_string(moveTaskId) + " do kolumny: " + moveNewStatus);
        }
        catch (const exception &e)
        {
            showStatus(string("Błąd: ") + e.what(), true);
        }
    }

    if (deleteTaskId != -1)
    {
        try
        {
            manager.removeTask(deleteTaskId);
            showStatus("Usunięto zadanie #" + to_string(deleteTaskId) + ".");
        }
        catch (const exception &e)
        {
            showStatus(string("Błąd: ") + e.what(), true);
        }
    }
}

void GuiApp::drawStatsView(float startX, float startY, float width, float height)
{
    (void)height;
    const vector<Task *> &tasks = manager.getTasks();

    int total = (int)tasks.size();
    int todo = 0, inProgress = 0, done = 0;
    int typeCounts[4] = {0, 0, 0, 0}; // Termin, Nawyk, Wydarzenie, Checklista
    int totalStreak = 0, maxStreak = 0;
    int totalSubtasks = 0, doneSubtasks = 0;

    for (const Task *t : tasks)
    {
        if (isDoneStatus(t->getStatus()))
        {
            done++;
        }
        else if (t->getStatus() == "W trakcie")
        {
            inProgress++;
        }
        else
        {
            todo++;
        }

        int idx = getTaskTypeIndex(t);
        if (idx >= 1 && idx <= 4)
        {
            typeCounts[idx - 1]++;
        }

        if (idx == 2)
        {
            vector<string> fields = splitStr(t->serialize(), '|');
            if (fields.size() >= 7)
            {
                int s = stoi(fields[6]);
                totalStreak += s;
                maxStreak = max(maxStreak, s);
            }
        }
        else if (idx == 4)
        {
            vector<pair<string, bool>> subs = parseSubtasksFromTask(t);
            totalSubtasks += (int)subs.size();
            for (const auto &sub : subs)
            {
                if (sub.second)
                    doneSubtasks++;
            }
        }
    }

    // 1. Cztery kafelki KPI u gory
    const float kpiGap = 18.0f;
    const float kpiW = (width - 3.0f * kpiGap) / 4.0f;
    const float kpiH = 126.0f;

    struct KpiCard
    {
        const char *label;
        string value;
        Color accent;
    };

    int pct = (total > 0) ? (done * 100 / total) : 0;
    KpiCard kpis[4] = {
        {"Wszystkie zadania", to_string(total), Color{45, 45, 45, 255}},
        {"Do zrobienia", to_string(todo), Color{110, 110, 110, 255}},
        {"W trakcie", to_string(inProgress), Color{30, 100, 190, 255}},
        {"Wykonane", to_string(done) + " (" + to_string(pct) + "%)", Color{35, 130, 65, 255}}};

    for (int i = 0; i < 4; i++)
    {
        float kx = startX + (float)i * (kpiW + kpiGap);
        Rectangle r = {kx, startY, kpiW, kpiH};
        drawCardBox(r, 0.14f, Color{252, 252, 250, 255}, Color{226, 226, 222, 255});
        drawUiText(kpis[i].label, kx + 20.0f, startY + 20.0f, 22.0f, Color{115, 115, 115, 255});
        drawUiText(kpis[i].value.c_str(), kx + 20.0f, startY + 56.0f, 40.0f, kpis[i].accent);
    }

    // 2. Pasek ogolnego postepu
    float progY = startY + kpiH + 22.0f;
    Rectangle progCard = {startX, progY, width, 118.0f};
    drawCardBox(progCard, 0.14f, Color{255, 255, 255, 255}, Color{226, 226, 222, 255});

    string progTitle = "Ogólny postęp realizacji zadań: " + to_string(pct) + "%";
    drawUiText(progTitle.c_str(), startX + 24.0f, progY + 20.0f, 25.0f, Color{35, 35, 35, 255});

    Rectangle barBg = {startX + 24.0f, progY + 64.0f, width - 48.0f, 30.0f};
    DrawRectangleRounded(barBg, 0.45f, 8, Color{238, 238, 235, 255});
    if (total > 0 && done > 0)
    {
        float fillW = max(24.0f, barBg.width * ((float)done / (float)total));
        DrawRectangleRounded(Rectangle{barBg.x, barBg.y, fillW, barBg.height}, 0.45f, 8, Color{46, 150, 82, 255});
    }

    // 3. Dolna sekcja: podzial wg typu + szczegoly nawykow i checklist
    float bottomY = progY + 140.0f;
    float halfW = (width - 22.0f) * 0.5f;
    float panelH = 290.0f;

    // Lewy panel: typy zadan
    Rectangle leftPanel = {startX, bottomY, halfW, panelH};
    drawCardBox(leftPanel, 0.08f, Color{255, 255, 255, 255}, Color{226, 226, 222, 255});
    drawUiText("Podział według typu", leftPanel.x + 24.0f, leftPanel.y + 20.0f, 26.0f, Color{35, 35, 35, 255});

    const char *typeNames[4] = {"Termin", "Nawyk", "Wydarzenie", "Checklista"};
    Color typeColors[4] = {
        Color{215, 75, 75, 255},
        Color{135, 80, 205, 255},
        Color{55, 130, 215, 255},
        Color{215, 140, 45, 255}};

    for (int i = 0; i < 4; i++)
    {
        float ry = leftPanel.y + 72.0f + (float)i * 52.0f;
        drawUiText(typeNames[i], leftPanel.x + 24.0f, ry + 4.0f, 22.0f, Color{65, 65, 65, 255});

        float bx = leftPanel.x + 165.0f;
        float bw = max(80.0f, halfW - 245.0f);
        DrawRectangleRounded(Rectangle{bx, ry + 6.0f, bw, 22.0f}, 0.45f, 6, Color{240, 240, 237, 255});
        if (total > 0 && typeCounts[i] > 0)
        {
            float fw = max(16.0f, bw * ((float)typeCounts[i] / (float)total));
            DrawRectangleRounded(Rectangle{bx, ry + 6.0f, fw, 22.0f}, 0.45f, 6, typeColors[i]);
        }

        string cTxt = to_string(typeCounts[i]);
        drawUiText(cTxt.c_str(), bx + bw + 16.0f, ry + 4.0f, 22.0f, Color{50, 50, 50, 255});
    }

    // Prawy panel: nawyki i checklisty
    Rectangle rightPanel = {startX + halfW + 22.0f, bottomY, halfW, panelH};
    drawCardBox(rightPanel, 0.08f, Color{255, 255, 255, 255}, Color{226, 226, 222, 255});
    drawUiText("Nawyki i checklisty", rightPanel.x + 24.0f, rightPanel.y + 20.0f, 26.0f, Color{35, 35, 35, 255});

    string h1 = "Aktywne nawyki: " + to_string(typeCounts[1]);
    string h2 = "Najdłuższa seria (Streak): " + to_string(maxStreak);
    string h3 = "Łączna suma serii: " + to_string(totalStreak);
    int subPct = (totalSubtasks > 0) ? (doneSubtasks * 100 / totalSubtasks) : 0;
    string c1 = "Ukończone podzadania: " + to_string(doneSubtasks) + " / " + to_string(totalSubtasks) + " (" + to_string(subPct) + "%)";

    drawUiText(h1.c_str(), rightPanel.x + 24.0f, rightPanel.y + 76.0f, 23.0f, Color{70, 70, 70, 255});
    drawUiText(h2.c_str(), rightPanel.x + 24.0f, rightPanel.y + 116.0f, 23.0f, Color{110, 55, 180, 255});
    drawUiText(h3.c_str(), rightPanel.x + 24.0f, rightPanel.y + 156.0f, 23.0f, Color{70, 70, 70, 255});
    DrawLine((int)rightPanel.x + 24, (int)rightPanel.y + 202,
             (int)(rightPanel.x + rightPanel.width - 24), (int)rightPanel.y + 202, Color{232, 232, 228, 255});
    drawUiText(c1.c_str(), rightPanel.x + 24.0f, rightPanel.y + 224.0f, 23.0f, Color{175, 100, 15, 255});
}

void GuiApp::drawAddTaskView(float startX, float startY, float width, float height)
{
    (void)height;
    float formW = min(width, 780.0f);

    drawCardBox(Rectangle{startX, startY, formW, 610.0f}, 0.04f,
                Color{252, 252, 250, 255}, Color{226, 226, 222, 255});

    float x = startX + 28.0f;
    float y = startY + 24.0f;
    float innerW = formW - 56.0f;

    // 1. Wybor typu zadania
    drawUiText("Typ zadania:", x, y, 24.0f, Color{60, 60, 60, 255});
    y += 34.0f;

    const char *types[4] = {"Termin", "Nawyk", "Wydarzenie", "Checklista"};
    float tabW = (innerW - 30.0f) / 4.0f;
    for (int i = 0; i < 4; i++)
    {
        bool selected = (formType == i);
        Color bg = selected ? Color{42, 42, 42, 255} : Color{240, 240, 237, 255};
        Color hoverBg = selected ? Color{60, 60, 60, 255} : Color{228, 228, 224, 255};
        Color border = selected ? Color{30, 30, 30, 255} : Color{212, 212, 208, 255};
        Color txt = selected ? Color{255, 255, 255, 255} : Color{45, 45, 45, 255};

        if (drawPillButton(Rectangle{x + (float)i * (tabW + 10.0f), y, tabW, 44.0f},
                           types[i], 23.0f, bg, hoverBg, border, txt, true))
        {
            formType = i;
            activeInput = 0;
        }
    }
    y += 64.0f;

    // 2. Wspolne pola: Tytul i Tag
    float halfW = (innerW - 18.0f) * 0.5f;
    drawUiText("Tytuł zadania *", x, y, 22.0f, Color{70, 70, 70, 255});
    drawUiText("Tag (np. studia, dom)", x + halfW + 18.0f, y, 22.0f, Color{70, 70, 70, 255});
    y += 30.0f;

    drawInputField(Rectangle{x, y, halfW, 46.0f}, formTitle, sizeof(formTitle), 10, activeInput, "Wpisz tytuł...");
    drawInputField(Rectangle{x + halfW + 18.0f, y, halfW, 46.0f}, formTag, sizeof(formTag), 11, activeInput, "ogolne");
    y += 66.0f;

    auto drawPrioritySelector = [&](float py)
    {
        drawUiText("Priorytet:", x, py, 22.0f, Color{70, 70, 70, 255});
        py += 30.0f;
        const char *pLabels[3] = {"1 - Niski", "2 - Średni", "3 - Wysoki"};
        float pw = (innerW - 20.0f) / 3.0f;
        for (int p = 1; p <= 3; p++)
        {
            bool sel = (formPriority == p);
            Color bg = sel ? Color{226, 240, 255, 255} : Color{243, 243, 240, 255};
            Color hover = sel ? Color{210, 232, 255, 255} : Color{232, 232, 228, 255};
            Color border = sel ? Color{110, 165, 235, 255} : Color{214, 214, 210, 255};
            Color txt = sel ? Color{25, 92, 175, 255} : Color{55, 55, 55, 255};

            if (drawPillButton(Rectangle{x + (float)(p - 1) * (pw + 10.0f), py, pw, 44.0f},
                               pLabels[p - 1], 22.0f, bg, hover, border, txt, true))
            {
                formPriority = p;
            }
        }
        return py + 64.0f;
    };

    // 3. Pola specyficzne dla wybranego typu
    if (formType == 0)
    { // Termin
        drawUiText("Termin (RRRR-MM-DD):", x, y, 22.0f, Color{70, 70, 70, 255});
        y += 30.0f;
        drawInputField(Rectangle{x, y, halfW, 46.0f}, formDate, sizeof(formDate), 12, activeInput, "2026-11-30");
        y += 66.0f;
        y = drawPrioritySelector(y);
    }
    else if (formType == 1)
    { // Nawyk
        drawUiText("Co ile dni wykonywać nawyk:", x, y, 22.0f, Color{70, 70, 70, 255});
        y += 30.0f;
        drawInputField(Rectangle{x, y, halfW, 46.0f}, formInterval, sizeof(formInterval), 13, activeInput, "1");
        y += 66.0f;
        y = drawPrioritySelector(y);
    }
    else if (formType == 2)
    { // Wydarzenie
        float thirdW = (innerW - 24.0f) / 3.0f;
        drawUiText("Data (RRRR-MM-DD):", x, y, 22.0f, Color{70, 70, 70, 255});
        drawUiText("Od (GG:MM):", x + thirdW + 12.0f, y, 22.0f, Color{70, 70, 70, 255});
        drawUiText("Do (GG:MM):", x + 2.0f * (thirdW + 12.0f), y, 22.0f, Color{70, 70, 70, 255});
        y += 30.0f;

        drawInputField(Rectangle{x, y, thirdW, 46.0f}, formDate, sizeof(formDate), 12, activeInput, "2026-11-30");
        drawInputField(Rectangle{x + thirdW + 12.0f, y, thirdW, 46.0f}, formStartTime, sizeof(formStartTime), 14, activeInput, "10:00");
        drawInputField(Rectangle{x + 2.0f * (thirdW + 12.0f), y, thirdW, 46.0f}, formEndTime, sizeof(formEndTime), 15, activeInput, "11:00");
        y += 66.0f;

        drawUiText("Miejsce:", x, y, 22.0f, Color{70, 70, 70, 255});
        y += 30.0f;
        drawInputField(Rectangle{x, y, innerW, 46.0f}, formLocation, sizeof(formLocation), 16, activeInput, "np. Uczelnia, sala 217A");
        y += 66.0f;
    }
    else if (formType == 3)
    { // Checklista
        y = drawPrioritySelector(y);
        drawUiText("Podzadania (oddziel średnikiem ';'):", x, y, 22.0f, Color{70, 70, 70, 255});
        y += 30.0f;
        drawInputField(Rectangle{x, y, innerW, 46.0f}, formSubtasks, sizeof(formSubtasks), 17, activeInput, "np. punkt 1; punkt 2; punkt 3");
        y += 66.0f;
    }

    // 4. Przyciski zatwierdzenia i anulowania
    float actionY = startY + 530.0f;
    if (drawPillButton(Rectangle{x, actionY, 240.0f, 52.0f}, "Dodaj zadanie", 25.0f,
                       Color{42, 42, 42, 255}, Color{65, 65, 65, 255},
                       Color{30, 30, 30, 255}, Color{255, 255, 255, 255}, true))
    {
        string title = trimStr(formTitle);
        if (title.empty())
        {
            showStatus("Podaj tytuł zadania przed zapisaniem.", true);
            return;
        }

        string tag = trimStr(formTag);
        if (tag.empty())
        {
            tag = "ogolne";
        }

        int newId = getNextTaskId();
        Task *created = nullptr;

        try
        {
            if (formType == 0)
            {
                string due = trimStr(formDate);
                if (due.empty())
                    due = "2026-11-30";
                created = new DeadlineTask(newId, title, tag, due, formPriority);
            }
            else if (formType == 1)
            {
                int interval = 1;
                string intStr = trimStr(formInterval);
                if (!intStr.empty())
                {
                    interval = max(1, stoi(intStr));
                }
                created = new HabitTask(newId, title, tag, interval, formPriority);
            }
            else if (formType == 2)
            {
                string evDate = trimStr(formDate);
                string sTime = trimStr(formStartTime);
                string eTime = trimStr(formEndTime);
                string loc = trimStr(formLocation);
                if (evDate.empty())
                    evDate = "2026-11-30";
                if (sTime.empty())
                    sTime = "10:00";
                if (eTime.empty())
                    eTime = "11:00";
                if (loc.empty())
                    loc = "Brak";
                created = new EventTask(newId, title, tag, evDate, sTime, eTime, loc);
            }
            else if (formType == 3)
            {
                ChecklistTask *cl = new ChecklistTask(newId, title, tag, formPriority);
                vector<string> parts = splitStr(formSubtasks, ';');
                for (const string &part : parts)
                {
                    string clean = trimStr(part);
                    if (!clean.empty())
                    {
                        cl->addSubtask(clean, false);
                    }
                }
                created = cl;
            }

            if (created != nullptr)
            {
                manager.addTask(created);
                showStatus("Dodano nowe zadanie #" + to_string(newId) + ": " + title);
                resetAddForm();
                currentView = AppView::LIST;
            }
        }
        catch (const exception &e)
        {
            delete created;
            showStatus(string("Błąd danych formularza: ") + e.what(), true);
        }
    }

    if (drawPillButton(Rectangle{x + 256.0f, actionY, 150.0f, 52.0f}, "Anuluj", 24.0f,
                       Color{242, 242, 239, 255}, Color{228, 228, 224, 255},
                       Color{210, 210, 206, 255}, Color{55, 55, 55, 255}, true))
    {
        resetAddForm();
        currentView = AppView::LIST;
    }
}