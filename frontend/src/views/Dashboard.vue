<template>
  <v-container fluid class="pa-6 dashboard-bg">

    <!-- ===== STATS ROW 1 ===== -->
    <v-row dense class="mb-3">

      <!-- TEMPERATURE -->
      <v-col cols="12" sm="6" md="3">
        <v-card class="stat-card" style="border-top: 3px solid #ff6b6b;">
          <v-card-text class="stat-body">
            <div class="stat-label">🌡 TEMPERATURE</div>
            <div class="stat-value" style="color:#ff6b6b;">
              {{ safe(convertTemp(latest.temp)) }}
              <span class="stat-unit">°{{ units.temp }}</span>
            </div>
            <div class="unit-buttons">
              <v-btn size="x-small" variant="outlined" :color="units.temp==='C'?'#ff6b6b':'#4a6880'" @click="units.temp='C'">°C</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.temp==='F'?'#ff6b6b':'#4a6880'" @click="units.temp='F'">°F</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.temp==='K'?'#ff6b6b':'#4a6880'" @click="units.temp='K'">K</v-btn>
            </div>
            <div class="stat-condition">{{ tempCondition(latest.temp) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- HUMIDITY -->
      <v-col cols="12" sm="6" md="3">
        <v-card class="stat-card" style="border-top: 3px solid #00d4ff;">
          <v-card-text class="stat-body">
            <div class="stat-label">💧 HUMIDITY</div>
            <div class="stat-value" style="color:#00d4ff;">
              {{ safe(latest.hum) }}<span class="stat-unit">%</span>
            </div>
            <div class="unit-buttons unit-buttons--hidden"></div>
            <div class="stat-condition">{{ humCondition(latest.hum) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- HEAT INDEX -->
      <v-col cols="12" sm="6" md="3">
        <v-card class="stat-card" style="border-top: 3px solid #ffb700;">
          <v-card-text class="stat-body">
            <div class="stat-label">🔥 HEAT INDEX</div>
            <div class="stat-value" style="color:#ffb700;">
              {{ safe(convertTemp(latest.heatIndex)) }}
              <span class="stat-unit">°{{ units.temp }}</span>
            </div>
            <div class="unit-buttons">
              <v-btn size="x-small" variant="outlined" :color="units.temp==='C'?'#ffb700':'#4a6880'" @click="units.temp='C'">°C</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.temp==='F'?'#ffb700':'#4a6880'" @click="units.temp='F'">°F</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.temp==='K'?'#ffb700':'#4a6880'" @click="units.temp='K'">K</v-btn>
            </div>
            <div class="stat-condition">{{ tempCondition(latest.heatIndex) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- SOIL MOISTURE -->
      <v-col cols="12" sm="6" md="3">
        <v-card class="stat-card" style="border-top: 3px solid #00ff9d;">
          <v-card-text class="stat-body">
            <div class="stat-label">🌱 SOIL MOISTURE</div>
            <div class="stat-value" style="color:#00ff9d;">
              {{ safe(latest.soil) }}<span class="stat-unit">%</span>
            </div>
            <div class="unit-buttons unit-buttons--hidden"></div>
            <div class="stat-condition">{{ soilCondition(latest.soil) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

    </v-row>

    <!-- ===== STATS ROW 2 ===== -->
    <v-row dense class="mb-4">

      <!-- PRESSURE -->
      <v-col cols="12" sm="6">
        <v-card class="stat-card" style="border-top: 3px solid #00bcd4;">
          <v-card-text class="stat-body">
            <div class="stat-label">🌡️ PRESSURE</div>
            <div class="stat-value" style="color:#00bcd4;">
              {{ safe(convertPressure(latest.pres), 2) }}
              <span class="stat-unit">{{ units.pressure }}</span>
            </div>
            <div class="unit-buttons">
              <v-btn size="x-small" variant="outlined" :color="units.pressure==='hPa'?'#00bcd4':'#4a6880'" @click="units.pressure='hPa'">hPa</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.pressure==='atm'?'#00bcd4':'#4a6880'" @click="units.pressure='atm'">atm</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.pressure==='mmHg'?'#00bcd4':'#4a6880'" @click="units.pressure='mmHg'">mmHg</v-btn>
            </div>
            <div class="stat-condition">{{ presCondition(latest.pres) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- ALTITUDE -->
      <v-col cols="12" sm="6">
        <v-card class="stat-card" style="border-top: 3px solid #ce93d8;">
          <v-card-text class="stat-body">
            <div class="stat-label">⛰️ ALTITUDE</div>
            <div class="stat-value" style="color:#ce93d8;">
              {{ safe(convertAltitude(latest.altitude)) }}
              <span class="stat-unit">{{ units.altitude }}</span>
            </div>
            <div class="unit-buttons">
              <v-btn size="x-small" variant="outlined" :color="units.altitude==='m'?'#ce93d8':'#4a6880'" @click="units.altitude='m'">m</v-btn>
              <v-btn size="x-small" variant="outlined" :color="units.altitude==='ft'?'#ce93d8':'#4a6880'" @click="units.altitude='ft'">ft</v-btn>
            </div>
            <div class="stat-condition">{{ altCondition(latest.altitude) }}</div>
          </v-card-text>
        </v-card>
      </v-col>

    </v-row>

    <!-- ===== CHARTS ===== -->
    <v-row>
      <v-col cols="12" md="6">
        <v-card class="chart-card pa-4">
          <div class="chart-title">TEMPERATURE &amp; HUMIDITY</div>
          <div class="chart-wrapper">
            <canvas ref="chartTH"></canvas>
          </div>
        </v-card>
      </v-col>

      <v-col cols="12" md="6">
        <v-card class="chart-card pa-4">
          <div class="chart-title">PRESSURE &amp; SOIL MOISTURE</div>
          <div class="chart-wrapper">
            <canvas ref="chartPS"></canvas>
          </div>
        </v-card>
      </v-col>
    </v-row>

    <!-- ===== DATA TABLE ===== -->
    <v-row class="mt-4">
      <v-col cols="12">
        <v-card class="chart-card pa-4">
          <div class="table-header">
            <span class="chart-title" style="margin-bottom:0;">📋 LIVE DATA LOG</span>
            <div class="table-controls">
              <span class="table-meta">{{ tableData.length }} / {{ MAX_HISTORY }} readings</span>
              <v-btn size="x-small" variant="outlined" color="#4a6880" @click="rawData=[]">CLEAR</v-btn>
            </div>
          </div>
          <div class="table-scroll-wrapper">
            <table class="data-table">
              <thead>
                <tr>
                  <th>#</th>
                  <th>TIME</th>
                  <th>🌡 TEMP (°{{ units.temp }})</th>
                  <th>💧 HUMIDITY (%)</th>
                  <th>🔥 HEAT INDEX (°{{ units.temp }})</th>
                  <th>🌱 SOIL (%)</th>
                  <th>🌡️ PRESSURE ({{ units.pressure }})</th>
                  <th>⛰️ ALTITUDE ({{ units.altitude }})</th>
                  <th>STATUS</th>
                </tr>
              </thead>
              <tbody>
                <tr
                  v-for="(row, i) in tableData"
                  :key="i"
                  :class="{ 'row-latest': i === tableData.length - 1 }"
                >
                  <td class="td-index">{{ i + 1 }}</td>
                  <td class="td-time">{{ row.label }}</td>
                  <td :style="{ color: tempColor(row.temp) }">{{ safe(convertTemp(row.temp)) }}</td>
                  <td :style="{ color: humColor(row.hum) }">{{ safe(row.hum) }}</td>
                  <td :style="{ color: tempColor(row.heatIndex) }">{{ safe(convertTemp(row.heatIndex)) }}</td>
                  <td :style="{ color: soilColor(row.soil) }">{{ safe(row.soil) }}</td>
                  <td :style="{ color: presColor(row.pres) }">{{ safe(convertPressure(row.pres), 2) }}</td>
                  <td :style="{ color: '#ce93d8' }">{{ safe(convertAltitude(row.altitude)) }}</td>
                  <td>
                    <span class="status-badge" :style="{ background: rowStatusBg(row), color: rowStatusColor(row) }">
                      {{ rowStatus(row) }}
                    </span>
                  </td>
                </tr>
                <tr v-if="tableData.length === 0">
                  <td colspan="9" class="td-empty">Waiting for data…</td>
                </tr>
              </tbody>
            </table>
          </div>
        </v-card>
      </v-col>
    </v-row>

    <!-- ===== CONSOLE LOG ===== -->
    <v-row class="mt-4">
      <v-col cols="12">
        <v-card class="chart-card pa-4">
          <div class="console-header">
            <span class="chart-title" style="margin-bottom:0;">⚙ CONSOLE LOG</span>
            <v-btn size="x-small" variant="outlined" color="#4a6880" @click="logs=[]">CLEAR</v-btn>
          </div>
          <div ref="logContainer" class="console-panel">
            <div v-if="logs.length === 0" class="log-empty">Waiting for data…</div>
            <div v-for="(entry, i) in logs" :key="i" class="log-line">
              <span class="log-time">{{ entry.time }}</span>
              <span :class="['log-level', 'log-' + entry.level]">{{ entry.level.toUpperCase() }}</span>
              <span class="log-msg">{{ entry.msg }}</span>
            </div>
          </div>
        </v-card>
      </v-col>
    </v-row>

  </v-container>
</template>

<script setup>
import { ref, computed, watch, onMounted, onUnmounted, nextTick } from 'vue';
import Chart from 'chart.js/auto';

const API_BASE = 'http://10.229.210.105:8080';
const MAX_HISTORY = 60;

const latest  = ref({});
const rawData = ref([]);
const logs    = ref([]);
const logContainer = ref(null);

const chartTH = ref(null);
const chartPS = ref(null);

let chartTHInstance, chartPSInstance, interval;

/* ===== UNITS ===== */
const units = ref({ temp: 'C', pressure: 'hPa', altitude: 'm' });

/* ===== TABLE DATA (computed so unit toggles re-render instantly) ===== */
const tableData = computed(() => rawData.value);

/* ===== LOGGER ===== */
function addLog(level, msg) {
  const time = new Date().toLocaleTimeString();
  logs.value.push({ time, level, msg });
  if (logs.value.length > 200) logs.value.shift();
  nextTick(() => {
    if (logContainer.value) logContainer.value.scrollTop = logContainer.value.scrollHeight;
  });
}

/* ===== SAFE DISPLAY ===== */
function safe(val, dp = 1) {
  return val != null ? Number(val).toFixed(dp) : '--';
}

/* ===== CONVERSIONS ===== */
function convertTemp(t) {
  if (t == null) return null;
  if (units.value.temp === 'F') return t * 9 / 5 + 32;
  if (units.value.temp === 'K') return t + 273.15;
  return t;
}
function convertPressure(p) {
  if (p == null) return null;
  if (units.value.pressure === 'atm')  return p / 1013.25;
  if (units.value.pressure === 'mmHg') return p * 0.750062;
  return p;
}
function convertAltitude(a) {
  if (a == null) return null;
  if (units.value.altitude === 'ft') return a * 3.28084;
  return a;
}

/* ===== CONDITIONS ===== */
function tempCondition(t) { if (t == null) return '—'; if (t < 10) return '❄ VERY COLD'; if (t < 30) return '✅ NORMAL'; return '🔥 HOT'; }
function humCondition(h)  { if (h == null) return '—'; if (h < 40) return '🏜 DRY'; if (h < 70) return '✅ NORMAL'; return '💦 HUMID'; }
function presCondition(p) { if (p == null) return '—'; if (p < 1000) return '⬇ LOW'; if (p > 1020) return '⬆ HIGH'; return '✅ NORMAL'; }
function soilCondition(s) { if (s == null) return '—'; if (s < 30) return '🏜 DRY'; if (s < 70) return '✅ NORMAL'; return '💧 WET'; }
function altCondition(a)  { if (a == null) return '—'; if (a < 200) return '🌊 LOW'; if (a < 1000) return '🏔 MID'; return '⛰ HIGH'; }

/* ===== TABLE CELL COLORS ===== */
function tempColor(t)  { if (t == null) return '#4a6880'; if (t < 10) return '#00d4ff'; if (t < 30) return '#00ff9d'; return '#ff6b6b'; }
function humColor(h)   { if (h == null) return '#4a6880'; if (h < 40) return '#ffb700'; if (h < 70) return '#00ff9d'; return '#00d4ff'; }
function presColor(p)  { if (p == null) return '#4a6880'; if (p < 1000) return '#00d4ff'; if (p > 1020) return '#ffb700'; return '#00ff9d'; }
function soilColor(s)  { if (s == null) return '#4a6880'; if (s < 30) return '#ffb700'; if (s < 70) return '#00ff9d'; return '#00d4ff'; }

/* ===== ROW STATUS ===== */
function rowStatus(row) {
  const issues = [];
  if (row.temp != null && row.temp >= 30) issues.push('HOT');
  if (row.temp != null && row.temp < 10)  issues.push('COLD');
  if (row.hum  != null && row.hum  >= 70) issues.push('HUMID');
  if (row.hum  != null && row.hum  < 40)  issues.push('DRY');
  if (row.soil != null && row.soil < 30)  issues.push('LOW SOIL');
  if (row.pres != null && row.pres < 1000) issues.push('LOW PRES');
  if (row.pres != null && row.pres > 1020) issues.push('HIGH PRES');
  return issues.length ? issues[0] : 'OK';
}
function rowStatusBg(row)    { return rowStatus(row) === 'OK' ? 'rgba(0,255,157,0.12)' : 'rgba(255,107,107,0.12)'; }
function rowStatusColor(row) { return rowStatus(row) === 'OK' ? '#00ff9d' : '#ff6b6b'; }

/* ===== CHART INIT ===== */
function makeOptions() {
  return {
    responsive: true,
    maintainAspectRatio: false,
    animation: { duration: 300 },
    interaction: { mode: 'index', intersect: false },
    plugins: {
      legend: {
        display: true,
        labels: { color: '#8ab4cc', font: { family: 'Courier New', size: 11 }, boxWidth: 12 }
      },
      tooltip: {
        backgroundColor: '#0a1929',
        titleColor: '#8ab4cc',
        bodyColor: '#cdd9e5',
        borderColor: '#1a3048',
        borderWidth: 1
      }
    },
    scales: {
      x:  { ticks: { color: '#4a6880', font: { family: 'Courier New', size: 10 }, maxTicksLimit: 8 }, grid: { color: 'rgba(26,48,72,0.6)' } },
      y:  { ticks: { color: '#4a6880', font: { family: 'Courier New', size: 10 } }, grid: { color: 'rgba(26,48,72,0.6)' } },
      y1: { position: 'right', ticks: { color: '#4a6880', font: { family: 'Courier New', size: 10 } }, grid: { drawOnChartArea: false } }
    }
  };
}

function initCharts() {
  chartTHInstance = new Chart(chartTH.value, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Temperature (°C)', data: [], borderColor: '#ff6b6b', backgroundColor: 'rgba(255,107,107,0.08)', borderWidth: 2, pointRadius: 2, tension: 0.3, fill: true, yAxisID: 'y' },
        { label: 'Humidity (%)',     data: [], borderColor: '#00d4ff', backgroundColor: 'rgba(0,212,255,0.08)',   borderWidth: 2, pointRadius: 2, tension: 0.3, fill: true, yAxisID: 'y1' }
      ]
    },
    options: makeOptions()
  });

  chartPSInstance = new Chart(chartPS.value, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        { label: 'Pressure (hPa)',    data: [], borderColor: '#00bcd4', backgroundColor: 'rgba(0,188,212,0.08)',  borderWidth: 2, pointRadius: 2, tension: 0.3, fill: true, yAxisID: 'y' },
        { label: 'Soil Moisture (%)', data: [], borderColor: '#00ff9d', backgroundColor: 'rgba(0,255,157,0.08)', borderWidth: 2, pointRadius: 2, tension: 0.3, fill: true, yAxisID: 'y1' }
      ]
    },
    options: makeOptions()
  });
}

/* ===== UPDATE CHARTS ===== */
function updateCharts(data) {
  if (!data || !data.length) return;

  const labels = data.map(d => d.label);
  const temp   = data.map(d => convertTemp(d.temp));
  const pres   = data.map(d => convertPressure(d.pres));
  const hum    = data.map(d => d.hum);
  const soil   = data.map(d => d.soil);

  chartTHInstance.data.datasets[0].label = `Temperature (°${units.value.temp})`;
  chartTHInstance.data.datasets[1].label = 'Humidity (%)';
  chartTHInstance.data.labels            = labels;
  chartTHInstance.data.datasets[0].data  = temp;
  chartTHInstance.data.datasets[1].data  = hum;

  chartPSInstance.data.datasets[0].label = `Pressure (${units.value.pressure})`;
  chartPSInstance.data.datasets[1].label = 'Soil Moisture (%)';
  chartPSInstance.data.labels            = labels;
  chartPSInstance.data.datasets[0].data  = pres;
  chartPSInstance.data.datasets[1].data  = soil;

  chartTHInstance.update();
  chartPSInstance.update();
}

/* ===== FETCH ===== */
async function fetchData() {
  addLog('info', `Fetching ${API_BASE}/api/weather/latest …`);
  try {
    const res  = await fetch(`${API_BASE}/api/weather/latest`);
    const json = await res.json();
    const data = json.data || json;
    latest.value = data;

    addLog('success', `OK — temp:${data.temp} hum:${data.hum} pres:${data.pres} soil:${data.soil} alt:${data.altitude}`);

    rawData.value.push({
      label:     new Date().toLocaleTimeString(),
      temp:      data.temp,
      hum:       data.hum,
      heatIndex: data.heatIndex,
      pres:      data.pres,
      soil:      data.soil,
      altitude:  data.altitude
    });
    if (rawData.value.length > MAX_HISTORY) rawData.value.shift();

    updateCharts(rawData.value);
  } catch (err) {
    addLog('error', `FETCH FAILED — ${err.message}`);
  }
}

watch(units, () => {
  if (rawData.value.length) updateCharts(rawData.value);
}, { deep: true });

onMounted(() => {
  initCharts();
  fetchData();
  interval = setInterval(fetchData, 5000);
});

onUnmounted(() => clearInterval(interval));
</script>

<style scoped>
.dashboard-bg {
  min-height: 100vh;
  background: #070f18;
}

/* ===== STAT CARDS ===== */
.stat-card {
  background: #0f1e2e !important;
  border: 1px solid #1a3048;
  border-radius: 12px;
  height: 100%;
}

.stat-body {
  display: flex;
  flex-direction: column;
  height: 160px;
  padding: 16px !important;
}

.stat-label {
  font-size: 0.68rem;
  letter-spacing: 0.14em;
  color: #4a6880;
  text-transform: uppercase;
  font-family: 'Courier New', monospace;
  margin-bottom: 4px;
}

.stat-value {
  font-family: 'Courier New', monospace;
  font-size: 2rem;
  font-weight: 700;
  line-height: 1;
  letter-spacing: -0.02em;
}

.stat-unit {
  font-size: 1rem;
  opacity: 0.8;
}

.unit-buttons {
  margin-top: 8px;
  display: flex;
  gap: 4px;
  height: 26px;
}

.unit-buttons--hidden {
  visibility: hidden;
}

.stat-condition {
  margin-top: auto;
  font-size: 0.72rem;
  letter-spacing: 0.1em;
  color: #4a6880;
  text-transform: uppercase;
  font-family: 'Courier New', monospace;
}

/* ===== CHART CARDS ===== */
.chart-card {
  background: #0f1e2e !important;
  border: 1px solid #1a3048;
  border-radius: 12px;
}

.chart-title {
  font-size: 0.68rem;
  letter-spacing: 0.14em;
  color: #4a6880;
  text-transform: uppercase;
  font-family: 'Courier New', monospace;
  margin-bottom: 12px;
}

.chart-wrapper {
  position: relative;
  height: 260px;
}

/* ===== DATA TABLE ===== */
.table-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.table-controls {
  display: flex;
  align-items: center;
  gap: 12px;
}

.table-meta {
  font-size: 0.68rem;
  font-family: 'Courier New', monospace;
  color: #2a4a60;
  letter-spacing: 0.08em;
}

.table-scroll-wrapper {
  overflow-x: auto;
  overflow-y: auto;
  max-height: 320px;
  border: 1px solid #1a3048;
  border-radius: 8px;
  scrollbar-width: thin;
  scrollbar-color: #1a3048 transparent;
}

.data-table {
  width: 100%;
  border-collapse: collapse;
  font-family: 'Courier New', monospace;
  font-size: 0.72rem;
  white-space: nowrap;
}

.data-table thead {
  position: sticky;
  top: 0;
  z-index: 1;
  background: #0a1520;
}

.data-table thead th {
  padding: 8px 14px;
  text-align: left;
  color: #4a6880;
  letter-spacing: 0.1em;
  text-transform: uppercase;
  font-size: 0.65rem;
  font-weight: 600;
  border-bottom: 1px solid #1a3048;
}

.data-table tbody tr {
  border-bottom: 1px solid rgba(26, 48, 72, 0.4);
  transition: background 0.15s;
}

.data-table tbody tr:hover {
  background: rgba(26, 48, 72, 0.35);
}

.data-table tbody tr.row-latest {
  background: rgba(0, 255, 157, 0.04);
}

.data-table tbody td {
  padding: 6px 14px;
  color: #8ab4cc;
  vertical-align: middle;
}

.td-index {
  color: #2a4a60 !important;
  font-size: 0.65rem;
  min-width: 30px;
}

.td-time {
  color: #2a5a7a !important;
  min-width: 80px;
}

.td-empty {
  text-align: center;
  color: #2a4a60;
  font-style: italic;
  padding: 24px !important;
}

.status-badge {
  display: inline-block;
  padding: 2px 8px;
  border-radius: 4px;
  font-size: 0.62rem;
  font-weight: 700;
  letter-spacing: 0.1em;
  text-transform: uppercase;
}

/* ===== CONSOLE LOG ===== */
.console-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.console-panel {
  background: #060d16;
  border: 1px solid #1a3048;
  border-radius: 8px;
  height: 200px;
  overflow-y: auto;
  padding: 10px 12px;
  font-family: 'Courier New', monospace;
  font-size: 0.72rem;
  scrollbar-width: thin;
  scrollbar-color: #1a3048 transparent;
}

.log-empty {
  color: #2a4a60;
  font-style: italic;
}

.log-line {
  display: flex;
  gap: 10px;
  padding: 2px 0;
  border-bottom: 1px solid rgba(26, 48, 72, 0.3);
  line-height: 1.5;
}

.log-time {
  color: #2a5a7a;
  min-width: 80px;
  flex-shrink: 0;
}

.log-level {
  min-width: 60px;
  flex-shrink: 0;
  font-weight: 700;
}

.log-info    { color: #4a6880; }
.log-success { color: #00ff9d; }
.log-error   { color: #ff6b6b; }
.log-warn    { color: #ffb700; }
.log-msg {
  color: #8ab4cc;
  word-break: break-all;
}
</style>