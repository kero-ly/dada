"""Host-side delivery checks; no SDK or board is needed."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("dada", ROOT / "tools/dada.py")
dada = importlib.util.module_from_spec(spec)
spec.loader.exec_module(dada)
BOARD = "jinyichen-esp32-c3-lcd-2.4"


class DeliveryTests(unittest.TestCase):
    def test_build_and_app_isolation(self):
        """The ESP-IDF fallback keeps one build directory per board and app."""
        first = dada.command("build", BOARD, "touch-counter", use_pio=False)
        second = dada.command("build", BOARD, "bringup", use_pio=False)
        self.assertIn("-DDADA_APP=touch-counter", first)
        self.assertIn("-DDADA_BOARD=" + BOARD, first)
        self.assertNotEqual(first[first.index("-B") + 1], second[second.index("-B") + 1])
        self.assertEqual(first[-1], "build")

    def test_platformio_is_preferred(self):
        """When pio exists, build/flash/monitor go through the pinned env."""
        build = dada.command("build", BOARD, "whale", use_pio=True)
        self.assertEqual(build[:4], ["pio", "run", "-d", build[3]])
        self.assertEqual(build[build.index("-e") + 1], "dada-c3-whale")

        flash = dada.command("flash", BOARD, "whale", "/dev/cu.example", use_pio=True)
        self.assertEqual(flash[flash.index("--target") + 1], "upload")
        self.assertEqual(flash[flash.index("--upload-port") + 1], "/dev/cu.example")

        monitor = dada.command("monitor", BOARD, "whale", "COM3", use_pio=True)
        self.assertEqual(monitor[0], "pio")
        self.assertEqual(monitor[monitor.index("--port") + 1], "COM3")

        # Two apps must map to two environments, never the same one.
        other = dada.command("build", BOARD, "snake", use_pio=True)
        self.assertNotEqual(build[build.index("-e") + 1], other[other.index("-e") + 1])

    def test_unverified_is_refused_under_platformio(self):
        """The flag cannot be expressed per environment, so it must not be
        silently dropped."""
        with self.assertRaisesRegex(ValueError, "allow-unverified"):
            dada.command("build", BOARD, "whale", use_pio=True, allow_unverified=True)

    def test_flash_requires_explicit_port(self):
        for action in ("flash", "monitor"):
            with self.assertRaisesRegex(ValueError, "port"):
                dada.command(action, BOARD, "touch-counter")
        cmd = dada.command("flash", BOARD, "touch-counter", "/dev/cu.example",
                           use_pio=False)
        self.assertEqual(cmd[-1], "flash")
        self.assertEqual(cmd[cmd.index("-p") + 1], "/dev/cu.example")

    def test_rejects_invalid_and_unknown_ids(self):
        for app in ("../touch-counter", "unknown"):
            with self.assertRaises(ValueError):
                dada.command("build", BOARD, app)
        with self.assertRaises(ValueError):
            dada.command("build", "unknown", "touch-counter")

    def test_capability_and_geometry_mismatch(self):
        # Mutate real manifest inputs in a temporary repository, not the source tree.
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            board_dir = root / "hardware/vendor/board"
            app_dir = root / "apps/touch-counter"
            board_dir.mkdir(parents=True)
            (board_dir / "board").mkdir()
            (board_dir / "board/CMakeLists.txt").write_text("# binding")
            app_dir.mkdir(parents=True)
            (app_dir / "app.json").write_text((ROOT / "apps/touch-counter/app.json").read_text())
            board = {"id": "vendor-board", "display": {"width": 240, "height": 320}}
            manifest = board_dir / "board.json"
            manifest.write_text(json.dumps(board))
            with self.assertRaisesRegex(ValueError, "input"):
                dada.resolve_selection("vendor-board", "touch-counter", root,
                                       allow_unverified=True)
            board["touch"] = {"controller": "example"}
            board["display"]["width"] = 80
            manifest.write_text(json.dumps(board))
            with self.assertRaisesRegex(ValueError, "display"):
                dada.resolve_selection("vendor-board", "touch-counter", root,
                                       allow_unverified=True)

    def test_verified_capabilities_govern_time(self):
        """An explicit verified list wins; time must be declared, not assumed."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            board_dir = root / "hardware/vendor/board"
            app_dir = root / "apps/focus-clock"
            board_dir.mkdir(parents=True)
            (board_dir / "board").mkdir()
            (board_dir / "board/CMakeLists.txt").write_text("# binding")
            app_dir.mkdir(parents=True)
            (app_dir / "app.json").write_text(
                (ROOT / "apps/focus-clock/app.json").read_text())
            board = {
                "id": "vendor-board",
                "display": {"width": 240, "height": 320},
                "touch": {"controller": "example"},
                "declared_capabilities": ["time"],
                "verified_capabilities": ["display", "touch"],
            }
            manifest = board_dir / "board.json"
            manifest.write_text(json.dumps(board))
            with self.assertRaisesRegex(ValueError, "time"):
                dada.resolve_selection("vendor-board", "focus-clock", root)
            dada.resolve_selection("vendor-board", "focus-clock", root,
                                   allow_unverified=True)

            board["verified_capabilities"] = ["display", "touch", "time"]
            manifest.write_text(json.dumps(board))
            datum = dada.resolve_selection("vendor-board", "focus-clock", root)
            self.assertEqual(datum[0], board_dir)

    def test_empty_verified_list_requires_explicit_bringup_opt_in(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            board_dir = root / "hardware/vendor/board"
            app_dir = root / "apps/touch-counter"
            board_dir.mkdir(parents=True)
            (board_dir / "board").mkdir()
            (board_dir / "board/CMakeLists.txt").write_text("# binding")
            app_dir.mkdir(parents=True)
            (app_dir / "app.json").write_text((ROOT / "apps/touch-counter/app.json").read_text())
            board = {
                "id": "vendor-board",
                "display": {"width": 240, "height": 320},
                "touch": {"controller": "example"},
                "declared_capabilities": [],
                "verified_capabilities": [],
            }
            manifest = board_dir / "board.json"
            manifest.write_text(json.dumps(board))

            with self.assertRaisesRegex(ValueError, "not hardware-verified"):
                dada.resolve_selection("vendor-board", "touch-counter", root)
            dada.resolve_selection("vendor-board", "touch-counter", root,
                                   allow_unverified=True)

            del board["touch"]
            manifest.write_text(json.dumps(board))
            with self.assertRaisesRegex(ValueError, "lacks declared capability"):
                dada.resolve_selection("vendor-board", "touch-counter", root,
                                       allow_unverified=True)

    def test_missing_verification_record_is_not_trusted(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            board_dir = root / "hardware/vendor/board"
            app_dir = root / "apps/touch-counter"
            board_dir.mkdir(parents=True)
            (board_dir / "board").mkdir()
            (board_dir / "board/CMakeLists.txt").write_text("# binding")
            app_dir.mkdir(parents=True)
            (app_dir / "app.json").write_text(
                (ROOT / "apps/touch-counter/app.json").read_text())
            (board_dir / "board.json").write_text(json.dumps({
                "id": "vendor-board",
                "display": {"width": 240, "height": 320},
                "touch": {"controller": "example"},
            }))
            with self.assertRaisesRegex(ValueError, "not hardware-verified"):
                dada.resolve_selection("vendor-board", "touch-counter", root)
            dada.resolve_selection("vendor-board", "touch-counter", root,
                                   allow_unverified=True)

    def test_unverified_flag_reaches_cmake_and_flash(self):
        cmd = dada.command("flash", BOARD, "touch-counter", "/dev/cu.example",
                           allow_unverified=True, use_pio=False)
        self.assertIn("-DDADA_ALLOW_UNVERIFIED=ON", cmd)
        self.assertEqual(cmd[-1], "flash")


if __name__ == "__main__":
    unittest.main()
