DELTA FORCE DRIVER SWAP - README
================================
WHAT THIS IS
  A Zygisk module that loads Turnip Vulkan 24.1.0 R18 (a6xx) ONLY into the Delta Force
  process. The rest of the phone (SystemUI, launcher, advanced blur) keeps the ROM's
  stock driver. Built for Poco F4 (munch), Adreno 650.

WHY
  Stock driver (Turnip Mesa 26.3.0-devel): blur works, Delta Force crashes.
  Turnip 24.1.0 R18: Delta Force works, blur renders black.
  Per-process = both work. See ROM_MAINTAINER_REPORT.md for full diagnosis.

HOW TO BUILD (no PC skills needed - GitHub builds it for you)
  1. Create a free GitHub account.
  2. Click 'New repository', name it anything, keep it private.
  3. Upload ALL files from this zip (unzipped) into the repo
     (Add file -> Upload files; drag the whole folder contents; Commit).
  4. Open the 'Actions' tab -> 'build' -> 'Run workflow'.
  5. Wait ~5 minutes, open the completed run, download the
     'df-driver-swap-munch' artifact.
  6. Flash the zip in Magisk, reboot, launch Delta Force.

VERIFY
  - System: advanced blur still works (stock driver untouched).
  - Game: VulkanCapsViewer-style check via 'adb shell dumpsys SurfaceFlinger' while
    the game runs is not per-process visible; simplest check = the game no longer
    crashes, and logcat shows 'DFDriverSwap: ... handle=0x... OK'.
  - If logcat shows 'FAILED', the game silently uses the stock driver (safe fallback).

WARNINGS
  * ANTI-CHEAT: Delta Force (Tencent ACE) may treat an injected/hooked process as
    tampering. Risk of account ban exists. Use at your own risk; ideally test on a
    secondary account first.
  * Requires Magisk (Zygisk enabled) or KernelSU + ZygiskNext. Add Delta Force to the
    Magisk denylist/shizuku-hide so root is not visible to it.
  * This is a reference implementation: a developer should review main.cpp before use.

FILES
  main.cpp                  - the module source (libzygisk + libadrenotools)
  CMakeLists.txt            - build script (auto-downloads libzygisk + libadrenotools)
  .github/workflows/build.yml - one-click GitHub build
  module.prop               - Magisk module metadata
  driver/libvulkan_freedreno.so - Turnip 24.1.0 R18 (a6xx), from K11MCH1 release
