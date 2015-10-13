The CORE directory contains platform independent parts for BasicSynth
Composer. BasicSynth Composer is designed as a "Document/View" model,
with the "document" parts in the CORE library and the "view" parts in
a platform specific project.

CORE implements project items, instrument editors, and sound generation
functionality. Graphics drawing for the instrument editor widgets is 
defined in a "Paint" function that must be implemented for each target
platform. GUI functions are accessed via abstract classes, defined in 
WindowTypes.h

Instrument editors are defined by a set of XML files, located in the
Forms directory. These xml file format is not documented yet, but how
they work should become obvious if you study a form file along with
the SynthWidget and WidgetForm classes.