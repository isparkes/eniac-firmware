var udp = require('dgram');
var buffer = require('buffer');
const axios = require('axios');

// --------------------creating a udp server --------------------

var server = udp.createSocket('udp4');

let btcprice = 0;
let btcpriceyesterday = 0;

// emits when any error occurs
server.on('error', function (error) {
  console.log('Error: ' + error);
});

// emits on new datagram msg
server.on('message', function (msg, info) {
  console.log('Data received from client : |' + msg.toString() + '|');
  console.log('Received %d bytes from %s\n', msg.length, info.address);

  if (msg.toString() === "RBTCUSDT") {
    response = Buffer.from(("" + btcprice).padEnd(16, "\0"));
  }

  else if (msg.toString() === "FBTCUSDT") {
    // Format for easy processing in a clock
    const splitup = btcprice.toString().split('.');
    const digits = splitup[0].length;
    console.log("Digits: " + digits)
    if (digits > 6) {
      response = Buffer.from("ERROR");
    } else {
      let fixedWidthReturn = btcprice.toString().replace('.', '').substring(0, 6);
      fixedWidthReturn = fixedWidthReturn.padStart(6, '0');
      fixedWidthReturn = digits + fixedWidthReturn;
      console.log('Formatted return: |' + fixedWidthReturn + '|');
      response = Buffer.from(fixedWidthReturn.padEnd(16, "\0"));
    }
  }

  else if (msg.toString() === "IBTCUSDT") {
    // Format for easy processing in a clock
    const splitup = btcprice.toString().split('.');
    const digits = splitup[0].length;
    // console.log("Digits: " + digits)
    if (digits > 6) {
      response = Buffer.from("ERROR");
    } else {
      console.log(btcprice + ":" + btcpriceyesterday);
      let fixedWidthReturn = splitup[0];
      fixedWidthReturn = fixedWidthReturn.padStart(6, '0');
      if (btcprice > btcpriceyesterday) {
        fixedWidthReturn = fixedWidthReturn + ";U"
      } else if (btcprice < btcpriceyesterday) {
        fixedWidthReturn = fixedWidthReturn + ";D"
      } else {
        fixedWidthReturn = fixedWidthReturn + ";-"
      }
      console.log('Formatted return: |' + fixedWidthReturn + '|');
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
      console.log('Sent ' + response.toString());
    }
  });
});

//emits when socket is ready and listening for datagram msgs
server.on('listening', function () {
  var address = server.address();
  var port = address.port;
  var family = address.family;
  var ipaddr = address.address;
  console.log('Server is listening at port ' + port);
  console.log('Server ip :' + ipaddr);
  console.log('Server is IP4/IP6 : ' + family);
});

//emits after the socket is closed using socket.close();
server.on('close', function () {
  console.log('Socket is closed !');
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
      btcpriceyesterday = Math.round(response.data.PriceYesterday * 100) / 100;
//      console.log("Price yesterday: " + btcpriceyesterday);
    })
    .catch(function (error) {
      console.log(error);
    });
}

// Get initial price
getBTCPrice();

// Loop forever and get prices
setInterval(() => {
  getBTCPrice();
}, 60000);


