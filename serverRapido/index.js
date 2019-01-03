let net = require('net')

var server = net.createServer(function(c) { //'connection' listener
  console.log('server connected');
  c.on('end', function() {
    console.log('server disconnected');
  })

  c.on('data', (data) => {
    console.log(""+data)
  })

  c.write('hello \r\n');
  //c.pipe(c);
});
server.listen(8080, function() { //'listening' listener
  console.log('server bound');
});
