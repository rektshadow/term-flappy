#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

const int X_SIZE = 32;
const int Y_SIZE = 16;
const int PIPE_COUNT = 3;
const int PIPE_SPACING = 15;
const char Q_KEY = 'q';
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string NC = "\033[0m";

struct Pixel {
    int x, y;
    Pixel(int x = 0, int y = 0) : x(x), y(y) {}
};

class Game {
private:
    Pixel bird;
    std::vector<Pixel> pipes;

    void draw() {
        std::string buffer = "\033[17A";

        for (int y = 0; y <= Y_SIZE; y++) {
            for (int x = 0; x <= X_SIZE; x++) {
                if (y == 0 || y == Y_SIZE || x == 0 || x == X_SIZE) {
                    buffer += NC + "[]";
                    continue;
                }

                bool pipe_drawn = false;
                for (const auto& pipe : pipes) {
                    if ((pipe.x >= x - 1 && pipe.x <= x + 1) &&
                        (pipe.y == y + 3 || pipe.y == y - 3)) {
                        buffer += GREEN + "[]";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x - 1 && pipe.y == y - 4) {
                        buffer += GREEN + "]/";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x && (pipe.y <= y - 4 || pipe.y >= y + 4)) {
                        buffer += GREEN + "][";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x + 1 && pipe.y == y - 4) {
                        buffer += GREEN + "\\[";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x - 1 && pipe.y == y + 4) {
                        buffer += GREEN + "]\\";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x + 1 && pipe.y == y + 4) {
                        buffer += GREEN + "/[";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x + 1 && (pipe.y <= y - 5 || pipe.y >= y + 5)) {
                        buffer += GREEN + " [";
                        pipe_drawn = true;
                        break;
                    } else if (pipe.x == x - 1 && (pipe.y <= y - 5 || pipe.y >= y + 5)) {
                        buffer += GREEN + "] ";
                        pipe_drawn = true;
                        break;
                    }
                }

                if (pipe_drawn) continue;

                if (bird.y == y && bird.x == x)
                    buffer += YELLOW + ")>";
                else if (bird.y == y && bird.x == x + 1)
                    buffer += YELLOW + "_(";
                else if (bird.y == y && bird.x == x + 2)
                    buffer += YELLOW + " _";
                else if (bird.y == y - 1 && bird.x == x)
                    buffer += YELLOW + ") ";
                else if (bird.y == y - 1 && bird.x == x + 1)
                    buffer += YELLOW + "__";
                else if (bird.y == y - 1 && bird.x == x + 2)
                    buffer += YELLOW + " \\";
                else
                    buffer += NC + "  ";
            }
            buffer += "\n";
        }

        std::cout << buffer;
        std::cout.flush();
    }

    void update_pipes() {
        for (auto& pipe : pipes) {
            pipe.x--;
            if (pipe.x < -1) {
                pipe.x = X_SIZE + 1;
                pipe.y = (rand() % 7) + 5;
            }
        }
    }

    bool hit_test() {
        if (bird.y >= Y_SIZE - 1) {
            return true;
        }

        for (const auto& pipe : pipes) {
            if ((bird.x - 2 < pipe.x + 2) &&
                (bird.x > pipe.x - 2) &&
                ((bird.y < pipe.y - 2) || (bird.y > pipe.y + 1))) {
                return true;
            }
        }

        return false;
    }

    static int kbhit() {
        struct termios oldt, newt;
        int ch;
        int oldf;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);

        if(ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }

        return 0;
    }

    static int getch() {
        struct termios oldt, newt;
        int ch;

        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        ch = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        return ch;
    }

public:
    Game() : bird(10, 10), pipes(PIPE_COUNT) {
        for (int i = 0; i < PIPE_COUNT; i++) {
            pipes[i] = Pixel(X_SIZE + 1 + i * PIPE_SPACING, (rand() % 7) + 5);
        }
    }

    void run() {
        int frame = 0;

        std::cout << "Press SPACE to jump and Q to quit.\n";

        for (int i = 0; i <= Y_SIZE; i++) {
            std::cout << std::endl;
        }

        draw();

        std::cout << "Press any key to start...\n";
        getch();

        while (true) {
            if (kbhit()) {
                int ch = getch();
                if (ch == ' ') {
                    bird.y -= 2;
                } else if (ch == Q_KEY) {
                    break;
                }
            }

            if (frame == 2) {
                bird.y++;
                update_pipes();
                frame = 0;
            }

            if (hit_test()) {
                break;
            }

            draw();

            frame++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\nGame Over!\n";
    }
};

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    Game flappy_bird;
    flappy_bird.run();

    return 0;
}