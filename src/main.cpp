#include "app/app.h"

#include <iostream>

using app::App;

int main() {
    auto app = App::create();

    if (std::holds_alternative<app::Error>(app)) {
        std::cerr << std::get<app::Error>(app) << "\n";
        return 1;
    }
}
