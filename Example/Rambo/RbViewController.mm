//
//  RbViewController.m
//  Rambo
//
//  Created by czqasngit on 07/26/2018.
//  Copyright (c) 2018 czqasngit. All rights reserved.
//

#import "RbViewController.h"
#import <Rambo/Rambo.h>

static int read_data(void *opaque, uint8_t *buffer, int buffer_size, int buffer_start) {
    
    RbViewController *_self = (__bridge RbViewController *)opaque ;
    [_self.handle seekToFileOffset:buffer_start] ;
    NSData *data = [_self.handle readDataOfLength:buffer_size] ;
    if (data.length > 0) {
        [data getBytes:buffer length:data.length] ;
        return (int)data.length ;
    }
    return -1 ;
}


@interface RbViewController ()
@property (nonatomic, strong)UIImageView *imageView ;
@end

@implementation RbViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.imageView = [[UIImageView alloc] initWithFrame:CGRectMake(100, 100, 200, 200)] ;
    self.imageView.contentMode = UIViewContentModeScaleAspectFit ;
    [self.view addSubview:self.imageView] ;
    self.imageView.backgroundColor = [UIColor orangeColor] ;
    self.handle = [NSFileHandle fileHandleForReadingAtPath:[NSBundle.mainBundle pathForResource:@"4" ofType:@"avi"]] ;
    NSDictionary *attr = [[NSFileManager defaultManager] attributesOfItemAtPath:[NSBundle.mainBundle pathForResource:@"4" ofType:@"avi"] error:NULL] ;
    uint64_t size = [attr[NSFileSize] longLongValue] ;
    char *error_message = NULL  ;
    RamboContext *context = rambo_alloc_rambo_context(read_data, size, (__bridge  void *)self) ;
    if(!context) return ;
    if (!error_message) {
        dispatch_async(dispatch_get_global_queue(0,0), ^{
            int size = 5 ;
            double times[5] = {10,60,90,120,150} ;
            RamboRGBAFrame *frames = rambo_copy_frames(context, times, size) ;
            NSMutableArray *tmps = [[NSMutableArray alloc] initWithCapacity:size] ;
            for (int i = 0; i < size; i ++) {
                RamboRGBAFrame *frame = frames + i ; 
                CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, frame->data, frame->size, NULL) ;
                CGImageRef image = CGImageCreate(context->information->width,
                                                 context->information->height,
                                                 8,
                                                 32,
                                                 4 * context->information->width,
                                                 CGColorSpaceCreateDeviceRGB(),
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big,
                                                 provider,
                                                 NULL,
                                                 false,
                                                 kCGRenderingIntentDefault) ;
                if(image) {
                    [tmps addObject:[UIImage imageWithCGImage:image]] ;
                }
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                self.imageView.animationImages = tmps ;
                self.imageView.animationDuration = 2 ;
                [self.imageView startAnimating] ;
            }) ;
            rambo_frames_free(frames, size, false) ;
            rambo_context_free(context) ;
        }) ;
    }
    
}

@end
