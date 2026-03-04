import { createApp } from 'vue'
import App from './App.vue'
import router from './router'

// Vuetify
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'
import 'vuetify/styles'
import '@mdi/font/css/materialdesignicons.css'

const vuetify = createVuetify({
  components,
  directives,
  theme: {
    defaultTheme: 'lightMode',
    themes: {
      lightMode: {
        dark: false,
        colors: { primary: '#1976D2' }
      },
      darkMode: {
        dark: true,
        colors: { primary: '#90CAF9' }
      }
    }
  }
})

createApp(App).use(router).use(vuetify).mount('#app')