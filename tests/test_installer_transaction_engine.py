import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from installer.engine.transaction_engine import (
    FileEntry, InstallerEngine, ModulePlan, TransactionError,
)


class InstallerTransactionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.root = self.base / "vita-root"
        self.payload = self.base / "payload"
        self.payload.mkdir()

    def make_plan(self, operation, version, files=(), owned=("settings/demo",), data=("ur0:data/RtSettingsExtended/Demo/",)):
        entries = []
        for target, content in files:
            source = self.payload / target.rsplit("/", 1)[-1]
            source.write_bytes(content)
            entries.append(FileEntry(
                path=target,
                source=source.name,
                sha256=hashlib.sha256(content).hexdigest(),
                size_bytes=len(content),
            ))
        return ModulePlan(
            module_id="demo-module",
            version=version,
            operation=operation,
            payload_root=self.payload,
            files=tuple(entries),
            owned_paths=tuple(owned),
            data_paths=tuple(data),
        )

    def test_install_writes_verified_files_and_registry(self):
        engine = InstallerEngine(self.root)
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"enabled=1\n"),
        ])
        transaction_id = engine.apply(plan)
        self.assertRegex(transaction_id, r"^[a-f0-9]{32}$")
        self.assertEqual((self.root / "settings/demo/config.ini").read_bytes(), b"enabled=1\n")
        registry = json.loads((self.root / ".rtse/registry.json").read_text())
        self.assertEqual(registry["modules"]["demo-module"]["version"], "1.0.0")
        journal = json.loads((self.root / ".rtse/transactions" / transaction_id / "journal.json").read_text())
        self.assertEqual(journal["status"], "committed")

    def test_update_then_rollback_restores_previous_version(self):
        engine = InstallerEngine(self.root)
        engine.apply(self.make_plan("install", "1.0.0", [
            ("settings/demo/old.ini", b"old-value"),
        ]))
        update_id = engine.apply(self.make_plan("update", "2.0.0", [
            ("settings/demo/new.ini", b"new-value"),
        ]))
        self.assertFalse((self.root / "settings/demo/old.ini").exists())
        self.assertEqual((self.root / "settings/demo/new.ini").read_bytes(), b"new-value")

        engine.rollback(update_id)
        self.assertEqual((self.root / "settings/demo/old.ini").read_bytes(), b"old-value")
        self.assertFalse((self.root / "settings/demo/new.ini").exists())
        registry = json.loads((self.root / ".rtse/registry.json").read_text())
        self.assertEqual(registry["modules"]["demo-module"]["version"], "1.0.0")

    def test_remove_deletes_owned_files_but_never_user_data(self):
        engine = InstallerEngine(self.root)
        engine.apply(self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"keep-user-settings-separate"),
        ]))
        user_data = self.base / "user-data" / "tai-saver"
        user_data.mkdir(parents=True)
        archive = user_data / "known-good.tar"
        archive.write_bytes(b"valuable backup")

        engine.apply(self.make_plan("remove", "1.0.0", files=()))
        self.assertFalse((self.root / "settings/demo/config.ini").exists())
        self.assertTrue(archive.exists())
        registry = json.loads((self.root / ".rtse/registry.json").read_text())
        self.assertNotIn("demo-module", registry["modules"])

    def test_failed_registry_commit_rolls_back_payload(self):
        engine = InstallerEngine(self.root)
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"must-not-remain"),
        ])
        original = engine._write_json_atomic

        def fail_registry(path, value):
            if path == engine.registry_path:
                raise OSError("injected registry failure")
            return original(path, value)

        with mock.patch.object(engine, "_write_json_atomic", side_effect=fail_registry):
            with self.assertRaises(TransactionError):
                engine.apply(plan)
        self.assertFalse((self.root / "settings/demo/config.ini").exists())
        self.assertFalse((self.root / ".rtse/registry.json").exists())
        journals = list((self.root / ".rtse/transactions").glob("*/journal.json"))
        self.assertEqual(len(journals), 1)
        self.assertEqual(json.loads(journals[0].read_text())["status"], "rolled_back")

    def test_refuses_to_overwrite_unowned_file(self):
        engine = InstallerEngine(self.root)
        target = self.root / "settings/demo/config.ini"
        target.parent.mkdir(parents=True)
        target.write_bytes(b"owned by user")
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"replacement"),
        ])
        with self.assertRaisesRegex(TransactionError, "unowned file"):
            engine.apply(plan)
        self.assertEqual(target.read_bytes(), b"owned by user")

    def test_rejects_payload_hash_mismatch(self):
        engine = InstallerEngine(self.root)
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"payload"),
        ])
        wrong = FileEntry(
            path=plan.files[0].path,
            source=plan.files[0].source,
            sha256="0" * 64,
            size_bytes=plan.files[0].size_bytes,
        )
        with self.assertRaisesRegex(TransactionError, "SHA-256 mismatch"):
            engine.apply(ModulePlan(
                plan.module_id, plan.version, plan.operation, plan.payload_root,
                (wrong,), plan.owned_paths, plan.data_paths,
            ))
        self.assertFalse((self.root / "settings/demo/config.ini").exists())

    def test_rejects_symlinked_destination_parent(self):
        engine = InstallerEngine(self.root)
        outside = self.base / "outside"
        outside.mkdir()
        (self.root / "settings").symlink_to(outside, target_is_directory=True)
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/config.ini", b"no escape"),
        ])
        with self.assertRaises(TransactionError):
            engine.apply(plan)
        self.assertEqual(list(outside.iterdir()), [])

    def test_rejects_path_traversal_in_module_plan(self):
        engine = InstallerEngine(self.root)
        plan = self.make_plan("install", "1.0.0", [
            ("settings/demo/../../escape", b"no escape"),
        ])
        with self.assertRaises(TransactionError):
            engine.apply(plan)


if __name__ == "__main__":
    unittest.main(verbosity=2)
