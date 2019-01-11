let net = require('net')
let fs = require('fs')
let th2 = require('through2')
let trunc_file = require('./lib/truncate_last')

const NEW_LINE = 10
const COMMA = 44

var server = net.createServer(function(c) { //'connection' listener

  console.log('server connected');
  c.on('end', function() {
    console.log('server disconnected');
  })
  
  let file_name = Date.now().toString() + ".json"
  let file_path = `out/${file_name}`
  let writeStream = fs.createWriteStream(file_path)
  writeStream.write("[")

  c.setTimeout(1000 * 60 * 3,() => {
    trunc_file(file_path, 1, () => {
      fs.appendFile(file_path,"]",(err) => {
        if (err) console.err(`File name ${file_name} error: ${err}`)
      })
    })
    c.unpipe()
    c.end()
  })
  
  c.pipe(th2((chunk, enc, callback) => {    
    for (let i = 0; i < chunk.length; i++) {
      if (chunk[i] == NEW_LINE) {
        chunk[i] = COMMA
      }
    }
    callback(null, chunk)
  })).pipe(writeStream).addListener('finish',() => fs.appendFile(file_path,"]",(err) => {
    if (err) console.err(`File name ${file_name} error: ${err}`)
  }))

  c.on('error', (err) => {
    if (err) console.log(err)
  })

});
server.listen(8080, function() { //'listening' listener
  console.log('server bound');
});
