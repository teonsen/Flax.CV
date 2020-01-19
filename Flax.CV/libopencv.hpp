#ifndef _LIBOPENCV_HPP_
#define _LIBOPENCV_HPP_

//#ifndef __OPENCV_ALL_HPP__
//#error Not Included "opencv.hpp"
//#endif
//#ifndef __OPENCV_VERSION_HPP__
//#error Not Included "version.hpp"
//#endif

#define STR_EXP(__A) #__A
#define STR(__A) STR_EXP(__A)

#define SUFFIX_LIB ".lib"
#define PREFIX_OPENCV "opencv_"

#define LIB_NAME_RELEASE(__A)  STR(PREFIX_OPENCV) STR(__A) STR(CV_VERSION_MAJOR) STR(CV_VERSION_MINOR) STR(CV_VERSION_REVISION) STR(SUFFIX_LIB)
#define LIB_NAME_DEBUG(__A)    STR(PREFIX_OPENCV) STR(__A) STR(CV_VERSION_MAJOR) STR(CV_VERSION_MINOR) STR(CV_VERSION_REVISION) "d" STR(SUFFIX_LIB)

#define LIB_NAME(__A)          STR(__A) STR(SUFFIX_LIB)

#define LIB_NAME_3RD_PARTY_RELEASE(__A)    STR(__A) STR(SUFFIX_LIB)
#define LIB_NAME_3RD_PARTY_DEBUG(__A)      STR(__A) "d" STR(SUFFIX_LIB)

#define CV_LIB_CALIB3D     "calib3d"
#define CV_LIB_CORE        "core"
#define CV_LIB_FEATURES2D  "features2d"
#define CV_LIB_FLANN       "flann"
#define CV_LIB_HIGHGUI     "highgui"
#define CV_LIB_IMGCODECS   "imgcodecs"
#define CV_LIB_IMGPROC     "imgproc"
#define CV_LIB_ML          "ml"
#define CV_LIB_OBJDETECT   "objdetect"
#define CV_LIB_PHOTO       "photo"
#define CV_LIB_SHAPE       "shape"
#define CV_LIB_STITCHAING  "stitching"
#define CV_LIB_SUPERRES    "superres"
#define CV_LIB_TS          "ts"
#define CV_LIB_VIDEO       "video"
#define CV_LIB_VIDEOIO     "videoio"
#define CV_LIB_VIDEOSTAB   "videostab"
#define CV_LIB_WORLD       "world"
#define CV_LIB_IPPICVMT    "ippicvmt"

// 3rd party libraries
#define CV_LIB_ILMIMF      "IlmImf"
#define CV_LIB_IPPIW       "ippiw"
#define CV_LIB_ITTNOTIFY   "ittnotify"
#define CV_LIB_JASPER      "libjasper"
#define CV_LIB_JPEG_TURBO  "libjpeg-turbo"
#define CV_LIB_PNG         "libpng"
#define CV_LIB_PROTOBUF    "libprotobuf"
#define CV_LIB_TIFF        "libtiff"
#define CV_LIB_WEBP        "libwebp"
#define CV_LIB_ZIP         "zlib"

#if _DEBUG
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_TS))
#ifdef _OPENCV_BUILD_STATIC_
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_CALIB3D))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_CORE))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_FEATURES2D))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_FLANN))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_HIGHGUI))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_IMGCODECS))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_IMGPROC))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_ML))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_OBJDETECT))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_PHOTO))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_SHAPE))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_STITCHAING))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_SUPERRES))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_VIDEO))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_VIDEOIO))
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_VIDEOSTAB))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_ILMIMF))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_IPPIW))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_ITTNOTIFY))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_JASPER))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_JPEG_TURBO))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_PNG))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_PROTOBUF))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_TIFF))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_WEBP))
#pragma comment(lib, LIB_NAME_3RD_PARTY_DEBUG(CV_LIB_ZIP))
#pragma comment(lib, LIB_NAME(CV_LIB_IPPICVMT))
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"vfw32.lib")    // Video for Windows
#else
#pragma comment(lib, LIB_NAME_DEBUG(CV_LIB_WORLD))
#endif
#else
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_TS))
#ifdef _OPENCV_BUILD_STATIC_
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_CALIB3D))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_CORE))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_FEATURES2D))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_FLANN))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_HIGHGUI))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_IMGCODECS))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_IMGPROC))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_ML))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_OBJDETECT))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_PHOTO))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_SHAPE))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_STITCHAING))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_SUPERRES))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_VIDEO))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_VIDEOIO))
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_VIDEOSTAB))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_ILMIMF))
//#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_IPPIW))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_ITTNOTIFY))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_JASPER))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_JPEG_TURBO))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_PNG))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_PROTOBUF))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_TIFF))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_WEBP))
#pragma comment(lib, LIB_NAME_3RD_PARTY_RELEASE(CV_LIB_ZIP))
//#pragma comment(lib, LIB_NAME(CV_LIB_IPPICVMT))
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"vfw32.lib")    // Video for Windows
#else
#pragma comment(lib, LIB_NAME_RELEASE(CV_LIB_WORLD))
#endif
#endif  // ifdef _DEBUG

#endif  // ifndef _LIBOPENCV_HPP_