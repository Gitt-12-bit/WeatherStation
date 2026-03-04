import { createRouter, createWebHistory } from 'vue-router'
import Home from '@/views/Home.vue'
import Control from '@/views/Control.vue'
import Dashboard from '@/views/Dashboard.vue'
import Analysis from '@/views/Analysis.vue'

const routes = [
  { path: '/',          name: 'Home',      component: Home      },
  { path: '/control',   name: 'Control',   component: Control   },
  { path: '/dashboard', name: 'Dashboard', component: Dashboard },
  { path: '/analysis',  name: 'Analysis',  component: Analysis  },
]

export default createRouter({
  history: createWebHistory(),
  routes
})