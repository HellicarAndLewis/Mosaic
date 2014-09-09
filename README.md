Mosaic
======

A real time interactive mosaic made by combining a live video feed
with still imagery from social networks.

GPU based background/foreground segmentation with sparse blob detection 
is used to activate the mirror, allowing for interactive changing of individual 
tile scale on movement.

#### Applications

This project contains several applications which are meant to be run
together, but can run standalone. The project can be separated into
two main parts two. First there is a node js based administration
panel that retrieves instagram images in realtime. Using the
administration panel one can approve or decline an instagram image as
part of the mosaic. Secondly there are a couple of standalone, (Mac
only, tested on 10.9.3) application that analyze the instagram images,
create the mosaic and the smoothly scrolling grids.

| Application        | Info                                                       | Directory                              |
| ------------------ | ---------------------------------------------------------- | -------------------------------------- |
| AppMosaic          | Generates a mosaic in real time from a video stream        | `projects/mosaic`                      |
| AppGridLeft        | A smooth scrolling grid of images                          | `projects/grid`                        |
| AppGridRight       | Same as AppGridLeft, other scrolling direction             | `projects/grid`                        |
| AppImageProcessor  | Performs image analysis                                    | `projects/topshop`                     |
| Image Downloaded   | Downloads approved images from instagran (node.js)         | `projects/admin/apps/image_downloader` |
| Administrion Panel | Approve / decline instagram images (node.js)               | `projects/admin/apps/instagram_admin`  |


#### Installation


##### Compile App{Mosaic,GridLeft,GridRight,ImageProcessor}

We created a scripted build process for compiling the Apps* which makes compiling
the applications fairly simple. The App* do need a couple of external libraries which 
may run fine now, but they can/will change in the future so refer to the versions
of the list of dependencies when you get into trouble. Note that at this time, the 
application will only run on Mac 10.9.

````sh
mkdir Mosaic
cd Mosaic
git clone https://github.com/HellicarAndLewis/Mosaic.git .
cd build
./build_mac_dependencies.sh
./release.sh
````

When you get into trouble with compiling, please get in touch with [roxlu](http://www.roxlu.com)

##### Administration Panel and Image Downloader

For more information about how to install the administration panel and the 
image downloader see the README in `projects/admin/apps/instagram_admin`.
