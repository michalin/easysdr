# SDR Receiver Using a Tayloe Quadrature Detector
 simple SDR Receiver that can receive frequencies up to approximately 30MHz. So simple that it can even be built on a breadboard.

### What you need
- The [circuit](schematic/schematic.pdf) on Breadboard or PCB
- Soundcard or USB sound capture device with stereo input and a sample rate of at least 48 kbit/s.
- Windows or Linux PC. A Raspberry Pi or similar board might also work, but I didn't test this.

### Arduino Libraries
- Etherkit Si5351 Arduino Library (if Si5351 is used)

### Installation
- Download and install [Quisk](https://james.ahlstrom.name/quisk/)
- Configure the serial port the ESP32 is connected to in the variable *openradio_serial_port* in [.quisk_conf.py](sketch/.quisk_conf.py), i.e. `openradio_serial_port = "/dev/ttyUSB0"` on Linux or `openradio_serial_port = "COM1"` on Windows.
- On Windows: copy the script [.quisk_conf.py](sketch/.quisk_conf.py) into the "Documents" folder of your user directory.
- On Linux: Copy the file into the root of your home directory
- Set `#define USE_SI5351 1` in the [ESP32 sketch](sketch/sketch.ino) if you are using an Si5351. 
- Flash it to your ESP32 dev module.


