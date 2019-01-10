let net = require('net')
let fs = require('fs')
let th2 = require('through2')

const NEW_LINE = 10
const COMMA = 44

var server = net.createServer(function(c) { //'connection' listener
  let data_buff = []
  let inRead = false
  console.log('server connected');
  c.on('end', function() {
    console.log('server disconnected');
  })

  let file_name = Date.now().toString() + ".json"
  let writeStream = fs.createWriteStream(`out/${file_name}`)
  
  c.pipe(th2((chunk, enc, callback) => {
    chunk = Buffer.concat([Buffer.from("["), chunk, Buffer.from("]")])
    for (let i = 0; i < chunk.length; i++) {
      if (chunk[i] == NEW_LINE) {
        chunk[i] = COMMA
      }
    }
    callback(null, chunk)
  })).pipe(writeStream)

  
  c.write('hello \r\n');
  //c.pipe(c);
});
server.listen(8080, function() { //'listening' listener
  console.log('server bound');
});
