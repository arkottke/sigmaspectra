
# SigmaSpectra

[![License](https://img.shields.io/badge/license-GLPv3-blue.svg)](https://github.com/arkottke/sigmaspectra/blob/master/LICENSE.txt) 
[![Build Status](https://travis-ci.org/arkottke/sigmaspectra.svg?branch=master)](https://travis-ci.org/arkottke/sigmaspectra)
[![Build Status](https://ci.appveyor.com/api/projects/status/lllrg71eoxcetnoq?svg=true)](https://ci.appveyor.com/project/arkottke/sigmaspectra)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c01fc8b255fe4fb8b079783e3481ec5d)](https://www.codacy.com/app/arkottke/sigmaspectra?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=arkottke/sigmaspectra&amp;utm_campaign=Badge_Grade)

## Installation

SigmaSpectra has been compiled for a number of operating systems. Compiled
versions can be found under
[Releases](https://github.com/arkottke/sigmaspectra/releases).  Windows 32 and
64 bit installers and archives are available, as well as Linux
[AppImages](http://appimage.org/).

## Citation

The SigmaSpectra procedure should be referenced to the following paper:

```
@article{kottke2008,
  title={A semi-automated procedure for selecting and scaling recorded
      earthquake motions for dynamic analysis},
  author={Kottke, Albert and Rathje, Ellen M},
  journal={Earthquake Spectra},
  volume={24},
  number={4},
  pages={911--932},
  year={2008}
}
```

## Using SigmaSpectra

Now that you have installed SigmaSpectra there are some important
aspects of the program you need to be informed about:

1. A complete reference to the program can be found via the program's help
   menu, or by pressing F1.  The documentation can also be found in PDF form in
   the program's installation path and provided
   [here](https://github.com/arkottke/sigmaspectra/blob/master/manual/manual.pdf).

2. SigmaSpectra does not calculate a target acceleration-response spectrum.
   The target acceleration-response spectrum must calculated externally and
   then input into the program by copying and pasting from a spreadsheet, or
   manually entering values.  When copying and pasting from a spreadsheet the
   rows are automatically adding to fit the data.

3. SigmaSpectra requires that the time series in the motion library follow the
   [PEER Ground Motion Database](http://ngawest2.berkeley.edu/) format with
   respect to both header information and file naming convention.  The file
   naming convention is '<EARTHQUAKE>/<STATION><COMPONENT>.AT2'.  SigmaSpectra
   requires that the station and component portions of the name be retained as
   they are used to distinguish components from a given station.
