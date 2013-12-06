OpenNI2-Kinect2Driver
=====================

OpenNI2 Kinect v2 DevPrev Driver

![OpenNI2-Kinect2Driver](https://pbs.twimg.com/media/BazF6FECcAA-P4V.png:large)

## Requirements

 * Visual Studio 2012
 * Kinect for Windows v2 Developer Preview
 * Kinect for Windows SDK v2 Developer Preview 1311
 * [OpenNI2 v2.2.0.33](https://github.com/OpenNI/OpenNI2)

## Notice
 
* Please use at your own risk
* Still in the experimental stage. It just runs, but may have a lack of features or may be buggy.
* Does not run with NiTE. NiTE requires proprietary driver parameters that we cannot implement. 

## Important

* This is preliminary software and/or hardware and APIs are preliminary and subject to change.
* ソフトウェアやハードウェア、APIは暫定的なものであり正式版では変更される可能性があります。

## Installation

1. fork & clone [OpenNI2 v2.2.0.33](https://github.com/OpenNI/OpenNI2)(commit 7bef8f639e).
2. Open Visual Studio 2012.
3. Update Toolkit v110.
4. Move Kinect2 folder to "OpenNI2\Source\Drivers".
5. Add project.
6. Unload the "Kinect" project(v1 bridge).
7. Build Solution.
8. Change startup project to "NiViewer".
9. Copy "OpenNI2\ThirdParty\GL\glut64.dll" to "OpenNI2\Bin\x64-Debug" or "OpenNI2\Bin\x64-Release"
10. Run NiViewer.
