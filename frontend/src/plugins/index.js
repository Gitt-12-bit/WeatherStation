import 'vuetify/styles'
import { createVuetify } from 'vuetify'

export function registerPlugins(app) {
  const vuetify = createVuetify()
  app.use(vuetify)
  console.log("Vuetify initialized")
}