const wsUri = "ws://localhost:%d";
let output;

function uuidv4() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}

function convertToEpoch(dateStr) {
  if (dateStr) return new Date(dateStr).getTime();
  return null
}

function init() {
  var HTML = `
   <meta charset = "utf-8" />
   <title>WebSocket Test</title>
   <h2>WebSocket Test</h2>
   <div id = "output"></div>
  `;

  // https://stackoverflow.com/a/51432177/14227520
  document.body.insertAdjacentHTML("beforeend", HTML);

  output = document.getElementById("output");

  if(typeof(output) === 'undefined' || output === null) {
    output = document.createElement('div');
    output.setAttribute("id", "output");
  }
  testWebSocket();
}

function testWebSocket() {
  websocket = new WebSocket(wsUri);

  websocket.onopen = function(evt) {
    onOpen(evt)
  };

  websocket.onclose = function(evt) {
    onClose(evt)
  };

  websocket.onerror = function(evt) {
    onError(evt)
  };

  websocket.onmessage = function(evt) {
    onMessage(evt)
  }
}

function onMessage(evt) {
  const req = JSON.parse(evt.data)
  if (req.eventId === "callApiMethod") {
    const [ funcName, args ] = req.eventArgs;
    const fn = eval(funcName)
    if (fn) {
      writeToScreen("Received " + JSON.stringify(args), "received")
      const timeFromEpoch = fn(...args)
      const payload = {
        responseRequestId: uuidv4(),
        eventId: 'apiCallResponse',
        eventArgs: [ { success: true, result: timeFromEpoch } ]
      }
      doSend(JSON.stringify(payload), "sent");
    }
  }
}

function onOpen(evt) {
  writeToScreen("CONNECTED to: " + wsUri);
  doSend("Connected from: " + window.navigator.userAgent);
}

function onClose(evt) {
  writeToScreen("DISCONNECTED");
}

function onError(evt) {
  writeToScreen('<span style = "color: red;">ERROR:</span> ' + evt.data);
}

function doSend(message, id) {
  writeToScreen("SENT: " + message, id);
  websocket.send(message);
}

function writeToScreen(message, id) {
  const element = document.getElementById(id);
  if (!element) {
    let pre = document.createElement("p");
    if (id) pre.setAttribute("id", id)
    pre.style.wordWrap = "break-word";
    pre.innerHTML = message;
    output.appendChild(pre);
  } else {
    element.innerHTML = message;
  }
}

init()
