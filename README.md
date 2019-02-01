# codecCPP

This is under construction.


## Building

It can be built by:

    cp ExampleRELEASE.local configure/RELEASE.local
    edit file configure/RELEASE.local
    make

In **configure/RELEASE.local**  change the definitions
of **EPICS4_DIR** and **EPICS_BASE**.


## Background

This was inspired by the codec support in

https://github.com/areaDetector/ADSupport

## Acknowledgements

codecCPP includes code taken from

https://github.com/Blosc/c-blosc.git

The source code from **c-blosc/blosc** was copied to **codecCPP/bloscCodecSrc/c-blosc**.

This code was then build via the epics-base build system.

## Documentation

Douumentation is available at

https://mrkraimer.github.io/website/bloscCodec/bloscCodec.html





