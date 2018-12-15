const VMU_DISP_PIXEL_HEIGHT = 32;
const VMU_DISP_PIXEL_WIDTH = 48;
const FRAMERATE = 20;

const evmu = require('./evmu');

const fs = require('fs');
const path = require('path');
const fileBuffer = fs.readFileSync(path.join(__dirname, '/index.html'));

evmu.preRun = function() {
  // evmu.FS.mkdir('/roms');
  // evmu.FS.mount(evmu.FS.filesystems.NODEFS, {
  //   // blobs: [{ name: 'minigame.vmi', data: fileBuffer }]
  // }, '/roms');

  evmu.FS.mkdir('./roms');
  evmu.FS.mount(evmu.FS.filesystems.NODEFS, { root: './roms' }, './roms');
};

evmu.onRuntimeInitialized = function() {
  evmu.gyInit();

  const gyVmu = new evmu.VMUWrapper();
  gyVmu.deviceCreate();
  // gyVmu.loadBios('bin/vmu_bios.bin');
  // gyVmu.resetCPU();
  gyVmu.flashLoadImage('roms/minigame.vmi');
  gyVmu.resetCPU();

  var then = new Date().getTime();

  function renderVMU() {
    var now = new Date().getTime();
    var diff = (now - then) / 1000.0;
    if (diff >= 1.0 / FRAMERATE) {
      then = now;
      gyVmu.deviceUpdate(1.0 / FRAMERATE);
      process.stdout.write('\033c');
      for (var y = 0; y < VMU_DISP_PIXEL_HEIGHT; ++y) {
        var str = '';
        for (var x = 0; x < VMU_DISP_PIXEL_WIDTH; ++x) {
          var pixel = gyVmu.displayPixelGet(x, y);
          str += pixel ? ' ' : '#';
        }
        console.log(str);
      }
    }

    process.nextTick(renderVMU);
  }

  process.nextTick(renderVMU);

  // evmu.gyUninit();
};
