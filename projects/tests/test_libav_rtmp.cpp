/*


  test_libav_rtmp
  ---------------

  Example that connects to a RTMP server and decodes the video stream.
  You can test this locally by creating a rtmp stream with avconv, like:

           ./avconv -re -i test.mov -f flv -listen 1 rtmp://localhost/

  This command will start streaming the test.mov file over rtmp whenever 
  someone connects to it.
  

 */

#include <stdlib.h>
#include <stdio.h>

extern "C" {
#  include <libavformat/avformat.h>
#  include <libavcodec/avcodec.h>
}
//#include <libavutil/common.h>

struct Player {
  AVFormatContext* fmt_ctx;
  AVCodec* input_codec;
  AVCodecContext* codec_ctx;
  AVFrame* frame;
  int video_stream_dx;
};

static int player_init(Player* p);

int main() {

  printf("\n\ntest_libav_rtmp\n\n");

  int len;
  int got_picture;
  int r;
  Player p;

  if (0 != player_init(&p)) {
    printf("Error: cannot init the player.\n");
    exit(1);
  }

  while(1) {

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /* read a new frame. */
    r = av_read_frame(p.fmt_ctx, &pkt);
    if (0 != r) {
      printf("Error: something went wrong with reading a packet.\n");
      av_free_packet(&pkt);
      continue;
    }

    /* skip non-video packets */
    if (pkt.stream_index != p.video_stream_dx) {
      av_free_packet(&pkt);
      continue;
    }

    printf("Read a frame. size: %d, dts: %lld, stream: %d\n", pkt.size, pkt.dts, pkt.stream_index);

    /* @todo - check if this is really a video frame. */

    /* decode the video frame. */
    len = avcodec_decode_video2(p.codec_ctx, p.frame, &got_picture, &pkt);
    if (len < 0) {
      printf("Error: cannot decode the packet.\n");
      av_free_packet(&pkt);
      continue;
    }

    av_free_packet(&pkt);

    if (0 != got_picture) {
      
      printf(">>>>>> YEP.\n");
    }
  }

  return 0;
}

/* @todo - avformat_network_deinit() */
/* @todo - nicely close with avformat_close_input() */
/* @todo - call avformat_close_input, see https://libav.org/doxygen/master/transcode_aac_8c-example.html#a6 */
static int player_init(Player* p) {
  int r;
  int i;

  if (NULL == p) { return -1; } 

  p->fmt_ctx = NULL;
  p->input_codec = NULL;
  p->frame = NULL;
  p->video_stream_dx = -1;
  
  /* init libav */
  av_register_all();
  avformat_network_init();
  avcodec_register_all();

  /* open the stream */
 
  //r = avformat_open_input(&p->fmt_ctx, "rtmp://localhost", NULL, NULL);
  r = avformat_open_input(&p->fmt_ctx, "rtmp://edge01.fms.dutchview.nl/botr/bunny.flv", NULL, NULL);
  if (r < 0) {
    printf("Error: trying to open the rtmp stream.\n");
    return -2;
  }

  r = avformat_find_stream_info(p->fmt_ctx, NULL);
  if (r < 0) {
    printf("Error: cannot find stream info.\n");
    return -3;
  }

  /* check if there are streams */
  if (0 == p->fmt_ctx->nb_streams) {
    printf("Error: no streams found.\n");
    return -4;
  }

  /* find the video stream */
  for (i = 0; i < p->fmt_ctx->nb_streams; ++i) {
    if (AVMEDIA_TYPE_VIDEO == p->fmt_ctx->streams[i]->codec->codec_type) {
      p->video_stream_dx = i;
    }
  }
  if (-1 == p->video_stream_dx) {
    printf("Warning: cannot find a video stream.\n");
    return -5;
  }

  /* find decoder */
  p->input_codec = avcodec_find_decoder(p->fmt_ctx->streams[0]->codec->codec_id);  
  if (NULL == p->input_codec) {
    printf("Error: cannot find input codec.\n");
    return -6;
  }

  /* open codec. */
  r = avcodec_open2(p->fmt_ctx->streams[0]->codec, p->input_codec, NULL);
  if (r < 0) {
    printf("Error: cannod open the coded for the rtmp stream.\n");
    return -7;
  }

  /* allocate the frame that we used when decoding packets. */
  p->frame = av_frame_alloc();
  if (NULL == p->frame) {
    printf("Error: cannot allocate the avframe.\n");
    return -8;
  }

  p->codec_ctx = p->fmt_ctx->streams[p->video_stream_dx]->codec;

  printf("Verbose: stream codec id: %d\n", p->fmt_ctx->streams[p->video_stream_dx]->codec->codec_id);
  printf("input_code: %p\n", p->input_codec);
  printf("r: %d\n", r);

  return 0;
}
