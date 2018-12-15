const VMU_DISP_PIXEL_HEIGHT = 32;
const VMU_DISP_PIXEL_WIDTH = 48;
const FRAMERATE = 10;

var Module = {
  onRuntimeInitialized: function() {
    Module.gyInit();

    const gyVmu = new Module.VMUWrapper();
    gyVmu.deviceCreate();
    // gyVmu.loadBios('bin/vmu_bios.bin');
    // gyVmu.resetCPU();
    gyVmu.flashLoadImage('roms/minigame.vmi');
    gyVmu.resetCPU();

    var c = document.getElementById("evmu");
    var ctx = c.getContext("2d");

    const nextAnimFrame = window.requestAnimationFrame || function(callback){window.setTimeout(callback,1.0 / FRAMERATE)};

    const renderVMU = function() {
      gyVmu.deviceUpdate(1.0 / FRAMERATE);

      const width = ctx.canvas.width;
      const height = ctx.canvas.height;
      ctx.clearRect(0, 0, width, height);

      for (var y = 0; y < VMU_DISP_PIXEL_HEIGHT; ++y) {
        for (var x = 0; x < VMU_DISP_PIXEL_WIDTH; ++x) {
          var pixel = gyVmu.displayPixelGet(x, y);
          if (pixel) {
            ctx.fillStyle = 'rgba(' + pixel + ', ' + pixel + ', ' + pixel + ', 255)';
            ctx.fillRect(x, y, 1, 1);
          }
        }
      }

      nextAnimFrame(renderVMU);
    };

    renderVMU();

    // Module.gyUninit();
  }
};
