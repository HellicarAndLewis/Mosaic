#include <video/Stream.h>

namespace vid {

  Stream::Stream() 
    :fmt_ctx(NULL)
    ,codec_ctx(NULL)
    ,input_codec(NULL)
    ,video_stream_dx(-1)
    ,video_stream_timebase(-1)
    ,is_stream_open(-1)
    ,is_eof(-1)
    ,on_frame(NULL)
    ,on_event(NULL)
    ,user(NULL)
  {
    /* init libav */
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

#if !defined(NDEBUG)
    av_log_set_level(AV_LOG_DEBUG);
#endif
  }
  
  Stream::~Stream() {
  }

  int Stream::init(std::string url) {

    int r, i;

    /* validate */
    if (0 == url.size()) {
      RX_ERROR("Invalid url");
      return -1;
    }

    if (NULL != fmt_ctx) {
      RX_ERROR("Trying to initialize the stream but we're already initialized.");
      return -2;
    }

    /* open the stream */
    r = avformat_open_input(&fmt_ctx, url.c_str(), NULL, NULL);
    if (r < 0) {
      RX_ERROR("Trying to open the rtmp stream.");
      return -3;
    }

    is_stream_open = 1;

    r = avformat_find_stream_info(fmt_ctx, NULL);
    if (r < 0) {
      RX_ERROR("Cannot find stream info.");
      return -4;
    }
    
    /* check if there are streams */
    if (0 == fmt_ctx->nb_streams) {
      printf("No streams found.");
      return -5;
    }

    /* find the video stream */
    for (i = 0; i < fmt_ctx->nb_streams; ++i) {
      if (AVMEDIA_TYPE_VIDEO == fmt_ctx->streams[i]->codec->codec_type) {
        video_stream_dx = i;
      }
    }
    if (-1 == video_stream_dx) {
      RX_ERROR("Warning: cannot find a video stream.");
      return -6;
    }

    /* find decoder */
    input_codec = avcodec_find_decoder(fmt_ctx->streams[video_stream_dx]->codec->codec_id);  
    if (NULL == input_codec) {
      RX_ERROR("Cannot find input codec.");
      return -7;
    }

    /* open codec. */
    codec_ctx = avcodec_alloc_context3(NULL);
    if (NULL == codec_ctx) {
      RX_ERROR("Cannot allocate the codec context.");
      ::exit(1);
    }

    r = avcodec_copy_context(codec_ctx, fmt_ctx->streams[video_stream_dx]->codec);
    if (r != 0) {
      RX_ERROR("Cannot copy the codec context!");
      ::exit(1);
    }

    r = avcodec_open2(codec_ctx, input_codec, NULL);
    if (r < 0) {
      RX_ERROR("Cannot open the coded for the rtmp stream.");
      return -8;
    }

    /* we are keeping track of frames in our jitter buffer, so this must be set.
       See: https://libav.org/doxygen/master/group__lavc__decoding.html#ga3e4048fd6893d4a001bdaa3d251f3c03 
       refcounted_frames: https://libav.org/doxygen/master/structAVCodecContext.html 
    */
    codec_ctx->refcounted_frames = 1;

    /* get the timebase in nanosec */
    video_stream_timebase = (double)(av_q2d(fmt_ctx->streams[video_stream_dx]->time_base));
    if (video_stream_timebase <= 0) {
      RX_ERROR("The video stream timebase is invalid.");
      return -11;
    }

    return 0;
  }
  
  int Stream::update() {

    static char serr[1024] = { 0 };
    AVFrame* frame;
    AVPacket pkt;
    int len;
    int got_picture;
    int r;

    /* We're very safe :) */
    if (NULL == fmt_ctx) { return -1; } 
    if (NULL == codec_ctx) { return -2; } 
    if (NULL == frame) { return -3; } 
    if (NULL == input_codec) { return -4; } 
    if (-1 == video_stream_dx) { return -5; } 
    if (-1 == is_stream_open) { return -6; } 
    if (1 == is_eof) { return -7; } 

    /* initialize the packet that will hold the encoded data */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /* read a new frame. */
    r = av_read_frame(fmt_ctx, &pkt);
    if (AVERROR_EOF == r) {
      RX_VERBOSE("End of stream")
      av_free_packet(&pkt);
      is_eof = 1;

      if (on_event) {
        on_event(VID_EVENT_EOF, user);
      }

      return 0;
    }
    else if (0 != r) {
      /* try to get a descriptive error. */
      r = av_strerror(r, serr, sizeof(serr));
      if (0 != r) {
        RX_ERROR("Cannot get a string representation of the error :$. %d", r);
      }

      RX_ERROR("Error: something went wrong with reading a packet: %d, %s", r, serr);
      av_free_packet(&pkt);
      return -7;
    }

    /* skip non-video packets */
    if (pkt.stream_index != video_stream_dx) {
      av_free_packet(&pkt);
      return 0;
    }

    /* allocate the frame that we used when decoding packets. */
    frame = av_frame_alloc();
    if (NULL == frame) {
      RX_ERROR("Cannot allocate the avframe.");
      av_free_packet(&pkt);
      return -9;
    }

    /* decode the video frame. */
    len = avcodec_decode_video2(codec_ctx, frame, &got_picture, &pkt);
    if (len < 0) {
      RX_ERROR("Error: cannot decode the packet: %d", len);
      av_free_packet(&pkt);
      return -8;
    }

    /* free the packet again. */
    av_free_packet(&pkt);

    /* pass the decoded frame to the user. */
    if (0 != got_picture && NULL != on_frame) {
      on_frame(frame, user);
    }

    return 0;
  }

  int Stream::shutdown() {

    /* close and free the codec context */
    if (NULL != codec_ctx) {
      avcodec_close(codec_ctx);
      av_free(codec_ctx);
    }

    /* close input stream, which will free all internal members. */
    if (1 == is_stream_open) {
      avformat_close_input(&fmt_ctx);
    }

    fmt_ctx = NULL;
    codec_ctx = NULL;
    input_codec = NULL;
    video_stream_dx = -1;
    video_stream_timebase = -1;
    is_stream_open = -1;
    is_eof = -1;

    return 0;
  }


} /* namespace vid */
