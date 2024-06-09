#include"Compose.h"
namespace jxtd{
	namespace misc{
		namespace cv{
			Compose:: Compose(const std::vector<jxtd::misc::cv::Transforms>& transforms){
				for(int i=0;i<transforms.size();i++){
					this->trans.insert(this->trans.end(),transforms[i].trans.begin(),transforms[i].trans.end());
					trans_seq.push_back(transforms[i]);	
				}
			}
			Compose:: Compose(const std::vector<std::string>& transforms){
				for(int i=0;i<transforms.size();i++){
					trans_seq.push_back(Transforms::transforms_pool[transforms[i]]);
					this->trans.insert(this->trans.end(),Transforms::transforms_pool[transforms[i]].trans.begin(),Transforms::transforms_pool[transforms[i]].trans.end());
				}
			}
			Compose& Compose::push_back(const jxtd::misc::cv::Transforms& transforms){
				this->trans.insert(this->trans.end(),transforms.trans.begin(),transforms.trans.end());
				trans_seq.push_back(transforms);	
				return *this;
			}
			Compose& Compose::pop_back(){
				int size = trans_seq.size();
				int t_size = (trans_seq[size-1].trans).size();
				for(int i=0;i<t_size;i++){
					trans.pop_back();//这里是将顶层的transforms包含的所有func全部退出
				}
				trans_seq.pop_back();
				return *this;
			}
			int Compose::size() const{
				return trans_seq.size();
			}
		}
	}
}
