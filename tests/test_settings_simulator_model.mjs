import assert from "node:assert/strict";
import { readFileSync } from "node:fs";

const model = JSON.parse(readFileSync("data/settings-simulator.json", "utf8"));
const html = readFileSync("site/index.html", "utf8");
const app = readFileSync("site/app.js", "utf8");
const css = readFileSync("site/styles.css", "utf8");

assert.equal(model.schemaVersion, 1);
assert.equal(model.navigation.viewport.width, 960);
assert.equal(model.navigation.viewport.height, 544);
assert.equal(model.safety.mode, "simulation-only");
assert.equal(model.safety.hardwareWrites, false);
assert.equal(model.safety.pluginActivation, false);
assert.equal(model.safety.storageWrites, false);
assert.ok(model.exclusiveGroups.some(g => g.members.includes("YAMT") && g.members.includes("StorageMgr")));
assert.ok(model.exclusiveGroups.some(g => g.members.includes("PSVshell") && g.members.includes("LOLIcon")));
assert.match(html, /id="screen"/);
assert.match(html, /id="content"/);
assert.match(html, /id="back"/);
assert.match(app, /addEventListener\("keydown"/);
assert.match(app, /data-mode/);
assert.match(css, /aspect-ratio:960\/544/);
console.log("Settings simulator model and static UI contract: PASS");
