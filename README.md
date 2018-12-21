# codecCPP

This is under construction.


## Building

If a proper RELEASE.local file exists one directory levels above **codecCPP**, then just type:

    make

It can also be built by:

    cp configure/ExampleRELEASE.local configure/RELEASE.local
    edit file configure/RELEASE.local
    make

In **configure/RELEASE.local** it may only be necessary to change the definitions
of **EPICS4_DIR** and **EPICS_BASE**.


## Background

This was inspired by the codec support in

https://github.com/areaDetector/ADSupport

## Acknowledgements

codecCPP includes code taken from

https://github.com/Blosc/c-blosc.git

The source code from **c-blosc/blosc** was copied to **codecCPP/codecBloscSrc/c-blosc**.

This code was then build via the epics-base build system.





