#!/usr/bin/env python3
"""Validate Rt:Settings Extended package/module manifests and build an install plan."""
from __future__ import annotations
import argparse, json, re, sys
from pathlib import Path, PurePosixPath

VERSION_RE = re.compile(r"^(\d+)\.(\d+)\.(\d+)(?:[+-][A-Za-z0-9.-]+)?$")
FIRMWARE_RE = re.compile(r"^(\d+)\.(\d+)(?:\.(\d+))?$")
FRAMEWORK_ID = "rtse-framework"

class ManifestError(ValueError):
    pass

def version_tuple(value: str) -> tuple[int, int, int]:
    match = VERSION_RE.fullmatch(value or "")
    if not match:
        raise ManifestError(f"Invalid semantic version: {value!r}")
    return tuple(int(part) for part in match.groups())

def firmware_tuple(value: str) -> tuple[int, int, int]:
    match = FIRMWARE_RE.fullmatch(value or "")
    if not match:
        raise ManifestError(f"Invalid firmware version: {value!r}")
    return tuple(int(part or 0) for part in match.groups())

def safe_relative(value: str, label: str) -> str:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ManifestError(f"{label} must be a non-empty POSIX relative path")
    path = PurePosixPath(value)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise ManifestError(f"Unsafe {label}: {value!r}")
    return value.rstrip("/")

def resolve_inside(base: Path, value: str, label: str) -> Path:
    if not isinstance(value, str) or not value or "\\" in value:
        raise ManifestError(f"{label} must be a non-empty relative path")
    if Path(value).is_absolute() or ".." in PurePosixPath(value).parts:
        raise ManifestError(f"Unsafe {label}: {value!r}")
    candidate = (base / value).resolve()
    if not candidate.is_relative_to(base.resolve()):
        raise ManifestError(f"{label} escapes its allowed directory: {value!r}")
    return candidate

def read_json(path: Path) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ManifestError(f"Cannot read {path}: {exc}") from exc
    if not isinstance(value, dict):
        raise ManifestError(f"{path} must contain a JSON object")
    return value

def validate_package(package_path: Path, selected: set[str] | None = None) -> dict:
    package_path = package_path.resolve()
    installer_root = package_path.parent
    repository_root = installer_root.parent
    package = read_json(package_path)
    if package.get("format_version") != 1 or package.get("product_id") != "rtsettings-extended":
        raise ManifestError("Unsupported package format or product ID")
    if package.get("display_name") != "Rt:Settings Extended":
        raise ManifestError("Unexpected product display name")
    version_tuple(package.get("release_version", ""))
    framework = package.get("framework")
    if not isinstance(framework, dict):
        raise ManifestError("framework must be an object")
    version_tuple(framework.get("version", ""))
    firmware_tuple(framework.get("minimum_firmware", ""))
    if framework.get("payload_root") != "framework/payload":
        raise ManifestError("framework.payload_root must be framework/payload")

    entries = package.get("modules")
    if not isinstance(entries, list) or not entries:
        raise ManifestError("package must declare at least one module")
    modules: dict[str, dict] = {}
    manifest_paths: set[str] = set()
    required_ids: set[str] = set()
    for entry in entries:
        if not isinstance(entry, dict):
            raise ManifestError("module entries must be objects")
        module_id = entry.get("id", "")
        if not re.fullmatch(r"[a-z][a-z0-9-]+", module_id):
            raise ManifestError(f"Invalid module ID: {module_id!r}")
        if module_id in modules:
            raise ManifestError(f"Duplicate module ID: {module_id}")
        if not isinstance(entry.get("required"), bool) or not isinstance(entry.get("default_selected"), bool):
            raise ManifestError(f"{module_id} selection flags must be booleans")
        if entry["required"]:
            required_ids.add(module_id)
        manifest_rel = safe_relative(entry.get("manifest", ""), "module manifest")
        if manifest_rel in manifest_paths:
            raise ManifestError(f"Duplicate module manifest path: {manifest_rel}")
        manifest_paths.add(manifest_rel)
        manifest_path = resolve_inside(installer_root, manifest_rel, "module manifest")
        module = read_json(manifest_path)
        if module.get("manifest_version") != 1 or module.get("id") != module_id:
            raise ManifestError(f"Module identity/version mismatch for {module_id}")
        for field in ("name", "description", "category"):
            if not isinstance(module.get(field), str) or not module[field].strip():
                raise ManifestError(f"{module_id}.{field} is required")
        version_tuple(module.get("version", ""))
        version_tuple(module.get("minimum_framework", ""))
        firmware_tuple(module.get("minimum_firmware", ""))
        safe_relative(module.get("entry", ""), f"{module_id}.entry")
        safe_relative(module.get("payload_root", ""), f"{module_id}.payload_root")
        owned = module.get("owned_paths")
        if not isinstance(owned, list) or not owned:
            raise ManifestError(f"{module_id} must declare owned_paths")
        normalized_owned = [safe_relative(path, f"{module_id}.owned_paths") for path in owned]
        if len(set(normalized_owned)) != len(normalized_owned):
            raise ManifestError(f"{module_id} declares duplicate owned paths")
        data_paths = module.get("data_paths", [])
        if not isinstance(data_paths, list):
            raise ManifestError(f"{module_id}.data_paths must be an array")
        for data_path in data_paths:
            if not isinstance(data_path, str) or not data_path.startswith("ur0:") or ".." in PurePosixPath(data_path.replace(":", "/")).parts:
                raise ManifestError(f"Unsafe user-data path in {module_id}: {data_path!r}")
        schema = module.get("settings_schema")
        if schema:
            if not isinstance(schema, str) or "\\" in schema:
                raise ManifestError(f"Invalid settings schema path for {module_id}")
            schema_path = (manifest_path.parent / schema).resolve()
            if not schema_path.is_relative_to(repository_root.resolve()) or not schema_path.is_file():
                raise ManifestError(f"Settings schema is missing or escapes repository for {module_id}: {schema}")
        uninstall = module.get("uninstall", {})
        if uninstall.get("remove_owned_files") is not True or uninstall.get("preserve_data_by_default") is not True:
            raise ManifestError(f"{module_id} must remove only owned files and preserve user data by default")
        modules[module_id] = {"manifest": module, "entry": entry, "manifest_path": manifest_path, "owned": normalized_owned}

    owners: list[tuple[str, str]] = []
    for module_id, info in modules.items():
        for path in info["owned"]:
            for other_id, other_path in owners:
                if path == other_path or path.startswith(other_path.rstrip("/") + "/") or other_path.startswith(path.rstrip("/") + "/"):
                    raise ManifestError(f"Overlapping ownership: {module_id}:{path} and {other_id}:{other_path}")
            owners.append((module_id, path))

    requested = set(selected) if selected is not None else {
        module_id for module_id, info in modules.items() if info["entry"]["default_selected"]
    }
    requested |= required_ids
    unknown = requested - set(modules)
    if unknown:
        raise ManifestError("Unknown selected modules: " + ", ".join(sorted(unknown)))
    chosen = set(requested)
    pending = list(requested)
    while pending:
        module_id = pending.pop()
        module = modules[module_id]["manifest"]
        for dep in module.get("dependencies", []):
            dep_id = dep.get("id", "")
            minimum = version_tuple(dep.get("minimum_version", ""))
            if dep_id == FRAMEWORK_ID:
                if version_tuple(framework["version"]) < minimum:
                    raise ManifestError(f"{module_id} requires framework {dep.get('minimum_version')} or newer")
                continue
            if dep_id not in modules:
                raise ManifestError(f"{module_id} depends on missing module {dep_id}")
            if version_tuple(modules[dep_id]["manifest"]["version"]) < minimum:
                raise ManifestError(f"{module_id} requires {dep_id} {dep.get('minimum_version')} or newer")
            if dep_id not in chosen:
                chosen.add(dep_id)
                pending.append(dep_id)

    for module_id in chosen:
        conflicts = set(modules[module_id]["manifest"].get("conflicts", [])) & chosen
        if conflicts:
            raise ManifestError(f"{module_id} conflicts with selected module(s): {', '.join(sorted(conflicts))}")

    order: list[str] = []
    visiting: set[str] = set()
    visited: set[str] = set()
    def visit(module_id: str) -> None:
        if module_id in visiting:
            raise ManifestError(f"Dependency cycle detected at {module_id}")
        if module_id in visited:
            return
        visiting.add(module_id)
        for dep in modules[module_id]["manifest"].get("dependencies", []):
            dep_id = dep.get("id")
            if dep_id in chosen:
                visit(dep_id)
        visiting.remove(module_id)
        visited.add(module_id)
        order.append(module_id)
    for module_id in sorted(chosen):
        visit(module_id)

    return {
        "product": package["display_name"],
        "release_version": package["release_version"],
        "framework_version": framework["version"],
        "selected_modules": order,
        "owned_paths": {module_id: modules[module_id]["owned"] for module_id in order},
        "user_data_preserved": True,
    }

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", nargs="?", type=Path, default=Path(__file__).resolve().parents[1] / "package.example.json")
    parser.add_argument("--select", action="append", default=None, help="Module ID to select; may be repeated")
    args = parser.parse_args()
    try:
        plan = validate_package(args.package, set(args.select) if args.select is not None else None)
    except ManifestError as exc:
        print(f"INVALID: {exc}", file=sys.stderr)
        return 1
    print(json.dumps(plan, indent=2))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
