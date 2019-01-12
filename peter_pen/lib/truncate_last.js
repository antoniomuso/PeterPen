let fs = require('fs')

function truncate_last_byte(filename ,to_vanquish_size, cb) {
    fs.stat(filename, (err, stats) => {
        if (err) throw err;
        fs.truncate(filename, stats.size - to_vanquish_size, (err) => {
            if (err) throw err;
            if (cb) cb()
        })
    });
}

module.exports = truncate_last_byte