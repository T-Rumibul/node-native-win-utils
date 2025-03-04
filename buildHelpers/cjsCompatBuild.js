const exec = require('child_process').exec;
const fs = require('fs');
const path = require('path');



// const dirnameFilePath = path.resolve(__dirname, '..', 'src', 'dirnameLocal')

const srcFilePath = path.resolve(__dirname, '..', 'src')

// const dirNameLocalOriginal = fs.readFileSync(dirnameFilePath + '.mts').toString();
const indexOriginal = fs.readFileSync(path.resolve(srcFilePath, 'index.mts')).toString();

const extRegex = /(import\s+.*?\s+from\s+['"][^'"]+)(\.[a-zA-Z0-9]+)(['"])/g;


// Write cjs compatible code
fs.writeFileSync(path.resolve(srcFilePath, 'dirnameLocal.cts'), `export const __dirnameLocal = __dirname`)
fs.writeFileSync(path.resolve(srcFilePath, 'index.cts'), `//@ts-nocheck \n${indexOriginal.replace(extRegex, '$1.cjs$3')}`)
fs.writeFileSync(path.resolve(srcFilePath, 'keyCodes.cts'), fs.readFileSync(path.resolve(srcFilePath, 'keyCodes.mts')))


exec('npx tsc',
    function (error, stdout, stderr) {
        console.log(stdout);
        console.log(stderr);
        // Restore original file
            fs.unlinkSync(path.resolve(srcFilePath, 'dirnameLocal.cts'))
            fs.unlinkSync(path.resolve(srcFilePath, 'index.cts'))
            fs.unlinkSync(path.resolve(srcFilePath, 'keyCodes.cts'))
        if (error !== null) {
             console.log('exec error: ' + error);
        }
    });



