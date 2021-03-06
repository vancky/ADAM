# ADAM: All-Data Asteroid Modelling

ADAM is a program for 3D shape reconstruction of asteroids from disk-resolved observations. Current version supports  lightcurves, albedo variegation, adaptive optics, occultations, and range-Doppler radar images. Raw images are used directly, boundary extraction is not required. Source code for thermal modelling is also included, but not currently integrated into main program.

## Required libraries
- KissFFT (https://sourceforge.net/projects/kissfft)
- Iniparser (https://github.com/ndevilla/iniparser)
- Wcstools (http://tdc-www.harvard.edu/wcstools)
- Lapacke
- Lapack
- Blas
- Cblas

KissFFT, Iniparser and Wcstools folders are included for convenience; they come with their own licenses.

## How to build
- Build Iniparser
- Build Wcstools
- Change Makefile to reflect installed libraries, i.e. change reflapacke, reflapack, atlas and atlcbas.
- make adam

This program has been tested on Linux with gcc 5.3.

## Usage
ADAM uses ini files to process data. For an example of asteroid Hertha, 135_oct.ini and 135_subdiv.ini (usage: ./adam 135_oct.ini). For all the available config options and short descriptions, see the file Adam.ini.
Adam.pdf contains some useful tips. For theoretical background, read [Shape reconstruction from generalized projections](http://urn.fi/URN:ISBN:978-952-15-3673-1).

## License

This software is licensed under [CC Attribution 4.0 international License](https://creativecommons.org/licenses/by/4.0/legalcode).

If you use ADAM in your research, please cite
Viikinkoski, M; Kaasalainen, M.; Durech, J.: *ADAM: a general method  for using various data types in asteroid reconstruction*, A&A Vol 576, 2015.

## Contact
Bug reports, data, feature suggestions and comments are welcome.

Matti Viikinkoski (matti.viikinkoski@gmail.com)

## Updates
#### 10.7.2016
- Added support for occultation chord offsets
- Added support for optimizing occultation chord offsets

#### 23.7.2016
- Added support for calibrated lightcurves

#### 30.7.2016
- Support for albedo variegation

#### 29.8.2016
- Documentation expanded
- Utility program (make thermal) to calculate temperature distribution of an asteroid using FFT. Shadowing effects are taken into account.
- Makefile for the intel compiler. ICC produces considerably faster code than the GCC. 

#### 11.9.2016
- Occultations: Improved handling of negative chords

#### 29.10.2016
- Python and Matlab utilities for displaying shapes and plotting projections and occultations. See Readme in the Utils directory.

#### 8.2.2017
- Support for the Hapke scattering law
- Stability of the subdivision surface fitting is slightly improved 
- Some minor tweaks and fixes

#### 22.2.2017
- Support for sparse photometry (See 135_subdiv_sparse.ini)
- Weighting of individual lightcurves

#### 23.3.2017
- Support for contour(full or sparse)  fitting, see contour.ini for an example

#### 10.12.2017
- Support for Center-of-Mass regularization (Set COMWeight= in [Optimization])
- Support for hard thresholding AO data (Set SetZero=20 in [AO?] to zero all the pixels that are smaller than 0.2*max value)
- A matlab gui for viewing ADAM reconstructed shapes (See install and usage.txt in Utils/matlab/adam_gui)
- Some minor bugfixes

#### 8.9.2018
- Support for fitting AO albedo [FitAOAlbedo=1 in [Optimization], FitAlbedo=1 in [LC], remember to set AlbedoFile= for output]
- Inertia regularization [set INERWeight=? in [Optimization]]
- Butterfly subdivision

#### 2.6.2020
- Bugfixes and some minor improvements
- Rewritten albedo code
