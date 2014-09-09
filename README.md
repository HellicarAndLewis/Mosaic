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

#### Dependencies

| Library           | Version                                     | Download URL or GIT                                                                                                    |
| ----------------- | ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------  |
| libuv             | a4f03504ceb7c7db6f05cc176bb37cd4b632bda3    | [libuv](https://github.com/joyent/libuv)                                                                               |
| libpng            | 1.2.51                                      | [png](http://prdownloads.sourceforge.net/libpng/libpng-1.2.51.tar.gz?download)                                         |
| libjpeg           | v9a                                         | [jpg](http://www.ijg.org/files/jpegsrc.v9a.tar.gz)                                                                     |
| libz              | 1.2.8                                       | [libz](http://zlib.net/zlib-1.2.8.tar.gz)                                                                              |
| glfw3             | a5281df501926ab8d9af99c5ea2af7fb87e3b505    | [glfw](https://github.com/glfw/glfw)                                                                                   |
| libav             | 749b1f1359b5af0a08221923b016551b18ab6171    | [libav](https://libav.org/)                                                                                            |
| librxp_player     | ccd77f4302ecf315bff7ba8b66f593fa42fb38bf    | [rxp_player](https://github.com/roxlu/rxp_player)                                                                      |
| libtheora         | r19181                                      | [libtheora svn](https://github.com/roxlu/rxp_player)                                                                   |
| libvorbis         | 1.3.3                                       | [libvorbis](http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.3.tar.gz)                                          |
| libogg            | 1.3.1                                       | [libor](http://downloads.xiph.org/releases/ogg/libogg-1.3.1.tar.gz)                                                    |
| libcurl           | 7.37.1                                      | [libcurl](http://curl.haxx.se/download/curl-7.37.1.tar.gz)                                                             |
| yasm              | 1.3.0                                       | [yasm](http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz)                                                |
| libjansson        | 2.6                                         | [jansson](http://www.digip.org/jansson/releases/jansson-2.6.tar.gz)                                                    |
| libcairo          | 29a8b4e970379ca04a7db8e63c71bb34c0e349ce    | [cairo](git://anongit.freedesktop.org/git/cairo)                                                                       |
| pixman            | 0.32.6                                      | [pixman](http://cairographics.org/releases/pixman-0.32.6.tar.gz)                                                       |
| freetype          | 336735d8de0bc5f9ef8018ae11d322cb6e59fa4a    | [freetype](git://git.sv.nongnu.org/freetype/freetype2.git)                                                             |
| opencv            | 3.0.0-alpha                                 | [opencv](https://github.com/Itseez/opencv/archive/3.0.0-alpha.zip)                                                     |
| rapidxml          | 1.13                                        | [rapidxml](https://sourceforge.net/projects/rapidxml/files/rapidxml/rapidxml%201.13/rapidxml-1.13.zip/download)        |
| glad              | ada89398e8b7b4af64648e328347b70d48600a2a    | [glad](https://github.com/Dav1dde/glad.git)                                                                            |
| imagemagick       | 6.8.9                                       | [imagemagick](ftp://ftp.imagemagick.org/pub/ImageMagick/binaries/ImageMagick-x86_64-apple-darwin13.2.0.tar.gz)         |             
| gettext           | 0.19.2                                      | [gettext](http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.2.tar.xz)                                                    |        
| libtracker        | 75814ff80f5f32f9b4e633b525072d178f3957f6    | [tracker](https://github.com/roxlu/tracker)                                                                            |           


We're using the following working set

| Application           | Version      |
| --------------------- | ------------ |
| autoconf              | 2.69         |
| libtool               | 2.4.2        |
| automake              | 1.14         |
| pkgconfig             | 0.28         |

We're using the following Mac 10.9 frameworks

| Framework & system libs   |
| ------------------------- |
| CoreFoundation            |
| AVFoundation              |
| CoreMedia                 |
| Cocoa                     |
| OpenGL                    |
| IOKit                     |
| CoreVideo                 |
| OpenCL                    |
| AudioUnit                 |
| CoreAudio                 |
| AudioToolbox              |
| ssl                       |
| crypto                    |
| ldap                      |
| bz1                       |