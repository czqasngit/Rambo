

Pod::Spec.new do |s|
  s.name             = 'Rambo'
  s.version          = '1.0.0'
  s.summary          = 'Image extract tool kit base on ffmpeg'
  s.description      = 'Extract image from video file/stream base on ffmpeg written by C'
  s.homepage         = 'https://github.com/czqasngit/Rambo'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'czqasngit' => 'czqasn_6@163.com' }
  s.source           = { :git => 'https://github.com/czqasngit/Rambo.git', :tag => s.version.to_s }
  s.ios.deployment_target = '9.0'
  s.source_files = 'Rambo/Classes/**/*'
  s.libraries = 'bz2','c++','iconv','z'
  s.public_header_files = 'Pod/Classes/**/*.{h}'
  s.subspec 'FFMpeg' do |ss|
    ss.source_files = 'Rambo/FFmpeg.framework/Headers/**.{h,m}'
    ss.public_header_files = 'Rambo/FFmpeg.framework/Headers/**.h'
    ss.vendored_frameworks = 'Rambo/FFmpeg.framework'
    ss.preserve_paths = 'Rambo/FFmpeg.framework'
    ss.pod_target_xcconfig = { 'LD_RUNPATH_SEARCH_PATHS' => '$(PODS_ROOT)/Rambo/' }
    ss.frameworks =  'VideoToolBox', 'AudioToolBox', 'CoreMedia'
  end
end
