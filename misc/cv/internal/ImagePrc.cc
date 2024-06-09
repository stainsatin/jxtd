#include "ImagePrc.h"
#include<functional>
namespace jxtd {
	namespace misc {
		namespace cv {
			namespace internal {

				::cv::Mat* rectangle(::cv::Mat* src, int x, int y, int width, int height)noexcept
				{
					if (src->empty())
						return nullptr;
					if (x < 0 || y < 0 || height < 0 || width < 0)
						return nullptr;
					if (x + width > src->cols || y + height > src->rows)
						return nullptr;
                                                             
					::cv::Mat dst(height, width, src->type());

					::cv::Rect rect(x,y,width,height);
					dst = (*src)(rect);
					dst.copyTo(*src);

					if (!src->data) {
						return nullptr;
					}

					if (!src->isContinuous())
					  *src = src->clone();

					return src;

				}

				::cv::Mat* rotate(::cv::Mat* src, int angle)noexcept
				{
					if (src->empty())
						return nullptr;
					if (angle % 90 != 0)
						return nullptr;

					angle = angle%360;
                                        if(angle<0)
						angle=360+angle;

					::cv::Mat dst,tmp;
			                if(angle==90){
					     ::cv::transpose(*src,tmp);
					     ::cv::flip(tmp,dst,1);
					      dst.copyTo(*src);
					}
					else if(angle==180){
					     ::cv::flip(*src,dst,-1);
					     dst.copyTo(*src);
					}
					else if(angle==270){
                                             ::cv::transpose(*src,tmp);
                                             ::cv::flip(tmp,dst,0);
                                              dst.copyTo(*src);
					}
					else{
					     return src;
					}

					if (!src->data) {
						return nullptr;
					}

					if (!src->isContinuous())
						*src = src->clone();

					return src;

				}

				::cv::Mat* convert(::cv::Mat* src, ColorSpace color)noexcept
				{
					if (src->empty())
						return nullptr;
					if (src->channels() == 1)
					{
						switch (color) {
							case BGR:cvtColor(*src, *src, ::cv::COLOR_GRAY2BGR);break;
							case RGB:cvtColor(*src, *src, ::cv::COLOR_GRAY2RGB);break;
							default:
								return nullptr;
						}
					}
					else{
						switch (color) {
							case BGR:;break;
							case HSV: cvtColor(*src, *src, ::cv::COLOR_BGR2HSV);break;
							case RGB: cvtColor(*src, *src, ::cv::COLOR_BGR2RGB);break;
							case GRAY: cvtColor(*src, *src, ::cv::COLOR_BGR2GRAY);break;
							case YUV: cvtColor(*src, *src, ::cv::COLOR_BGR2YUV);break;
							default:return nullptr;
						}
					}


					if (!src->data) {
						return nullptr;
					}

					if (!src->isContinuous())
						*src = src->clone();

					return src;
				}


				::cv::Mat* extract(::cv::Mat* src, int channel)noexcept
				{
					if (src->empty())
						return nullptr;
					if (channel >= src->channels() || channel < 0)
						return nullptr;


					std::vector<::cv::Mat>mv;
					::cv::split(*src, mv);
					mv[channel].copyTo(*src);

					if (!src->data) {
						return nullptr;
					}

					if (!src->isContinuous())
						*src = src->clone();

					return src;

				}

				::cv::Mat* flatten(::cv::Mat* src) noexcept
				{
					if (src->empty())
						return nullptr;
				
					if(src->dims>4)
						return nullptr;
					
					int size = 1;
					for(int i = 0;i<src->dims;i++)
					{
						size*=src->size[i];
					}
					size*=src->channels();
					
					src->reshape(1,size);
					if (!src->isContinuous())
						*src = src->clone();

					return src;
				}

				::cv::Mat* zip(::cv::Mat* dst, const std::vector<::cv::Mat>& srcs)noexcept
				{
					if (dst == nullptr)
						return nullptr;
					int nums = srcs.size() + 1;
					std::vector<::cv::Mat> OutputImage(nums);
					if (nums == 0)
						return nullptr;
                                       int output_width,output_height;
                                    	output_width = dst->cols;
                                    	output_height = dst->rows;
					dst->copyTo(OutputImage[0]);
					for (int i = 1;i < nums;i++)
					{
						::cv::resize(srcs[i - 1], OutputImage[i], ::cv::Size(output_width, output_height));
					}

					::cv::merge(OutputImage, *dst);

					if (dst->isContinuous())
						*dst = dst->clone();

					return dst;

				}

				::cv::Mat* reshape(::cv::Mat* src, const std::vector<int>& shape, Fill f)noexcept
				{
					if (src->empty())
						return nullptr;

					int dataNums = shape[0] * shape[1];
					int srcNums = src->cols * src->rows;
					if (dataNums == srcNums)
						src->reshape(0, shape[0]);

					else {
						switch (f)
						{
						case linear:{
							::cv::Mat dst;
							::cv::resize(*src, dst, ::cv::Size(shape[0], shape[1]), ::cv::INTER_LINEAR);
							dst.copyTo(*src);
							dst.release();
							break;
						}

						case zero: {
							::cv::Mat dst(shape[0], shape[1], CV_16SC3, ::cv::Scalar(-1, -1, -1));
							float rx = shape[1] * 1.0 / src->cols;
							float ry = shape[0] * 1.0 / src->rows;
							for (int i = 0;i < src->rows;i++)
							{
								for (int j = 0;j < src->cols;j++)
								{
									int dst_x = (int)(rx * j);
									int dst_y = (int)(ry * i);
									::cv::Vec3b* p = dst.ptr<::cv::Vec3b>(dst_y, dst_x);
									::cv::Vec3b* src_p = src->ptr<::cv::Vec3b>(i, j);
									for (int k = 0;k < 3;k++)
									{
										if (p->val[k] == -1)
											p->val[k] = src_p->val[k];
										else
											p->val[k] = (p->val[k] + src_p->val[k]) / 2;
									}

								}
							}
							int size = dst.cols * dst.rows * dst.elemSize();
							for (int i = 0;i < size;i++)
							{
								if (dst.data[i] = -1)
									dst.data[i] = 0;
							}
							break;
						}

						case meanv: {
							::cv::Mat dst(shape[0], shape[1], CV_16SC3, ::cv::Scalar(-1, -1, -1));
							float rx = shape[1] * 1.0 / src->cols;
							float ry = shape[0] * 1.0 / src->rows;
							for (int i = 0;i < src->rows;i++)
							{
								for (int j = 0;j < src->cols;j++)
								{
									int dst_x = (int)(rx * j);
									int dst_y = (int)(ry * i);
									::cv::Vec3b* p = dst.ptr<::cv::Vec3b>(dst_y, dst_x);
									::cv::Vec3b* src_p = src->ptr<::cv::Vec3b>(i, j);
									for (int k = 0;k < 3;k++)
									{
										if (p->val[k] == -1)
											p->val[k] = src_p->val[k];
										else
											p->val[k] = (p->val[k] + src_p->val[k]) / 2;
									}

								}
							}
							::cv::Scalar mean;  //Mean
							::cv::Scalar stddev;  //Variance

							::cv::meanStdDev(*src, mean, stddev);
							int size = dst.cols * dst.rows * dst.elemSize();
							for (int i = 0;i < size;i += dst.elemSize())
							{
								if (dst.data[i] = -1)
									dst.data[i] = mean.val[0];
								if (dst.data[i + 1] = -1)
									dst.data[i + 1] = mean.val[1];
								if (dst.data[i + 2] = -1)
									dst.data[i + 2] = mean.val[2];
							}

							break;
						}
						default:return nullptr;
						}
					}
					if (!src->isContinuous())
						*src = src->clone();

					return src;
				}

				::cv::Mat* normalize(::cv::Mat* src, int axis, const std::vector<int> m0, const std::vector<int> v0)noexcept
				{
					if (src->empty())
						return nullptr;

					if (axis >= src->dims)
						return nullptr;

					std::vector<int>stp(src->dims);
					for (int i = 0;i < src->dims;i++)
						stp[i] = src->size[i];

					std::vector<int>flag(src->dims);
					for (int i = 0;i < src->dims;i++)
						flag[i] = 0;

					/*
					* The dimension is uncertain, so the number of layers of the loop is uncertain.
					Use a lambda expression wrapped in std:: function to recurse 
					(the lambda expression itself cannot  recurse as anonymous function)
					*/
					std::function<void(int)>f = [&](int dim) {
						if (dim == src->dims)
						{
							uchar* p = src->data;
							for (int k = 0;k < src->dims;k++)
								p += src->step[k] * flag[k];

							*p = (*p - m0[flag[axis]]) / v0[flag[axis]];
							return;
						}
						else {
							if (dim == axis) {
								f(dim + 1);
							}
							else {
								for (flag[dim] = 0;flag[dim] < src->size[dim];flag[dim]++)
								{
								   f(dim + 1);
								}
							}
						}
					};
                                                         
					for (flag[axis] = 0;flag[axis] < src->size[axis];flag[axis]++) {
						f(0);
					}
					if (!src->isContinuous())
                                                *src = src->clone();

                                        return src;
				}
			}
		}
	}
}
