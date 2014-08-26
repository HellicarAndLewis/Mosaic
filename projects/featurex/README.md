# Feature Extractor and Comparator

Library that creates descriptors for image tiles. This includes a CPU and GPU
based analyzer. The GPU analyzer is used to read an input texture, calculate
the necessary descriptor using shaders as much as possible while the CPU analyzer
is meant to be run fast, but not in real time per se. The CPU analyzer runs in 
it's own thread. Both the CPU and GPU analyzers generate Descriptor objects that 
describe the input image.

## Input <> Output

The featurex library has two inputs and two outputs, one pair the CPU and GPU analyzers.
The CPU analyzer takes image paths as input and outputs descriptors. 
The GPU analyzer takes a openGL texture as input and outputs descriptors.

*CPU analyzer*
````

  +--------------+      +--------------+      +------------+
  |              |      |              |      |            |
  | Image files  |----->| AnalyzerCPU  |----->| Descriptor |
  |              |      |              |      |            |
  +--------------+      +--------------+      +------------*  

````

*GPU analyzer*
````

  +--------------+      +--------------+      +------------+
  |              |      |              |      |            |
  | Texture ID   |----->| AnalyzerGPU  |----->| Descriptor |
  |              |      |              |      |            |
  +--------------+      +--------------+      +------------*  

````

## Todo

 - Allow the fex::config.cols and fex::config.rows to change, which means that 
   we need to destroy and reallocate some openGL objects (FBO, textures).


[roxlu] (www.twitter.com/roxlu)
