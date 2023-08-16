# set(USE_SYSTEM_FFMPEG 1)

if (USE_SYSTEM_FFMPEG)
message( STATUS "attempting to use system ffmpeg")
  set(HAS_FFMPEG 1)
elseif(EXISTS ${CMAKE_SOURCE_DIR}/ffmpeg)
  message( STATUS "ffmpeg exists")
  set(HAS_FFMPEG 1)
else()
  message( WARNING "\nYou will need to have ffmpeg libraries to use the video examples. \nInstall the library on you system and uncomment \"set(USE_SYSTEM_FFMPEG 1)\" in `ffmpeg.cmake` or Get a \"shared\" build of it from here: \nhttps://github.com/BtbN/FFmpeg-Builds/releases\nThen extract it to the root source directory and rename it to 'ffmpeg'\n" )
endif()


if(HAS_FFMPEG AND NOT USE_SYSTEM_FFMPEG)
  #--------------FFMPEG
  #LIBSWRESAMPLE
  add_library(swresample SHARED IMPORTED)
  find_library(SWRESAMPLE_LIBRARY_LOC NAME swresample PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET swresample PROPERTY IMPORTED_LOCATION ${SWRESAMPLE_LIBRARY_LOC})
  target_include_directories(swresample INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  # LIBAVCODEC
  add_library(avcodec SHARED IMPORTED)
  find_library(AVCODEC_LIBRARY_LOC NAME avcodec PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET avcodec PROPERTY IMPORTED_LOCATION ${AVCODEC_LIBRARY_LOC})
  target_include_directories(avcodec INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  target_link_libraries(avcodec INTERFACE swresample)
  #LIBAVFORMAT
  add_library(avformat SHARED IMPORTED)
  find_library(AVFORMAT_LIBRARY_LOC NAME avformat PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET avformat PROPERTY IMPORTED_LOCATION ${AVFORMAT_LIBRARY_LOC})
  target_include_directories(avformat INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  target_link_libraries(avformat INTERFACE swresample)
  #LIBAVUTIL
  add_library(avutil SHARED IMPORTED)
  find_library(AVUTIL_LIBRARY_LOC NAME avutil PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET avutil PROPERTY IMPORTED_LOCATION ${AVUTIL_LIBRARY_LOC})
  target_include_directories(avutil INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  target_link_libraries(avutil INTERFACE swresample)
  #LIBSWSCALE
  add_library(swscale SHARED IMPORTED)
  find_library(SWSCALE_LIBRARY_LOC NAME swscale PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET swscale PROPERTY IMPORTED_LOCATION ${SWSCALE_LIBRARY_LOC})
  target_include_directories(swscale INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  target_link_libraries(swscale INTERFACE swresample)
  #LIBAVFILTER
  add_library(avfilter SHARED IMPORTED)
  find_library(AVFILTER_LIBRARY_LOC NAME avfilter PATHS 
  "${CMAKE_SOURCE_DIR}/ffmpeg/lib" NO_DEFAULT_PATH)
  set_property(TARGET avfilter PROPERTY IMPORTED_LOCATION ${AVFILTER_LIBRARY_LOC})
  target_include_directories(avfilter INTERFACE ${CMAKE_SOURCE_DIR}/ffmpeg/include)
  target_link_libraries(avfilter INTERFACE swresample)

endif()

if(HAS_FFMPEG)
  add_library(video
  video.c
  )
  target_link_libraries(video PRIVATE avutil avformat avcodec swscale swresample avfilter)
  target_include_directories(video PRIVATE ${CMAKE_SOURCE_DIR})
endif()