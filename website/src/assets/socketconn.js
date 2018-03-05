import Vue from 'vue'
import VueSocketio from 'vue-socket.io'

Vue.use(VueSocketio, 'http://192.168.192.65:3000')

var socketconn = new Vue({
  data () {
    return {
      ip: 'http://192.168.192.65:3000'
    }
  },
  sockets: {
    connect: function () {
      console.log('socket connected')
    }
  }
})

export default socketconn
