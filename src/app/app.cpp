#include "app.h"
#include "instance.h"
#include "util.h"

namespace app {

App::App(vk::UniqueInstance&& instance):
    instance(std::move(instance))
{
}

std::variant<App, Error> App::create() {
    auto instance_result = createInstance();
    RETURN_ON_ERROR(instance_result, InstanceCreateError)

    vk::UniqueInstance& instance = std::get<vk::UniqueInstance>(instance_result);

    return App(std::move(instance));
}

} // namespace app
