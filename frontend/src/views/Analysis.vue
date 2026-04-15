<template>
  <v-container fluid class="pa-6">
    <v-row>
      <v-col cols="12" md="4">
        <v-card class="pa-4 ctrl-card">
          <div class="section-label">Range Selection</div>
          <div class="date-row">
            <label class="date-label">From</label>
            <input type="date" v-model="start" class="date-input" />
          </div>
          <div class="date-row mt-2">
            <label class="date-label">To</label>
            <input type="date" v-model="end" class="date-input" />
          </div>
          <v-btn block color="cyan" variant="outlined" @click="getHistory" :loading="loading" class="mt-4">
            Run Analysis
          </v-btn>
          <div v-if="dataRows.length" class="stats-summary mt-4">
            <div class="summary-title">SUMMARY</div>
            <div class="summary-row"><span>Avg Temp</span><span style="color:#ff6b6b">{{ avgTemp }}°C</span></div>
            <div class="summary-row"><span>Avg Humidity</span><span style="color:#00d4ff">{{ avgHum }}%</span></div>
            <div class="summary-row"><span>Avg Pressure</span><span style="color:#00ff9d">{{ avgPres }} hPa</span></div>
            <div class="summary-row"><span>Records</span><span style="color:#ffb700">{{ dataRows.length }}</span></div>
          </div>
        </v-card>
      </v-col>

      <v-col cols="12" md="8">
        <v-card class="pa-4 chart-card">
          <div class="chart-title">TEMPERATURE OVER RANGE</div>
          <div style="position:relative; height:220px">
            <canvas ref="tempChart"></canvas>
          </div>
        </v-card>
        <v-card class="pa-4 chart-card mt-4">
          <div class="chart-title">HUMIDITY & PRESSURE OVER RANGE</div>
          <div style="position:relative; height:220px">
            <canvas ref="humPresChart"></canvas>
          </div>
        </v-card>
      </v-col>
    </v-row>

    <v-card class="mt-4 table-card">
      <v-card-title class="section-label pa-4">HISTORICAL DATA</v-card-title>
      <div class="table-wrap">
        <table>
          <thead>
            <tr>
              <th>Time</th>
              <th>Temp (°C)</th>
              <th>Humidity (%)</th>
              <th>Pressure (hPa)</th>
              <th>Soil (%)</th>
            </tr>
          </thead>
          <tbody>
            <tr v-if="dataRows.length === 0">
              <td colspan="5" style="text-align:center; color:#4a6880; padding:24px;">
                No data — select a date range and click Run Analysis
              </td>
            </tr>
            <tr v-for="row in dataRows" :key="row.timestamp">
              <td class="time-val">{{ new Date(row.timestamp * 1000).toLocaleString() }}</td>
              <td class="temp-val">{{ row.temp?.toFixed(1) }}</td>
              <td class="hum-val">{{ row.hum?.toFixed(1) }}</td>
              <td class="pres-val">{{ row.pres?.toFixed(1) }}</td>
              <td class="soil-val">{{ row.soil }}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref, computed, onUnmounted } from 'vue';
import Chart from 'chart.js/auto';

const API_BASE = 'http://10.194.108.105:8080';
const start = ref('');
const end = ref('');
const dataRows = ref([]);
const loading = ref(false);
const tempChart = ref(null);
const humPresChart = ref(null);
let chartT = null;
let chartHP = null;

const avgTemp = computed(() => dataRows.value.length ? (dataRows.value.reduce((s, r) => s + r.temp, 0) / dataRows.value.length).toFixed(1) : '--');
const avgHum  = computed(() => dataRows.value.length ? (dataRows.value.reduce((s, r) => s + r.hum,  0) / dataRows.value.length).toFixed(1) : '--');
const avgPres = computed(() => dataRows.value.length ? (dataRows.value.reduce((s, r) => s + r.pres, 0) / dataRows.value.length).toFixed(1) : '--');

async function getHistory() {
  if (!start.value || !end.value) return;
  loading.value = true;
  const sTs = Math.floor(new Date(start.value).getTime() / 1000);
  const eTs = Math.floor(new Date(end.value).getTime() / 1000) + 86399;
  try {
    const res = await fetch(`${API_BASE}/api/weather/${sTs}/${eTs}`);
    const json = await res.json();
    dataRows.value = (json.data || []).sort((a, b) => a.timestamp - b.timestamp);
    updateCharts();
  } catch (e) {
    console.error('History fetch failed', e);
  } finally {
    loading.value = false;
  }
}

const chartDefaults = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: { legend: { labels: { color: '#4a6880', font: { size: 11 } } } },
  scales: {
    x: { ticks: { color: '#4a6880', maxTicksLimit: 8 }, grid: { color: 'rgba(26,48,72,0.5)' } },
    y: { ticks: { color: '#4a6880' }, grid: { color: 'rgba(26,48,72,0.5)' } }
  }
};

function updateCharts() {
  const labels = dataRows.value.map(r => new Date(r.timestamp * 1000).toLocaleDateString());

  if (chartT) chartT.destroy();
  chartT = new Chart(tempChart.value, {
    type: 'line',
    data: {
      labels,
      datasets: [{ label: 'Temp °C', data: dataRows.value.map(r => r.temp), borderColor: '#ff6b6b', backgroundColor: 'rgba(255,107,107,0.08)', tension: 0.4, pointRadius: 2 }]
    },
    options: chartDefaults
  });

  if (chartHP) chartHP.destroy();
  chartHP = new Chart(humPresChart.value, {
    type: 'line',
    data: {
      labels,
      datasets: [
        { label: 'Hum %',    data: dataRows.value.map(r => r.hum),  borderColor: '#00d4ff', backgroundColor: 'rgba(0,212,255,0.08)',  tension: 0.4, pointRadius: 2, yAxisID: 'y' },
        { label: 'Pres hPa', data: dataRows.value.map(r => r.pres), borderColor: '#00ff9d', backgroundColor: 'rgba(0,255,157,0.08)', tension: 0.4, pointRadius: 2, yAxisID: 'y1' }
      ]
    },
    options: {
      ...chartDefaults,
      scales: {
        x: chartDefaults.scales.x,
        y:  { ...chartDefaults.scales.y, position: 'left' },
        y1: { ...chartDefaults.scales.y, position: 'right', grid: { drawOnChartArea: false } }
      }
    }
  });
}

onUnmounted(() => { chartT?.destroy(); chartHP?.destroy(); });
</script>

<style scoped>
.ctrl-card, .chart-card, .table-card { background: #0f1e2e !important; border: 1px solid #1a3048 !important; border-radius: 12px; }
.section-label { font-size: 0.7rem; letter-spacing: 0.12em; color: #4a6880; text-transform: uppercase; margin-bottom: 12px; }
.chart-title { font-size: 0.7rem; letter-spacing: 0.12em; color: #4a6880; text-transform: uppercase; margin-bottom: 12px; }
.date-label { font-size: 0.7rem; color: #4a6880; text-transform: uppercase; letter-spacing: 0.1em; display: block; margin-bottom: 4px; }
.date-input {
  width: 100%; padding: 8px 10px;
  background: #0b1622; border: 1px solid #1a3048;
  border-radius: 6px; color: #c8dff0;
  font-family: 'Courier New', monospace; font-size: 0.85rem;
  outline: none;
}
.date-input:focus { border-color: #00d4ff; }
.date-row { display: flex; flex-direction: column; }
.stats-summary { border-top: 1px solid #1a3048; padding-top: 12px; }
.summary-title { font-size: 0.65rem; letter-spacing: 0.12em; color: #4a6880; text-transform: uppercase; margin-bottom: 8px; }
.summary-row { display: flex; justify-content: space-between; font-family: 'Courier New', monospace; font-size: 0.8rem; color: #8892b0; padding: 3px 0; }
.table-wrap { overflow-x: auto; max-height: 320px; overflow-y: auto; }
table { width: 100%; border-collapse: collapse; font-size: 0.82rem; }
thead th { font-family: 'Courier New', monospace; font-size: 0.65rem; letter-spacing: 0.1em; color: #4a6880; text-align: left; padding: 10px 14px; border-bottom: 1px solid #1a3048; position: sticky; top: 0; background: #0f1e2e; text-transform: uppercase; }
tbody tr { border-bottom: 1px solid rgba(26,48,72,0.5); transition: background 0.15s; }
tbody tr:hover { background: rgba(0,212,255,0.04); }
tbody td { padding: 9px 14px; font-family: 'Courier New', monospace; font-size: 0.78rem; }
.time-val { color: #4a6880; }
.temp-val { color: #ff6b6b; }
.hum-val  { color: #00d4ff; }
.pres-val { color: #00ff9d; }
.soil-val { color: #ffb700; }
</style>