<template>
  <v-container>
    <v-row justify="center">
      <v-col cols="12" md="8">
        <v-card class="pa-8" elevation="2" rounded="lg">
          <v-card-title class="text-center text-h6 font-weight-regular mb-1">
            Combination
          </v-card-title>
          <v-card-subtitle class="text-center mb-6">
            Enter your four digit passcode
          </v-card-subtitle>

          <div class="d-flex justify-center mb-6">
            <v-otp-input
              v-model="passcode"
              length="4"
              type="number"
              variant="outlined"
              focus-all
            ></v-otp-input>
          </div>

          <div class="d-flex justify-center">
            <v-btn color="primary" @click="sendPasscode" :loading="loading">
              SUBMIT
            </v-btn>
          </div>

          <v-alert
            v-if="message"
            :type="messageType"
            class="mt-4"
            density="compact"
            closable
            @click:close="message = ''"
          >
            {{ message }}
          </v-alert>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script>
import axios from 'axios'

export default {
  name: 'Control',
  data() {
    return {
      passcode: '',
      loading: false,
      message: '',
      messageType: 'success',
    }
  },
  methods: {
    async sendPasscode() {
      if (!this.passcode || this.passcode.length !== 4 || isNaN(Number(this.passcode))) {
        this.message = 'Please enter a valid 4-digit numeric passcode.'
        this.messageType = 'warning'
      return
  }

  this.loading = true
  this.message = ''

  try {
    const params = new URLSearchParams()
    params.append('passcode', String(this.passcode))
    
    console.log('Sending passcode:', this.passcode)  // ← add this to verify

    const res = await axios.post('/api/set/combination', params.toString(), {
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' }
    })

    if (res.data.status === 'complete') {
      this.message = 'Passcode saved successfully!'
      this.messageType = 'success'
      this.passcode = ''
    } else {
      this.message = 'Failed to save passcode. Please try again.'
      this.messageType = 'error'
    }
  } catch (err) {
    console.error(err)
    this.message = 'Error connecting to backend.'
    this.messageType = 'error'
  } finally {
    this.loading = false
  }
}
  }
}
</script>