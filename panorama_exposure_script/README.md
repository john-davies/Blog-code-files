# Panormama Exposure Compensation Script

## Introduction

The panorama exposure compensation script creates a series of ufraw commands which slowly changes the exposure compensation for a series of images. This is useful for stiched panoramas where images into the sun, for example, need a different exposure compensation to those away from the sun. This script applies incremental exposure changes to a series of images such that the whole panorama is exposed correctly and the changes are no so great between images so that the stiching algorithm fails.

Note that the script assumes that the panorama is a full 360 degrees.

The script will also optionally generate extra exposures for each image which can be used for exposure fusion to further enhance the final panorama.

## Configuration File

The script is controlled via a configuration file with the following parameters:

* *start_exp* - exposure compensation for the starting image, usually the brightest image
* *end_exp* - exposure compensation for the darkest image which is assumed to be half way round the panorama, i.e. 180 degrees from the start
* *exp_fusion_1* - first additional exposure compensation for each image
* *exp_fusion_2* - second additional exposure compensation for each image
* *file_ext* - file type to be processed, usually a RAW type file, e.g. NEF
* *output_ext* - output file type, e.g. JPG or PNG
* *out_path* - output path for the created images

*Note that the config file is essentially a bash script so any script command can be included.*

## Running the scripts

The script is run as follows:

    panoexpmod <config file>

The output is the series of ufraw commands that need to be run to create the files. The ufraw commands are not run immediately to allow some sanity checking of the output. The output would usually be redirected to a separate file and that file run as a batch file to create the images.

If either of *exp_fusion_1* or *exp_fusion_2* is set to zero then no corresponding "fusion" image will be generated. 
