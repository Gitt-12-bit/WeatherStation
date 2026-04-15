<template>
  <v-app theme="dark">
    <v-app-bar flat color="#050d14" border="bottom" elevation="2">
      <v-app-bar-title>
        <span style="font-weight:bold; color:#00bcd4; letter-spacing:2px;">WEATHER OS</span>
      </v-app-bar-title>
      <v-btn 
        v-for="item in navItems" 
        :key="item.key"
        @click="view = item.key"
        :variant="view === item.key ? 'tonal' : 'text'"
        :color="view === item.key ? 'cyan' : 'white'"
        class="mx-1"
      >
        {{ item.label }}
      </v-btn>
    </v-app-bar>

    <v-main>
      <HomeView v-if="view === 'home'" @start="view = 'dash'" />
      <DashboardView v-if="view === 'dash'" />
      <AnalysisView v-if="view === 'analysis'" />
    </v-main>
  </v-app>
</template>

<script setup>
import { ref } from 'vue'
import HomeView from './views/Home.vue'
import DashboardView from './views/Dashboard.vue'
import AnalysisView from './views/Analysis.vue'

const view = ref('home')
const navItems = [
  { key: 'home', label: 'Home' },
  { key: 'dash', label: 'Dashboard' },
  { key: 'analysis', label: 'Analysis' },
]
</script>