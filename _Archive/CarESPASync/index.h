//const char index_html[] PROGMEM = R"rawliteral(
const char index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
  <title>ESP8266| Raspberry Pi Control Server</title>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html {
       font-family: Arial;
       display: inline-block;
       margin: 0px auto;
       text-align: center;
      }
      h2 { font-size: 3.0rem; }
      p { font-size: 3.0rem; }
      .units { font-size: 1.2rem; }
      .esp-values
      {
        font-size: 1.5rem;
        vertical-align:middle;
        padding-bottom: 15px;
      }
    </style>
  </head>
  <body>
    <h2>ESP8266|Raspberry Pi Control Server</h2>
    <p>
      <span class="esp-values">Ignition Status: </span> 
      <span id="ignitionStatus" style="color:purple">Unknown</span>
    </p>
    <p>
      <span class="esp-values">Raspberry Pi Status: </span>
      <span id="raspiStatus" style="color:purple">Unknown</span>
    </p>
    <p>
      <span class="esp-values">Raspberry Pi Shutdown Triggered: </span> 
      <span id="raspiShutdown" style="color:purple">Unknown</span>
    </p>
    <p>
      <span class="esp-values">Raspberry Pi Shutdown Detected: </span> 
      <span id="raspiShutdownDetected" style="color:purple">Unknown</span>
    </p>
  </body>

  <!--https://stackoverflow.com/questions/247483/http-get-request-in-javascript-->
  <script>

  


  function changeColor(colorId, responseValue) 
  {
    var elementId = document.getElementById(colorId);

    
    if (responseValue == "Off" || responseValue == "Not Triggered" || responseValue == "Not Detected")
    {
      elementId.style.color = "red";
    }
    else if (responseValue == "On" || responseValue == "Triggered" || responseValue == "Detected")
    {
      elementId.style.color = "green";
    }
    else
    {
      elementId.style.color = "purple";
    }
      
  }

  
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
    var urlVar = "/"+statusId;
    xhttp.open("GET", urlVar, true);
    xhttp.send(null);
  }


  var updateDelay = 500;
  setInterval(() => {updateText("ignitionStatus");}, updateDelay);
  setInterval(() => {updateText("raspiStatus");}, updateDelay );
  setInterval(() => {updateText("raspiShutdown");}, updateDelay );
  setInterval(() => {updateText("raspiShutdownDetected");}, updateDelay );



  
  
  </script>
  </html>)rawliteral";
