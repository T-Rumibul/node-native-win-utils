[![License][license-src]][license-href]
[![Node-API v8](https://raw.githubusercontent.com/nodejs/abi-stable-node/b062e657e639aad36280e0c6f54e181563ef4b19/assets/Node-API%20v8%20Badge.svg)](https://nodejs.org/api/n-api.html)
[![npm version](https://img.shields.io/npm/v/node-native-win-utils.svg)](https://www.npmjs.com/package/node-native-win-utils)
[![npm downloads](https://img.shields.io/npm/dm/node-native-win-utils.svg)](https://www.npmjs.com/package/node-native-win-utils)
[![Platform: Windows only](https://img.shields.io/badge/platform-Windows%20only-blue.svg)](https://github.com/T-Rumibul/node-native-win-utils)

# Node Native Win Utils

I did it for myself because I didn't feel like dealing with libraries like 'node-ffi' to implement this functionality. Maybe someone will find it useful. It's WINDOWS OS ONLY


## Features

- Global keyboard event listener (`keyDown` / `keyUp`)
- Window information & screenshots
- Mouse movement, clicks, drag & drop
- Keyboard emulation (`keyPress`, `typeString`)
- OpenCV integration (template matching, blur, grayscale, histogram equalization, color manipulation, ROI, drawing)
- Tesseract OCR text recognition
- Screen capture
- Mouse event listener
- Prebuilt binaries (no Python or build tools required on Windows)
- ESM + CommonJS support

## Requirements

- **Windows 10 or later** (x64)
- Node.js >= 18 (prebuilts for recent versions via `prebuildify`)

## Installation

```bash
npm install node-native-win-utils
```

## Usage

```ts
import {
  KeyboardListener,
  KeyCodeHelper,
  KeyListener,
  getWindowData,
  captureWindow,
  captureWindowN,
  captureScreenToFile,
  mouseMove,
  mouseClick,
  mouseDrag,
  typeString,
  keyPress,
  textRecognition,
  OpenCV,
  startMouseListener,
  stopMouseListener,
} from "node-native-win-utils";
```

### Keyboard

```ts
// Singleton listener
const listener = KeyboardListener.listener();
listener.on("keyDown", (data) => console.log("Down:", data.keyName));
listener.on("keyUp", (data) => console.log("Up:", data.keyName));

// Simulate key press
keyPress(KeyCodeHelper.A);           // once
keyPress(KeyCodeHelper.Enter, 2);    // twice
```

### Mouse & Typing

```ts
mouseMove(500, 300);
mouseClick();                    // left click
mouseClick("right");
mouseClick("left", "down") // left button down
mouseClick("left", "up") // left button up
mouseDrag(100, 100, 800, 600, 50); // optional speed

typeString("Hello from Node!", 30); // 30ms delay per char
```

### Window Operations

```ts
const data = getWindowData("Notepad");
console.log(data); // { width, height, x, y }

captureWindow("Notepad", "screenshot.png");
const buffer = captureWindowN("Notepad"); // Buffer
```

### Screen Capture

```ts
await captureScreenToFile("full-screen.png");
```

### Text Recognition (Tesseract OCR)

```ts
import path from "path";

const text = textRecognition(
  path.join(__dirname, "traineddata"), // path to .traineddata files
  "eng",
  path.join(__dirname, "image.png")
);
console.log(text);
```

### OpenCV (image processing)

```ts
import { OpenCV } from "node-native-win-utils";

const img = new OpenCV("image.png"); // or ImageData { width, height, data: Uint8Array }

// Chainable methods
const processed = img
  .blur(5, 5)
  .bgrToGray()
  .equalizeHist()
  .darkenColor([240, 240, 240], [255, 255, 255], 0.5) // lower, upper, factor
  .drawRectangle([10, 10], [200, 100], [255, 0, 0], 3)
  .getRegion([50, 50, 300, 200]);

processed.imwrite("processed.png");

// Template matching
const match = img.matchTemplate(templateImage, /* method */, /* mask */);
console.log(match); // { minValue, maxValue, minLocation, maxLocation }
```

### Mouse Event Listener

```ts
startMouseListener(({ x, y, type }) => {
  console.log(`${type} at ${x},${y}`); // move at 400 300
});

mouseClick("left"); // trigger event

// terminates a thread
stopMouseListener(); 
```

## Building from source

```bash
npm install
npm run build
```

#### Requires:
```
Visual Studio Build Tools (C++).
Python 3
```

## License
--

[OpenCV License](https://github.com/opencv/opencv/blob/master/LICENSE)

[MIT License](https://github.com/T-Rumibul/node-native-win-utils/blob/main/LICENSE)
  

[license-src]: https://img.shields.io/badge/License-MIT-yellow.svg

[license-href]: https://github.com/T-Rumibul/node-native-win-utils/blob/main/LICENSE


**Made with ❤️ for Windows automation.**  
Issues / PRs welcome!
