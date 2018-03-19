#include "error.h"

namespace app {

std::ostream& operator<<(std::ostream& os, const InstanceCreateError& err) {
    os << "InstanceCreateError(" << vk::to_string(err.result) << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Error& err) {
    std::visit([&](auto&& arg){ os << arg; }, err);
    return os;
}

} // namespace app
