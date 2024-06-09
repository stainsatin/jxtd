#pragma once
#include<string>
#include<vector>
#include<map>
#include<functional>
#include"computation/infer/Tensor.h"
namespace jxtd{
	namespace misc{
		namespace cv{
		  	class Compose;
			class Transforms{
			public:
				using tran_ptr = jxtd::computation::infer::Tensor (*) (const jxtd::computation::infer::Tensor&)noexcept;
				Transforms()=default;
				Transforms(const std::string& name);
				Transforms& operator()(jxtd::computation::infer::Tensor& tensor);
				template<typename do_transform_t>
				typename std::enable_if<std::is_nothrow_invocable_r<jxtd::computation::infer::Tensor, do_transform_t, const jxtd::computation::infer::Tensor&>::value, bool>::type 
				add_trans(std::string name,do_transform_t t){
					Transforms::trans_pool[name] = t;
					trans.push_back(t);
					return 1;
				}
				Transforms& add_compose(const Compose& compose);
				Transforms& add_transforms(const Transforms& transforms);
				Transforms& start(const jxtd::computation::infer::Tensor& image);
				jxtd::computation::infer::Tensor get_output() const;
				std::string get_name() const;
				static std::map<std::string,tran_ptr> trans_pool;//定义好的预处理操作
				static std::map<std::string,Transforms> transforms_pool;
				std::vector<tran_ptr> trans; 
				//其他API
				std::vector<std::string> get_all_transforms();
			private:
				jxtd::computation::infer::Tensor buf;
				std::string name;
				
			};
		}
	}
}

