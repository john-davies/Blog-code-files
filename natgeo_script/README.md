# National Geographic Magazine Builder

To build cng2jpg: `gcc cng2jpg.c -Wall -o cng2jpg`

* mm.sh: builds a PDF of the images
* ocr.sh: builds a PDF and extracts the text of each page using tesseract ocr.
* ocr_embed.sh: builds a PDF and includes the ocr text in the PDF

In each case the scripts are run as follows:

`./ocr_embed.sh <dir containing cng files>`

Note that these instructions are for a Linux installation. However below are some comments that I've received about installing on a Mac:

The first run on my Mac seemed to get me about 90% of the way there. Here are some of the adjustments I made for my Mac OS 10.13.6. Fair warning, my install was not nearly this straightforward. This is my best guess at the combination of things that got it to work.

Using Homebrew, I installed some of the dependencies.

````
brew install imagemagick
brew install exiftool
brew install aspell
brew install ghostscript
brew install tesseract
````

Had to upgrade Python to get pip installed.

````
brew upgrade python
sudo python3 -m pip install --upgrade pip
````
OR
````
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py
````
Installing Pillow was fussy so I recommend installing it before installing hocr-tools.

````
sudo python3 -m pip install --upgrade Pillow
sudo python3 -m pip install hocr-tools
````

Used Xcode command line tools to compile your script.

`gcc cng2jpg.c -Wall -o cng2jpg`

Copied the new program to my bin folder so I could call it like a regular command.

`sudo cp cng2jpg /usr/local/bin/cng2jpg`

And now the changes to the `ocr_embed.sh` script for Mac. I’ll start at the bottom and work my way up so line numbers match your code.
Added two lines after line 136. Exiftool was not working because my version of hocr-pdf made PDF files with a broken xref table that prevented exiftool from working. Running the PDF through Ghostscript fixed it so exiftool would work.

````
gs -sDEVICE=pdfwrite -q -sstdout=%stderr -o ${1}/output.pdf ${1}/${PDF_filename} > /dev/null 2>&1
mv ${1}/output.pdf ${1}/${PDF_filename}
````

Lines 124 & 125 changed `${1:` to `=${folder_name:`. The ‘1’ was working on the full path, but I needed just the name of the last folder.

Line 122 changed `${#1}` to `${#folder_name}`.

Line 111 changed to two lines.

````
folder_name=${1##*/}
PDF_filename=${folder_name}.pdf
````

Lines 44 & 49 changed `hocr txt` to `hocr txt quiet` to stop the unnecessary tesseract version listing for each page.

Line 41 changed `${filename#*/}` to `${filename##*/}`. Adding the extra # got rid of the whole path in front of the file name.

Added one line after line 31. See line 26 for explanation.

`unset IFS`

Line 30 changed from this:

`./cng2jpg ${filename} ${filename/\//”/zz_”}.jpg`

to this:

`cng2jpg ${filename} ${filename%/*}/zz_${filename##*/}.jpg`

Added one line after line 26. Setting the IFS allowed the script to handle files with spaces in the name. The map files with spaces in the name were getting broken up and lost in the process on my Mac. This seemed like the least disruptive change to your code.

`IFS=$'\n'`

Line 25 took ‘./’ off of cng2jpg just like line 30 because I moved the program to the /usr/local/bin folder.
