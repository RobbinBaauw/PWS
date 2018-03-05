<template>
  <div class="index">
      <p>{{ datastring }}</p>
      <canvas id="canvas" :width="canvaswidth * 10" :height="canvasheight * 10" style="border:1px solid #d3d3d3;" @click="clickCanvas"></canvas>

      <v-text-field
        label="Lengte"
        v-model="currentlength"
      ></v-text-field>

      <v-select
        v-bind:items="choices"
        v-model="currenttype"
        label="Type"
        item-text="type"
        single-line
        return-object
        bottom
      ></v-select>

      <v-text-field
        label="Hoek"
        v-model="currentangle"
      ></v-text-field>
      <v-text-field
        label="Beginrichting"
        v-model="beginrichting"
      ></v-text-field>

      <v-btn color="primary" @click="drawbutton()">Teken</v-btn>
      <v-btn color="primary" @click="clearbutton()">Clear</v-btn>
      <v-btn color="primary" @click="deletebuttonfrom()">Verwijder vanaf hier</v-btn>
      <v-btn color="primary" @click="deletebuttonto()">Verwijder tot hier</v-btn>
      <v-btn color="error" @click="sendData()">Verstuur</v-btn>
  </div>
</template>

<script>
import socketconn from '../assets/socketconn'

export default {
  name: 'Index',
  data () {
    return {
      lines: [],
      canvasheight: 80,
      canvaswidth: 200,
      tweepi: 2 * Math.PI,
      angleradian:  Math.PI / 180,
      currentlength: "",
      currenttype: "",
      currentangle: "",
      elementstring: "",
      scalevalue: 10,
      beginrichting: "",
      c: "",
      ctx: "",
      selectedLineIndex: 0,
      datastring: "",
      choices: [{id: "2", type: "Links"}, {id: "1", type: "Rechtdoor"}, {id: "3", type: "Rechts"}]
    }
  },
  mounted: function()
  {
    this.c = document.getElementById("canvas");
    this.ctx = this.c.getContext("2d");
  },
  methods: {
    clearbutton: function()
    {
      this.lines = []
      this.datastring = ""
      this.verwerkLines()
    },
    sendData: function()
    {
      socketconn.$socket.emit('sendData', this.datastring, function(data) {
        if(data)
        {
          this.$notify({
            group: 'notifications',
            title: 'Gelukt!',
            type: 'success',
            text: 'Data wordt verzonden'
          })
        }
        else
        {
          this.$notify({
            group: 'notifications',
            title: 'Mislukt!',
            type: 'error',
            text: 'Er is ergens een fout opgetreden'
          })
        }

      })
    },
    drawbutton: function()
    {
      this.currenttype = this.currenttype.id
      //je kan niet met een hoek van nul graden naar links of naar rechts
      if((this.currenttype == '2' || this.currenttype == '3') && this.currentangle == '0')
      {
          alert('Doei he')
      }
      //alle waarde moeten verplicht op waardes gezet worden.
      else if(!this.currentlength || !this.currenttype || !this.currentangle)
      {
          console.log("Length: " + this.currentlength)
          console.log("Type: " + this.currenttype)
          console.log("Angle: " + this.currentangle)
          alert('Doei he')
      }
      else
      {
          if (this.lines.length < 1)
          {
              // als geen beginrichting is ingevuld dan zet hij hem op 90 graden
              if(!this.beginrichting)
              {
                  this.beginrichting = '90'
              }
              // weet niet
              this.lines.push({xend: 0, yend: 0, angleend: parseInt(this.beginrichting)})
          }
          //start berekening punten
          this.verwerkPunten();
      }
    },
    deletebuttonfrom: function()
    {
        for(var i = this.lines.length; this.lines.length > this.selectedLineIndex; i--)
        {
            this.lines.splice(i)
        }
        this.verwerkLines()
    },
    deletebuttonto: function()
    {
        this.lines.splice(0, this.selectedLineIndex + 1)
        this.lines.unshift({xend: this.lines[0].xbegin, yend: this.lines[0].ybegin})
        this.verwerkLines()
    },
    clickCanvas: function(e)
    {
      var r = this.c.getBoundingClientRect(),
            x = e.clientX - r.left,
            y = e.clientY - r.top

      var foundresult = false;

      for(var i = 1; i < this.lines.length; i++)
      {
          // ctx.clearRect(0, 0, c.width, c.height);

          this.ctx.beginPath();

          this.ctx.moveTo(
            (this.lines[i].xbegin + this.canvaswidth / 2) * this.scalevalue,
            (-this.lines[i].ybegin + this.canvasheight / 2) * this.scalevalue)

          if(this.lines[i].type == '1')
          {
              this.ctx.lineTo(
                (this.lines[i].xend + this.canvaswidth / 2) * this.scalevalue,
                (-this.lines[i].yend + this.canvasheight / 2) * this.scalevalue);
          }
          else
          {
              this.ctx.arc(
                (this.lines[i].circle.xmiddle + this.canvaswidth / 2) * this.scalevalue,
                (-this.lines[i].circle.ymiddle + this.canvasheight / 2) * this.scalevalue,
                (this.lines[i].circle.radius) * this.scalevalue,
                Math.PI - (this.lines[i].circle.beginradian - Math.PI),
                Math.PI - (this.lines[i].circle.endradian - Math.PI),
                this.lines[i].circle.gocounterclockwise);
          }

          this.ctx.strokeStyle = this.lines[i].color
          this.ctx.lineWidth = 7


          if (this.ctx.isPointInStroke(x, y))
          {
              this.verwerkLines()

              this.ctx.beginPath();

              this.ctx.moveTo(
                (this.lines[i].xbegin + this.canvaswidth / 2) * this.scalevalue,
                (-this.lines[i].ybegin + this.canvasheight / 2) * this.scalevalue)

              if(this.lines[i].type == '1')
              {
                this.ctx.lineTo(
                 (this.lines[i].xend + this.canvaswidth / 2) * this.scalevalue,
                  (-this.lines[i].yend + this.canvasheight / 2) * this.scalevalue);
              }
              else
              {
                this.ctx.arc(
                  (this.lines[i].circle.xmiddle + this.canvaswidth / 2) * this.scalevalue,
                  (-this.lines[i].circle.ymiddle + this.canvasheight / 2) * this.scalevalue,
                  (this.lines[i].circle.radius) * this.scalevalue,
                  Math.PI - (this.lines[i].circle.beginradian - Math.PI),
                  Math.PI - (this.lines[i].circle.endradian - Math.PI),
                  this.lines[i].circle.gocounterclockwise);
              }

              this.ctx.strokeStyle = 'black'
              this.ctx.lineWidth = 9
              this.ctx.stroke()

              this.strokeLineI(i)

              this.selectedLineIndex = i
              foundresult = true
              console.log(this.selectedLineIndex)

          }
      }
      if (!foundresult)
      {
          this.verwerkLines()
          this.selectedLineIndex = 0
      }
    },
    verwerkPunten: function()
    {
      //berekeningen
      if(this.currenttype == '2' || this.currenttype == '3')
      {
          // lengte = (hoek / 180) * (2PI r) die bouw je om naar r = (lengte * 180) / (hoek * 2PI) om de radius van de cirkel te verkrijgen
          var circleRadius = (this.currentlength * 180) / (this.currentangle * this.tweepi);
          var endradian;
          var beginradian;

          if (this.currenttype == '2') // Type = 2 betekent naar links
          {
              // type naar links, dus je voegt de hoek toe; als je de eenheidscirkel in je hoofd neemt ga je van -0.5pi (0,-1) naar boven toe, dus het wordt minder negatief. Om te verkrijgen wat je moet toevoegen deel je je ingevoerde hoek door 180 en vermenigvuldig je hem met 2pi
              endradian = (-0.5 * Math.PI) + ((this.currentangle * this.tweepi) / 180);
              //hij begint de hoek op -0.5PI;
              beginradian = (-0.5 * Math.PI);
          }
          else if(this.currenttype == '3') // Type = 3 betekent naar rechts
          {
              // Je gaat vanaf 0.5 PI naar rechts dus dan moet je min de hoek doen ipv plus bij links.
              endradian = (0.5 * Math.PI) - ((this.currentangle * this.tweepi) / 180);
              //Beginradian is 0.5PI;
              beginradian = (0.5 * Math.PI);
          }

          var endradianminus;
          // voeg klein getal bij of haal een klein getal van hoeken af om later te zien of de lijn naar boven gaat of naar beneden.
          if(endradian > beginradian)
          {
              endradianminus = endradian - 0.001;
          }
          else
          {
              endradianminus = endradian + 0.001;
          }
          //cos genomen van de endradian zodat we de x krijgen. Deze keer de circleRadius zodat je niet alleen waardes krijgt tussen de nul en 1 zoals in een eenheidscirkel. Nu is het punt dus nog steeds uitgedrukt vanuit 0.5pi of -0.5pi
          var xdiffend = Math.cos(endradian) * circleRadius;
          // hetzelfde maar dan voor y
          var ydiffend = Math.sin(endradian) * circleRadius;

          //Hetzelfde voor endradianminus
          var xdiffminus = Math.cos(endradianminus) * circleRadius;
          var ydiffminus = Math.sin(endradianminus) * circleRadius;

          //bereken
          if (this.currenttype == '2')
          {
              //Het verschil in y moet uitgedrukt worden vanuit 0,0 ipv -0.5pi of 0.5pi
              ydiffend += circleRadius;
              ydiffminus += circleRadius;
          }

          else if(this.currenttype == '3')
          {
              ydiffend -= circleRadius;
              ydiffminus -= circleRadius;
          }

          var prevangle = this.lines[this.lines.length - 1].angleend;
          // https://academo.org/demos/rotation-about-point/
          // gebruik van onderstaande formules, zie link hierboven voor meer info
          //x′=xcos⁡θ−ysin⁡θ
          //y'=ycosθ+xsinθ
          // prevangle * angleradian zodat de hoek in graden wordt omgebouwd naar radialen
          //lines.xend is vorige eindpunt, daar voeg je de net berekende punten aan toe, die je om 0,0 draait met de eindhoek van de vorige lijn
          var xend = this.lines[this.lines.length - 1].xend + (xdiffend * Math.cos(prevangle * this.angleradian) - ydiffend * Math.sin(prevangle * this.angleradian));
          var yend = this.lines[this.lines.length - 1].yend + (ydiffend * Math.cos(prevangle * this.angleradian) + xdiffend * Math.sin(prevangle * this.angleradian));
          // hetzelfde met het punt vlak voor het eindpunt
          var xendminus = this.lines[this.lines.length - 1].xend + (xdiffminus * Math.cos(prevangle * this.angleradian) - ydiffminus * Math.sin(prevangle * this.angleradian));
          var yendminus = this.lines[this.lines.length - 1].yend + (ydiffminus * Math.cos(prevangle * this.angleradian) + xdiffminus * Math.sin(prevangle * this.angleradian));

          //Voeg aan de begin en eindradialen de eindhoek van de vorige lijn toe, zodat die daar ook voor gecorrigeerd zijn voordat het middelpunt wordt berekend
          beginradian += prevangle * this.angleradian
          endradian += prevangle * this.angleradian

          // bereken middelpunt cirkel met behulp van endradian en het verwachte punt
          var xmiddle = xend - Math.cos(endradian) * circleRadius;
          var ymiddle = yend - Math.sin(endradian) * circleRadius;

          var xbegin = this.lines[this.lines.length - 1].xend;
          var ybegin = this.lines[this.lines.length - 1].yend;

          // Richting van punt naar punt is delta y / delta x, ricthing van de lijn recht erop dus -delta x / delta y, naar graden atan(inverse tangens)
          var richting = -(xmiddle - xend) / (ymiddle - yend);
          var hoek = Math.atan(richting) * (180 / Math.PI);

          // als hij naar beneden gaat bij een positieve a, doe - / + 180 (maakt niet zoveel uit), naar boven bij een negatieve a, doe - / + 180
          if(richting > 0 && yendminus - yend > 0)
          {
              hoek -= 180;
          }
          else if(richting < 0 && yendminus - yend < 0)
          {
              hoek -= 180;
          }

          var gocounterclockwise = false;
          //
          if (beginradian < endradian)
          {
              gocounterclockwise = true
          }
          // push variables in array
          this.lines.push({
              color: this.getRandomColor(),
              xbegin: xbegin,
              ybegin: ybegin,
              xend: xend,
              yend: yend,
              angleend: hoek,
              type: this.currenttype,
              length: this.currentlength,
              circle: {
                  xmiddle: xmiddle,
                  ymiddle: ymiddle,
                  radius: circleRadius,
                  beginradian: beginradian,
                  endradian: endradian,
                  gocounterclockwise: gocounterclockwise
              }
          })
      }
      //als currentype niet 2 of 3 is dan dus 1(rechtdoor)
      else
      {
          var xbegin = this.lines[this.lines.length - 1].xend;
          var ybegin = this.lines[this.lines.length - 1].yend;

          var prevangle = this.lines[this.lines.length - 1].angleend;
          var xend = this.lines[this.lines.length - 1].xend + (this.currentlength * Math.cos(prevangle * this.angleradian));
          var yend = this.lines[this.lines.length - 1].yend + (this.currentlength * Math.sin(prevangle * this.angleradian));

          var hoek = this.lines[this.lines.length - 1].angleend;
          //voeg toe aan array
          this.lines.push({
              color: this.getRandomColor(),
              xbegin: xbegin,
              ybegin: ybegin,
              xend: xend,
              yend: yend,
              angleend: hoek,
              type: this.currenttype,
              length: this.currentlength,
          })
      }

      this.currenttype = {}
      this.currentlength = ""
      this.beginrichting = ""
      this.currentangle = ""

      var requireddata = []
      for(var i = 1; i < this.lines.length; i++)
      {
          var currentline = this.lines[i]
          if(currentline.type == "2" || currentline.type == "3")
          {
              requireddata.push([
                this.roundNumber(currentline.xbegin, 3),
                this.roundNumber(currentline.ybegin, 3),
                this.roundNumber(currentline.xend, 3),
                this.roundNumber(currentline.yend, 3),
                this.roundNumber(currentline.circle.xmiddle, 3),
                this.roundNumber(currentline.circle.ymiddle, 3),
                currentline.type,
                currentline.length,
                this.roundNumber(currentline.circle.radius, 3),
                this.roundNumber(currentline.angleend, 3),
                this.roundNumber(currentline.circle.beginradian, 3),
                this.roundNumber(currentline.circle.endradian, 3)])
          }
          else
          {
              requireddata.push([
                this.roundNumber(currentline.xbegin, 3),
                this.roundNumber(currentline.ybegin, 3),
                this.roundNumber(currentline.xend, 3),
                this.roundNumber(currentline.yend, 3),
                null,
                null,
                currentline.type,
                currentline.length,
                null,
                this.roundNumber(currentline.angleend, 3),
                null,
                null])
          }
      }

      var sendstring = requireddata.length + "_"

      for(var y = 0; y < requireddata.length; y++)
      {
          if(sendstring == requireddata.length + "_")
          {
              sendstring += requireddata[y].join()
          }
          else
          {
              sendstring += "|" + requireddata[y].join()
          }

      }

      console.log(sendstring)
      this.datastring = sendstring
      //start tekenen op canvas
      this.verwerkLines()
    },
    roundNumber: function(number, decimals)
    {
      return parseFloat(Math.round(number * Math.pow(10, decimals)) / Math.pow(10, decimals))
    },
    verwerkLines: function()
    {
      //aanmaken canvas
      this.ctx.clearRect(0, 0, this.c.width, this.c.height);
      //kijkt of hij het eerste lijnstuk gaat tekenen of een andere.
      for(var i = 1; i < this.lines.length; i++)
      {
          this.strokeLineI(i)
      }
    },
    getRandomColor: function()
    {
      var letters = '0123456789ABCDEF';
      var color = '#';
      for (var i = 0; i < 6; i++) {
        color += letters[Math.floor(Math.random() * 16)];
      }
      return color;
    },
    strokeLineI: function(i)
    {
      this.ctx.beginPath();
      // voeg toe van 1000 en 500 om op het middelpunt van het canvas te beginnnen
      this.ctx.moveTo(
        (this.lines[i].xbegin + this.canvaswidth / 2) * this.scalevalue,
        (-this.lines[i].ybegin + this.canvasheight / 2) * this.scalevalue)

      if(this.lines[i].type == '1')
      {
          this.ctx.lineTo(
            (this.lines[i].xend + this.canvaswidth / 2) * this.scalevalue,
            (-this.lines[i].yend + this.canvasheight / 2) * this.scalevalue);
      }
      else
      {
          this.ctx.arc(
            (this.lines[i].circle.xmiddle + this.canvaswidth / 2) * this.scalevalue,
            (-this.lines[i].circle.ymiddle + this.canvasheight / 2) * this.scalevalue,
            this.lines[i].circle.radius * this.scalevalue,
            Math.PI - (this.lines[i].circle.beginradian - Math.PI),
            Math.PI - (this.lines[i].circle.endradian - Math.PI),
            this.lines[i].circle.gocounterclockwise);
      }
      this.ctx.strokeStyle = this.lines[i].color
      this.ctx.lineWidth = 4
      this.ctx.stroke();
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
</style>
