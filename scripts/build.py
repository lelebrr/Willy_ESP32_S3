from pathlib import Path
import csv
import logging
import sys
from datetime import datetime
from SCons.Script import Import

# Import PlatformIO's SCons environment
Import("env")
senv = env  # avoid shadowing with the action's 'env' parameter

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] %(levelname)s: %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

# Optional: keep your extra flag
senv.Append(CXXFLAGS=["-Wno-conversion-null"])

# ---- Detect MCU and map bootloader offsets ----
board_config = senv.BoardConfig()
mcu = (board_config.get("build.mcu") or senv.get("BOARD_MCU") or "").lower()

BOOT_OFFSETS = {
    "esp32":   0x1000,  # ESP32
    "esp32s3": 0x0000,  # ESP32-S3
    "esp32c5": 0x2000,  # ESP32-C5
}
boot_offset = BOOT_OFFSETS.get(mcu, 0x0000)  # safe fallback

# Default offsets (adjust if your partitions.csv uses custom addresses)
PART_TABLE_OFFSET = 0x8000
APP_OFFSET        = 0x10000

# Paths
build_dir = Path(senv.subst("$BUILD_DIR"))
proj_dir  = Path(senv.subst("$PROJECT_DIR"))
pioenv    = senv.subst("${PIOENV}")

boot_bin = build_dir / "bootloader.bin"
part_bin = build_dir / "partitions.bin"
app_bin  = build_dir / "firmware.bin"

out_bin  = proj_dir / f"Willy-{pioenv}.bin"

# Esptool from PlatformIO + Python executable
esptool_pkg = senv.PioPlatform().get_package_dir("tool-esptoolpy")
esptool_py  = str(Path(esptool_pkg) / "esptool")
python_exe  = senv.get("PYTHONEXE", "python")

chip_arg = mcu if mcu else "esp32"

def _merge_bins_callback(target, source, env):
    """
    Post-action callback executed after firmware.bin is built.
    Merges bootloader, partitions, and app into a single binary.
    NOTE: This function signature must be (target, source, env) so SCons can call it.
    """
    try:
        logger.info("Iniciando merge de binários...")

        # Validate inputs
        if not all([boot_bin, part_bin, app_bin]):
            logger.error("Caminhos dos binários não definidos")
            env.Exit(1)

        # Check files
        missing = [p for p in [boot_bin, part_bin, app_bin] if not p.exists()]
        if missing:
            logger.error("Arquivos necessários não encontrados, merge abortado:")
            for p in missing:
                logger.error(f" - {p}")
            env.Exit(1)

        logger.info(f"Arquivos encontrados: boot={boot_bin}, part={part_bin}, app={app_bin}")

    except Exception as e:
        logger.error(f"Erro na validação inicial: {e}")
        env.Exit(1)

    # Quote paths for Windows safety
    def q(p): return f"\"{p}\""

    # ---- Read partition CSV to get test partition size and ota_0 offset ----
    part_csv_name = board_config.get("build.partitions") or env.GetProjectOption(
        "board_build.partitions", default=""
    )
    part_csv = proj_dir / part_csv_name if part_csv_name else proj_dir / "partitions.csv"
    ota_size = None
    ota0_offset = None
    if part_csv.exists():
        try:
            logger.info(f"Lendo arquivo de partições: {part_csv}")
            with open(part_csv, newline="", encoding="utf-8") as f:
                reader = csv.reader(f)
                for row_num, row in enumerate(reader, 1):
                    if not row or row[0].startswith("#"):
                        continue
                    cols = [c.strip() for c in row]
                    if len(cols) < 5:
                        logger.warning(f"Linha {row_num} em {part_csv} tem menos de 5 colunas: {cols}")
                        continue
                    try:
                        name, ptype, subtype, offset, size = cols[:5]
                        subtype = subtype.lower()
                        if subtype in ("ota_0", "factory") and ota_size is None:
                            ota_size = int(size, 0)
                            ota0_offset = int(offset, 0)
                            logger.info(f"Partição OTA encontrada: tamanho={ota_size}, offset={ota0_offset}")
                    except ValueError as e:
                        logger.warning(f"Erro ao parsear linha {row_num} em {part_csv}: {e}")
                        continue
        except Exception as e:
            logger.error(f"Erro ao ler arquivo de partições {part_csv}: {e}")
            env.Exit(1)
    else:
        logger.warning(f"Arquivo de partições não encontrado: {part_csv}")

    # ---- Firmware size check against test partition ----
    if ota_size:
        try:
            fw_size = app_bin.stat().st_size
            percent = (fw_size / ota_size) * 100 if ota_size else 0
            bar_len = 20
            filled = int(bar_len * fw_size / ota_size) if ota_size > 0 else 0
            bar = "=" * filled + " " * (bar_len - filled)
            logger.info(
                f"WILLY: [{bar}] {percent:.1f}% (usado 0x{fw_size:X} bytes de 0x{ota_size:X} da partição OTA)"
            )
            if fw_size > ota_size:
                logger.error("Erro: firmware.bin excede o tamanho da partição OTA")
                env.Exit(1)
        except OSError as e:
            logger.error(f"Erro ao obter tamanho do arquivo {app_bin}: {e}")
            env.Exit(1)

    try:
        cmd = " ".join([
            q(python_exe), "-m", "platformio", "pkg", "exec", "-p", q("tool-esptoolpy"), "--", "esptool.py",
            "--chip", chip_arg,
            "merge-bin",
            "--output", q(out_bin),
            hex(boot_offset), q(boot_bin),
            hex(PART_TABLE_OFFSET), q(part_bin),
            hex(APP_OFFSET), q(app_bin),
        ])

        logger.info("Executando merge de binários...")
        logger.debug(f"Comando: {cmd}")
        rc = env.Execute(cmd)
        if rc != 0:
            logger.error(f"Falha no merge com código de saída {rc}")
            env.Exit(rc)

        try:
            size = out_bin.stat().st_size
        except OSError as e:
            logger.error(f"Erro ao obter tamanho do arquivo final {out_bin}: {e}")
            env.Exit(1)

        logger.info(f"Merge bem-sucedido -> {out_bin} ({size} bytes)")

        if ota0_offset:
            if size < (ota0_offset + ota_size):
                logger.info("Binário final válido para upload")
            else:
                logger.error(
                    f"Erro: tamanho do binário 0x{size:X} excede offset ota_0 0x{(ota0_offset+ota_size):X}"
                )
                env.Exit(1)

    except Exception as e:
        logger.error(f"Erro durante o merge: {e}")
        env.Exit(1)

# Automatically run after firmware.bin is generated
senv.AddPostAction(str(app_bin), _merge_bins_callback)

# Optional manual target: depend on the three binaries and run the same callback
# Wrap it in a lambda so SCons can call it without (target, source, env) if needed.
senv.AddCustomTarget(
    name="build-firmware",
    dependencies=[str(boot_bin), str(part_bin), str(app_bin)],
    actions=[_merge_bins_callback],
    title="Build Firmware (merge)",
    description="Merge bootloader + partitions + app into a single .bin file"
)

# Keep your upload-nobuild helper
senv.AddCustomTarget(
    name="upload-nobuild",
    dependencies=None,
    actions=[f"platformio run -t upload -t nobuild -e {pioenv}"],
    title="Upload Nobuild",
    description="Run upload without rebuilding"
)
