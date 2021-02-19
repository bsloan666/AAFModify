AAFModify
=========
An executable based on AAFlib to modify a plug-in's parameters based on characteristics of its associated media
---------------------------------------------------------------------------------------------------------------

This code borrows from a utility, aafembed, that ships with the Advanced Authoring Format SDK (AAF-src-1.2.0-DR1).

The hope is that by sharing this repo with experts in the AAF format, a solution to a very specific problem will be 
answered, namely: _What steps are required to locate and change the scale and offset parameters of Resize and 3DWarp
plug-ins according to their associated media?_

The behavior of this program might simply be:

```
aafmodify /path/to/avid_aaf_file.aaf
```

...where the AAF file is modified in place. Or if possible:

```
aafmodify /path/to/input_avid_aaf_file.aaf /path/to/output_avid_aaf_file.aaf
```

...where changes to parameters in the input AAF file are saved to the output AAF file. 

Makefile
--------
The Makefile provided is extremely biased toward the author's dev environment. Developers on MacOS should have little 
difficulty tailoring it to build on their systems. When thusly customized, running:
```
make
```
...in the terminal should build AAFModify. As an alternative to the Makefile build method, anyone with a working build of 
AAF-src-1.2.0-DR1 can simply rename aafmodify.cpp to aafembed.cpp, temporarily replace the source in the SDK's
Utilities/aafembed directory and run:
```
cmake --build .
```
...to compile and link the program (the executable will be called "aafembed" in that case).

What Does it Do?
----------------
In its current state, when invoked on the command line with:

```
aafmodify -d "" /path/to/AAF_file.aaf
```

The program will print the tokens "OpDef" as it iterates over each opdef and "ParamDef" as it iterates over each paramdef within that opdef:
```
OpDef
  Param def
  Param def
  (2 paramdefs)
OpDef
  Param def
  (1 paramdefs)
OpDef
  Param def
  Param def
  (2 paramdefs)
```
...then print out the filename associated with each source clip:

```
  Using path=/C015_C005_1122AG.mxf

  Using path=/D638_C006_0930GX.mxf

  Using path=/D638_C004_09303J.mxf

  Using path=/MUS0040_101_comp_v4E04AE67V.mxf

  Using path=/GMA0196_102_comp_v220320F2V.mxf

```

What Should it Do?
------------------
Ideally it would locate the rational numbers associated with the AVX_SCALE and AVX_POSITION variables within
the Resize and 3DWarp plugins (those most often used by Avid editors for animated and static repositions) and
modify them to account for the framing of the original camera media, which, unlike avid media may differ in 
its dimensions by clip.

The arithmetic for the proposed operation is a solved problem. 

The challenge is to find all of the repositioned clips and adjust their scale and position. 
