#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include "video.h"
// hack in order to get cmake to link against local swscale swresample libraries
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
void __cmake_hack_(){
    SwsVector *a;
    double scalar;
    sws_scaleVec(a, scalar);
    struct SwrContext *s;
    swr_init(s);
}

void* video_playback(void* vdata)
{
    video_data * v = vdata;
    int64_t duration_base;
    int64_t duration;
    int64_t delay_us;
    int64_t pts_us = 0;
    int ret;

    while(1){
            duration_base = av_gettime();
        while (1) {
            if (av_read_frame(v->fmt_ctx, v->packet) < 0)
                break;

            if (v->packet->stream_index == v->video_stream_index) {
                if (avcodec_send_packet(v->dec_ctx, v->packet) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                    break;
                }

                while (1) {
                    ret = avcodec_receive_frame(v->dec_ctx, v->frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    }

                    v->frame->pts = v->frame->best_effort_timestamp;

                    // push the decoded frame into the filter graph
                    if (av_buffersrc_add_frame_flags(v->buffersrc_ctx, v->frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filter graph\n");
                        break;
                    }

                    // pull filtered frames from the filter graph
                    while (1) {
                        ret = av_buffersink_get_frame(v->buffersink_ctx, v->filt_frame);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                            break;
                        }

                        duration = av_gettime() - duration_base;
                        delay_us = av_rescale_q(v->frame->pts,v->buffersink_ctx->inputs[0]->time_base, AV_TIME_BASE_Q) - duration;
                        if (delay_us > 0) {
                            av_usleep(delay_us);
                        }
                        av_frame_unref(v->filt_frame);
                    }
                    av_frame_unref(v->frame);
                }
            }
            av_packet_unref(v->packet);
        }
        if (v->loop){
            // loop video playback
            av_seek_frame(v->fmt_ctx, -1, 0, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(v->dec_ctx);
        } else {
            break;
        }
    }
    pthread_mutex_lock(&v->lock);
    avfilter_graph_free(&v->filter_graph);
    avcodec_free_context(&v->dec_ctx);
    avformat_close_input(&v->fmt_ctx);
    av_frame_free(&v->frame);
    av_frame_free(&v->filt_frame);
    av_packet_free(&v->packet);
    pthread_mutex_unlock(&v->lock);
    
    return 0;
}

int playvideo( video_data * vdata)
{
    vdata->video_stream_index = -1;
    vdata->last_pts = AV_NOPTS_VALUE;
    const AVCodec *dec;

    if (avformat_open_input(&vdata->fmt_ctx, vdata->src, NULL, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Can't open: '%s'\n", vdata->src);
        return 1;
    }

    if (avformat_find_stream_info(vdata->fmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Can't find stream information\n");
        return 1;
    }

    // select the video stream
    if ((vdata->video_stream_index = av_find_best_stream(vdata->fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Can't find a video stream in the input file\n");
        return 1;
    }

    // create decoding context
    vdata->dec_ctx = avcodec_alloc_context3(dec);
    if (!vdata->dec_ctx){
        return 1;
    }
    avcodec_parameters_to_context(vdata->dec_ctx, vdata->fmt_ctx->streams[vdata->video_stream_index]->codecpar);

    // initialize the video decoder
    if (avcodec_open2(vdata->dec_ctx, dec, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Can't open video decoder\n");
        return 1;
    }

    // initialize filter graph - used for flipY and setting output pixel format
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");

    vdata->filter_graph = avfilter_graph_alloc();
    if (!vdata->filter_graph) {
        fprintf(stderr, "Could not allocate filter graph\n");
        return 1;
    }

    AVRational time_base = vdata->fmt_ctx->streams[vdata->video_stream_index]->time_base;
    char * args = av_asprintf("video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", vdata->dec_ctx->width, vdata->dec_ctx->height, vdata->dec_ctx->pix_fmt,
            time_base.num, time_base.den,
            vdata->dec_ctx->sample_aspect_ratio.num, vdata->dec_ctx->sample_aspect_ratio.den);
    int created_buffersrc = (avfilter_graph_create_filter(&vdata->buffersrc_ctx, buffersrc, "in",
                                    args, NULL, vdata->filter_graph)) >= 0;
    av_free(args);

    if (created_buffersrc) {
        int created_buffersink = avfilter_graph_create_filter(&vdata->buffersink_ctx, buffersink, "out",
                                        NULL, NULL, vdata->filter_graph) >= 0;
        if (created_buffersink) {
            // set output pixel format
            enum AVPixelFormat pix_fmts[] = { vdata->output_format, AV_PIX_FMT_NONE };
            int pixel_format_set = av_opt_set_int_list(vdata->buffersink_ctx, "pix_fmts", pix_fmts,
                                    AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN) >= 0;
            if (pixel_format_set) {
                AVFilterInOut *outputs = avfilter_inout_alloc();
                AVFilterInOut *inputs  = avfilter_inout_alloc();
                if (!outputs || !inputs) {
                    fprintf(stderr, "Could not allocate filter inputs or output\n");
                    return 1;
                }
                outputs->name       = av_strdup("in");
                outputs->filter_ctx = vdata->buffersrc_ctx;
                outputs->pad_idx    = 0;
                outputs->next       = NULL;

                inputs->name       = av_strdup("out");
                inputs->filter_ctx = vdata->buffersink_ctx;
                inputs->pad_idx    = 0;
                inputs->next       = NULL;

                char * filter_flags = vdata->flipY ? "vflip" : "null";
                avfilter_graph_parse_ptr(vdata->filter_graph, filter_flags,
                                                &inputs, &outputs, NULL);
                avfilter_graph_config(vdata->filter_graph, NULL);

                avfilter_inout_free(&inputs);
                avfilter_inout_free(&outputs);
            } else {
                av_log(NULL, AV_LOG_ERROR, "Can't set output pixel format\n");
            }
        } else {
            av_log(NULL, AV_LOG_ERROR, "Can't create buffer sink\n");
        }
    } else {
        av_log(NULL, AV_LOG_ERROR, "Can't create buffer source\n");
    }

    vdata->frame = av_frame_alloc();
    vdata->filt_frame = av_frame_alloc();
    vdata->packet = av_packet_alloc();
    if (!vdata->frame || !vdata->filt_frame || !vdata->packet) {
        fprintf(stderr, "Could not allocate frame or packet\n");
        return 1;
    }

    if (pthread_mutex_init(&vdata->lock, NULL) != 0) {
        fprintf(stderr, "Could not initialize mutex lock\n");
        return 1;
    }
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, video_playback, vdata);

    return 0;
}
