let SOCKET_URL = "ws://" + window.location.host + "/ws";


var wifiServ = new WifiServ();
var viewModel = new Binder(wifiServ, ko).bind();
var applyBindings = new Promise(resolve => {
    resolve(viewModel);
});
setTimeout(() => {
    applyBindings.then(() => ko.applyBindings(viewModel));
}, 100);
wifiServ.initWebSocket(SOCKET_URL);
