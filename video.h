#ifndef VIDEO_DECODE_H
#define VIDEO_DECODE_H

#include <stdint.h>
#include <libavfilter/buffersink.h>
#include <libavformat/avformat.h>
#include <pthread.h>

typedef struct video_data {
    AVFormatContext *fmt_ctx;
    AVCodecContext *dec_ctx;
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
    int video_stream_index;
    int64_t last_pts;
    AVFrame *filt_frame;
    AVPacket *packet;
    AVFrame *frame;
    pthread_mutex_t lock;
    const int flipY;
    const int loop;
    const char * src;
    enum AVPixelFormat output_format;
} video_data;

int playvideo(video_data * vdata);


#endif
