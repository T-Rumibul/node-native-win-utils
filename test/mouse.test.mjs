import t from "tap";
import {
  startMouseListener,
  stopMouseListener,
  mouseClick,
  mouseMove,
  mouseDrag
} from "../dist/index.mjs";

function sleep(time) {
  return new Promise((resolve, reject) => {
    setTimeout(() => {
      resolve();
    }, time);
  });
}

t.test("Mouse listener", async (t) => {
  const pushedButtons = []
  startMouseListener(({ x, y, type }) => {
    pushedButtons.push(type)
  })

  mouseClick('left')
  const e = stopMouseListener()
  console.log(e)
  await sleep(1000)

  t.ok(pushedButtons.includes('leftDown') && pushedButtons.includes('leftDown'), 'Mouse listener should handle mouse events')
  t.end();
});


t.test("Mouse Move", async (t) => {
  const events = []
  startMouseListener(({ x, y, type }) => {
    events.push({x, y, type})
    
  })
  await sleep(1000)
  mouseMove(1052, 678)
  await sleep(1000)
  stopMouseListener()
  await sleep(1000)
  t.equal(events[events.length - 1].x, 1052, 'Mouse should move to 1052, 678, could fail if you move your mouse while test is running')
  t.equal(events[events.length - 1].y, 678, 'Mouse should move to 1052, 678, could fail if you move your mouse while test is running')
  t.end();
});


t.test("Mouse Drag", async (t) => {
  const events = []
  startMouseListener(({ x, y, type }) => {
    events.push({x, y, type})
    
  })
  await sleep(1000)
  mouseDrag(1052, 678, 1055, 679, 1)
  await sleep(1000)
  stopMouseListener()
  await sleep(1000)
  t.equal(events[0].type, 'move', 'Should start with moving into position')
  t.equal(events[0].x, 1052, 'Should start on 1052, 678, could fail if you move your mouse while test is running')
  t.equal(events[0].y, 678, 'Should start on 1052, 678, could fail if you move your mouse while test is running')
  t.equal(events[1].type, 'leftDown', 'Should press left button')
  t.equal(events[1].x, 1052, 'Should press left button at start position 1052, 678, could fail if you move your mouse while test is running')
  t.equal(events[1].y, 678, 'Should press left button at start position 1052, 678, could fail if you move your mouse while test is running')
  t.equal(events[events.length - 1].type, 'leftUp', 'Should end with leftUp')
  t.equal(events[events.length - 1].x, 1055, 'Should end on 1055, 679, could fail if you move your mouse while test is running')
  t.equal(events[events.length - 1].y, 679, 'Should end on 1055, 679, could fail if you move your mouse while test is running')
  t.end();
});