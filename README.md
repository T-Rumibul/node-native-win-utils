
[![License][license-src]][license-href] 
![Node-API v8 Badge](https://github.com/nodejs/abi-stable-node/blob/doc/assets/Node-API%20v8%20Badge.svg)

#### USDT TRC20 - TYAJ3K3MZraJhWimxxeCKcJ2SYABkVsrzi
####  USDT TON - UQDokuYZXr4OHvfslDqUoFYcp1_F8tcjQPk_TvqSSDk7SIa7
####  BTC - 1A3mNKFdWKXZ7Bcnez8LbWbYHgck1g4GeV
####  NOT - UQDokuYZXr4OHvfslDqUoFYcp1_F8tcjQPk_TvqSSDk7SIa7

  

####  Will be very thankful for any support

  

# Node Native Win Utils

  

I did it for myself because I didn't feel like dealing with libraries like 'node-ffi' to implement this functionality. Maybe someone will find it useful. It's WINDOWS OS ONLY

  

This package is a native addon for Node.js that allows you to perform various utility operations on Windows systems. It includes key event listeners, window data retrieval, window screenshot capture functionality, mouse movement, mouse click, mouse drag, and typing functionality, also I included precompiled libs of OpenCV(core, imgcodecs, imgproc)

  
# Installation

  

You can install the package using npm:

  

```shell

  

npm  install  node-native-win-utils

  

```

  

# Usage

  

## Importing the Package

  

To use the package, import the necessary functions, types, and classes:

  

```javascript

import {

KeyboardListener

getWindowData,

captureWindow,

captureWindowN,

mouseMove,

mouseClick,

mouseDrag,

typeString,

OpenCV,

} from  "node-native-win-utils";

```

  

## Keyboard Event Listener

  

The package provides KeyboardListener singletone class, which allow you to register event listeners for key down and key up events, respectively. The event callbacks receive the `keyCode` as a parameter:

  

```javascript

const listener = KeyboardListener.listener()

listener.on("keyDown", (data: {
        keyCode: number;
        keyName: string;
      }) => {
//your code
})
listener.on("keyUp", (data: {
        keyCode: number;
        keyName: string;
      }) => {
    //your code
})
```

## Key Press

  

The keyPress function allows you to simulate a key press event. Provide the keyCode as a parameter and optionally the number of times to press the key:

  

```javascript

keyPress(65); // Press 'A' key once

  

keyPress(65, 3); // Press 'A' key three times

```

## Key Code Helper

  

The KeyCodeHelper enum provides a set of key codes that can be used with key event functions:

  

```javascript

  

import { KeyCodeHelper } from  "node-native-win-utils";

  

console.log(KeyCodeHelper.A); // Outputs the key code for 'A'

```

  

## Window Data

  

The `getWindowData` function retrieves information about a specific window identified by its name. It returns an object with properties `width`, `height`, `x`, and `y`, representing the window dimensions and position:

  

```javascript

const  windowData  =  getWindowData("Window Name");

  

console.log("Window data:", windowData);

  

// Window data: { width: 800, height: 600, x: 50, y: 50 }

```

  

## Window Capture

  

The `captureWindow` function allows you to capture a screenshot of a specific window identified by its name. Provide the window name and the output path as parameters:

  

```javascript

captureWindow("Window Name", "output.png");

  

// Output: output.png with a screenshot of the window

```

  

## Mouse Movement

  

The `mouseMove` function allows you to move the mouse to a specific position on the screen. Provide the `posX` and `posY` coordinates as parameters:

  

```javascript

mouseMove(100, 200);

```

  

## Mouse Click

  

The `mouseClick` function allows you to perform a mouse click event. Optionally, you can specify the mouse button as a parameter (`"left"`, `"middle"`, or `"right"`). If no button is specified, a left mouse click is performed by default:

  

```javascript

mouseClick(); // Left mouse click

  

mouseClick("right"); // Right mouse click

```

  

## Mouse Drag

  

The `mouseDrag` function allows you to simulate dragging the mouse from one position to another. Provide the starting and ending coordinates (`startX`, `startY`, `endX`, `endY`) as parameters. Optionally, you can specify the speed at which the mouse should be dragged:

  

```javascript

mouseDrag(100, 200, 300, 400);

  

mouseDrag(100, 200, 300, 400, 100); // Drag with speed 100

```

  

## Typing

  

The `typeString` function allows you to simulate typing a string of characters. Provide the string to type as the `stringToType` parameter. Optionally, you can specify

  

a delay between each character (in milliseconds) using the `delay` parameter:

  

```javascript

typeString("Hello, world!");

  

typeString("Hello, world!", 100); // Type with a delay of 100ms between characters

```

  

## Key Listener Class

  

The `KeyListener` class extends the EventEmitter class and simplifies working with the `keyDownHandler` and `keyUpHandler` functions. You can register event listeners for the "keyDown" and "keyUp" events using the `on` method:

  

```javascript

const  listener  =  new  KeyListener();

  

listener.on("keyDown", (data) => {

console.log("Key down:", data.keyCode, data.keyName);

});

  

// Key down: 8 Backspace

  

listener.on("keyUp", (data) => {

console.log("Key up:", data.keyCode, data.keyName);

});

  

// Key up: 8 Backspace

```

  

## OpenCV

  

The `OpenCV` class extends the capabilities of the native addon package by providing various image processing functionalities. It allows users to perform operations such as matching templates, blurring images, converting color formats, drawing rectangles, getting image regions, and writing images to files.

  

#### Constructor

  

```typescript

const  image  =  new  OpenCV(image: string  |  ImageData)

```

  

Creates a new instance of the `OpenCV` class with the specified image data. The `image` parameter can be either a file path (string) or an existing `ImageData` object.

  

#### Properties

  

##### `imageData: ImageData`

  

Holds the underlying image data that will be used for image processing operations.

  

##### `width: number`

  

Read-only property that returns the width of the image in pixels.

  

##### `height: number`

  

Read-only property that returns the height of the image in pixels.

  

#### Methods

  

##### `matchTemplate(template: ImageData, method?: number | null, mask?: ImageData): OpenCV`

  

Matches a template image with the current image and returns a new `OpenCV` instance containing the result.

  

-  `template: ImageData`: The template image data to be matched.

-  `method?: number | null`: (Optional) The matching method to be used. If not provided, the default method will be used.(currently no implemented)

-  `mask?: ImageData`: (Optional) An optional mask image data to be used during the matching process.

  

##### `blur(sizeX: number, sizeY: number): OpenCV`

  

Applies a blur filter to the current image and returns a new `OpenCV` instance containing the blurred result.

  

-  `sizeX: number`: The size of the blur kernel in the X direction.

-  `sizeY: number`: The size of the blur kernel in the Y direction.

  

##### `bgrToGray(): OpenCV`

  

Converts the current image from the BGR color format to grayscale and returns a new `OpenCV` instance containing the grayscale result.

  

##### `drawRectangle(start: Point, end: Point, rgb: Color, thickness: number): OpenCV`

  

Draws a rectangle on the current image and returns a new `OpenCV` instance containing the modified result.

  

-  `start: Point`: The starting point (top-left) of the rectangle.

-  `end: Point`: The ending point (bottom-right) of the rectangle.

-  `rgb: Color`: The color of the rectangle in the RGB format (e.g., `{ r: 255, g: 0, b: 0 }` for red).

-  `thickness: number`: The thickness of the rectangle's border.

  

##### `getRegion(region: ROI): OpenCV`

  

Extracts a region of interest (ROI) from the current image and returns a new `OpenCV` instance containing the extracted region.

  

-  `region: ROI`: An object specifying the region of interest with properties `x`, `y`, `width`, `height`.

  

##### `imwrite(path: string): void`

  

Writes the current image to a file specified by the `path`.

  

-  `path: string`: The file path where the image will be saved.

  

## Functions

  

| Function        | Parameters                                                                                   | Return Type |
|-----------------|----------------------------------------------------------------------------------------------|-------------|
| getWindowData   | `windowName: string`                                                                          | `WindowData`|
| captureWindow   | `windowName: string, outputPath: string`                                                      | `void`      |
| mouseMove       | `posX: number, posY: number`                                                                  | `boolean`   |
| mouseClick      | `button?: "left" \| "middle" \| "right"`                                                      | `boolean`   |
| mouseDrag       | `startX: number, startY: number, endX: number, endY: number, speed?: number`                  | `boolean`   |
| typeString      | `stringToType: string, delay?: number`                                                        | `boolean`   |
| captureWindowN  | `windowName: string`                                                                          | `Buffer`    |
| keyPress        | `keyCode: number, repeat?: number`                                                            | `boolean`   |
| textRecognition | `trainedDataPath: string, dataLang: string, imagePath: string`                                | `string`    |
| captureScreenToFile | `path: string`                                                                            | `boolean`   |
  

## Examples

  

Here are some examples of using the package:

```javascript
console.log(textRecognition(path.join(__dirname, 'traineddata'), 'eng', path.join(__dirname, 'images', '1.jpg'))) // ---> <recognized text>

```

```javascript

// Example usage of the OpenCV class

import { OpenCV } from  "node-native-win-utils";

  

const  image  =  new  OpenCV("path/to/image.png");

  

const  template  =  new  OpenCV("path/to/template.png");

const  matchedImage  =  image.matchTemplate(template.imageData);

  

const  blurredImage  =  image.blur(5, 5);

  

const  grayscaleImage  =  image.bgrToGray();

  

const  regionOfInterest  = { x:  100, y:  100, width:  200, height:  150 };

const  regionImage  =  image.getRegion(regionOfInterest);

  

const  redColor  = { r:  255, g:  0, b:  0 };

const  thickRectangle  =  image.drawRectangle(

{ x:  50, y:  50 },

{ x:  150, y:  150 },

redColor,

3

);

  

matchedImage.imwrite("output/matched.png");

blurredImage.imwrite("output/blurred.png");

grayscaleImage.imwrite("output/grayscale.png");

regionImage.imwrite("output/region.png");

thickRectangle.imwrite("output/thick_rectangle.png");

```

  

```javascript

// If you want to aply blur and convert to gray then do it that order:

image.blur(5, 5).bgrToGray();

// Otherwise you will get an error.

```

  

Please note that the above example demonstrates the usage of different methods available in the `OpenCV` class. Make sure to replace `"path/to/image.png"` and `"path/to/template.png"` with actual image file paths.

  

```javascript

import {

getWindowData,

captureWindow,

mouseMove,

mouseClick,

mouseDrag,

typeString,

KeyListener,

} from  "node-native-win-utils";

  

// Retrieve window data

  

const  windowData  =  getWindowData("My Window");

  

console.log("Window data:", windowData);

  

// Window data: { width: 1024, height: 768, x: 100, y: 100 }

  

// Capture window screenshot

  

captureWindow("My Window", "output.png");

  

// Output: output.png with a screenshot of the window

  

// Move the mouse

  

mouseMove(100, 200);

  

// Perform mouse click

  

mouseClick(); // Left mouse click

  

mouseClick("right"); // Right mouse click

  

// Simulate mouse drag

  

mouseDrag(100, 200, 300, 400);

  

mouseDrag(100, 200, 300, 400, 100); // Drag with speed 100

  

// Simulate typing

  

typeString("Hello, world!");

  

typeString("Hello, world!", 100); // Type with a delay of 100ms between characters

  

// Use KeyboardListener class

  

const  listener  =  KeyboardListener.listener();

  

listener.on("keyDown", (data) => {

console.log("Key down:", data.keyCode, data.keyName);

});

  

// Key down: keyCode keyName

  

listener.on("keyUp", (data) => {

console.log("Key up:", data.keyCode, data.keyName);

});

  

// Key up: keyCode keyName

  
  

```

[OpenCV License](https://github.com/opencv/opencv/blob/master/LICENSE)
[MIT License](https://github.com/T-Rumibul/node-native-win-utils/blob/main/LICENSE)
  

[license-src]: https://img.shields.io/github/license/nuxt-modules/icon.svg?style=for-the-badge&colorA=18181B&colorB=28CF8D

[license-href]: https://github.com/T-Rumibul/node-native-win-utils/blob/main/LICENSE
