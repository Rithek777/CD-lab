const sampleSelect = document.querySelector("#sampleSelect");
const loadSampleButton = document.querySelector("#loadSampleButton");
const analyzeButton = document.querySelector("#analyzeButton");
const mlirInput = document.querySelector("#mlirInput");
const formatSelect = document.querySelector("#formatSelect");
const thresholdInput = document.querySelector("#thresholdInput");
const showIrInput = document.querySelector("#showIrInput");
const runStatus = document.querySelector("#runStatus");
const inputMeta = document.querySelector("#inputMeta");
const reportMeta = document.querySelector("#reportMeta");
const metrics = document.querySelector("#metrics");
const allocations = document.querySelector("#allocations");
const rawReport = document.querySelector("#rawReport");
const resultPane = document.querySelector(".resultPane");

function setStatus(text) {
  runStatus.textContent = text;
}

function updateInputMeta() {
  const lines = mlirInput.value ? mlirInput.value.split(/\r?\n/).length : 0;
  inputMeta.textContent = `${lines} lines`;
}

async function fetchJson(url, options) {
  const response = await fetch(url, options);
  const payload = await response.json();
  if (!response.ok) {
    throw new Error(payload.error || "request failed");
  }
  return payload;
}

async function loadSamples() {
  const payload = await fetchJson("/api/samples");
  sampleSelect.innerHTML = "";
  for (const sample of payload.samples) {
    const option = document.createElement("option");
    option.value = sample;
    option.textContent = sample;
    sampleSelect.appendChild(option);
  }
  if (payload.samples.includes("simple_alloc.mlir")) {
    sampleSelect.value = "simple_alloc.mlir";
  }
  await loadSelectedSample();
}

async function loadSelectedSample() {
  const sample = sampleSelect.value;
  const payload = await fetchJson(`/api/sample?name=${encodeURIComponent(sample)}`);
  mlirInput.value = payload.content;
  updateInputMeta();
  setStatus("Loaded");
}

function metric(label, value) {
  return `<div class="metric"><b>${value}</b><span>${label}</span></div>`;
}

function classNameFor(classification) {
  if (classification === "NECESSARY") return "necessary";
  if (classification === "POSSIBLY_UNNECESSARY") return "possible";
  return "";
}

function renderJsonReport(report, raw) {
  resultPane.classList.remove("showRaw");
  const items = report.allocations || [];
  const totalMb = items.reduce((sum, item) => sum + (item.estimated_mb || 0), 0);
  metrics.innerHTML = [
    metric("allocations", items.length),
    metric("estimated MB", totalMb.toFixed(2)),
    metric("threshold MB", Number(report.threshold_mb || 0).toFixed(2)),
  ].join("");

  allocations.innerHTML = items.map((item, index) => {
    const classification = item.classification || "UNKNOWN";
    return `
      <article class="allocation ${classNameFor(classification)}">
        <h3>#${index + 1} ${classification}</h3>
        <dl class="kv">
          <dt>Source</dt><dd>${item.source_file}:${item.source_line}:${item.source_column}</dd>
          <dt>IR operation</dt><dd>${item.ir_operation_name}</dd>
          <dt>Size</dt><dd>${item.estimated_bytes} bytes / ${item.estimated_mb ?? "unknown"} MB</dd>
          <dt>Reason</dt><dd>${item.reason}</dd>
          <dt>Suggestion</dt><dd>${item.suggested_transformation}</dd>
          ${item.ir ? `<dt>IR</dt><dd><code>${escapeHtml(item.ir)}</code></dd>` : ""}
        </dl>
      </article>
    `;
  }).join("");

  rawReport.textContent = raw;
  reportMeta.textContent = `${items.length} allocation${items.length === 1 ? "" : "s"}`;
}

function renderTextReport(text) {
  resultPane.classList.add("showRaw");
  rawReport.textContent = text;
  reportMeta.textContent = "Text report";
}

function escapeHtml(text) {
  return String(text)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

async function analyze() {
  setStatus("Running");
  analyzeButton.disabled = true;
  try {
    const format = formatSelect.value;
    const payload = await fetchJson("/api/analyze", {
      method: "POST",
      headers: {"Content-Type": "application/json"},
      body: JSON.stringify({
        mlir: mlirInput.value,
        format,
        thresholdMb: Number(thresholdInput.value || 0),
        showIr: showIrInput.checked,
      }),
    });
    if (format === "json" && payload.report) {
      renderJsonReport(payload.report, payload.stdout);
    } else {
      renderTextReport(payload.stdout || payload.stderr);
    }
    setStatus("Complete");
  } catch (error) {
    resultPane.classList.add("showRaw");
    rawReport.textContent = error.message;
    reportMeta.textContent = "Error";
    setStatus("Error");
  } finally {
    analyzeButton.disabled = false;
  }
}

loadSampleButton.addEventListener("click", loadSelectedSample);
analyzeButton.addEventListener("click", analyze);
mlirInput.addEventListener("input", updateInputMeta);

loadSamples().catch((error) => {
  setStatus("Error");
  rawReport.textContent = error.message;
  resultPane.classList.add("showRaw");
});

