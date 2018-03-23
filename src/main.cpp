#include "app/app.h"

#include <iostream>

using app::App;

int main() {
    auto app = App::create();
    app.mainLoop();
}
