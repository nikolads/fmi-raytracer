#pragma once

#include "deps.h"

#include <variant>

namespace app {

std::variant<vk::UniqueInstance, vk::Result> createInstance();

}
