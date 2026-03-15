Import("env")
import os
import subprocess
import hashlib
import shutil
import logging
from datetime import datetime

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] %(levelname)s: %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

PIOENV = env.subst("$PIOENV")
MQJS_PATH = os.path.join(".pio/libdeps", PIOENV, "mquickjs")

BUILD_DIR = "lib/mquickjs_headers"
GEN = os.path.join(BUILD_DIR, "mqjs_stdlib_generator")
BUILD_SHA256 = os.path.join(BUILD_DIR, "mqjs_stdlib.build.sha256")

BJS_INTERPRETER_PATH = "src/modules/bjs_interpreter/"
WATCH_FILE = os.path.join(BJS_INTERPRETER_PATH, "mqjs_stdlib.c")

SRC = [
    WATCH_FILE,
    os.path.join(MQJS_PATH, "mquickjs_build.c"),
]

CFLAGS = [
    "-Wall",
    "-O2",
    "-m32",
    "-DMQJS_GENERATOR",
    "-I" + MQJS_PATH,
]

HOST_CC = "gcc"

INCLUDES = [
    'mqjs_native_decl',
    'user_classes_js',
]

def sha256_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            # Normalize CRLF to LF so signatures are stable across OSes.
            h.update(chunk.replace(b"\r\n", b"\n"))
    return h.hexdigest()

def _project_option(name: str):
    try:
        return env.GetProjectOption(name, default="")
    except Exception:
        return ""

def _resolve_host_cc():
    override = os.environ.get("MQJS_HOST_CC") or _project_option("mqjs_host_cc")
    return override if override else HOST_CC

def _has_define(name: str) -> bool:
    try:
        flags = env.ParseFlags(env['BUILD_FLAGS'])
    except Exception:
        return False
    for define in flags.get('CPPDEFINES', []):
        if define == name:
            return True
        if isinstance(define, list) and define and define[0] == name:
            return True
    return False

def _gen_enabled() -> bool:
    # Default to enabled for backward compatibility.
    if _has_define("DISABLE_MQJS_HEADERS_GEN"):
        return False
    if _has_define("GEN_MQJS_HEADERS"):
        value = get_build_flag_value("GEN_MQJS_HEADERS")
        if value is not None and str(value).strip().lower() in ("0", "false", "no", "off"):
            return False
    return True

def _normalize_option_value(value) -> str:
    if value is None:
        return ""
    if isinstance(value, (list, tuple)):
        # Keep order; PlatformIO preserves flag order.
        return "\n".join(_normalize_option_value(v) for v in value)
    return str(value)

def compute_signature():
    # Any change here should force a rebuild.
    parts = []
    parts.append("v=3")
    parts.append(f"watch_sha256={sha256_file(WATCH_FILE)}")

    mqjs_build_c = os.path.join(MQJS_PATH, "mquickjs_build.c")
    parts.append(f"mquickjs_build_sha256={sha256_file(mqjs_build_c)}")
    mqjs_build_h = os.path.join(MQJS_PATH, "mquickjs_build.h")
    parts.append(f"mquickjs_build_sha256={sha256_file(mqjs_build_h)}")

    h = hashlib.sha256()
    h.update("\n".join(parts).encode("utf-8"))
    return h.hexdigest()

def needs_rebuild():
    if not os.path.exists(os.path.join(BJS_INTERPRETER_PATH, "mqjs_stdlib.h")):
        return True
    if not os.path.exists(BUILD_SHA256):
        return True

    with open(BUILD_SHA256, "r") as f:
        old = f.read().strip()

    return compute_signature() != old

def write_build_stamp():
    with open(BUILD_SHA256, "w") as f:
        f.write(compute_signature())

def get_build_flag_value(flag_name):
    build_flags = env.ParseFlags(env['BUILD_FLAGS'])
    flags_with_value_list = [build_flag for build_flag in build_flags.get('CPPDEFINES') if type(build_flag) == list]
    defines = {k: v for (k, v) in flags_with_value_list}
    return defines.get(flag_name)

def generate_headers():
    try:
        logger.info("Iniciando geração de headers MQJS...")

        if not os.path.exists(MQJS_PATH):
            logger.warning(f"Caminho MQJS não encontrado: {MQJS_PATH}")
            return

        if get_build_flag_value("LITE_VERSION") is not None or get_build_flag_value("DISABLE_INTERPRETER") is not None:
            logger.info("Headers MQJS desabilitados por flags de build")
            return

        if not _gen_enabled():
            logger.info("Geração de headers MQJS pulada.")
            return

        os.makedirs(BUILD_DIR, exist_ok=True)

        if not needs_rebuild():
            logger.info("Headers MQJS já estão atualizados.")
            return

    except Exception as e:
        logger.error(f"Erro na validação inicial: {e}")
        return

    try:
        host_cc = _resolve_host_cc()
        if not shutil.which(host_cc):
            if os.path.exists(os.path.join(BJS_INTERPRETER_PATH, "mqjs_stdlib.h")):
                logger.warning("Compilador host não encontrado; pulando geração de headers mqjs_stdlib.")
                logger.info("Configure MQJS_HOST_CC ou mqjs_host_cc para um compilador compatível com GCC, ou use Docker.")
                logger.info("Instruções de instalação disponíveis no log de debug.")
                return
            raise RuntimeError("Compilador host não encontrado e mqjs_stdlib.h está faltando.")

        # Build the generator as a native host executable. The output headers are
        # still forced to 32-bit target format via the generator's `-m32` flag.
        logger.info(f"Compilando gerador com {host_cc}...")
        gcc_result = subprocess.run(
            [host_cc, *CFLAGS, "-o", GEN, *SRC],
            capture_output=True,
            text=True,
        )
        if gcc_result.returncode != 0:
            logger.error(f"Compilação falhou com código {gcc_result.returncode}")
            if gcc_result.stdout:
                logger.error(f"gcc stdout: {gcc_result.stdout}")
            if gcc_result.stderr:
                logger.error(f"gcc stderr: {gcc_result.stderr}")
            raise subprocess.CalledProcessError(
                gcc_result.returncode,
                gcc_result.args,
                output=gcc_result.stdout,
                stderr=gcc_result.stderr,
            )

        logger.info("Gerando headers QuickJS para alvos 32-bit...")

        with open(os.path.join(BJS_INTERPRETER_PATH, "mqjs_stdlib.h"), "w") as f:
            result = subprocess.run([GEN, "-m32"], capture_output=True, text=True, check=True)
            for line in INCLUDES:
                f.write(f'#include "{line}.h"\n')
            f.write("\n")
            f.write(result.stdout)

    except Exception as e:
        logger.error(f"Erro ao gerar headers MicroQuickJS: {e}")
        logger.info("Este erro ocorre porque o arquivo mqjs_stdlib.c foi modificado.")
        logger.info("Para fazer alterações neste arquivo, instale ferramentas build-essential e certifique-se de que gcc está disponível.")
        logger.info("Alternativamente, construa usando Docker: docker compose run platformio_build")
        # Não relançar a exceção para não quebrar o build

    write_build_stamp()

generate_headers()
