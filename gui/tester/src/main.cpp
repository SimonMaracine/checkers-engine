#include "window.hpp"

int main(int argc, char** argv) {
    MainWindow* window = new MainWindow;
    window->show(argc, argv);

    return Fl::run();
}
