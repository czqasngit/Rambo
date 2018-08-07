//
//  Rambo.c
//  Pods
//
//  Created by legendry on 2018/7/26.
//

#include "Rambo.h"

inline static void _PrintErrorCode(int error_code) {
    int size = 1024 * 2 ;
    char *buf = (char *)malloc(size) ;
    memset(buf, 0, size) ;
    int ret = av_strerror(error_code, buf, size) ;
    if(ret == 0) {
        printf("FFmpeg error: %s \n", buf);
    }else {
        printf("FFmpeg error: unknow error") ;
    }
    free(buf) ;
}

int rambo_read_data(void *opaque, uint8_t *buf, int buf_size) {
    RamboContext *context = (RamboContext *)opaque ;
    int result = context->read_data(context->avio_opaque, buf, buf_size, (int)context->buffer_context->current_pos) ;
    context->buffer_context->current_pos += result ;
    return result ;
}
int64_t rambo_seek(void *opaque, int64_t offset, int whence) {
    RamboContext *context = (RamboContext *)opaque ;
    switch (whence) {
        case SEEK_SET:
            context->buffer_context->current_pos = offset ;
            break;
        case SEEK_CUR:
            context->buffer_context->current_pos += offset ;
            break ;
        case SEEK_END:
            context->buffer_context->current_pos = context->buffer_context->total_size + offset ;
            break ;
        default:
            return -1 ;
            break;
    }
    return context->buffer_context->current_pos ;
}

RamboContext *rambo_alloc_rambo_context(int (*read_data)(void *opaque, uint8_t *buf, int buf_size, int buf_start),
                                        int64_t total_size,
                                        void *opaque)  {
    RamboContext *rc = (RamboContext *)malloc(sizeof(RamboContext)) ;
    memset((void *)rc, 0, sizeof(RamboContext)) ;
    rc->buffer_context = (RamboBufferContext *)malloc(sizeof(RamboBufferContext)) ;
    rc->buffer_context->current_pos = 0 ;
    rc->buffer_context->total_size = total_size ;
    rc->read_data = read_data ;
    rc->avio_opaque = opaque ;
    int buffer_size = 1024 * 4 ;
    uint8_t *buffer = NULL ;
    buffer = (uint8_t *)av_malloc(buffer_size) ;
    av_register_all() ;
    avcodec_register_all() ;
    rc->av_format_context = avformat_alloc_context() ;
    if (rc->av_format_context == NULL) {
        printf("Rambo error: avformat_alloc_context failed. \n") ;
        goto end ;
    }
    rc->avio_context = avio_alloc_context(buffer, buffer_size, 0, rc, rambo_read_data, NULL, rambo_seek) ;
    rc->av_format_context->pb = rc->avio_context ;
    if (rc->av_format_context->pb == NULL) {
        printf("Rambo error: avio_alloc_context failed \n") ;
        goto end ;
    }
    int ret = avformat_open_input(&rc->av_format_context, NULL, NULL, NULL) ;
    if (ret != 0) {
        _PrintErrorCode(ret);
        goto end ;
    }
    ret = avformat_find_stream_info(rc->av_format_context, NULL) ;
    if (ret < 0) {
        _PrintErrorCode(ret);
        goto end ;
    }
    int stream_video_index = -1 ;
    for (int i = 0 ; i < rc->av_format_context->nb_streams; i ++) {
        if (rc->av_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_video_index = i ;
        }
    }
    if (stream_video_index == -1) {
        printf("Rambo error: Not found video stream.") ;
        goto end ;
    }
    AVStream *video_stream = rc->av_format_context->streams[stream_video_index] ;
    rc->av_video_codec = avcodec_find_decoder(video_stream->codecpar->codec_id) ;
    rc->av_codec_context = avcodec_alloc_context3(rc->av_video_codec) ;
    avcodec_parameters_to_context(rc->av_codec_context, video_stream->codecpar) ;
    rc->information = (RamboVideoInformation *)malloc(sizeof(RamboVideoInformation)) ;
    rc->information->duration = video_stream->duration ;
    rc->information->width = rc->av_codec_context->width ;
    rc->information->height = rc->av_codec_context->height ;
    rc->information->fps =  video_stream->time_base.den / video_stream->time_base.num ;
    ret = avcodec_open2(rc->av_codec_context, rc->av_video_codec, NULL) ;
    if (ret != 0) {
        _PrintErrorCode(ret);
        goto end ;
    }
    rc->stream_index = stream_video_index ;
    return rc ;
end:
    if(buffer) {
        if(rc->avio_context) {
            if(rc->avio_context->buffer) av_free(rc->avio_context->buffer) ;
            avio_context_free(&rc->avio_context) ;
        } else {
            av_free(buffer) ;
        }
    }
    if(rc->av_format_context) {
        avformat_close_input(&rc->av_format_context);
        avformat_free_context(rc->av_format_context) ;
    }
    
    if(rc->av_codec_context) avcodec_free_context(&rc->av_codec_context);
    if(rc->information) free(rc->information) ;
    if(rc->buffer_context) free(rc->buffer_context);
    if(rc) free(rc);
    return NULL ;
}



RamboRGBAFrame *rambo_copy_frame(RamboContext *context, double time) {
    if (!context || !context->avio_context) return NULL ;
    RamboRGBAFrame *frame = NULL ;
    struct SwsContext *sws_context = NULL ;
    AVStream *stream = context->av_format_context->streams[context->stream_index] ;
    float ts = time * (stream->time_base.den * 1.0 / stream->time_base.num) ;
    int seek_ret = av_seek_frame(context->av_format_context, context->stream_index, floor(ts), AVSEEK_FLAG_BACKWARD) ;
    if (seek_ret < 0) {
        _PrintErrorCode(seek_ret);
        goto end ;
    }
    frame = (RamboRGBAFrame *)malloc(sizeof(RamboRGBAFrame)) ;
    int rgba_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGBA, context->information->width, context->information->height, 1) ;
    if (rgba_buffer_size <= 0) {
        goto end ;
    }
    frame->data = (uint8_t *)malloc(rgba_buffer_size) ;
    frame->size = rgba_buffer_size ;
    sws_context = sws_getContext(context->information->width,
                                                    context->information->height,
                                                    context->av_codec_context->pix_fmt,
                                                    context->information->width,
                                                    context->information->height,
                                                    AV_PIX_FMT_RGBA,
                                                    SWS_BICUBIC,
                                                    NULL,
                                                    NULL,
                                                    NULL) ;
    int success = 0 ;
    AVPacket packet ;
    av_init_packet(&packet) ;
    while (!success && av_read_frame(context->av_format_context, &packet) >= 0 ) {
        if (packet.stream_index == context->stream_index) {
            int ret = -1 ;
            ret = avcodec_send_packet(context->av_codec_context, &packet) ;
            if(ret == 0) {
                AVFrame *originFrame = av_frame_alloc() ;
                AVFrame *dstFrame = av_frame_alloc() ;
                while ((ret = avcodec_receive_frame(context->av_codec_context, originFrame)) == 0) {
                    if (originFrame->key_frame) {
                        av_image_fill_arrays(dstFrame->data,
                                             dstFrame->linesize,
                                             frame->data, AV_PIX_FMT_RGBA,
                                             context->information->width,
                                             context->information->height,
                                             1) ;
                        sws_scale(sws_context,
                                  (const uint8_t *const *)originFrame->data,
                                  originFrame->linesize,
                                  0,
                                  context->information->height,
                                  dstFrame->data,
                                  dstFrame->linesize) ;
                        success = 1 ;
                    }
                    av_frame_unref(originFrame);
                }
                av_frame_free(&originFrame) ;
                av_frame_free(&dstFrame) ;
            }
        }
        av_packet_unref(&packet) ;
    }
    if (!success) {
        goto end ;
    }
    if(sws_context) sws_freeContext(sws_context) ;
    return frame ;
    
end:
    if(frame) rambo_frame_free(frame, true) ;
    if(sws_context) sws_freeContext(sws_context) ;
    return NULL ;
    
}

void sort(double *times, int time_size) {
    for (int i = time_size - 1; i >= 0; i --) {
        for (int j = 0; j < i ; j ++) {
            int a = *(times + j) ;
            int b = *(times + j + 1) ;
            if (a > b) {
                *(times + j) = b ;
                *(times + j + 1) = a ;
            }
        }
    }
}

RamboRGBAFrame *rambo_copy_frames(RamboContext *context ,double *times, int time_size) {
    RamboRGBAFrame *frames = (RamboRGBAFrame *)malloc(sizeof(RamboRGBAFrame) * time_size) ;
    for (int i = 0; i < time_size; i ++) {
        RamboRGBAFrame *frame = rambo_copy_frame(context, *(times + i)) ;
        if(frame) {
            *(frames + i) = *frame ;
            free(frame) ;
        }
    }
    return frames ;
}



void rambo_frame_free(RamboRGBAFrame *frame, bool freeAssociatedData) {
    if (frame) {
        if (freeAssociatedData && frame->data) free(frame->data) ;
        free(frame) ;
    }
}
void rambo_frames_free(RamboRGBAFrame *frames, size_t size, bool freeAssociatedData) {
    for (int i = 0; i < size; i ++) {
        RamboRGBAFrame *frame = (frames + i) ;
        if (freeAssociatedData && frame->data) free(frame->data) ;
    }
    free(frames) ;
}
void rambo_context_free(RamboContext *context) {
    if (context) {
        if (context->avio_context) {
            if(context->avio_context->buffer) av_free(context->avio_context->buffer) ;
            avio_context_free(&context->avio_context);
        }
        if(context->av_format_context) {
            avformat_close_input(&context->av_format_context) ;
            avformat_free_context(context->av_format_context) ;
        }
        if(context->information) free(context->information) ;
        if(context->av_codec_context) avcodec_free_context(&context->av_codec_context);
        if(context->buffer_context) free(context->buffer_context);
        free(context) ;
    }
}


