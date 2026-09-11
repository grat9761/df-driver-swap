// Delta Force per-process driver swap for Poco F4 (munch)
// Keeps the ROM's stock Vulkan driver for the whole system; loads Turnip 24.1.0 R18
// only inside the Delta Force process via libadrenotools.
// Reference skeleton -- a developer should review before use. See README.txt.
#include <android/log.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <zygisk.hpp>
#include <adrenotools/driver.h>

#define TAG "DFDriverSwap"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Match by substring so regional package variants still hit (cn/global builds differ).
static bool isTarget(const char *niceName) {
    if (!niceName) return false;
    std::string n(niceName);
    return n.find("deltaforce") != std::string::npos;
}

static bool copyFile(const std::string &src, const std::string &dst) {
    int in = open(src.c_str(), O_RDONLY);
    if (in < 0) return false;
    int out = open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { close(in); return false; }
    char buf[65536]; ssize_t r;
    while ((r = read(in, buf, sizeof(buf))) > 0) {
        ssize_t o = 0;
        while (o < r) { ssize_t w = write(out, buf + o, r - o); if (w < 0) break; o += w; }
    }
    close(in); close(out);
    return r == 0;
}

static void copyDir(const std::string &src, const std::string &dst) {
    mkdir(dst.c_str(), 0755);
    DIR *d = opendir(src.c_str());
    if (!d) return;
    while (auto *e = readdir(d)) {
        std::string n = e->d_name;
        if (n == "." || n == "..") continue;
        if (e->d_type == DT_REG) copyFile(src + "/" + n, dst + "/" + n);
    }
    closedir(d);
}

class DriverSwap : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *) override { this->api = api; }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        if (!args || !isTarget(args->nice_name)) return;

        const char *modDirC  = api->getModuleDir();            // /data/adb/modules/dfdriverswap
        std::string appData  = args->app_data_dir ? args->app_data_dir : "";
        if (!modDirC || appData.empty()) { LOGE("missing paths"); return; }

        std::string modDir = modDirC;
        std::string work   = appData + "/files/dfdriver";      // private, dlopen-able, writable
        std::string drvDst = work + "/libvulkan_freedreno.so";
        mkdir((appData + "/files").c_str(), 0755);
        mkdir(work.c_str(), 0755);
        mkdir((work + "/tmp").c_str(), 0755);
        mkdir((work + "/hooks").c_str(), 0755);

        // 1) Stage the custom driver + adrenotools hook libs from the module dir
        //    (module dir is root-owned; app-private dir is the legal dlopen target)
        copyFile(modDir + "/driver_libvulkan_freedreno.so", drvDst);
        copyDir (modDir + "/hooks/arm64-v8a", work + "/hooks");

        // 2) Install the redirect: from now on, this process's libvulkan/driver loads
        //    resolve to the Turnip build. Stock driver is untouched for every other app.
        void *handle = adrenotools_open_libvulkan(
            RTLD_NOW,
            ADRENOTOOLS_DRIVER_CUSTOM,
            (work + "/tmp").c_str(),     // tmpLibDir (only used on API < 29; harmless here)
            (work + "/hooks").c_str(),   // hookLibDir: adrenotools hook libs, extracted above
            work.c_str(),                // customDriverDir
            "libvulkan_freedreno.so",    // customDriverName
            nullptr,                     // fileRedirectDir
            nullptr);                    // userMappingHandle

        LOGI("target=%s handle=%p %s", args->nice_name, handle,
             handle ? "OK" : "FAILED - game will use stock driver");
    }

private:
    zygisk::Api *api = nullptr;
};

REGISTER_ZYGISK_MODULE(DriverSwap)
