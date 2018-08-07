//
//  Rambo.h
//  Pods
//
//  Created by legendry on 2018/7/26.
//

#ifndef Rambo_h
#define Rambo_h


#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    
#include <FFmpeg/avcodec.h>
#include <FFmpeg/avformat.h>
#include <FFmpeg/imgutils.h>
#include <FFmpeg/swscale.h>

    
typedef struct RamboVideoInformation {
    int64_t duration ;
    int width ;
    int height ;
    int fps ;
}RamboVideoInformation;

typedef struct RamboBufferContext {
    int64_t current_pos ;
    int64_t total_size ;
}RamboBufferContext;


/**
 Rambo Context
 */
typedef struct RamboContext {
    // read data callback
    // opaque: rambo_alloc_rambo_context third param
    // buf: refilling buffer
    // buf_size: refilling buffer size
    // buf_start: read data start bytes index
    int (*read_data)(void *opaque, uint8_t *buf, int buf_size, int buf_start) ;
    AVFormatContext *av_format_context ;
    AVCodec *av_video_codec ;
    AVCodecContext *av_codec_context ;
    AVIOContext *avio_context ;
    //Video information
    RamboVideoInformation *information ;
    RamboBufferContext *buffer_context ;
    //rambo_alloc_rambo_context third param
    void *avio_opaque ;
    int stream_index ;
}RamboContext;
    


/**
 Bitmap data for RGBA
 */
typedef struct RamboRGBAFrame {
    uint8_t *data ;
    int size ;
}RamboRGBAFrame;


//,
/**
 New RamboContext

 @param read_data read data callback
 @param total_size data size
 @param opaque read_data callback first param
 @return RamboContext or null
 */
RamboContext *rambo_alloc_rambo_context(int (*read_data)(void *opaque, uint8_t *buf, int buf_size, int buf_start),
                                        int64_t total_size,
                                        void *opaque) ;
    
/**
 Get and decode a screenshot of a keyframe image at a specified point in time
 Tips: Since only key frames can be decoded, the images acquired at some point in time are consistent.
 @param context RamboContext context
 @param time time
 @return RamboRGBAFrame or null
 */
RamboRGBAFrame *rambo_copy_frame(RamboContext *context ,double time) ;
RamboRGBAFrame *rambo_copy_frames(RamboContext *context ,double *times, int time_size) ;

    
/**
 release RamboRGBAFrame memory

 @param frame RamboRGBAFrame
 @param freeAssociatedData whether release 'uint8_t *data'
 */
void rambo_frame_free(RamboRGBAFrame *frame, bool freeAssociatedData);
    

/**
 release RamboRGBAFrame(s) memory

 @param frames RamboRGBAFrame(s)
 @param size array size
 @param freeAssociatedData whether release 'uint8_t *data'
 */
void rambo_frames_free(RamboRGBAFrame *frames, size_t size, bool freeAssociatedData);
    

/**
 release RamboContext

 @param context RamboContext context
 */
void rambo_context_free(RamboContext *context) ;

    

#ifdef __cplusplus
}
#endif
    
#endif /* Rambo_h */

