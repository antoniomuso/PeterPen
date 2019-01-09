let net = require('net')
let fs = require('fs')

var server = net.createServer(function(c) { //'connection' listener
  let data_buff = []
  let inRead = false
  console.log('server connected');
  c.on('end', function() {
    console.log('server disconnected');
  })

  let file_name = Date.now().toString() + ".json"
  let writeStream = fs.createWriteStream(`out/${file_name}`)
  c.pipe(writeStream)

  
  c.write('hello \r\n');
  //c.pipe(c);
});
server.listen(8080, function() { //'listening' listener
  console.log('server bound');
});
