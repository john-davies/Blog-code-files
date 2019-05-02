# Blog Code Files

This repository contains the example code files related to my blog - https://theretiredengineer.wordpress.com/ In general each directory relates to a blog post and a link to that post is given below. All files, unless otherwise stated, are released under GPL V3.

### Panorama Photo Exposure Correction Script

Directory - panorama_exposure_script

Blog post - https://theretiredengineer.wordpress.com/2019/06/16/exposure-compensation-for-360-degree-panoramas/

Refer to README in that directory for documentation.

### LED Panel

Directory -  LED_Panel

Blog post -

### Timelapse Exposure Script

Directory - timelapse_exposure_script

Blog posts - https://theretiredengineer.wordpress.com/2017/06/25/scripting-exposure-compensation-for-timelapse/

### Timelapse Pan & Zoom Script

Directory - timelapse_pan_zoom

Blog post - https://theretiredengineer.wordpress.com/2017/09/10/timelapse-pan-and-zoom/

### National Geographic Magazine Builder

Directory - natgeo_script

Blog posts

* https://theretiredengineer.wordpress.com/2017/07/16/the-complete-national-geographic-dvd-on-linux/
* https://theretiredengineer.wordpress.com/2019/03/10/national-geographic-magazine-ocr/

To build cng2jpg: `gcc cng2jpg.c -Wall -o cng2jpg`

* mm.sh: builds a PDF of the images
* ocr.sh: builds a PDF and extracts the text of each page using tesseract ocr.
* ocr_embed.sh: builds a PDF and includes the ocr text in the PDF

In each case the scripts are run as follows:

`./ocr_embed.sh <dir containing cng files>`

### Webcam Timelapse Creator

Directory - webcam_timelapse_script

Blog post - https://theretiredengineer.wordpress.com/2017/10/01/timelapse-and-webcams/

### LiDAR to PLY Converter

Directory - lidar-ply

*( Note that this has now been superseeded by a newer version - https://github.com/john-davies/lidar-ply. This version is only kept for completeness. )*

Blog post - https://theretiredengineer.wordpress.com/2017/12/10/open-lidar-data/

Run `lidar2ply.lua` with no command line options to get help
