#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* wifiName = "STM32_RC_CAR";
const char* wifiPassword = "12345678";

IPAddress localIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

/* The car starts in manual mode. */
bool autonomousMode = false;

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">

  <meta name="viewport"
        content="width=device-width, initial-scale=1, user-scalable=no">

  <style>
    body {
      text-align: center;
      font-family: Arial, sans-serif;
      background: #eeeeee;
      touch-action: none;
      user-select: none;
    }

    .controls {
      display: grid;
      grid-template-columns: 90px 90px 90px;
      gap: 12px;
      justify-content: center;
      margin-top: 30px;
    }

    button {
      width: 90px;
      height: 70px;
      font-size: 28px;
      border: none;
      border-radius: 12px;
      color: white;
      background: #2878c8;
    }

    button:active {
      opacity: 0.7;
    }

    .stop {
      background: #d43c3c;
      font-size: 20px;
    }

    .speed {
      background: #555555;
    }

    .mode {
      background: #e08a1e;
      font-size: 18px;
    }

    .mode-display {
      width: 282px;
      margin: 22px auto 0;
      padding: 18px 0;
      border: 4px solid #333333;
      border-radius: 10px;
      background: #15261d;
      color: #75ff96;
      font-family: "Courier New", monospace;
      font-size: 22px;
      font-weight: bold;
      box-shadow: inset 0 0 8px #000000;
    }

    .autonomous-display {
      background: #261b14;
      color: #ffbc66;
    }
  </style>
</head>

<body>
  <h2>STM32 RC Car</h2>

  <div class="controls">
    <div></div>
    <button class="drive" data-command="F">&#9650;</button>
    <div></div>

    <button class="drive" data-command="L">&#9664;</button>
    <button class="stop" id="stopButton">STOP</button>
    <button class="drive" data-command="R">&#9654;</button>

    <div></div>
    <button class="drive" data-command="B">&#9660;</button>
    <div></div>

    <button class="speed" id="slowerButton">&#8722;</button>
    <button class="mode" id="modeButton">MODE</button>
    <button class="speed" id="fasterButton">+</button>
  </div>

  <div class="mode-display" id="modeDisplay">
    MANUAL MODE
  </div>

  <script>
    let repeatTimer = null;
    let activeCommand = "S";
    let autonomousMode = false;

    function sendCommand(command) {
      fetch("/command?value=" + encodeURIComponent(command), {
        cache: "no-store"
      }).catch(function() {});
    }

    function startDriving(command) {
      /*
       * Direction buttons only operate while the car is
       * in manual mode.
       */
      if (autonomousMode) {
        return;
      }

      activeCommand = command;
      sendCommand(command);

      clearInterval(repeatTimer);

      repeatTimer = setInterval(function() {
        sendCommand(activeCommand);
      }, 150);
    }

    function stopDriving() {
      clearInterval(repeatTimer);
      repeatTimer = null;
      activeCommand = "S";
      sendCommand("S");
    }

    function updateModeDisplay() {
      const display = document.getElementById("modeDisplay");

      if (autonomousMode) {
        display.textContent = "AUTONOMOUS MODE";
        display.classList.add("autonomous-display");
      } else {
        display.textContent = "MANUAL MODE";
        display.classList.remove("autonomous-display");
      }
    }

    function getCurrentMode() {
      fetch("/mode", {
        cache: "no-store"
      })
      .then(function(response) {
        return response.text();
      })
      .then(function(mode) {
        autonomousMode = mode.trim() === "A";
        updateModeDisplay();
      })
      .catch(function() {});
    }

    function toggleMode() {
      /*
       * Tell the ESP8266 to stop the car and change
       * between manual and autonomous mode.
       */
      clearInterval(repeatTimer);
      repeatTimer = null;
      activeCommand = "S";

      fetch("/toggle-mode", {
        cache: "no-store"
      })
      .then(function(response) {
        return response.text();
      })
      .then(function(mode) {
        autonomousMode = mode.trim() === "A";
        updateModeDisplay();
      })
      .catch(function() {});
    }

    document.querySelectorAll(".drive").forEach(function(button) {
      button.addEventListener("pointerdown", function(event) {
        event.preventDefault();

        if (button.setPointerCapture) {
          button.setPointerCapture(event.pointerId);
        }

        startDriving(button.dataset.command);
      });

      button.addEventListener("pointerup", function() {
        if (!autonomousMode) {
          stopDriving();
        }
      });

      button.addEventListener("pointercancel", function() {
        if (!autonomousMode) {
          stopDriving();
        }
      });
    });

    document.getElementById("stopButton")
      .addEventListener("pointerdown", function(event) {
        event.preventDefault();
        stopDriving();
      });

    document.getElementById("fasterButton")
      .addEventListener("pointerdown", function(event) {
        event.preventDefault();

        if (!autonomousMode) {
          sendCommand("+");
        }
      });

    document.getElementById("slowerButton")
      .addEventListener("pointerdown", function(event) {
        event.preventDefault();

        if (!autonomousMode) {
          sendCommand("-");
        }
      });

    document.getElementById("modeButton")
      .addEventListener("pointerdown", function(event) {
        event.preventDefault();
        toggleMode();
      });

    document.addEventListener("visibilitychange", function() {
      if (document.hidden && !autonomousMode) {
        stopDriving();
      }
    });

    window.addEventListener("pagehide", function() {
      if (!autonomousMode) {
        stopDriving();
      }
    });

    /* Read the current mode when the webpage opens. */
    getCurrentMode();
  </script>
</body>
</html>
)rawliteral";

bool validCommand(char command)
{
  return command == 'F' ||
         command == 'B' ||
         command == 'L' ||
         command == 'R' ||
         command == 'S' ||
         command == '+' ||
         command == '-';
}

void sendToSTM32(char command)
{
  Serial.write('<');
  Serial.write(command);
  Serial.write('>');
}

void showHomePage()
{
  server.sendHeader(
    "Cache-Control",
    "no-store, no-cache, must-revalidate"
  );

  server.send_P(
    200,
    "text/html; charset=utf-8",
    webpage
  );
}

void receiveWebCommand()
{
  if (!server.hasArg("value") ||
      server.arg("value").length() != 1)
  {
    server.send(400, "text/plain", "Invalid command");
    return;
  }

  char command = server.arg("value")[0];

  if (!validCommand(command))
  {
    server.send(400, "text/plain", "Invalid command");
    return;
  }

  /*
   * Ignore direction and speed commands while autonomous
   * mode is active. STOP is always accepted.
   */
  if (autonomousMode && command != 'S')
  {
    server.send(
      409,
      "text/plain",
      "Manual controls are disabled"
    );
    return;
  }

  sendToSTM32(command);
  server.send(200, "text/plain", "OK");
}

void sendCurrentMode()
{
  if (autonomousMode)
  {
    server.send(200, "text/plain", "A");
  }
  else
  {
    server.send(200, "text/plain", "M");
  }
}

void toggleMode()
{
  /*
   * Stop the motors before switching operating modes.
   */
  sendToSTM32('S');

  autonomousMode = !autonomousMode;

  if (autonomousMode)
  {
    sendToSTM32('A');
    server.send(200, "text/plain", "A");
  }
  else
  {
    sendToSTM32('M');
    server.send(200, "text/plain", "M");
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(wifiName, wifiPassword);

  server.on("/", showHomePage);
  server.on("/command", receiveWebCommand);
  server.on("/mode", sendCurrentMode);
  server.on("/toggle-mode", toggleMode);

  server.begin();

  /* Make sure the STM32 starts in manual mode. */
  sendToSTM32('M');
}

void loop()
{
  server.handleClient();
}