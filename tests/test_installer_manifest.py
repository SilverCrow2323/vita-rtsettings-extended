import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "installer" / "tools"))
from validate_package import ManifestError, validate_package

class InstallerManifestTests(unittest.TestCase):
    def test_repository_example_builds_default_plan(self):
        plan = validate_package(ROOT / "installer" / "package.example.json")
        self.assertEqual(plan["selected_modules"], ["tai-saver"])
        self.assertTrue(plan["user_data_preserved"])

    def write_fixture(self, root, module):
        installer = root / "installer"
        module_dir = installer / "modules" / "demo"
        module_dir.mkdir(parents=True)
        (installer / "package.json").write_text(json.dumps({
            "format_version": 1, "product_id": "rtsettings-extended",
            "display_name": "Rt:Settings Extended", "release_version": "1.0.0",
            "framework": {"version": "1.0.0", "minimum_firmware": "3.65", "payload_root": "framework/payload"},
            "modules": [{"id": "demo", "manifest": "modules/demo/module.json", "required": False, "default_selected": True}]
        }), encoding="utf-8")
        (module_dir / "module.json").write_text(json.dumps(module), encoding="utf-8")
        return installer / "package.json"

    def module(self, **overrides):
        value = {
            "manifest_version": 1, "id": "demo", "version": "1.0.0",
            "minimum_framework": "1.0.0", "minimum_firmware": "3.65",
            "entry": "settings/demo", "payload_root": "payload",
            "owned_paths": ["settings/demo/"], "data_paths": ["ur0:data/RtSettingsExtended/Demo/"],
            "dependencies": [{"id": "rtse-framework", "minimum_version": "1.0.0"}],
            "conflicts": [],
            "uninstall": {"remove_owned_files": True, "preserve_data_by_default": True}
        }
        value.update(overrides)
        return value

    def test_rejects_traversal_in_owned_paths(self):
        with tempfile.TemporaryDirectory() as tmp:
            package = self.write_fixture(Path(tmp), self.module(owned_paths=["../tai/config.txt"]))
            with self.assertRaises(ManifestError):
                validate_package(package)

    def test_rejects_missing_dependency(self):
        with tempfile.TemporaryDirectory() as tmp:
            package = self.write_fixture(Path(tmp), self.module(dependencies=[{"id": "missing", "minimum_version": "1.0.0"}]))
            with self.assertRaises(ManifestError):
                validate_package(package)

    def test_rejects_data_path_outside_ur0(self):
        with tempfile.TemporaryDirectory() as tmp:
            package = self.write_fixture(Path(tmp), self.module(data_paths=["ux0:data/unsafe/"]))
            with self.assertRaises(ManifestError):
                validate_package(package)

if __name__ == "__main__":
    unittest.main()
