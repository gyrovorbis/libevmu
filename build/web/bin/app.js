const VMU_DISP_PIXEL_HEIGHT = 32;
const VMU_DISP_PIXEL_WIDTH = 48;
const FRAMERATE = 10;

// can support dci only, or vms and vmi

function loadFile(file, callback) {
  $.ajax({
    url: "http://localhost:8080/" + file,
    type: 'GET',
    beforeSend: function (xhr) {
      xhr.overrideMimeType("text/plain; charset=x-user-defined");
    },
    success: function( data ) {
      console.log(data);
      try {
        Module['FS_createDataFile']("/", file, data, true, true);
        console.log('File loaded:', file)
        callback(data);
      } catch (e) {
        console.log('Failed to load:', e.message, e);
      }
    }
  });
}

var gyVmu;

var Module = {
  loadRom: function(file) {
    gyVmu.flashFormatDefault();
    gyVmu.flashRootBlockPrint();
    gyVmu.flashLoadImage(file);
    gyVmu.resetCPU();
  },
  onRuntimeInitialized: function() {
    Module.FS.mkdir('./roms');
    Module.FS.mount(Module.FS.filesystems.MEMFS, { root: './roms' }, './roms');

    Module.gyInit();

    gyVmu = new Module.VMUWrapper();
    gyVmu.deviceCreate();
    // gyVmu.loadBios('bin/vmu_bios.bin');
    // gyVmu.resetCPU();

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


    setTimeout(function() {
      loadFile('roms/minigame.vmi', function() {
        loadFile('roms/minigame.vms', function() {
          Module.loadRom('roms/minigame.vmi');
        });
      });
    }, 1000);

    // Module.gyUninit();
  }
};

function dragOverHandler(ev) {
  // Prevent default behavior (Prevent file from being opened)
  ev.preventDefault();
}

function dropHandler(ev) {
    // Prevent default behavior (Prevent file from being opened)
    ev.preventDefault();

    if (ev.dataTransfer.items) {
      // Use DataTransferItemList interface to access the file(s)
      for (var i = 0; i < ev.dataTransfer.items.length; i++) {
        // If dropped items aren't files, reject them
        if (ev.dataTransfer.items[i].kind === 'file') {
          var file = ev.dataTransfer.items[i].getAsFile();

          var fr = new FileReader();
          fr.onload = function () {
            var data = new Uint8Array(fr.result);
            try {
              Module['FS_createDataFile']("/", file.name, data, true, true, true);
            } catch(e) {
              console.log('file probably exists, trying anyway')
            }
            Module.loadRom(file.name);
          };
          fr.readAsArrayBuffer(file);

          // console.log('... file[' + i + '].name = ' + file.name);
        }
      }
    } else {
      // Use DataTransfer interface to access the file(s)
      for (var i = 0; i < ev.dataTransfer.files.length; i++) {
        // console.log('... file[' + i + '].name = ' + ev.dataTransfer.files[i].name);
      }
    }

}
