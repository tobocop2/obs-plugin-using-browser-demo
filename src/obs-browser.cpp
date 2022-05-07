#include <obs-module.h>
#include <util/platform.h>
#include <stdexcept>

#if defined(__APPLE__)
#include <dlfcn.h>
#endif

#ifdef ENABLE_WAYLAND
#include <obs-nix-platform.h>
#endif

#include "obs-browser.hpp"

void *get_browser_lib()
{
    // Disable panels on Wayland for now
    bool isWayland = false;
#ifdef ENABLE_WAYLAND
    isWayland = obs_get_nix_platform() == OBS_NIX_PLATFORM_WAYLAND;
#endif
    if (isWayland) {
        throw std::runtime_error("Wayland is not supported.");
        return nullptr;
    }

    obs_module_t *browserModule = obs_get_module("obs-browser");

    if (!browserModule) {
        throw std::runtime_error("Cannot get obs-browser module.");
        return nullptr;
    }

    return obs_get_module_lib(browserModule);
}

QCef* obs::browser::instance()
{
    void *lib = get_browser_lib();
    QCef *(*create_qcef)(void) = nullptr;

    if (!lib) {
        throw std::runtime_error("Cannot get obs-browser lib");
        return nullptr;
    }

    create_qcef = (decltype(create_qcef))os_dlsym(lib, "obs_browser_create_qcef");

    if (!create_qcef) {
        throw std::runtime_error("Cannot create qcef.");
        return nullptr;
    }

    return create_qcef();
}

int obs::browser::version()
{
    void *lib = get_browser_lib();
    int (*qcef_version)(void) = nullptr;

    if (!lib) {
        throw std::runtime_error("Cannot get obs-browser lib for version.");
        return 0;
    }

    qcef_version = (decltype(qcef_version))os_dlsym(lib, "obs_browser_qcef_version_export");

    if (!qcef_version) {
        throw std::runtime_error("Cannot get qcef version.");
        return 0;
    }

    return qcef_version();
}
