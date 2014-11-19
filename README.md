OpenNI2-Kinect2Driver
=====================

OpenNI2 Kinect v2 Driver

![OpenNI2-Kinect2Driver](https://pbs.twimg.com/media/BazF6FECcAA-P4V.png:large)

## Notice
 
* Please use at your own risk
* Still in the experimental stage. It just runs, but may have a lack of features or may be buggy.
* Does not run with NiTE. NiTE requires proprietary driver parameters that we cannot implement. 


## Installation

1. Install [OpenNI 2.2.0.33 Beta(x64)](http://structure.io/openni)
2. Download or Clone project.
3. Move "Bin" folder to "C:\Program Files\OpenNI2\Tools"
4. Start C:\Program Files\OpenNI2\Tools\NiViewer.exe

### Requirements

 * Kinect for Windows v2
 * Kinect for Windows SDK v2 1409
 * [OpenNI 2.2.0.33 Beta(x64)](http://structure.io/openni)


## Build

1. fork & clone [OpenNI2 v2.2.0.33 Source](https://github.com/occipital/openni2).
2. Open Visual Studio 2012.
3. Update Toolkit v110.
4. Move Kinect2 folder to "OpenNI2\Source\Drivers".
5. Add project.
6. Unload the "Kinect" project(v1 bridge).
7. Build Solution.
8. Change startup project to "NiViewer".
9. Copy "OpenNI2\ThirdParty\GL\glut64.dll" to "OpenNI2\Bin\x64-Debug" or "OpenNI2\Bin\x64-Release"
10. Run NiViewer.


### Requirements

 * Kinect for Windows v2 
 * Kinect for Windows SDK v2 1409
 * [OpenNI2 v2.2.0.33 Source](https://github.com/occipital/openni2)
 * Visual Studio 2012 or later
