#pragma once
#include<vector>
#include <opencv2/opencv.hpp>

namespace jxtd {
	namespace misc {
		namespace cv{
	    	   namespace internal{
				enum  ColorSpace {
				  	BGR,
					RGB,
			 		HSV,
					GRAY,
					YUV
				};

				enum Fill {
					linear,
					zero,
					meanv
				};
				::cv::Mat* rectangle(::cv::Mat* src, int x, int y, int width, int height)noexcept;
				::cv::Mat* rotate(::cv::Mat* src, int angle)noexcept;
				::cv::Mat* convert(::cv::Mat* src, ColorSpace color)noexcept;
				::cv::Mat* extract(::cv::Mat* src, int channel)noexcept;
				::cv::Mat* flatten(::cv::Mat* src)noexcept;
				::cv::Mat* zip(::cv::Mat* dst, const std::vector<::cv::Mat>& srcs)noexcept;
				::cv::Mat* reshape(::cv::Mat* src, const std::vector<int>& shape, Fill f)noexcept;
				::cv::Mat* normalize(::cv::Mat* src, int axis, const std::vector<int> m0, const std::vector<int> v0)noexcept;
			}
		}
	}
}
