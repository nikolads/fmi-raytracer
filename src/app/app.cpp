#include "app.h"
#include "instance.h"
#include "util.h"
#include "window.h"

namespace app {

App::App(UniqueGlfwWindow&& window, vk::UniqueInstance&& instance):
    window(std::move(window)),
    instance(std::move(instance))
{
}

std::variant<App, Error> App::create() {
    auto window_result = createWindow(800, 600, "GPU raytracer");
    RETURN_ON_NONE(window_result, GlfwError);

    auto& window = window_result.value();

    auto instance_result = createInstance();
    RETURN_ON_ERROR(instance_result, InstanceCreateError)

    auto& instance = std::get<vk::UniqueInstance>(instance_result);

    return App(std::move(window), std::move(instance));
}

} // namespace app
