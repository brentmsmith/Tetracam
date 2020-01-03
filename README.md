# Tetracam
A GUI for the 7 channel Tetracam MCAW Multispectral Camera

Current features include:

- pause/play button of video feeds
- a full screen display on a secondary screen
- RGB display of all monochrome channels based on camera filter, solar spectrum, and 2 degree color matching functions. 
- tunable RGB colors based on relativistic Doppler shift
- parallax corrected RGB display of 3 user selected channels based on phase correlation of edges in the images

Required libraries include:
- Boost 1.71
- OpenCV 4.1
- QT 12.4
- Intel TBB U8

Screenshots:

![Basic UI](https://github.com/brentmsmith/Tetracam/blob/master/BasicUI.png)

![Full Screen Mode](https://github.com/brentmsmith/Tetracam/blob/master/FullScreen.png)
