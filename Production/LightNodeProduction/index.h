//const char index_html[] PROGMEM = R"rawliteral(
    const char index_html[] = R"rawliteral(
    <!DOCTYPE HTML><html>
    <title>Light Control</title>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
        html 
        {
        font-family: Arial;
        display: inline-block;
        margin: 0px auto;
        text-align: center;
        }
        h2 
        { 
            font-size: 3.0rem; 
            color: white;
            -webkit-text-stroke: 0.5px black;
        }
        span 
        { 
            font-size: 1rem;
            color: black;
            padding: 2px 9px;
            border-radius: 3px;
            background-color: rgba(255, 255, 255, 0.685);
            border: 1px solid black;
        }
        .units { font-size: 1.2rem; }
        .esp-values
        {
            font-size: 1.5rem;
            vertical-align:middle;
            padding-bottom: 15px;
        }
        output
        {
            display: inline-block;
            min-width: 2.5em;
        }
        label, output
        {
            padding: 2px 9px;
            border-radius: 3px;
            font-family: 'Roboto', sans-serif;
            font-size: 1.1em;
            color: black;
            background-color: rgba(255, 255, 255, 0.685);
            border: 1px solid black;
        }
        fieldset
        {
            border: none;
        }
        input[type=range] 
        {
            width: 200px;
        }
        label[for=r], output[for=r]
        {
            background-color: #f00;
        }
        label[for=g], output[for=g]
        {
            background-color: #0f0;
        }
        label[for=b], output[for=b]
        {
            background-color: #00f;
        }
        #hexVal
        {
            margin-top: 25px;
            min-width: 4.5em;
            font-size: 3em;
            background: rgba(255,255,255,.3);
        }
        #colorOutput
        {
            width:50%;
            height:100px;
            background:white;
            vertical-align:middle;
            display: inline-block;
        }





        </style>
    </head>
    <body onload="readValuesFromNodeandSet()" id="bg">
        <h2>Light Control</h2>

        <input type="checkbox" id="AutoSend" checked="checked"/>
        <label for="autoSend">Auto Send</label>
        <input type="checkbox" id="AutoRead" checked="checked"/>
        <label for="autoRead">Auto Read</label>
        
        </br>
        </br>

        <input type="checkbox" id="OnOff" checked="checked"/>
        <label for="OnOffOutput">On/Off</label>
        
        </br>
        </br>

        <input type="radio" id="mode1" name="mode" value="1">
        <label for="mode1">Set All 1</label>
        <input type="radio" id="mode2" name="mode" value="2" checked="checked">
        <label for="mode2">Set All 2</label> 
        <input type="radio" id="mode3" name="mode" value="3">
        <label for="mode3">Rainbow</label>
        </br>
        </br>
        <input type="radio" id="mode4" name="mode" value="4">
        <label for="mode4">Rainbow Outside</label>
        
        
        <fieldset>
        <label for="bright">Brightness</label>
        <input type="range" min="0" max="255" id="bright" step="1" value="255">
        <output for="bright" id="outputBright">0</output>
        </fieldset>  

        <fieldset>
        <label for="delay">Delay</label>
        <input type="range" min="0" max="1000" id="delay" step="10" value="100">
        <output for="delay" id="outputDelay">0 sec</output>
        </fieldset>  
        
        <fieldset>
        <label for="r">R</label>
        <input type="range" min="0" max="255" id="r" step="1" value="255">
        <output for="r" id="outputR">0</output>
        </fieldset>  
        
        <fieldset>
        <label for="g">G</label>
        <input type="range" min="0" max="255" id="g" step="1" value="0">
        <output for="g" id="outputG">0</output>
        </fieldset>
        
        <fieldset>
        <label for="b">B</label>
        <input type="range" min="0" max="255" id="b" step="1" value="255">
        <output for="b" id="outputB">0</output>
        </fieldset>  

        </br>
        <span id="url">URL VAR</span>
        </br>
        </br>
        <button type="button" onclick="readValuesFromNodeandSet()">Read</button>
        <button type="button" onclick="sendValues()">Send</button>
        
    </body>

    <!--https://stackoverflow.com/questions/247483/http-get-request-in-javascript-->
    <script>


    
    function updateText(statusId) 
    {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() 
        {
        if (this.readyState == 4 && this.status == 200) 
        {
            document.getElementById(statusId).innerHTML = this.responseText;
            changeColor(statusId, this.responseText);
        }
        };
        var urlVar = "/value?"+statusId;
        xhttp.open("GET", urlVar, true);
        xhttp.send(null);
    }




    


    var PARAM_ALL = "all",
        PARAM_ONOFF = "of",
        PARAM_MODE = "m",
        PARAM_BRIGHTNESS = "br",
        PARAM_RED = "r",
        PARAM_GREEN = "g",
        PARAM_BLUE = "b",
        PARAM_DELAY = "d";

    //Slider
    var body = document.body, 
        of = document.querySelector('#OnOff'),
        m1 = document.querySelector('#mode1'),
        m2 = document.querySelector('#mode2'),
        m3 = document.querySelector('#mode3'),
        m4 = document.querySelector('#mode4'),
        r = document.querySelector('#r'),
        g = document.querySelector('#g'),
        b = document.querySelector('#b'),
        bright = document.querySelector('#bright'),
        delay = document.querySelector('#delay'),
        outputR = document.querySelector('#outputR'),
        outputG = document.querySelector('#outputG'),
        outputB = document.querySelector('#outputB'),
        outputBright = document.querySelector('#outputBright'),
        outputDelay = document.querySelector('#outputDelay');

    function getMode()
    {
      var result = 0;
      var modes = document.getElementsByName('mode');
      for (var i = 0, length = modes.length; i < length; i++) 
      {
          if (modes[i].checked) 
          {
          // do whatever you want with the checked radio
          result = i;
      
          // only one radio can be logically checked, don't check the rest
          break;
          }
      }
      return result;
    }

    function setMode(mode)
    {

      var modes = document.getElementsByName('mode');
      modes[mode].checked = true;

    }

    function buildSendURL() 
    {
      var ofValues = 0;
      
      if (of.checked == true)
      {
          ofValues = 1;
      }
      else
      {
          ofValues = 0;
      }
  
      var urlVar = "/set?"+PARAM_ONOFF+"="+ofValues+"&"+PARAM_MODE+"="+getMode()+"&"+PARAM_BRIGHTNESS+"="+bright.value+"&"+PARAM_RED+"="+r.value+"&"+PARAM_GREEN+"="+g.value+"&"+PARAM_BLUE+"="+b.value+"&"+PARAM_DELAY+"="+delay.value;
  
      return urlVar;

    }
    function sendValues() 
    {
      var xhttp = new XMLHttpRequest();  
      var urlVar = buildSendURL();
      document.getElementById("url").innerHTML = urlVar;  
      xhttp.open("GET", urlVar, true);
      xhttp.send(null);
    }


    function readValuesFromNodeandSet() 
    {
      var xhttp = new XMLHttpRequest();
      var reply = "no data";
      xhttp.onreadystatechange = function() 
      {
          if (xhttp.readyState == XMLHttpRequest.DONE) 
          {
              reply = xhttp.responseText;
              setWebpageValues(reply);
          }
      };  
      var urlVar = "/value?"+PARAM_ALL;
      
      xhttp.open("GET", urlVar, true);
      xhttp.send(null);
      
      //OnOFf, Mode, Bright, R, G, B, Delay
      //0       1       2    3  4  5  6
      return reply;
    }

        
    
    function setColor(){
    var r_hexVal = parseInt(r.value, 10).toString(16),
        g_hexVal = parseInt(g.value, 10).toString(16),
        b_hexVal = parseInt(b.value, 10).toString(16),
        hexVal = "#" + pad(r_hexVal) + pad(g_hexVal) + pad(b_hexVal);
    //document.getElementById( 'colorOutput' ).style.backgroundColor = hexVal; 
    document.getElementById( 'bg' ).style.backgroundColor = hexVal;
    }
    
    function setWebpageValues(reply)
    {
      reply = reply.split("&")
  
      if (parseInt(reply[0]) == 0)
      {
          of.checked = false;
      }
      else if (parseInt(reply[0]) == 1)
      {
          of.checked = true;
      }
  
      setMode(parseInt(reply[1]));
      console.log(reply)
      r.value = parseInt(reply[3]);
      g.value = parseInt(reply[4]);
      b.value = parseInt(reply[5]);
      bright.value = parseInt(reply[2]);
      delay.value = parseInt(reply[6]);
  
      outputR.value = r.value;
      outputG.value = g.value;
      outputB.value = b.value;
      outputBright.value = bright.value;
      outputDelay.value = delay.value/1000 + " sec";
      setColor();
  
      document.getElementById("url").innerHTML = buildSendURL();
   }
      //setWebpageValues();
  
    function pad(n)
    {
      return (n.length<2) ? "0"+n : n;
    }

    //Mode Checkbox Listener
    //m.addEventListener('change', function() {
    //  sendValues();
    //}, false);

    //OnOff Checkbox Listener
    of.addEventListener('change', function() {
    autoSend();
    }, false);

    m1.addEventListener('change', function() {
    autoSend();
    }, false);

    m2.addEventListener('change', function() {
    autoSend();
    }, false);

    m3.addEventListener('change', function() {
    autoSend();
    }, false);

    m4.addEventListener('change', function() {
    autoSend();
    }, false);

    //Change is when the slider is released after being moved
    r.addEventListener('change', function() {
    setColor();
    outputR.value = r.value;
    autoSend();
    }, false);

    //Input is when slider is being moved
    r.addEventListener('input', function() {
    setColor();
    outputR.value = r.value;
    }, false);
    
    g.addEventListener('change', function() {
    setColor();
    outputG.value = g.value;
    autoSend();
    }, false);
    
    g.addEventListener('input', function() {
    setColor();
    outputG.value = g.value;
    }, false);
    
    b.addEventListener('change', function() {
    setColor();
    outputB.value = b.value;
    autoSend();
    }, false);

    b.addEventListener('input', function() {
    setColor();
    outputB.value = b.value;
    }, false);
    
    bright.addEventListener('input', function() {
    outputBright.value = bright.value;
    }, false);
    
    bright.addEventListener('change', function() {
    outputBright.value = bright.value;
    autoSend();
    }, false);

    delay.addEventListener('input', function() {
    outputDelay.value = delay.value/1000 + " sec";
    }, false);
    delay.addEventListener('change', function() {
    outputDelay.value = delay.value/1000 + " sec";
    autoSend();
    }, false);

    
    function autoSend()
    {
      autosend = document.querySelector('#AutoSend');
      var urlVar = buildSendURL();
      document.getElementById("url").innerHTML = urlVar;  
      if (autosend.checked == true)
      {
          sendValues();
      }      
    }

    function autoRead()
    {
      autoread = document.querySelector('#AutoRead');
      if (autoread.checked == true)
      {
          readValuesFromNodeandSet();
      }      
    }

    //var updateDelay = 10000;
    setInterval(() => {autoRead();}, (10 * 1000));
    setInterval(() => {autoSend();}, (10 * 1000));
    //setInterval(() => {updateText("raspiShutdown");}, updateDelay );
    //setInterval(() => {updateText("raspiShutdownDetected");}, updateDelay );
    
    </script>
</html>)rawliteral";
