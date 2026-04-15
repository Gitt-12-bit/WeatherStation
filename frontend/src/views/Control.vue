<template>
  <v-app theme="dark">
    <v-app-bar flat color="#050d14" border="bottom">
      <v-toolbar-title class="font-weight-bold text-cyan">WEATHER OS</v-toolbar-title>
      <v-spacer></v-spacer>
      <v-btn @click="view = 'home'" :color="view === 'home' ? 'cyan' : ''">Home</v-btn>
      <v-btn @click="view = 'dash'" :color="view === 'dash' ? 'cyan' : ''">Dashboard</v-btn>
      <v-btn @click="view = 'analysis'" :color="view === 'analysis' ? 'cyan' : ''">Analysis</v-btn>
    </v-app-bar>

    <v-main class="bg-black">
      <HomeView v-if="view === 'home'" @start="view = 'dash'" />
      <DashboardView v-if="view === 'dash'" />
      <AnalysisView v-if="view === 'analysis'" />
    </v-main>
  </v-app>
</template>

<script setup>
import { ref } from 'vue';
import HomeView from './components/Home.vue';
import DashboardView from './components/Dashboard.vue';
import AnalysisView from './components/Analysis.vue';

const view = ref('home');
</script>