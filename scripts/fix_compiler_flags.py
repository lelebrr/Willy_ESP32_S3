Import("env")

# This script separates C and C++ flags to avoid warnings like:
# "cc1.exe: warning: command-line option '-Wno-volatile' is valid for C++/ObjC++ but not for C"
print("\n[FIX] Separating and refining compiler flags for a clean build...")

def fix_flags():
    # Flags to separate
    cpp_only = [
        "-Wno-volatile",
        "-Wno-deprecated-enum-enum-conversion"
    ]
    c_only = [
        "-Wno-discarded-qualifiers"
    ]

    # Common flags to suppress library noise and 3rd party issues
    # Added -Wno-stringop-overflow for helix-aac and other libraries
    common_suppression = [
        "-Wno-deprecated-declarations",
        "-Wno-cpp",
        "-Wno-stringop-overflow"
    ]

    # Helper to remove flag from an environment variable list or string
    def remove_flag(env_var, flag):
        flags = env.get(env_var, [])
        if isinstance(flags, list):
            while flag in flags:
                flags.remove(flag)
        elif isinstance(flags, str):
            if flag in flags:
                # Use regex or better string replacement to avoid partial matches
                import re
                env[env_var] = re.sub(r'\b' + re.escape(flag) + r'\b', '', flags).strip()

    # 1. Clean up shared CCFLAGS and ensure suppression is applied there as well
    for f in cpp_only + c_only + common_suppression:
        remove_flag("CCFLAGS", f)

    # Apply common suppression to CCFLAGS to ensure it's hit early and often
    for f in common_suppression:
        if f not in env.get("CCFLAGS", []):
            env.Append(CCFLAGS=[f])

    # 2. Add to language-specific flags
    # C++ Specific
    env.Append(CXXFLAGS=cpp_only)

    # C Specific
    env.Append(CFLAGS=c_only)

    # 3. Final safety cleanup of cross-pollination
    for f in cpp_only:
        remove_flag("CFLAGS", f)
    for f in c_only:
        remove_flag("CXXFLAGS", f)

# Execute the fix
fix_flags()

# Add framework libraries include paths for Arduino v3 compatibility
import os
framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
if framework_dir and os.path.isdir(os.path.join(framework_dir, "libraries")):
    lib_dir = os.path.join(framework_dir, "libraries")
    for lib in os.listdir(lib_dir):
        src_path = os.path.join(lib_dir, lib, "src")
        if os.path.isdir(src_path):
            env.Append(CPPPATH=[src_path])
    print("[FIX] Added framework libraries to include path.")

def clean_mquickjs_library(*args, **kwargs):
    pioenv = env.get("PIOENV", "")
    project_dir = env.get("PROJECT_DIR", "")
    if pioenv and project_dir:
        lib_path = os.path.join(project_dir, ".pio", "libdeps", pioenv, "mquickjs")
        if os.path.isdir(lib_path):
            files_to_delete = [
                "example.c",
                "example_stdlib.c",
                "mqjs.c",
                "mqjs_stdlib.c",
                "readline.c",
                "readline_tty.c"
            ]
            for f in files_to_delete:
                filepath = os.path.join(lib_path, f)
                if os.path.isfile(filepath):
                    try:
                        os.remove(filepath)
                        print(f"[FIX] Deleted {f} from mquickjs to prevent compilation.")
                    except OSError:
                        pass

clean_mquickjs_library()
