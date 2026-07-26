#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <fstream>

// configuration

const int WINDOW_W = 920;
const int WINDOW_H = 720;
const int BOARD_X = 30;
const int BOARD_Y = 100;
const int BOARD_W = 860;
const int BOARD_H = 570;
const int TASK_W = 150;
const int TASK_H = 80;
const int TASK_PAD = 12;
const int COLS = 5;
const int ROWS = 6;
const int MAX_TASKS = COLS * ROWS;
const sf::Color COLOR_BG(245, 230, 200);
const sf::Color COLOR_BORDER(139, 115, 85);
const sf::Color COLOR_BORDER_LIGHT(210, 190, 165);
const sf::Color COLOR_RED(200, 50, 50);
const sf::Color COLOR_BLUE(60, 120, 220);
const sf::Color COLOR_TASK_BG(255, 248, 235);
const sf::Color COLOR_TEXT(40, 30, 20);
const sf::Color COLOR_PLUS_BG(180, 200, 160);
const sf::Color COLOR_PLUS_HOVER(200, 220, 180);

// task struct

struct Task {
    std::string text;
    sf::Color color;
    std::time_t created;
    bool done;

    Task(const std::string& t = "new task", bool isDone = false)
        : text(t), color(isDone ? COLOR_BLUE : COLOR_RED), created(std::time(nullptr)), done(isDone) {
    }
};



std::vector<Task> tasks;
int editingIndex = -1;
std::string editBuffer;
bool showEditCursor = true;
sf::Clock cursorClock;
sf::Clock weekClock;
bool mouseOnPlus = false;
sf::Font font;



void addTask(const std::string& text = "new task") {
    tasks.push_back(Task(text, false));
}


// functions persisting

void saveTasks() {
    std::ofstream file("tasks.txt");
    if (!file.is_open()) return;

    for (const auto& task : tasks) {
        std::string colorStr = (task.color == COLOR_RED) ? "red" : "blue";
        file << task.text << "|"
            << colorStr << "|"
            << task.created << "|"
            << (task.done ? "1" : "0") << "\n";
    }
    file.close();
}

void loadTasks() {
    std::ifstream file("tasks.txt");
    if (!file.is_open()) {
        
        tasks.clear();
        saveTasks();          
        return;
    }

    // pre existing gile 
    tasks.clear();
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string text, colorStr, timeStr, doneStr;
        std::getline(ss, text, '|');
        std::getline(ss, colorStr, '|');
        std::getline(ss, timeStr, '|');
        std::getline(ss, doneStr, '|');

        Task t;
        t.text = text;
        t.color = (colorStr == "red") ? COLOR_RED : COLOR_BLUE;
        t.created = std::stoll(timeStr);
        t.done = (doneStr == "1");
        tasks.push_back(t);
    }
    file.close();
}



bool isPointInRect(const sf::Vector2i& p, const sf::FloatRect& r) {
    return static_cast<float>(p.x) >= r.left && static_cast<float>(p.x) <= r.left + r.width &&
        static_cast<float>(p.y) >= r.top && static_cast<float>(p.y) <= r.top + r.height;
}

sf::FloatRect getTaskRect(int index) {
    int col = index % COLS;
    int row = index / COLS;
    float x = static_cast<float>(BOARD_X + TASK_PAD + col * (TASK_W + TASK_PAD));
    float y = static_cast<float>(BOARD_Y + TASK_PAD + row * (TASK_H + TASK_PAD));
    return sf::FloatRect(x, y, static_cast<float>(TASK_W), static_cast<float>(TASK_H));
}

sf::FloatRect getPlusRect() {
    return sf::FloatRect(static_cast<float>(BOARD_X + BOARD_W - 70),
        static_cast<float>(BOARD_Y - 50),
        50.0f, 40.0f);
}

int getTaskAt(const sf::Vector2i& mousePos) {
    for (int i = 0; i < (int)tasks.size() && i < MAX_TASKS; ++i) {
        if (isPointInRect(mousePos, getTaskRect(i)))
            return i;
    }
    return -1;
}



void cleanupOldDoneTasks() {
    const std::time_t WEEK_SECONDS = 7 * 24 * 60 * 60;
    std::time_t now = std::time(nullptr);
    auto it = tasks.begin();
    while (it != tasks.end()) {
        if (it->done && (now - it->created) > WEEK_SECONDS) {
            it = tasks.erase(it);
        }
        else {
            ++it;
        }
    }
}



void drawRetroBorder(sf::RenderWindow& window, const sf::FloatRect& rect,
    const sf::Color& dark, const sf::Color& light, float thickness = 2.0f) {
    sf::RectangleShape line;
    line.setFillColor(light);
    line.setPosition(rect.left, rect.top);
    line.setSize(sf::Vector2f(rect.width, thickness));
    window.draw(line);
    line.setPosition(rect.left, rect.top);
    line.setSize(sf::Vector2f(thickness, rect.height));
    window.draw(line);

    line.setFillColor(dark);
    line.setPosition(rect.left + rect.width - thickness, rect.top);
    line.setSize(sf::Vector2f(thickness, rect.height));
    window.draw(line);
    line.setPosition(rect.left, rect.top + rect.height - thickness);
    line.setSize(sf::Vector2f(rect.width, thickness));
    window.draw(line);

    sf::Color innerLight(230, 215, 190);
    sf::Color innerDark(120, 100, 75);
    float t2 = 1.0f;
    line.setFillColor(innerLight);
    line.setPosition(rect.left + 4.0f, rect.top + 4.0f);
    line.setSize(sf::Vector2f(rect.width - 8.0f, t2));
    window.draw(line);
    line.setPosition(rect.left + 4.0f, rect.top + 4.0f);
    line.setSize(sf::Vector2f(t2, rect.height - 8.0f));
    window.draw(line);
    line.setFillColor(innerDark);
    line.setPosition(rect.left + rect.width - 4.0f - t2, rect.top + 4.0f);
    line.setSize(sf::Vector2f(t2, rect.height - 8.0f));
    window.draw(line);
    line.setPosition(rect.left + 4.0f, rect.top + rect.height - 4.0f - t2);
    line.setSize(sf::Vector2f(rect.width - 8.0f, t2));
    window.draw(line);
}

void drawTask(sf::RenderWindow& window, const Task& task, int index, bool isEditing) {
    sf::FloatRect rect = getTaskRect(index);

    sf::RectangleShape bg(sf::Vector2f(rect.width, rect.height));
    bg.setFillColor(COLOR_TASK_BG);
    bg.setPosition(rect.left, rect.top);
    window.draw(bg);

    sf::RectangleShape status(sf::Vector2f(8.0f, rect.height));
    status.setFillColor(task.color);
    status.setPosition(rect.left, rect.top);
    window.draw(status);

    drawRetroBorder(window, rect, COLOR_BORDER, COLOR_BORDER_LIGHT, 1.5f);

    std::string displayText = task.text;
    if (displayText.length() > 18) displayText = displayText.substr(0, 16) + "..";

    sf::Text txt;
    txt.setFont(font);
    txt.setString(displayText);
    txt.setCharacterSize(14);
    txt.setFillColor(COLOR_TEXT);
    txt.setPosition(rect.left + 14.0f, rect.top + 12.0f);

    if (isEditing) {
        std::string editDisplay = editBuffer;
        if (showEditCursor) editDisplay += "_";
        txt.setString(editDisplay);
        txt.setPosition(rect.left + 14.0f, rect.top + 12.0f);
        sf::RectangleShape highlight(sf::Vector2f(rect.width - 16.0f, rect.height - 20.0f));
        highlight.setFillColor(sf::Color(200, 220, 255, 60));
        highlight.setPosition(rect.left + 8.0f, rect.top + 8.0f);
        window.draw(highlight);
    }

    window.draw(txt);

    if (task.done) {
        sf::Text doneTxt;
        doneTxt.setFont(font);
        doneTxt.setString("✓");
        doneTxt.setCharacterSize(18);
        doneTxt.setFillColor(COLOR_BLUE);
        doneTxt.setPosition(rect.left + rect.width - 28.0f, rect.top + 4.0f);
        window.draw(doneTxt);
    }
}


// main

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Retro Task Board",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(30);

    
    std::vector<std::string> fontPaths = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/Courier New.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
        "/System/Library/Fonts/Monaco.dfont",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
    };
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) { fontLoaded = true; break; }
    }
    if (!fontLoaded) {
        std::cout << "Warning: Could not load font. Please ensure a font is installed.\n";
        font.loadFromFile("arial.ttf");
    }

     
    loadTasks();

    bool leftMouseDown = false;
    bool leftMouseReleased = false;
    sf::Vector2i lastMousePos;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    leftMouseDown = true;
                    leftMouseReleased = false;
                }
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    leftMouseDown = false;
                    leftMouseReleased = true;
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (editingIndex >= 0 && editingIndex < (int)tasks.size()) {
                    if (event.key.code == sf::Keyboard::Return) {
                        if (!editBuffer.empty()) {
                            tasks[editingIndex].text = editBuffer;
                        }
                        editingIndex = -1;
                        editBuffer.clear();
                        saveTasks();
                    }
                    else if (event.key.code == sf::Keyboard::Escape) {
                        editingIndex = -1;
                        editBuffer.clear();
                    }
                    else if (event.key.code == sf::Keyboard::BackSpace) {
                        if (!editBuffer.empty())
                            editBuffer.pop_back();
                    }
                }
            }
            if (event.type == sf::Event::TextEntered) {
                if (editingIndex >= 0 && editingIndex < (int)tasks.size()) {
                    if (event.text.unicode >= 32 && event.text.unicode < 128) {
                        char c = static_cast<char>(event.text.unicode);
                        if (editBuffer.length() < 30)
                            editBuffer += c;
                    }
                }
            }
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        mouseOnPlus = isPointInRect(mousePos, getPlusRect());

        if (leftMouseReleased) {
            leftMouseReleased = false;

            if (mouseOnPlus) {
                addTask("new task");
                saveTasks();
                continue;
            }

            int clicked = getTaskAt(mousePos);
            if (clicked >= 0 && clicked < (int)tasks.size()) {
                if (editingIndex != clicked) {
                    tasks[clicked].done = !tasks[clicked].done;
                    tasks[clicked].color = tasks[clicked].done ? COLOR_BLUE : COLOR_RED;
                    if (tasks[clicked].done) {
                        tasks[clicked].created = std::time(nullptr);
                    }
                    saveTasks();
                }
            }
            else {
                if (editingIndex >= 0) {
                    if (!editBuffer.empty()) {
                        tasks[editingIndex].text = editBuffer;
                    }
                    editingIndex = -1;
                    editBuffer.clear();
                }
            }
        }

        // couble click to edit task
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            static sf::Clock doubleClickClock;
            static sf::Vector2i lastClickPos;
            static bool hadClick = false;
            if (hadClick && doubleClickClock.getElapsedTime().asMilliseconds() < 400 &&
                std::abs(mousePos.x - lastClickPos.x) < 10 && std::abs(mousePos.y - lastClickPos.y) < 10) {
                int clicked = getTaskAt(mousePos);
                if (clicked >= 0 && clicked < (int)tasks.size()) {
                    editingIndex = clicked;
                    editBuffer = "";
                }
                hadClick = false;
            }
            else {
                hadClick = true;
                lastClickPos = mousePos;
                doubleClickClock.restart();
            }
        }

        
        if (cursorClock.getElapsedTime().asMilliseconds() > 500) {
            showEditCursor = !showEditCursor;
            cursorClock.restart();
        }

        // weekly delete time
        if (weekClock.getElapsedTime().asSeconds() > 60) {
            cleanupOldDoneTasks();
            saveTasks();
            weekClock.restart();
        }

      
        window.clear(COLOR_BG);

        sf::Text title;
        title.setFont(font);
        title.setString("📋 TASK BOARD");
        title.setCharacterSize(28);
        title.setFillColor(COLOR_BORDER);
        title.setPosition(40.0f, 20.0f);
        window.draw(title);

        sf::Text sub;
        sub.setFont(font);
        sub.setString("retro · click to toggle · double-click to edit");
        sub.setCharacterSize(12);
        sub.setFillColor(sf::Color(120, 100, 75));
        sub.setPosition(42.0f, 56.0f);
        window.draw(sub);

        // + task
        sf::FloatRect plusR = getPlusRect();
        sf::RectangleShape plusBg(sf::Vector2f(plusR.width, plusR.height));
        plusBg.setFillColor(mouseOnPlus ? COLOR_PLUS_HOVER : COLOR_PLUS_BG);
        plusBg.setPosition(plusR.left, plusR.top);
        window.draw(plusBg);
        drawRetroBorder(window, plusR, COLOR_BORDER, COLOR_BORDER_LIGHT, 1.5f);
        sf::Text plusText;
        plusText.setFont(font);
        plusText.setString("+");
        plusText.setCharacterSize(30);
        plusText.setFillColor(sf::Color(50, 60, 40));
        plusText.setPosition(plusR.left + 14.0f, plusR.top + 2.0f);
        window.draw(plusText);

       
        sf::Text counter;
        counter.setFont(font);
        std::string cstr = std::to_string(tasks.size()) + " tasks";
        if ((int)tasks.size() > MAX_TASKS)
            cstr += " (stacking: " + std::to_string(tasks.size() - MAX_TASKS) + " overlapped)";
        counter.setString(cstr);
        counter.setCharacterSize(12);
        counter.setFillColor(sf::Color(100, 85, 65));
        counter.setPosition(static_cast<float>(BOARD_X + 10), static_cast<float>(BOARD_Y - 40));
        window.draw(counter);

        
        sf::FloatRect boardRect(static_cast<float>(BOARD_X), static_cast<float>(BOARD_Y),
            static_cast<float>(BOARD_W), static_cast<float>(BOARD_H));
        sf::RectangleShape board(sf::Vector2f(static_cast<float>(BOARD_W), static_cast<float>(BOARD_H)));
        board.setFillColor(sf::Color(250, 245, 235));
        board.setPosition(static_cast<float>(BOARD_X), static_cast<float>(BOARD_Y));
        window.draw(board);
        drawRetroBorder(window, boardRect, COLOR_BORDER, COLOR_BORDER_LIGHT, 3.0f);

        
        for (int r = 0; r <= ROWS; ++r) {
            sf::RectangleShape line(sf::Vector2f(static_cast<float>(BOARD_W - TASK_PAD * 2), 1.0f));
            line.setFillColor(sf::Color(200, 190, 175, 60));
            line.setPosition(static_cast<float>(BOARD_X + TASK_PAD),
                static_cast<float>(BOARD_Y + TASK_PAD + r * (TASK_H + TASK_PAD)));
            window.draw(line);
        }
        for (int c = 0; c <= COLS; ++c) {
            sf::RectangleShape line(sf::Vector2f(1.0f, static_cast<float>(BOARD_H - TASK_PAD * 2)));
            line.setFillColor(sf::Color(200, 190, 175, 60));
            line.setPosition(static_cast<float>(BOARD_X + TASK_PAD + c * (TASK_W + TASK_PAD)),
                static_cast<float>(BOARD_Y + TASK_PAD));
            window.draw(line);
        }

        
        int displayCount = std::min((int)tasks.size(), MAX_TASKS);
        for (int i = 0; i < displayCount; ++i) {
            bool isEditing = (editingIndex == i);
            drawTask(window, tasks[i], i, isEditing);
        }

        
        if ((int)tasks.size() > MAX_TASKS) {
            sf::Text stackMsg;
            stackMsg.setFont(font);
            stackMsg.setString("⚡ " + std::to_string(tasks.size() - MAX_TASKS) + " tasks stacked");
            stackMsg.setCharacterSize(11);
            stackMsg.setFillColor(sf::Color(180, 100, 60));
            stackMsg.setPosition(static_cast<float>(BOARD_X + BOARD_W - 180),
                static_cast<float>(BOARD_Y + BOARD_H - 24));
            window.draw(stackMsg);

            for (int i = MAX_TASKS; i < std::min((int)tasks.size(), MAX_TASKS + 5); ++i) {
                sf::FloatRect r = getTaskRect(i % MAX_TASKS);
                sf::RectangleShape ov(sf::Vector2f(r.width - 6.0f, r.height - 6.0f));
                ov.setFillColor(sf::Color(180, 160, 140, 40));
                ov.setPosition(r.left + 3.0f + (i - MAX_TASKS) * 2.0f,
                    r.top + 3.0f + (i - MAX_TASKS) * 2.0f);
                window.draw(ov);
            }
        }


        if (editingIndex >= 0) {
            sf::Text help;
            help.setFont(font);
            help.setString("✎ editing: type text · ENTER to confirm · ESC to cancel");
            help.setCharacterSize(12);
            help.setFillColor(sf::Color(80, 70, 50));
            help.setPosition(static_cast<float>(BOARD_X + 20),
                static_cast<float>(BOARD_Y + BOARD_H + 14));
            window.draw(help);
        }
        else {
            sf::Text help;
            help.setFont(font);
            help.setString("click task to toggle done/not done · double-click to edit text");
            help.setCharacterSize(12);
            help.setFillColor(sf::Color(140, 125, 105));
            help.setPosition(static_cast<float>(BOARD_X + 20),
                static_cast<float>(BOARD_Y + BOARD_H + 14));
            window.draw(help);
        }

        // sattus bar
        sf::RectangleShape statusBar(sf::Vector2f(static_cast<float>(WINDOW_W), 22.0f));
        statusBar.setFillColor(sf::Color(220, 205, 180));
        statusBar.setPosition(0.0f, static_cast<float>(WINDOW_H - 22));
        window.draw(statusBar);

        std::time_t now = std::time(nullptr);
        struct tm timeInfo;
        localtime_s(&timeInfo, &now);
        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", &timeInfo);
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setString("  " + std::string(timeBuf) + "  |  tasks: " + std::to_string(tasks.size()) +
            "  |  done: " + std::to_string(std::count_if(tasks.begin(), tasks.end(),
                [](const Task& t) { return t.done; })));
        statusText.setCharacterSize(12);
        statusText.setFillColor(sf::Color(80, 70, 55));
        statusText.setPosition(10.0f, static_cast<float>(WINDOW_H - 20));
        window.draw(statusText);

        sf::Text legend;
        legend.setFont(font);
        legend.setString("● red = not done   ● blue = done (auto-removes after 1 week)");
        legend.setCharacterSize(11);
        legend.setFillColor(sf::Color(120, 105, 85));
        legend.setPosition(static_cast<float>(WINDOW_W - 380),
            static_cast<float>(WINDOW_H - 20));
        window.draw(legend);

        window.display();
    }

    return 0;
}
