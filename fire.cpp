#include <iostream>
#include <string_view>
#include <deque>
#include <cstdint>
#include <random>
#include <cmath>
#include <thread>
#include <chrono>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/ioctl.h>
#endif // Windows/Linux

template<typename... Args>
auto average(Args const&... values)
{
    return (((double)values) + ...) / sizeof...(values);
}

std::pair<size_t, size_t> get_terminal_size() {
    // Stolen from StackOverflow: https://stackoverflow.com/questions/23369503/get-size-of-terminal-window-rows-columns
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return std::make_pair(csbi.srWindow.Right-csbi.srWindow.Left+1,
                          csbi.srWindow.Bottom-csbi.srWindow.Top+1);
#elif defined(__linux__) || defined(__APPLE__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    return std::make_pair(w.ws_col,
                          w.ws_row);
#endif // Windows/Linux/MacOS
}

class fire {
public:
    fire() : _fire(), _gen(_rd()) {
        // Initialize with the balnks
        auto [width, height] = get_terminal_size();
        _width = width;
        _height = height;
        for (size_t i = 0; i < _height; ++i) {
            _fire.push_back(std::deque<uint8_t>(_width, 0));
        }
    }

    void seed() {
        // Seed the last row with the random characters
        for (size_t i = 0; i < _width; ++i) {
            _fire.back()[random_index()] = random_char();
        }
        // Add some more new blanks to make it look more like a fire
        for (size_t i = 0; i < _width; ++i) {
            _fire.back()[random_index()] = 0;
        }
    }

    void calculate() {
        // Calculate new value of the character based on the window to the right
        for (size_t i = 0; i < _height-1; ++i) {
            for (size_t j = 0; j < _width-1; ++j) {
                _fire[i][j] = average(_fire[i][j], _fire[i+1][j], _fire[i][j+1], _fire[i+1][j+1]);
            }
        }
        // then base of the window to the left to give it some smooth (and make it flow upwards instead of left-upwards)
        for (size_t i = 0; i < _height-1; ++i) {
            for (size_t j = _width-1; j > 0; --j) {
                _fire[i][j] = average(_fire[i][j], _fire[i+1][j], _fire[i][j-1], _fire[i+1][j-1]);
            }
        }
    }

    void print() {
        // print all but last line
        for (size_t i = 0; i < _height-1; ++i) {
            for (size_t j = 0; j < _width-2; ++j) {
                std::cout << fire_chars[_fire[i][j]];
            }
            std::cout << std::endl;
        }
    }

    void resize() {
        // recalculate the sire of the matrix based on the new terminal values
        auto [width, height] = get_terminal_size();

        if (height > _height) {
            _fire.push_front(std::deque<uint8_t>(_width, 0));
        } else if (height < _height) {
            _fire.pop_front();
        }

        if (width > _width) {
            for (auto& row : _fire) {
                row.push_front(0);
            }
        } else if (width < _width) {
            for (auto& row : _fire) {
                row.pop_front();
            }
        }

        _width = width;
        _height = height;
    }

private:
    double random() {
        return std::generate_canonical<double, 10>(_gen);
    }

    uint8_t random_char() {
        return std::floor(random() * (fire_chars.size() - 2)) + 1;
    }

    size_t random_index() {
        return std::floor(random() * (_fire.back().size() - 1));
    }

    std::deque<std::deque<uint8_t>> _fire;
    size_t _width, _height;

    std::random_device _rd;              // a seed source for the random number engine
    std::mt19937 _gen;                   // mersenne_twister_engine seeded with _rd()

    static constexpr std::string_view fire_chars = std::string_view(" ,;+ltgti!lI?/\\|)(1}{][rcvzjftJUOQocxfXhqwWB8&%$#@");
};

int main() {
    using namespace std::chrono_literals;

    auto f = fire();
    while (true) {
        system("clear");
        f.resize();
        f.seed();
        f.calculate();
        f.print();
        std::this_thread::sleep_for(30ms);
    }
}
