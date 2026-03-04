<template>
  <v-container fluid>

    <!-- Top Row: Date Pickers + Average Card -->
    <v-row class="mb-4" align="start">

      <!-- Date Range Inputs -->
      <v-col cols="12" sm="4">
        <v-card class="pa-4" elevation="1" rounded="lg">
          <v-text-field
            v-model="startDate"
            label="Start Date"
            type="date"
            density="compact"
            variant="outlined"
            class="mb-3"
            prepend-inner-icon="mdi:mdi-calendar"
          ></v-text-field>

          <v-text-field
            v-model="endDate"
            label="End Date"
            type="date"
            density="compact"
            variant="outlined"
            class="mb-4"
            prepend-inner-icon="mdi:mdi-calendar"
          ></v-text-field>

          <v-btn
            color="primary"
            variant="tonal"
            @click="runAnalysis"
            :loading="loading"
            block
          >
            Analyze
          </v-btn>
        </v-card>
      </v-col>

      <!-- Average Display Card -->
      <v-col cols="12" sm="4">
        <v-card class="pa-6 text-center" elevation="1" rounded="lg" min-height="160">
          <div class="text-subtitle-1 font-weight-medium mb-1">Average</div>
          <div class="text-caption text-medium-emphasis mb-4">For the selected period</div>
          <div class="d-flex align-end justify-center">
            <span class="text-h2 font-weight-bold">{{ displayAverage }}</span>
            <span class="text-subtitle-1 ml-1 mb-2 text-medium-emphasis">Gal</span>
          </div>
        </v-card>
      </v-col>

    </v-row>

    <!-- Alert for errors -->
    <v-alert
      v-if="errorMsg"
      type="error"
      class="mb-4"
      density="compact"
      closable
      @click:close="errorMsg = ''"
    >
      {{ errorMsg }}
    </v-alert>

    <!-- Line Chart: Water Management Analysis -->
    <v-sheet rounded="lg" elevation="1" class="mb-6">
      <div id="lineChart" style="width:100%; height:360px;"></div>
    </v-sheet>

    <!-- Scatter Plot: Height & Water Level Correlation -->
    <v-sheet rounded="lg" elevation="1">
      <div id="scatterChart" style="width:100%; height:360px;"></div>
    </v-sheet>

  </v-container>
</template>

<script>
import axios from 'axios'
import Highcharts from 'highcharts'

export default {
  name: 'Analysis',
  data() {
    return {
      startDate: '',
      endDate: '',
      average: null,
      loading: false,
      errorMsg: '',
      lineChartInstance: null,
      scatterChartInstance: null,
    }
  },
  computed: {
    displayAverage() {
      if (this.average === null) return '—'
      return Number(this.average).toFixed(1)
    }
  },
  mounted() {
    this.initCharts()
  },
  methods: {
    // Convert a date string like "2024-01-31" to a 10-digit Unix timestamp (start of day)
    dateToTimestamp(dateStr) {
      if (!dateStr) return null
      return Math.floor(new Date(dateStr).getTime() / 1000)
    },

    initCharts() {
      this.lineChartInstance = Highcharts.chart('lineChart', {
        title: { text: 'Water Management Analysis', align: 'left' },
        xAxis: {
          type: 'datetime',
          title: { text: 'Time' }
        },
        yAxis: {
          title: { text: 'Water Reserve' },
          labels: { format: '{value} Gal' },
          min: 0
        },
        series: [{
          type: 'line',
          name: 'Reserve',
          data: [],
          color: '#29b6f6'
        }],
        credits: { enabled: true }
      })

      this.scatterChartInstance = Highcharts.chart('scatterChart', {
        chart: { type: 'scatter' },
        title: { text: 'Height and Water Level Correlation Analysis', align: 'left' },
        xAxis: {
          title: { text: 'Water Height' },
          labels: { format: '{value} in' }
        },
        yAxis: {
          title: { text: 'Height' },
          labels: { format: '{value} in' }
        },
        tooltip: {
          pointFormat: 'Water Height: <b>{point.x} in</b><br>Radar: <b>{point.y} in</b>'
        },
        series: [{
          name: 'Analysis',
          data: [],
          color: '#42a5f5',
          marker: { radius: 4 }
        }],
        credits: { enabled: true }
      })
    },

    async runAnalysis() {
      if (!this.startDate || !this.endDate) {
        this.errorMsg = 'Please select both a start and end date.'
        return
      }

      // Convert dates to 10-digit Unix timestamps
      const startTs = this.dateToTimestamp(this.startDate)
      // End of the end date (add 86399 seconds to include the full day)
      const endTs = this.dateToTimestamp(this.endDate) + 86399

      this.loading = true
      this.errorMsg = ''

      try {
        // Fetch reserve data and average in parallel
        const [reserveRes, avgRes] = await Promise.all([
          axios.get(`/api/reserve/${startTs}/${endTs}`),
          axios.get(`/api/avg/${startTs}/${endTs}`)
        ])

        // Update average
        if (avgRes.data.status === 'found') {
          this.average = avgRes.data.data
        } else {
          this.average = 0
        }

        // Update charts
        if (reserveRes.data.status === 'found') {
          const records = reserveRes.data.data

          // Line chart: timestamp (ms) vs reserve
          const lineData = records.map(d => [d.timestamp * 1000, d.reserve])

          // Scatter: waterheight (x) vs radar (y)
          const scatterData = records.map(d => [d.waterheight, d.radar])

          this.lineChartInstance.series[0].setData(lineData, true)
          this.scatterChartInstance.series[0].setData(scatterData, true)
        } else {
          this.errorMsg = 'No data found for the selected date range.'
          this.lineChartInstance.series[0].setData([], true)
          this.scatterChartInstance.series[0].setData([], true)
        }

      } catch (err) {
        console.error(err)
        this.errorMsg = 'Failed to fetch data. Check backend connectivity.'
      } finally {
        this.loading = false
      }
    }
  }
}
</script>