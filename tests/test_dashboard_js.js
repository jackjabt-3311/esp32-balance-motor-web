"use strict";

const assert = require("assert");
const fs = require("fs");

const html = fs.readFileSync("BalanceController/data/index.html", "utf8");
const start = html.indexOf("<script>") + 8;
const end = html.lastIndexOf("</script>");
assert.ok(start > 7 && end > start, "dashboard must contain one inline script");
const script = html.slice(start, end);

// The production change that should fail these checks is removing fail-safe
// recovery or allowing a post-command refresh to reuse a stale poll.
assert.doesNotThrow(() => new Function(script), "dashboard JavaScript must compile");
assert.match(script, /const DATA_TIMEOUT_MS = 1000;/, "data requests need a bounded timeout");
assert.match(script, /new AbortController\(\)/, "data requests need an abort controller");
assert.match(script, /signal: controller\.signal/, "fetch must receive the abort signal");
assert.match(script, /if \(generation === dataGeneration\) render\(body, \{ reconcileRpm \}\)/, "stale data must not render");
assert.match(script, /setNormalControlsDisabled\(true\); showMessage/, "data failure must disable normal controls");
assert.match(script, /elements\.estopButton\.disabled = false/, "ESTOP must remain reachable");
assert.match(script, /refreshData\(\{ force: true, reconcileRpm: endpoint === "\/motor\/rpm" \}\)/, "commands need a new data generation");
assert.match(script, /if \(!rpmInteracting \|\| reconcileRpm\)/, "RPM response must reconcile despite focus");

// If an armed normal command completes after a newer ESTOP, it must not render
// the old cached snapshot and re-enable normal controls before fresh /data.
assert.match(script, /function invalidateDataRequest\(\)\s*\{[\s\S]*latestData = null;/, "forced refresh must clear cached authority");
assert.doesNotMatch(script, /finally \{[\s\S]*refreshData\([^)]*\)[\s\S]*if \(latestData\) render\(latestData\);/, "command completion must not re-render cached state");
assert.match(script, /pendingCommands -= 1; commandBusy = pendingCommands > 0; await refreshData\(\{ force: true/, "overlapping command completion must await a forced refresh");

class Deferred {
  constructor() {
    this.promise = new Promise((resolve, reject) => { this.resolve = resolve; this.reject = reject; });
  }
}

class MockElement {
  constructor() {
    this.disabled = false;
    this.value = "0";
    this.textContent = "";
    this.style = {};
    this.files = [];
    this.handlers = {};
    this.classList = { toggle() {} };
  }

  addEventListener(type, handler) { this.handlers[type] = handler; }
  click() { this.handlers.click(); }
}

async function flush() {
  for (let i = 0; i < 8; i += 1) await Promise.resolve();
}

async function exerciseActualCommandRace() {
  const ids = ["helmetStatus", "peopleCount", "motorAllowed", "lockReason", "lc1", "lc2", "lc3", "lc4", "dot", "motorToggle", "rpmSlider", "targetRpm", "actualRpm", "motorState", "estopButton", "faultReset", "calibrationStart", "calibrationFinish", "htmlUpload", "uploadButton", "commandMessage", "rpmProposed", "motorDetail"];
  const elements = Object.fromEntries(ids.map((id) => [id, new MockElement()]));
  const requests = [];
  const previous = { document: global.document, window: global.window, fetch: global.fetch, FormData: global.FormData };
  global.document = { getElementById: (id) => elements[id], activeElement: null };
  global.window = {
    setTimeout: () => 1,
    clearTimeout: () => {},
    location: { replace() {} },
  };
  global.FormData = class { append() {} };
  global.fetch = (url, options = {}) => {
    const deferred = new Deferred();
    requests.push({ url, options, deferred });
    if (options.signal) options.signal.addEventListener("abort", () => deferred.reject(new Error("AbortError")), { once: true });
    return deferred.promise;
  };
  const respond = (request, body) => request.deferred.resolve({ ok: true, json: async () => body });
  const armed = { helmet: "worn", people: 1, motorAllowed: true, lockReason: "ready", motorState: "armed", targetRpm: 0, actualRpm: 0, pwmPercent: 0, pulsesPerRev: 10, fault: "", lc1: 1, lc2: 1, lc3: 1, lc4: 1, x: 0, y: 0 };
  const ready = { ...armed, motorState: "ready" };
  try {
    new Function(script)();
    assert.equal(requests[0].url, "/data", "initial poll must request data");
    respond(requests[0], armed);
    await flush();
    assert.equal(elements.motorToggle.disabled, false, "armed state allows normal OFF before commands");

    elements.motorToggle.click();
    elements.estopButton.click();
    assert.equal(requests[1].url, "/motor/off");
    assert.equal(requests[2].url, "/motor/estop");
    assert.equal(elements.motorToggle.disabled, true);
    assert.equal(elements.rpmSlider.disabled, true);
    assert.equal(elements.estopButton.disabled, false);

    respond(requests[2], { ok: true }); // newer ESTOP completes first
    await flush();
    assert.equal(requests[3].url, "/data", "ESTOP completion forces data refresh");
    assert.equal(elements.motorToggle.disabled, true);
    assert.equal(elements.estopButton.disabled, false);

    respond(requests[1], { ok: true }); // older normal completion arrives last
    await flush();
    assert.equal(requests[4].url, "/data", "older completion starts a newer data generation");
    assert.equal(elements.motorToggle.disabled, true, "cached armed state must not re-enable normal control");
    assert.equal(elements.rpmSlider.disabled, true);
    assert.equal(elements.faultReset.disabled, true);
    assert.equal(elements.calibrationStart.disabled, true);
    assert.equal(elements.calibrationFinish.disabled, true);
    assert.equal(elements.estopButton.disabled, false, "ESTOP remains available during reconciliation");

    respond(requests[4], ready);
    await flush();
    assert.equal(elements.motorToggle.disabled, false, "only newest matching data may re-enable normal control");
  } finally {
    global.document = previous.document;
    global.window = previous.window;
    global.fetch = previous.fetch;
    global.FormData = previous.FormData;
  }
}

exerciseActualCommandRace().then(() => console.log("dashboard JavaScript contract OK")).catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
