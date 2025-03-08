import EventEmitter from 'events'
import path  from "path";
import fs from "fs";


// @ts-ignore
import nodeGypBuild = require('node-gyp-build');

import {keyCodes, KeyCodeHelper} from "./keyCodes.mjs";
import { __dirnameLocal } from "./dirnameLocal.mjs";

const bindings = nodeGypBuild(path.resolve(__dirnameLocal, ".."));




/**
 * Represents the data of a window.
 */
export type WindowData = {
  width: number;
  height: number;
  x: number;
  y: number;
};

/**
 * Represents the data of an image.
 */
export type ImageData = {
  width: number;
  height: number;
  data: Uint8Array;
};

/**
 * Represents the result of a template matching operation.
 */
export type MatchData = {
  minValue: number;
  maxValue: number;
  minLocation: { x: number; y: number };
  maxLocation: { x: number; y: number };
};

export type GetWindowData = (windowName: string) => WindowData;

export type CaptureWindow = (windowName: string) => Buffer;

/**
 * The handler to listen to key-events.
 * @param callback - The callback function to handle key-down events.
 */
export type SetKeyCallback = (callback: (keyCode: number) => void) => void;

/**
 * The handler to stop thread listening to key-events.
 * @param callback - The callback function to handle key-down events.
 */
export type UnsetKeyCallback = () => void;


/**
 * Function type for moving the mouse.
 */
export type MouseMove = (posX: number, posY: number) => boolean;

/**
 * Function type for simulating a mouse click.
 */
export type MouseClick = (button?: "left" | "middle" | "right") => boolean;

/**
 * Function type for simulating typing.
 */
export type TypeString = (stringToType: string, delay?: number) => boolean;

/**
 * Function type for simulating key press and release.
 */
export type PressKey = (keyCode: number) => boolean;

/**
 * Function type for simulating a mouse drag operation.
 */
export type MouseDrag = (
  starX: number,
  startY: number,
  endX: Number,
  endY: number,
  speed?: number
) => boolean;

/**
 * Represents a point in a two-dimensional space.
 */
export type Point = [x: number, y: number];

export type Color = [r: number, g: number, b: number];

/**
 * Represents a region of interest in an image.
 */
export type ROI = [x: number, y: number, width: number, height: number];

export type Imread = (path: string) => ImageData;

export type Imwrite = (image: ImageData) => Buffer;

export type MatchTemplate = (
  image: ImageData,
  template: ImageData,
  method?: number | null,
  mask?: ImageData
) => MatchData;

export type Blur = (
  image: ImageData,
  sizeX: number,
  sizeY: number
) => ImageData;
export type BgrToGray = (image: ImageData) => ImageData;

export type DrawRectangle = (
  image: ImageData,
  start: Point,
  end: Point,
  rgb: Color,
  thickness: number
) => ImageData;
export type GetRegion = (image: ImageData, region: ROI) => ImageData;

export type TextRecognition = (trainedDataPath: string, dataLang: string, imagePath: string) => string;
export type CaptureScreenAsync = () => Promise<Buffer>;

const {
  setKeyDownCallback,
  setKeyUpCallback,
  unsetKeyDownCallback,
  unsetKeyUpCallback,
  getWindowData,
  captureWindowN,
  captureScreenAsync,
  mouseMove,
  mouseClick,
  mouseDrag,
  typeString,
  pressKey,
  imread,
  imwrite,
  matchTemplate,
  blur,
  bgrToGray,
  drawRectangle,
  getRegion,
  textRecognition
}: {
  setKeyDownCallback: SetKeyCallback;
  setKeyUpCallback: SetKeyCallback;
  unsetKeyDownCallback: UnsetKeyCallback;
  unsetKeyUpCallback: UnsetKeyCallback;
  getWindowData: GetWindowData;
  captureWindowN: CaptureWindow;
  mouseMove: MouseMove;
  mouseClick: MouseClick;
  mouseDrag: MouseDrag;
  typeString: TypeString;
  pressKey: PressKey;
  imread: Imread;
  imwrite: Imwrite;
  matchTemplate: MatchTemplate;
  blur: Blur;
  bgrToGray: BgrToGray;
  drawRectangle: DrawRectangle;
  getRegion: GetRegion;
  textRecognition: TextRecognition;
  captureScreenAsync: CaptureScreenAsync;
} = bindings;

const rawPressKey = pressKey;

/**
 * Captures a window and saves it to a file.
 * @param windowName - The name of the window to capture.
 * @param path - The file path to save the captured image.
 * @returns True if the capture and save operation is successful, otherwise false.
 */
function captureWindow(windowName: string, path: string): boolean {
  const buffer = captureWindowN(windowName);
  if (!buffer) return false;
  fs.writeFileSync(path, new Uint8Array(buffer));
  return true;
}


/**
 * Captures a screen and saves it to a file.
 * @param path - The file path to save the captured image.
 * @returns True if the capture and save operation is successful, otherwise false.
 */
function captureScreenToFile(path: string): Promise<boolean> {
  return new Promise((resolve, reject) => {
  captureScreenAsync().then((buffer) => {
    fs.writeFileSync(path, new Uint8Array(buffer))
    resolve(true)
  }).catch((err => {
    reject(err)
  }));
  
  
})
}



/**
 * Interface representing a private keyboard listener that extends EventEmitter.
 * It declares event handlers for native keyboard events, which are forwarded from
 * the C++ bindings using thread-safe callbacks.
 */
interface KeyboardListenerPrivate extends EventEmitter {
  /**
   * Registers an event handler for the 'keyDown' event.
   * This event is fired when a key is pressed down. The C++ native binding calls
   * this callback using a thread-safe mechanism (via Napi::ThreadSafeFunction).
   * @param event - The event name ('keyDown').
   * @param callback - Function invoked with an object containing the keyCode and keyName.
   * @returns The current instance for method chaining.
   */
  on(
    event: "keyDown",
    callback: (data: { keyCode: number; keyName: string }) => void
  ): this;

  /**
   * Registers an event handler for the 'keyUp' event.
   * This event is fired when a key is released. The underlying C++ code safely
   * invokes this callback from a background thread using a thread-safe function.
   * @param event - The event name ('keyUp').
   * @param callback - Function invoked with an object containing the keyCode and keyName.
   * @returns The current instance for method chaining.
   */
  on(
    event: "keyUp",
    callback: (data: { keyCode: number; keyName: string }) => void
  ): this;
}


/**
 * Class that implements a private keyboard listener.
 * This class leverages native C++ bindings to hook into system keyboard events.
 * The C++ layer uses global ThreadSafeFunction objects to safely dispatch events
 * (using a dedicated monitoring thread, mutexes, and atomic flags) to JavaScript.
 * @extends EventEmitter
 */
class KeyboardListenerPrivate extends EventEmitter {

  /**
   * Constructs the keyboard listener and sets up native callbacks.
   * The callbacks (set via setKeyDownCallback and setKeyUpCallback) are defined in the
   * C++ binding layer. They are responsible for invoking these JavaScript callbacks
   * in a thread-safe manner once a key event is detected.
   */
  constructor() {
    super();
    // Set the callback for key down events.
    setKeyDownCallback((keyCode: number) => {
      // Look up the human-readable key name from a mapping.
      const keyName: string | undefined = keyCodes.get(keyCode.toString());
      // Emit the 'keyDown' event to all registered JavaScript listeners.
      this.emit("keyDown", {
        keyCode,
        keyName,
      });
    });
    // Set the callback for key up events.
    setKeyUpCallback((keyCode: number) => {
      // Look up the human-readable key name from a mapping.
      const keyName: string | undefined = keyCodes.get(keyCode.toString());
      // Emit the 'keyUp' event to all registered JavaScript listeners.
      this.emit("keyUp", {
        keyCode,
        keyName,
      });
    });
  }
}


/**
 * A singleton manager for the KeyboardListenerPrivate instance.
 * This class ensures that only one native keyboard listener is active at any time.
 * When the listener is destroyed, it calls unsetKeyDownCallback and unsetKeyUpCallback
 * to clean up native resources, mirroring the cleanup logic in the C++ bindings.
 */
class KeyboardListener {
  /**
   * Holds the singleton instance of KeyboardListenerPrivate.
   */
  private static listenerInstance: KeyboardListenerPrivate | null = null;

  /**
   * Returns the singleton instance of KeyboardListenerPrivate. If not already created,
   * it instantiates a new instance and sets up the native callbacks.
   * @returns The active KeyboardListenerPrivate instance.
   */
  static listener() {
    if (!this.listenerInstance) {
      this.listenerInstance = new KeyboardListenerPrivate();
    }
    return this.listenerInstance;
  }

  /**
   * Destroys the current KeyboardListenerPrivate instance and cleans up native callbacks.
   * This method calls unsetKeyDownCallback and unsetKeyUpCallback to release any
   * native resources (such as the global ThreadSafeFunctions) and stops the monitoring thread.
   */
  static destroy() {
    this.listenerInstance = null;
    unsetKeyDownCallback();
    unsetKeyUpCallback();
  }
}


/**
 * Represents the OpenCV class that provides image processing functionality.
 */
class OpenCV {
  imageData: ImageData;

  /**
   * Represents the OpenCV class that provides image processing functionality.
   */
  constructor(image: string | ImageData) {
    if (typeof image === "string") {
      this.imageData = imread(image);
    } else {
      this.imageData = image;
    }
  }

  /**
   * The width of the image.
   */
  get width() {
    return this.imageData.width;
  }

  /**
   * The height of the image.
   */
  get height() {
    return this.imageData.height;
  }

  /**
   * Matches a template image within the current image.
   * @param template - The template image data to search for.
   * @param method - The template matching method (optional).
   * @param mask - The optional mask image data to apply the operation (optional).
   * @returns The result of the template matching operation.
   */
  matchTemplate(template: ImageData, method?: number | null, mask?: ImageData) {
    return matchTemplate(this.imageData, template, method, mask);
  }

  /**
   * Applies a blur filter to the image.
   * @param sizeX - The horizontal size of the blur filter.
   * @param sizeY - The vertical size of the blur filter.
   * @returns A new OpenCV instance with the blurred image data.
   */
  blur(sizeX: number, sizeY: number) {
    return new OpenCV(blur(this.imageData, sizeX, sizeY));
  }

  /**
   * Converts the image from BGR to grayscale.
   * @returns A new OpenCV instance with the grayscale image data.
   */
  bgrToGray() {
    return new OpenCV(bgrToGray(this.imageData));
  }

  /**
   * Draws a rectangle on the image.
   * @param start - The starting point of the rectangle.
   * @param end - The ending point of the rectangle.
   * @param rgb - The color (RGB) of the rectangle.
   * @param thickness - The thickness of the rectangle's border.
   * @returns A new OpenCV instance with the image containing the drawn rectangle.
   */
  drawRectangle(start: Point, end: Point, rgb: Color, thickness: number) {
    return new OpenCV(
      drawRectangle(this.imageData, start, end, rgb, thickness)
    );
  }

  /**
   * Extracts a region of interest (ROI) from the image.
   * @param region - The region of interest defined as [x, y, width, height].
   * @returns A new OpenCV instance with the extracted region of interest.
   */
  getRegion(region: ROI) {
    return new OpenCV(getRegion(this.imageData, region));
  }

  /**
   * Writes the image data to a file.
   * @param path - The file path to save the image.
   */
  imwrite(path: string) {
    const buffer = imwrite(this.imageData);
    if (!buffer) return;
    fs.writeFileSync(path, new Uint8Array(buffer));
  }
}
function keyPress(keyCode: number, repeat?: number): Promise<boolean> {
  return new Promise((resolve, reject) => {
        if(!repeat) {
          let result = rawPressKey(keyCode)
          if(!result) reject('Something went wrong');
          return resolve(true);
        }
        for(let i = 0; i <= repeat; i++) {
          let result = rawPressKey(keyCode);
          if(!result) reject('Something went wrong');
        }
        return resolve(true);
  })
}




export {
  getWindowData,
  captureWindow,
  captureWindowN,
  mouseMove,
  mouseClick,
  mouseDrag,
  typeString,
  keyPress,
  rawPressKey,
  KeyCodeHelper,
  textRecognition,
  captureScreenToFile,
  KeyboardListener,
  OpenCV
};
