import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'), // <-- This tells Vite that '@' = 'src/'
    },
  },
  server: {
    proxy: {
      '/api': {
        target: 'http://127.0.0.1:8080', // backend address
        changeOrigin: true,
        secure: false,
      },
    },
  },
})