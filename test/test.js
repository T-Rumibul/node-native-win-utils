// TODO: Write proper tests 

//import {textRecognition} from '../dist';
const {textRecognition} = require('../dist');

const path = require('path')

console.log(textRecognition(path.join(__dirname, 'traineddata'), 'eng', path.join(__dirname, 'images', '1.jpg')))
