# Rambo

[![Version](https://img.shields.io/cocoapods/v/Rambo.svg?style=flat)](https://cocoapods.org/pods/Rambo)
[![License](https://img.shields.io/cocoapods/l/Rambo.svg?style=flat)](https://cocoapods.org/pods/Rambo)
[![Platform](https://img.shields.io/cocoapods/p/Rambo.svg?style=flat)](https://cocoapods.org/pods/Rambo)

Extract image from video file/stream base on ffmpeg written by C

基于C编写的ffmpeg从视频文件/流中提取图像
![Rambo_demo-2](http://pba6dsu9x.bkt.clouddn.com/Rambo_demo-2.gif)

## Architecture
Currently FFmpeg.framework only supports x86_64, you can use the following tools to compile FFmpeg.framework that supports more architectures.

当前FFmpeg.framework仅支持x86_64,你可以使用以下工具编译支持更多架构的FFmpeg.framework.

https://github.com/kewlbear/FFmpeg-iOS-build-script/blob/master/build-ffmpeg-iOS-framework.sh

###### Note: Please remove double FFmpegVersionNumber = $FFMPEG_VERSION;'
###### 注意:请删除 'double FFmpegVersionNumber = $FFMPEG_VERSION;'

## Futues

Extract image from video file/stream

从视频文件或者流中提取指定位置的图像

## Usage(Objective-C)

```
RamboContext *context = rambo_alloc_rambo_context(read_data, size, (__bridge  void *)self) ;
RamboRGBAFrame *frames = rambo_copy_frames(context, times, size) ;
rambo_frames_free(frames, size, false) ;
rambo_context_free(context) ;
```

## Example

To run the example project, clone the repo, and run `pod install` from the Example directory first.

## Requirements

## Installation

Rambo is available through [CocoaPods](https://cocoapods.org). To install
it, simply add the following line to your Podfile:

```ruby
pod 'Rambo'
```

## Author

czqasn, czqasn_6@163.com

## License

Rambo is available under the MIT license. See the LICENSE file for more info.


