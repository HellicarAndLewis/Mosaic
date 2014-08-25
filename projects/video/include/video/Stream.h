/*
---------------------------------------------------------------------------------
 
                                               oooo
                                               `888
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                `888""8P d88' `88b  `88b..8P'   888  `888  `888
                 888     888   888    Y888'     888   888   888
                 888     888   888  .o8"'88b    888   888   888
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
 
                                                  www.roxlu.com
                                             www.apollomedia.nl
                                          www.twitter.com/roxlu
 
---------------------------------------------------------------------------------

  Test url:
  ----------
  web: http://www.hdwplayer.com/index.php/rtmp-streaming
  rtmp: rtmp://edge01.fms.dutchview.nl/botr/bunny.flv

  References:
  ----------
  See "Reading from an opened file" for some info on how to decode a stream: https://libav.org/doxygen/master/group__lavf__decoding.html#gae804b99aec044690162b8b9b110236a4

*/
#ifndef VIDEO_STREAM_H
#define VIDEO_STREAM_H

#include <stdint.h>
#include <vector>
#include <string>
#include <video/Types.h>

extern "C" {
#  include <libavformat/avformat.h>
#  include <libavcodec/avcodec.h>
}

#define ROXLU_USE_LOG
#include <tinylib.h>

namespace vid  {
  
  typedef void(*stream_on_frame)(AVFrame* frame, void* user);     /* gets called whenever we've decoded a frame */
  
  class Stream {

  public:
    Stream();
    ~Stream();
    int init(std::string url);                                   /* initialize our members and connect to the given stream url */
    int update();                                                /* must be called often; this will process any incoming data. */
    int shutdown();                                              /* cleans up allocated and initialized members */
    int eof();                                                   /* returns 1 when eof, else < 0 */

  public:
    AVFormatContext* fmt_ctx;                                    /* the AVFormatContext* ; our main context */  
    AVCodecContext* codec_ctx;                                   /* pointer to the video context */
    AVCodec* input_codec;                                        /* the codec that is used by the video stream */
    int video_stream_dx;                                         /* the video stream index; */ 
    double video_stream_timebase;                                /* the timebase of the video stream in seconds */
    int is_stream_open;                                          /* is set to 1 when the stream has been opened successfully */
    int is_eof;                                                  /* is set to 1 when the end of the stream is reached */

    stream_on_frame on_frame;                                   /* whenever we decode an frame, this will be called. */
    video_on_event on_event;                                    /* is called whenever e.g. the stream get disconnected or eof is read */
    void* user;                                                 /* user data; gets passed into the callback. */
  };

  inline int Stream::eof() {
    return is_eof;
  }

} /* namespace vid */

#endif
