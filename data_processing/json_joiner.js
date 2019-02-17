const fs = require('fs')
const path = require('path')
let path_json1 = process.argv[2]
let path_json2 = process.argv[3]

let json1 = require(path_json1)
let json2 = require(path_json2)

let json_concat = json1.concat(json2)
fs.writeFileSync(
    path.join(
        `${process.argv[4] ? process.argv[4] : path.dirname(path_json1)}`
        ,`${path.basename(path_json1, '.json')}_concat_${path.basename(path_json2), '.json'}.json`
        ), JSON.stringify(json_concat))