// TODO: Write proper tests 

import {textRecognition} from '../dist/index.mjs';
//const {textRecognition} = require('../dist');

import path from 'path'

console.log(textRecognition(path.join(import.meta.dirname, 'traineddata'), 'eng', path.join(import.meta.dirname, 'images', '1.jpg')))
