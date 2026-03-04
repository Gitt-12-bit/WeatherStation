<template>
  <v-container fluid>

    <!-- Overflow Dialog -->
    <v-dialog v-model="overflow" width="350" persistent>
      <v-card class="pa-6 text-center">
        <v-icon icon="mdi:mdi-alert-circle" color="error" size="48" class="mb-3"></v-icon>
        <h2 class="text-h5 mb-2">Overflow Detected!</h2>
        <p class="text-body-2 mb-4">Water level has exceeded 100% capacity.</p>
        <v-btn color="error" @click="overflow = false">Dismiss</v-btn>
      </v-card>
    </v-dialog>

    <!-- Top Row: Slider + Area Chart -->
    <v-row class="mb-4">

      <!-- Radar Slider (Height indicator) -->
      <v-col cols="12" sm="2">
        <v-sheet class="pa-4 d-flex flex-column align-center justify-center" height="100%" rounded="lg" elevation="1">
          <v-slider
            v-model="waterheight"
            :min="0"
            :max="94.5"
            direction="vertical"
            color="teal"
            track-color="blue-lighten-4"
            thumb-label="always"
            style="height: 280px"
          ></v-slider>
          <div class="text-caption mt-2">Height(in)</div>
        </v-sheet>
      </v-col>

      <!-- Area Chart -->
      <v-col cols="12" sm="10">
        <v-sheet rounded="lg" elevation="1">
          <div id="areaChart" style="width:100%; height:320px;"></div>
        </v-sheet>
      </v-col>

    </v-row>

    <!-- Bottom Row: Gauge + Liquid Progress -->
    <v-row>

      <!-- Gauge Chart -->
      <v-col cols="12" sm="8">
        <v-sheet rounded="lg" elevation="1">
          <div id="gaugeChart" style="width:100%; height:300px;"></div>
        </v-sheet>
      </v-col>

      <!-- Liquid Progress / Percentage Card -->
      <v-col cols="12" sm="4">
        <v-sheet class="pa-4 d-flex flex-column align-center justify-center" height="300" rounded="lg" elevation="1">
          <div class="text-subtitle-1 font-weight-medium mb-1">Water Level</div>
          <div class="text-caption text-medium-emphasis mb-4">Capacity Remaining</div>
          <div class="liquid-wrapper">
            <div class="liquid-container">
              <div class="liquid-fill" :style="{ height: clampedPercentage + '%' }"></div>
              <div class="liquid-text">{{ percentage.toFixed(1) }}%</div>
            </div>
          </div>
        </v-sheet>
      </v-col>

    </v-row>
  </v-container>
</template>

<script>
import Highcharts from 'highcharts'
import HighchartsMore from 'highcharts/highcharts-more'
import SolidGauge from 'highcharts/modules/solid-gauge'
import mqtt from 'mqtt'

// Register Highcharts modules (safe to call multiple times)
HighchartsMore(Highcharts)
SolidGauge(Highcharts)

export default {
  name: 'Dashboard',
  data() {
    return {
      radar: 0,
      waterheight: 0,
      reserve: 0,
      percentage: 0,
      overflow: false,
      areaChart: null,
      gaugeChart: null,
      mqttClient: null,
    }
  },
  computed: {
    clampedPercentage() {
      return Math.min(100, Math.max(0, this.percentage))
    }
  },
  mounted() {
    this.initAreaChart()
    this.initGaugeChart()
    this.connectMQTT()
  },
  beforeUnmount() {
    if (this.mqttClient) {
      this.mqttClient.end()
    }
  },
  methods: {
    initAreaChart() {
      this.areaChart = Highcharts.chart('areaChart', {
        chart: { zoomType: 'x', animation: false },
        title: { text: 'Water Reserves (10 min)', align: 'left' },
        tooltip: { shared: true },
        xAxis: {
          type: 'datetime',
          title: { text: 'Time' }
        },
        yAxis: {
          title: { text: 'Water level' },
          labels: { format: '{value} Gal' },
          min: 0,
          max: 1000
        },
        legend: { enabled: true },
        series: [{
          name: 'Water',
          type: 'area',
          data: [],
          color: '#7cb5ec',
          fillOpacity: 0.5
        }],
        credits: { enabled: true }
      })
    },

    initGaugeChart() {
      this.gaugeChart = Highcharts.chart('gaugeChart', {
        chart: { type: 'gauge', animation: false },
        title: { text: 'Water Reserves', align: 'left' },
        tooltip: { shared: true },
        pane: {
          startAngle: -90,
          endAngle: 89.9,
          background: null,
          center: ['50%', '75%'],
          size: '110%'
        },
        yAxis: {
          min: 0,
          max: 1000,
          tickPixelInterval: 72,
          tickPosition: 'inside',
          tickColor: Highcharts.defaultOptions.chart.backgroundColor || '#FFFFFF',
          tickLength: 20,
          tickWidth: 2,
          minorTickInterval: null,
          labels: { distance: 20, style: { fontSize: '14px' } },
          lineWidth: 0,
          plotBands: [
            { from: 0, to: 200, color: '#DF5353', thickness: 20 },
            { from: 200, to: 600, color: '#DDDF0D', thickness: 20 },
            { from: 600, to: 1000, color: '#55BF3B', thickness: 20 }
          ]
        },
        series: [{
          type: 'gauge',
          name: 'Water Capacity',
          data: [0],
          tooltip: { valueSuffix: ' Gal' },
          dataLabels: {
            format: '{y} Gal',
            borderWidth: 0,
            color: (Highcharts.defaultOptions.title &&
              Highcharts.defaultOptions.title.style &&
              Highcharts.defaultOptions.title.style.color) || '#333333',
            style: { fontSize: '16px' }
          },
          dial: {
            radius: '80%',
            backgroundColor: 'gray',
            baseWidth: 12,
            baseLength: '0%',
            rearLength: '0%'
          },
          pivot: { backgroundColor: 'gray', radius: 6 }
        }],
        credits: { enabled: true }
      })
    },

    connectMQTT() {
      // Replace with your backend IP — same host as the page by default
      const brokerUrl = `ws://${window.location.hostname}:9001`
      this.mqttClient = mqtt.connect(brokerUrl)

      this.mqttClient.on('connect', () => {
        console.log('MQTT connected')
        this.mqttClient.subscribe('620171757')  // must match Mqtt.publish topic in views.py
      })

      this.mqttClient.on('error', (err) => {
        console.error('MQTT error:', err)
      })

      this.mqttClient.on('message', (topic, message) => {
        try {
          const data = JSON.parse(message.toString())

          this.radar = data.radar ?? 0
          this.waterheight = data.waterheight ?? 0
          this.reserve = data.reserve ?? 0
          this.percentage = data.percentage ?? 0

          // Update area chart — keep last 600 points (10 min at 1/sec)
          const series = this.areaChart.series[0]
          const shift = series.data.length >= 600
          series.addPoint([Date.now(), this.reserve], true, shift)

          // Update gauge
          this.gaugeChart.series[0].points[0].update(this.reserve)

          // Overflow check
          if (this.percentage > 100) {
            this.overflow = true
          }
        } catch (e) {
          console.error('Failed to parse MQTT message:', e)
        }
      })
    }
  }
}
</script>

<style scoped>
/* Liquid progress indicator */
.liquid-wrapper {
  display: flex;
  justify-content: center;
  align-items: center;
}

.liquid-container {
  position: relative;
  width: 120px;
  height: 120px;
  border-radius: 50%;
  overflow: hidden;
  background: #e0e0e0;
  border: 4px solid #7b1fa2;
  box-shadow: 0 0 20px rgba(123, 31, 162, 0.4);
}

.liquid-fill {
  position: absolute;
  bottom: 0;
  left: 0;
  width: 100%;
  background: linear-gradient(180deg, #ab47bc, #7b1fa2);
  transition: height 0.8s ease;
  border-radius: 0 0 50% 50% / 0 0 20px 20px;
}

.liquid-fill::before {
  content: '';
  position: absolute;
  top: -8px;
  left: -20%;
  width: 140%;
  height: 16px;
  background: rgba(255, 255, 255, 0.3);
  border-radius: 50%;
  animation: wave 2s linear infinite;
}

@keyframes wave {
  0%   { transform: translateX(0); }
  50%  { transform: translateX(10px); }
  100% { transform: translateX(0); }
}

.liquid-text {
  position: absolute;
  inset: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.4rem;
  font-weight: 700;
  color: white;
  text-shadow: 0 1px 3px rgba(0,0,0,0.5);
  z-index: 2;
}
</style>