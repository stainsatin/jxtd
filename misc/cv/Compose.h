#pragma once
#include"Transforms.h"
namespace jxtd{
	namespace misc{
		namespace cv{
			class Compose:public jxtd::misc::cv::Transforms{
				public:
				Compose()=default;
				Compose(const std::vector<jxtd::misc::cv::Transforms>& transforms);
				Compose(const std::vector<std::string>& transforms);
				Compose& push_back(const jxtd::misc::cv::Transforms& transforms);
				Compose& pop_back();
				int size() const;
				
				private:
				std::vector<jxtd::misc::cv::Transforms> trans_seq;
			};
		}
	}
}
