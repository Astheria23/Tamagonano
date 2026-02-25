#include "WebDashboard.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <time.h>

static AsyncWebServer server(80);
static PageType *pageRef = nullptr;
static PomodoroState *pomoRef = nullptr;

void setWebPageRef(PageType *page) { pageRef = page; }
void setWebPomodoroRef(PomodoroState *state) { pomoRef = state; }

static const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Tamagonano Dashboard</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:'Segoe UI',sans-serif;background:#0f0f0f;color:#e0e0e0;display:flex;flex-direction:column;align-items:center;padding:20px}
h1{font-size:1.6rem;margin-bottom:6px;color:#00e5ff}
.sub{font-size:.8rem;color:#666;margin-bottom:18px}
.card{background:#1a1a1a;border:1px solid #333;border-radius:12px;padding:16px;width:100%;max-width:400px;margin-bottom:14px}
.card h2{font-size:1rem;color:#aaa;margin-bottom:10px}
.status-row{display:flex;justify-content:space-between;padding:4px 0;font-size:.9rem}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;margin-right:6px}
.dot.on{background:#4caf50}
.dot.off{background:#f44336}
.btn-row{display:flex;gap:8px;flex-wrap:wrap;margin-top:8px}
.btn{flex:1;min-width:80px;padding:10px;border:none;border-radius:8px;font-size:.85rem;font-weight:600;cursor:pointer;color:#fff;transition:opacity .2s}
.btn:active{opacity:.7}
.btn-page{background:#1565c0}
.btn-pomo{background:#e65100}
.btn-misc{background:#2e7d32}
#statusArea{font-size:.85rem;color:#999;margin-top:4px;min-height:20px}
</style>
</head>
<body>
<h1>Tamagonano</h1>
<p class="sub">ESP32-C3 Dashboard</p>

<div class="card">
 <h2>Status</h2>
 <div id="statusBox">Loading...</div>
</div>

<div class="card">
 <h2>Navigation</h2>
 <div class="btn-row">
  <button class="btn btn-page" onclick="send('/cmd?action=prev')">&#9664; Prev</button>
  <button class="btn btn-page" onclick="send('/cmd?action=next')">Next &#9654;</button>
 </div>
</div>

<div class="card">
 <h2>Pomodoro</h2>
 <div class="btn-row">
  <button class="btn btn-pomo" onclick="send('/cmd?action=pomo_toggle')">Start / Pause</button>
  <button class="btn btn-pomo" onclick="send('/cmd?action=pomo_reset')">Reset</button>
 </div>
</div>

<div class="card">
 <h2>Quick Actions</h2>
 <div class="btn-row">
  <button class="btn btn-misc" onclick="send('/cmd?action=face')">Face</button>
  <button class="btn btn-misc" onclick="send('/cmd?action=clock')">Clock</button>
  <button class="btn btn-misc" onclick="send('/cmd?action=pomodoro')">Pomodoro</button>
 </div>
</div>

<div id="statusArea"></div>

<script>
function send(url){
 fetch(url).then(r=>r.text()).then(t=>{
  document.getElementById('statusArea').textContent='> '+t;
  refreshStatus();
 });
}
function refreshStatus(){
 fetch('/status').then(r=>r.json()).then(d=>{
  let h='';
  h+='<div class="status-row"><span><span class="dot '+(d.wifi?'on':'off')+'"></span>WiFi</span><span>'+(d.wifi?d.ip:'Disconnected')+'</span></div>';
  h+='<div class="status-row"><span><span class="dot '+(d.ntp?'on':'off')+'"></span>NTP</span><span>'+(d.ntp?'Synced':'Not synced')+'</span></div>';
  h+='<div class="status-row"><span>Page</span><span>'+d.page+'</span></div>';
  h+='<div class="status-row"><span>Pomodoro</span><span>'+d.pomo+'</span></div>';
  h+='<div class="status-row"><span>Uptime</span><span>'+d.uptime+'</span></div>';
  h+='<div class="status-row"><span>Free Heap</span><span>'+d.heap+' B</span></div>';
  document.getElementById('statusBox').innerHTML=h;
 });
}
refreshStatus();
setInterval(refreshStatus,2000);
</script>
</body>
</html>
)rawliteral";

static const char *pageNames[] = {"Face", "Clock", "Pomodoro"};

void setupWebServer() {
  // Dashboard
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", DASHBOARD_HTML);
  });

  // Status JSON
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    char json[320];
    uint32_t sec = millis() / 1000;
    uint32_t h = sec / 3600, m = (sec % 3600) / 60, s = sec % 60;

    char pomoStr[32];
    if (pomoRef) {
      uint32_t pm = pomoRef->remainingSec / 60;
      uint32_t ps = pomoRef->remainingSec % 60;
      snprintf(pomoStr, sizeof(pomoStr), "%s %02lu:%02lu",
               pomoRef->running ? "Running" : "Paused",
               static_cast<unsigned long>(pm),
               static_cast<unsigned long>(ps));
    } else {
      snprintf(pomoStr, sizeof(pomoStr), "N/A");
    }

    snprintf(json, sizeof(json),
             "{\"wifi\":%s,\"ip\":\"%s\",\"ntp\":%s,\"page\":\"%s\","
             "\"pomo\":\"%s\",\"uptime\":\"%luh%02lum%02lus\",\"heap\":%lu}",
             wifiConnected ? "true" : "false",
             WiFi.localIP().toString().c_str(),
             ntpSynced ? "true" : "false",
             pageRef ? pageNames[*pageRef] : "?",
             pomoStr,
             static_cast<unsigned long>(h),
             static_cast<unsigned long>(m),
             static_cast<unsigned long>(s),
             static_cast<unsigned long>(ESP.getFreeHeap()));
    request->send(200, "application/json", json);
  });

  // Commands
  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("action")) {
      request->send(400, "text/plain", "Missing action");
      return;
    }
    String action = request->getParam("action")->value();

    if (action == "prev" && pageRef) {
      *pageRef = static_cast<PageType>((*pageRef + 2) % 3);
      request->send(200, "text/plain", String("Page: ") + pageNames[*pageRef]);
    } else if (action == "next" && pageRef) {
      *pageRef = static_cast<PageType>((*pageRef + 1) % 3);
      request->send(200, "text/plain", String("Page: ") + pageNames[*pageRef]);
    } else if (action == "face" && pageRef) {
      *pageRef = PAGE_FACE;
      request->send(200, "text/plain", "Page: Face");
    } else if (action == "clock" && pageRef) {
      *pageRef = PAGE_CLOCK;
      request->send(200, "text/plain", "Page: Clock");
    } else if (action == "pomodoro" && pageRef) {
      *pageRef = PAGE_POMODORO;
      request->send(200, "text/plain", "Page: Pomodoro");
    } else if (action == "pomo_toggle" && pomoRef) {
      if (pomoRef->remainingSec == 0) {
        pomoRef->remainingSec = 25 * 60;
        pomoRef->running = true;
        pomoRef->lastTickMs = millis();
      } else {
        pomoRef->running = !pomoRef->running;
        pomoRef->lastTickMs = millis();
      }
      request->send(200, "text/plain",
                     pomoRef->running ? "Pomodoro: Running" : "Pomodoro: Paused");
    } else if (action == "pomo_reset" && pomoRef) {
      pomoRef->running = false;
      pomoRef->remainingSec = 25 * 60;
      pomoRef->lastTickMs = millis();
      request->send(200, "text/plain", "Pomodoro: Reset");
    } else {
      request->send(400, "text/plain", "Unknown action");
    }
  });

  server.begin();
}
