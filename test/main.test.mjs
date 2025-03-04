import t from "tap";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import { exec } from "child_process";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

import {
  OpenCV,
  captureWindow,
  keyPress,
  KeyListener,
  mouseMove,
  mouseClick,
  mouseDrag,
  typeString,
  textRecognition,
  getWindowData,
} from "../dist/index.mjs";

//
// OpenCV Tests
//
t.test("OpenCV: constructor and getters", async (t) => {
  const dummyImage = {
    width: 100,
    height: 200,
    data: new Uint8Array(100 * 200 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  t.equal(opencv.width, 100, "Width should be 100");
  t.equal(opencv.height, 200, "Height should be 200");
  t.end();
});

t.test("OpenCV: blur returns a new instance", async (t) => {
  const dummyImage = {
    width: 100,
    height: 100,
    data: new Uint8Array(100 * 100 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  const blurred = opencv.blur(5, 5);
  t.ok(blurred instanceof OpenCV, "blur() should return an OpenCV instance");
  t.end();
});

t.test("OpenCV: bgrToGray returns a new instance", async (t) => {
  const dummyImage = {
    width: 50,
    height: 50,
    data: new Uint8Array(50 * 50 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  const gray = opencv.bgrToGray();
  t.ok(gray instanceof OpenCV, "bgrToGray() should return an OpenCV instance");
  t.end();
});

t.test("OpenCV: drawRectangle returns a new instance", async (t) => {
  const dummyImage = {
    width: 80,
    height: 80,
    data: new Uint8Array(80 * 80 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  const start = [10, 10];
  const end = [50, 50];
  const color = [255, 0, 0];
  const result = opencv.drawRectangle(start, end, color, 2);
  t.ok(
    result instanceof OpenCV,
    "drawRectangle() should return an OpenCV instance"
  );
  t.end();
});

t.test("OpenCV: getRegion returns a new instance", async (t) => {
  const dummyImage = {
    width: 120,
    height: 120,
    data: new Uint8Array(120 * 120 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  const region = [20, 20, 50, 50];
  const regionImage = opencv.getRegion(region);
  t.ok(
    regionImage instanceof OpenCV,
    "getRegion() should return an OpenCV instance"
  );
  t.end();
});

t.test("OpenCV: imwrite writes a file", async (t) => {
  const dummyImage = {
    width: 30,
    height: 30,
    data: new Uint8Array(30 * 30 * 3),
  };
  const opencv = new OpenCV(dummyImage);
  const filePath = path.join(__dirname, "test-output.png");
  opencv.imwrite(filePath);
  t.ok(fs.existsSync(filePath), "imwrite() should create the output file");
  fs.unlinkSync(filePath); // Clean up
  t.end();
});

//
// captureWindow Test
//
t.test("captureWindow: writes file if window is captured", async (t) => {
  const filePath = path.join(__dirname, "capture-window-test.png");
  exec('start cmd.exe /c "timeout 5"', function (error, stdout, stderr) {

    const result = captureWindow("C:\\Windows\\system32\\cmd.exe", filePath);
    
      t.ok(
        fs.existsSync(filePath),
        "captureWindow() should write a file when successful"
      );
      fs.unlinkSync(filePath);
    
  });
});
//
// getWindowData Test
//
t.test("getWindowData", async (t) => {
  exec('start cmd.exe /c "timeout 5"', function () {
    
    const result = getWindowData("C:\\Windows\\system32\\cmd.exe");
      // could not match cuz of different test env
      // t.equal(
      //   result,
      //   { width: 979, height: 512, x: 85, y: 78 }, "window data doesn't match"
      // );
    t.type(result, "Object", "getWindowData should return an object")
    t.type(result.width, "Number", "should be of type number")
    t.type(result.height, "Number", "should be of type number")
    t.type(result.x, "Number", "should be of type number")
    t.type(result.y, "Number", "should be of type number")
  });
});
//
// keyPress Test
//
t.test("keyPress: resolves true for a valid key code", async (t) => {
  try {
    const res = await keyPress(65, 1); // Simulate pressing key "A"
    t.equal(res, true, "keyPress() should resolve to true");
  } catch (err) {
    t.fail(`keyPress() threw an error: ${err}`);
  }
  t.end();
});

//
// KeyListener Tests
//
t.test("KeyListener: has event emitter properties", async (t) => {
  const listener = new KeyListener();
  let keyDownEmitted = false;
  let keyUpEmitted = false;

  t.equal(
    typeof listener.on,
    "function",
    "KeyListener should have an on() method"
  );
  t.ok(
    listener instanceof KeyListener,
    "listener should be an instance of KeyListener"
  );
  t.end();
});

//
// Text Recognition Tests
//
t.test("Text Recognition", async (t) => {
  const text = textRecognition(path.resolve(__dirname, "traineddata"), "eng", path.resolve(__dirname, "images", '1.png'))
  console.log(text.trim())
  t.equal(
    text.trim(),
    "Visual Studio Code\nEditing evolved",
    "KeyListener should have an on() method"
  );
  t.end();
});



//
// Other Exported Functions
//
t.test("Exported functions: types check", async (t) => {
  t.type(mouseMove, "function", "mouseMove should be a function");
  t.type(mouseClick, "function", "mouseClick should be a function");
  t.type(mouseDrag, "function", "mouseDrag should be a function");
  t.type(typeString, "function", "typeString should be a function");
  t.end();
});
