#include"Transforms.h"
#include"Compose.h"
namespace jxtd{
	namespace misc{
		namespace cv{
			std::map<std::string,Transforms::tran_ptr> Transforms:: trans_pool ={};
		 	std::map<std::string,Transforms> Transforms:: transforms_pool = {};
			Transforms::Transforms(const std::string& name){
				this->name = name;
				Transforms::transforms_pool[name] = *this;
			}
			Transforms& Transforms:: operator()(jxtd::computation::infer::Tensor& tensor){
				buf = tensor;
				Transforms::transforms_pool[name] = *this;
				return *this;
			}
			Transforms& Transforms::add_compose(const Compose& compose){
				this->trans.insert(this->trans.end(),compose.trans.begin(),compose.trans.end());
				Transforms::transforms_pool[name] = *this;
				return *this;
			}
				
			Transforms& Transforms::add_transforms(const Transforms& transforms){
				this->trans.insert(this->trans.end(),transforms.trans.begin(),transforms.trans.end());
				Transforms::transforms_pool[name] = *this;
				return *this;
			}
			Transforms& Transforms::start(const jxtd::computation::infer::Tensor& tensor){
				jxtd::computation::infer::Tensor tmp(tensor);
				for(int i=0;i<trans.size();i++){
					tmp = trans[i](tmp);
				}
				buf = tmp;
				Transforms::transforms_pool[this->name] = *this;
				return *this;
			}
			jxtd::computation::infer::Tensor Transforms::get_output() const{
				return buf;
			}

			std::string Transforms:: get_name() const{
				return name;
			}
			std::vector<std::string> Transforms:: get_all_transforms(){
				std::vector<std::string> all_trans(Transforms::transforms_pool.size());
				auto it = Transforms::transforms_pool.begin();
				int i=0;
				for(it=Transforms::transforms_pool.begin();it!=Transforms::transforms_pool.end();it++){
					all_trans[i++] = it->first;
				}
				return all_trans;
			}
			
		}
	}
}
