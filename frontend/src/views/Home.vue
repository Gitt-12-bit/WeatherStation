<template>
  <div class="home-container">
    <div class="weather-animation">
      <div class="sun"></div>
      <div class="orbit">
        <div class="cloud">☁️</div>
        <div class="rain">💧</div>
      </div>
    </div>
    <h1 class="glitch-text">STATION 620171757</h1>
    <p class="subtitle">Next-Gen Environmental Monitoring</p>
    <v-btn size="x-large" color="cyan" variant="outlined" @click="$emit('start')" class="mt-8">
      Enter Dashboard
    </v-btn>
  </div>
</template>

<style scoped>
.home-container {
  height: 90vh; display: flex; flex-direction: column; 
  align-items: center; justify-content: center; background: radial-gradient(circle, #0a192f 0%, #000 100%);
}
.weather-animation { position: relative; width: 200px; height: 200px; margin-bottom: 40px; }
.sun { 
  width: 80px; height: 80px; background: #ffcc00; border-radius: 50%; 
  position: absolute; top: 60px; left: 60px; box-shadow: 0 0 50px #ffcc00;
}
.orbit {
  width: 100%; height: 100%; border: 1px dashed rgba(0, 255, 255, 0.2);
  border-radius: 50%; animation: rotate 10s linear infinite;
}
.cloud, .rain { position: absolute; font-size: 2rem; }
.cloud { top: -10px; left: 50%; }
.rain { bottom: -10px; left: 50%; }

@keyframes rotate { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }
.glitch-text { font-family: 'Orbitron', sans-serif; font-size: 3rem; color: #00f2ff; letter-spacing: 5px; }
.subtitle { color: #8892b0; margin-top: 10px; }
</style>