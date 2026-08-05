#!/usr/bin/env python3
"""Remove toolchain scaffolding STM32CubeMX generates that PlatformIO doesn't need.

Runs automatically before every `pio run` (see platformio.ini's
extra_scripts). Can also be run by hand after "Generate Code" in CubeMX:
    python tools/cubemx_cleanup.py [--dry-run]

Keeps Core/ (used as src_dir/include_dir, see platformio.ini), the .ioc file,
and .mxproject (CubeMX needs it to track previously generated files across
regenerations). Everything removed here is either an IDE project for a
toolchain we don't build with, or a copy of HAL/CMSIS sources that
framework-stm32cubef4 already provides.
"""
import shutil
import sys
from pathlib import Path

try:
    # Plain `python tools/cubemx_cleanup.py` run.
    PROJECT_ROOT = Path(__file__).resolve().parent.parent
except NameError:
    # SCons execs extra_scripts without setting __file__; it hands us an
    # "env" global instead, which knows the project root.
    Import("env")  # noqa: F821
    PROJECT_ROOT = Path(env.subst("$PROJECT_DIR"))  # noqa: F821

DIRS_TO_REMOVE = [
    "EWARM",           # IAR
    "MDK-ARM",         # Keil
    "SW4STM32",        # System Workbench
    "STM32CubeIDE",    # CubeIDE
    "TrueSTUDIO",
    "Drivers",         # duplicate HAL/CMSIS - framework-stm32cubef4 supplies these
    "Middlewares",
    ".settings",
]

FILES_TO_REMOVE = [
    ".project",
    ".cproject",
]

FILE_GLOBS_TO_REMOVE = [
    "*.uvprojx",
    "*.uvoptx",
    "*.launch",
]


def main(dry_run: bool = False) -> None:
    removed = []
    skipped = []

    def remove_dir(path: Path) -> None:
        if dry_run:
            removed.append(path)
            return
        try:
            shutil.rmtree(path)
            removed.append(path)
        except OSError as exc:
            skipped.append((path, exc))

    def remove_file(path: Path) -> None:
        if dry_run:
            removed.append(path)
            return
        try:
            path.unlink()
            removed.append(path)
        except OSError as exc:
            skipped.append((path, exc))

    for name in DIRS_TO_REMOVE:
        path = PROJECT_ROOT / name
        if path.is_dir():
            remove_dir(path)

    for name in FILES_TO_REMOVE:
        path = PROJECT_ROOT / name
        if path.is_file():
            remove_file(path)

    for pattern in FILE_GLOBS_TO_REMOVE:
        for path in PROJECT_ROOT.glob(pattern):
            remove_file(path)

    verb = "Would remove" if dry_run else "Removed"
    for path in removed:
        print(f"{verb}: {path.relative_to(PROJECT_ROOT)}")

    # Locked files (e.g. CubeMX still has EWARM/ open) shouldn't fail the build -
    # they'll just get cleaned up next time this runs.
    for path, exc in skipped:
        print(f"Skipped (in use?): {path.relative_to(PROJECT_ROOT)} ({exc})")

    if not removed and not skipped:
        print("Nothing to clean.")


# No `if __name__ == "__main__":` guard: SCons exec()s this file in a
# context where __name__ isn't reliably "__main__", so just always run.
# sys.argv is the parent `pio`/`python` invocation's argv either way, so
# --dry-run only ever triggers from an explicit CLI run.
main(dry_run="--dry-run" in sys.argv)
