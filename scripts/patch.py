import hashlib
import requests # type: ignore
from typing import TYPE_CHECKING, Any

if TYPE_CHECKING:
    Import: Any = None
    env: Any = None


import glob
import gzip
from os import makedirs, remove, rename
from os.path import basename, dirname, exists, isfile, join

try:
    Import("env")  # type: ignore

    FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32-libs") # type: ignore

    if FRAMEWORK_DIR is not None:
        board_mcu = env.BoardConfig() # type: ignore
        mcu = board_mcu.get("build.mcu", "")
        patchflag_path = join(FRAMEWORK_DIR,mcu, "lib", ".patched")

        # patch file only if we didn't do it befored
        if not isfile(join(FRAMEWORK_DIR,mcu, "lib", ".patched")):
            original_file = join(FRAMEWORK_DIR,mcu, "lib", "libnet80211.a")
            patched_file = join(
                FRAMEWORK_DIR, mcu, "lib", "libnet80211.a.patched"
            )

            # Recovery: if original is missing but .old exists, restore it first
            if not isfile(original_file) and isfile(original_file + ".old"):
                rename(original_file + ".old", original_file)

            objcopy = env.subst("$OBJCOPY")
            env.Execute(
                "%s --weaken-symbol=ieee80211_raw_frame_sanity_check %s %s"
                % (objcopy, original_file, patched_file)
            )

            if isfile(patched_file):
                if isfile("%s.old" % (original_file)):
                    remove("%s.old" % (original_file))

                if isfile(original_file):
                    rename(original_file, "%s.old" % (original_file))

                rename(patched_file, original_file)

                def _touch(path: str) -> None:
                    with open(path, "w") as fp:
                        fp.write("")

                env.Execute(lambda *args, **kwargs: _touch(patchflag_path))
            else:
                print("Patch: Failed to create patched file. Keeping original.")
except Exception as e:
    print(f"Warning: Patch script failed - {e}")


def hash_file(file_path):
    """Generate SHA-256 hash for a single file."""
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        # Read the file in chunks to avoid memory issues
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()


def hash_files(file_paths):
    """Generate a combined hash for multiple files."""
    combined_hash = hashlib.sha256()

    for file_path in file_paths:
        file_hash = hash_file(file_path)
        combined_hash.update(file_hash.encode("utf-8"))  # Update with the file's hash

    return combined_hash.hexdigest()


def save_checksum_file(hash_value, output_file):
    """Save the hash value to a specified output file."""
    with open(output_file, "w") as f:
        f.write(hash_value)


def load_checksum_file(input_file):
    """Load the hash value from a specified input file."""
    with open(input_file, "r") as f:
        return f.readline().strip()


def minify_css(c: Any) -> Any:
    try:
        minify_req = requests.post(
            "https://www.toptal.com/developers/cssminifier/api/raw",
            {"input": c.read().decode('utf-8')},
            timeout=10,
        )
        if minify_req.status_code == 200 and minify_req.text:
            return minify_req.text.encode('utf-8')
    except Exception as e:
        print(f"Warning: CSS minification failed ({e}), using original.")
    c.seek(0)
    return c.read()


def minify_js(js):
    try:
        minify_req = requests.post(
            'https://www.toptal.com/developers/javascript-minifier/api/raw',
            {'input': js.read().decode('utf-8')},
            timeout=10,
        )
        if minify_req.status_code == 200 and minify_req.text:
            return minify_req.text.encode('utf-8')
    except Exception as e:
        print(f"Warning: JS minification failed ({e}), using original.")
    js.seek(0)
    return js.read()


def minify_html(html):
    try:
        minify_req = requests.post(
            'https://www.toptal.com/developers/html-minifier/api/raw',
            {'input': html.read().decode('utf-8')},
            timeout=10,
        )
        if minify_req.status_code == 200 and minify_req.text:
            return minify_req.text.encode('utf-8')
    except Exception as e:
        print(f"Warning: HTML minification failed ({e}), using original.")
    html.seek(0)
    return html.read()


# gzip web files
def prepare_www_files() -> None:
    project_dir = env.get("PROJECT_DIR", "")  # type: ignore
    if not project_dir:
        print("Error: PROJECT_DIR not found in environment.")
        return

    HEADER_FILE = join(str(project_dir), "include", "webFiles.h")
    filetypes_to_gzip = ["html", "css", "js"]
    data_src_dir = join(str(project_dir), "embedded_resources/web_interface")
    checksum_file = join(data_src_dir, "checksum.sha256")
    checksum = ""

    if not exists(data_src_dir):
        print(f'Error: Source directory "{data_src_dir}" does not exist!')
        return

    if exists(checksum_file):
        checksum = load_checksum_file(checksum_file)

    files_to_gzip = []
    for extension in filetypes_to_gzip:
        files_to_gzip.extend(glob.glob(join(data_src_dir, "*." + extension)))

    files_checksum = hash_files(files_to_gzip)
    if files_checksum == checksum:
        print("[GZIP & EMBED INTO HEADER] - Nothing to process.")
        return

    print(f"[GZIP & EMBED INTO HEADER] - Processing {len(files_to_gzip)} files.")

    makedirs(dirname(HEADER_FILE), exist_ok=True)

    with open(HEADER_FILE, "w") as header:
        header.write(
            "#ifndef WEB_FILES_H\n#define WEB_FILES_H\n\n#include <Arduino.h>\n\n"
        )
        header.write(
            "// THIS FILE IS AUTOGENERATED DO NOT MODIFY IT. MODIFY FILES IN /embedded_resources/web_interface\n\n"
        )

        for file in files_to_gzip:
            gz_file = file + ".gz"
            with open(file, "rb") as src, gzip.open(gz_file, "wb") as dst:
                ext = basename(file).rsplit(".", 1)[-1].lower()
                if ext == 'html':
                    minified = minify_html(src)
                elif ext == 'css':
                    minified = minify_css(src)
                elif ext == 'js':
                    minified = minify_js(src)
                else:
                    raise ValueError(f"Unsupported file type: {ext}")

                # # Output minified file
                # min_file = file + ".min"
                # with open(min_file, "wb") as minf:
                #     minf.write(minified)

                dst.write(minified)

            with open(gz_file, "rb") as gz:
                compressed_data: bytes = getattr(gz, "read")() # explicit type hinting
                var_name = basename(file).replace(".", "_")

                header.write(f"const uint8_t {var_name}[] PROGMEM = {{\n")

                # Write hex values, inserting a newline every 15 bytes
                for i in range(0, len(compressed_data), 15):
                    chunk = [compressed_data[j] for j in range(i, min(i + 15, len(compressed_data)))]
                    hex_chunk = ", ".join(
                        f"0x{byte:02X}" for byte in chunk
                    )
                    header.write(f"  {hex_chunk},\n")

                header.write("};\n\n")
                header.write(
                    f"const uint32_t {var_name}_size = {len(compressed_data)};\n\n"
                )

            remove(gz_file)  # Clean up temporary gzip file

        header.write("#endif // WEB_FILES_H\n")

    save_checksum_file(files_checksum, checksum_file)

    print(f"[DONE] Gzipped files embedded into {HEADER_FILE}")


def patch_onewire() -> None:
    """Fix malformed #undef directives in OneWire.cpp (lines 599-600).
    The library incorrectly writes '#undef noInterrupts() ...' with extra tokens
    instead of just '#undef noInterrupts'. This causes GCC warnings."""
    project_dir = env.get("PROJECT_DIR", "")  # type: ignore
    pioenv = env.get("PIOENV", "")  # type: ignore
    if not project_dir or not pioenv:
        return

    onewire_file = join(str(project_dir), ".pio", "libdeps", str(pioenv), "OneWire", "OneWire.cpp")
    if not isfile(onewire_file):
        return

    with open(onewire_file, "r", encoding="utf-8") as f:
        content = f.read()

    old = '#  undef noInterrupts() {portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;portENTER_CRITICAL(&mux)'
    new = '#  undef noInterrupts'
    old2 = '#  undef interrupts() portEXIT_CRITICAL(&mux);}'
    new2 = '#  undef interrupts'

    if old in content:
        content = content.replace(old, new)
        content = content.replace(old2, new2)
        with open(onewire_file, "w", encoding="utf-8") as f:
            f.write(content)
        print("[PATCH] Fixed OneWire.cpp malformed #undef directives.")
    else:
        print("[PATCH] OneWire.cpp already patched or not applicable.")


try:
    patch_onewire()
except Exception as e:
    print(f"Warning: patch_onewire failed: {e}")

try:
    prepare_www_files()
except Exception as e:
    print(f"Warning: prepare_www_files failed: {e}")
