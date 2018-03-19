#include "instance.h"
#include "util.h"

#include <array>
#include <experimental/array>
#include <vector>

using std::experimental::make_array;

namespace app {

std::variant<vk::UniqueInstance, vk::Result> createInstance() {
    #ifdef NDEBUG
        const auto validationLayers = make_array<char*>();
    #else
        const auto validationLayers = make_array(
            "VK_LAYER_LUNARG_api_dump",
            "VK_LAYER_LUNARG_standard_validation"
        );
    #endif

    const auto appInfo = vk::ApplicationInfo(
        "GPU Raytracer",            //applicationName
        VK_MAKE_VERSION(0, 1, 0),   //applicationVersion
        "No Engine",                //engineName
        VK_MAKE_VERSION(1, 0, 0),   //engineVersion
        VK_API_VERSION_1_0          //apiVersion
    );

    uint32_t glfwExtensionCount = 0;
    const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions;
    extensions.reserve(glfwExtensionCount + 1);
    extensions.insert(extensions.begin(), glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    const auto createInfo = vk::InstanceCreateInfo(
        {},                         //instanceCreateFlags
        &appInfo,                   //pApplicationInfo
        validationLayers.size(),    //enabledLayerCount
        validationLayers.data(),    //ppEnabledLayerNames
        extensions.size(),          //enabledExtensionCount
        extensions.data()           //ppEnabledExtensionNames
    );

    auto instance = asVariant(vk::createInstanceUnique(createInfo, nullptr));
    return instance;
}

} // namespace app
