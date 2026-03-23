var udp = require('dgram');
var buffer = require('buffer');
const axios = require('axios');
const { getCachedIndicator, updateSnapshots } = require('./priceTracker');
const log = require('./logger');

// --------------------creating a udp server --------------------

var server = udp.createSocket('udp4');

let btcprice = 0;

// emits when any error occurs
server.on('error', function (error) {
  log.info('Error: ' + error);
});

// emits on new datagram msg
server.on('message', function (msg, info) {
  log.debug(`Data received from client : |${msg.toString()}|`);
  log.debug(`Received ${msg.length} bytes from ${info.address}`);

  if (msg.toString() === "RBTCUSDT") {
    response = Buffer.from(("" + btcprice).padEnd(16, "\0"));
  }

  else if (msg.toString() === "FBTCUSDT") {
    // Format for easy processing in a clock
    const splitup = btcprice.toString().split('.');
    const digits = splitup[0].length;
    log.debug("Digits: " + digits)
    if (digits > 6) {
      response = Buffer.from("ERROR");
    } else {
      let fixedWidthReturn = btcprice.toString().replace('.', '').substring(0, 6);
      fixedWidthReturn = fixedWidthReturn.padStart(6, '0');
      fixedWidthReturn = digits + fixedWidthReturn;
      log.debug('Formatted return: |' + fixedWidthReturn + '|');
      response = Buffer.from(fixedWidthReturn.padEnd(16, "\0"));
    }
  }

  else if (msg.toString() === "IBTCUSDT") {
    // Format for easy processing in a clock
    const splitup = btcprice.toString().split('.');
    const digits = splitup[0].length;
    log.debug("Digits: " + digits)
    if (digits > 6) {
      response = Buffer.from("ERROR");
    } else {
      let fixedWidthReturn = splitup[0];
      fixedWidthReturn = fixedWidthReturn.padStart(6, '0');
      fixedWidthReturn = fixedWidthReturn + ";" + getCachedIndicator();
      log.debug('Formatted return: |' + fixedWidthReturn + '|');
      response = Buffer.from(fixedWidthReturn.padEnd(16, "\0"));
    }
  }

  else {
    response = Buffer.from("ERROR");
  }

  //sending msg
  server.send(response, info.port, info.address, function (error) {
    if (error) {
      client.close();
    } else {
      log.debug('Sent ' + response.toString());
    }
  });
});

//emits when socket is ready and listening for datagram msgs
server.on('listening', function () {
  var address = server.address();
  var port = address.port;
  var family = address.family;
  var ipaddr = address.address;
  log.info('Server is listening at port ' + port);
  log.info('Server ip :' + ipaddr);
  log.info('Server is IP4/IP6 : ' + family);
});

//emits after the socket is closed using socket.close();
server.on('close', function () {
  log.info('Socket is closed !');
});

server.bind(2222);

function getBTCPrice() {
  let url = "https://api.diadata.org/v1/assetQuotation/Bitcoin/0x0000000000000000000000000000000000000000";

  axios({
    method: 'get',
    url
  })
    .then(function (response) {
//      console.log(response.data);
//      console.log("Price: " + btcprice);
      btcprice = Math.round(response.data.Price * 100) / 100;
      updateSnapshots(btcprice, response.data.Time);
    })
    .catch(function (error) {
      log.info('Price fetch error: ' + error);
    });
}

// Get initial price
getBTCPrice();

// Loop forever and get prices
setInterval(() => {
  getBTCPrice();
}, 60000);


