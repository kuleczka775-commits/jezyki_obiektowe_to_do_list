#include "GuiApp.h"
#include "Exceptions.h"

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
        // Kody 32..383 pokrywaja ASCII oraz polskie znaki (a, c, e, l, n, o, s, z, z)
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
}

GuiApp::GuiApp(TaskManager &manager, FileManager &fileManager, int width, int height)
    : manager(manager),
      fileManager(fileManager),
      screenWidth(width),
      screenHeight(height),
      currentView(AppView::LIST),
      statusMessage(""),
      statusTimer(0.0f) {}

void GuiApp::showStatus(const string &msg)
{
    statusMessage = msg;
    statusTimer = 3.0f;
}

void GuiApp::run()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Osobisty Menedżer Zadań");
    SetWindowMinSize(1024, 650);
    SetTargetFPS(60);

    loadUiFont();

    // Wieksza czcionka i styl przyciskow w raygui
    GuiSetFont(uiFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 28);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x262626ff);
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0xeeeeecff);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xd8d8d4ff);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0xe2e2deff);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0xb8b8b2ff);
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, 0x111111ff);

    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

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

    if (customFontLoaded)
    {
        UnloadFont(uiFont);
    }
    CloseWindow();
}

void GuiApp::drawSidebar()
{
    const float sidebarWidth = 350.0f;

    DrawRectangle(0, 0, (int)sidebarWidth, screenHeight, Color{247, 247, 245, 255});
    DrawLine((int)sidebarWidth, 0, (int)sidebarWidth, screenHeight, Color{225, 225, 222, 255});

    drawUiText("Menedżer Zadań", 24, 28, 38, Color{35, 35, 35, 255});
    drawUiText("Workspace", 24, 74, 22, Color{130, 130, 130, 255});

    if (GuiButton(Rectangle{20.0f, 130.0f, sidebarWidth - 40.0f, 58.0f}, "Wszystkie zadania"))
    {
        currentView = AppView::LIST;
    }
    if (GuiButton(Rectangle{20.0f, 202.0f, sidebarWidth - 40.0f, 58.0f}, "Widok tablicy"))
    {
        currentView = AppView::BOARD;
    }
    if (GuiButton(Rectangle{20.0f, 274.0f, sidebarWidth - 40.0f, 58.0f}, "Statystyki"))
    {
        currentView = AppView::STATS;
    }

    float saveBtnY = (float)screenHeight - 84.0f;
    if (GuiButton(Rectangle{20.0f, saveBtnY, sidebarWidth - 40.0f, 58.0f}, "Zapisz do pliku"))
    {
        try
        {
            fileManager.saveToFile(manager);
            showStatus("Zapisano zadania do pliku.");
        }
        catch (const FileException &e)
        {
            showStatus(string("Błąd zapisu: ") + e.what());
        }
    }
}

void GuiApp::drawMainContent()
{
    const float startX = 395.0f;
    const float y = 28.0f;

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

    drawUiText(title, startX, y, 48, Color{30, 30, 30, 255});
    DrawLine((int)startX, (int)y + 66, screenWidth - 45, (int)y + 66, Color{230, 230, 230, 255});

    string countInfo = "Załadowane zadania w bazie: " + to_string(manager.getTasks().size());
    drawUiText(countInfo.c_str(), startX, y + 92, 28, Color{90, 90, 90, 255});

    if (statusTimer > 0.0f)
    {
        drawUiText(statusMessage.c_str(), startX, (float)screenHeight - 56.0f, 24, Color{46, 125, 50, 255});
    }
}