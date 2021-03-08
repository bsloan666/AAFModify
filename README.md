AAFModify
=========
An executable based on AAFlib to modify a plug-in's parameters based on characteristics of its associated media
---------------------------------------------------------------------------------------------------------------
This project is based modifications aafembed.cpp that ships with the Advanced 
Authoring Format SDK (AAF-src-1.2.0-DR1).

Motivation
----------
We wish to modify the content of an AAF file that has been exported from Avid, before importing into Resolve.

Avid editors often change a clip's framing and scale, sometimes dynamically, as a storytelling device. Edit 
Decision Lists (EDLs) may accurately represent the timing and transitions within an edit, and may also be 
exported from Avid and imported into Resolve. However, EDLs lack information about how an Avid editor may have 
changed framing or scale per clip or per frame. BlackMagic Design's DaVinci Resolve software is capable of 
importing Avid-originated AAF files and building a timeline that preserves the AAF file's original cuts as well as 
re-position and re-size operations, both animated and static.

One difficulty in this workflow is that Avid editors cut with proxy media that is cropped and scaled to a uniform size, 
DNxHD, 1920x1080 pixels, unlike media that will be used in Resiolve to asemble the final picture, whose dimensions may vary.
This fact accounts for a lot of unnecessary manual labor downstream of the Avid, changing the offsets and scales 
associated with the editor's HD media to values that will create the same effect on the un-cropped larger scale media.

We will stress here that The _formula_ for adjusting the values is not mysterious, though the correct parameters for 
a particular clip depend on a lookup of the media file name information in the AAF file.

The part that is mysterious (or at least obscure) is how to find a particular effect associated with a particular clip
and change its paramters.


The hope is that by sharing this repo with experts in the AAF format, a solution to a very specific problem will be 
answered, namely: _What steps are required to locate and change the scale and offset parameters of Resize and 3DWarp
plug-ins according to their associated media?_


Makefile
--------
The Makefile provided is biased toward the author's dev environment. Developers on MacOS should have little 
difficulty tailoring it to build on their systems. When thusly customized, running:
```
make deps
```
...to download and compile the AAFSDK library, utilities and examples. Then:
```
make
```
...in the terminal to compile and link aafmodify and cutsonly.

What Does AAFModify  Do?
----------------
In its current state, when invoked on the command line with the following command:

```
aafmodify /path/to/AAF_file.aaf
```

...the program will print the strings associated various objects found in the file, their types, variable names and values. Aafmodify and cutsonly have different behaviors as a function of how the original examples traverse the AAF file. 


What Should it Do?
------------------
The desired behavior of a succesful solution program might simply be:

```
aafmodify /path/to/avid_aaf_file.aaf
```

...where the AAF file is modified in place. Or possibly:

```
aafmodify /path/to/input_avid_aaf_file.aaf /path/to/output_avid_aaf_file.aaf
```

...where changes to parameters in the input AAF file are saved to the output AAF file.

Ideally the solution will locate the rational numbers associated with the AVX_SCALE and AVX_POSITION variables within
the Resize and 3DWarp plugins (those most often used by Avid editors for animated and static repositions) and
modify them to account for the framing of the original camera media, which, unlike avid media may differ in 
its dimensions by clip.

The arithmetic for the proposed operation is a solved problem. 

The challenge is to find all of the clips that have been repositioned within Avid, change their scale and position and save the result in AAF format. 
