const char index_html[] = R"(<!DOCTYPE HTML>
  <html>
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
      .dht-labels{
        font-size: 1.5rem;
        vertical-align:middle;
        padding-bottom: 15px;
      }
    </style>
  </head>
  <body>
    <h2>ESP8266 Server</h2>
    <p>
      <span class="dht-labels">Ignition Status</span> 
      <span id="temperature">%TEMPERATURE%</span>
    </p>
    <p>
      <span class="dht-labels">Raspberry Pi Status</span>
      <span id="humidity">%HUMIDITY%</span>
    </p>
    <p>
      <span class="dht-labels">Temperature</span> 
      <span id="temperature">%TEMPERATURE%</span>
    </p>
  </body>
  </html>)";
