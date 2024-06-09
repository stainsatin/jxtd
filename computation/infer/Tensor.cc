#include "Tensor.h"
#include <fstream>
#include <sstream>
#include <iostream>
namespace jxtd{
	namespace computation{
		namespace infer{
			Tensor::Tensor()=default;
			Tensor::Tensor(const Tensor& tensor):Tensor(){
				this->src = tensor.src;
			}
			Tensor& Tensor::operator=(const Tensor& tensor){
				this->src = tensor.src;
				return *this;
			}
			Tensor::Tensor(Tensor&& tensor) noexcept{
				this->src = tensor.src;
				tensor.src.clear();
			}
			Tensor& Tensor:: operator=(Tensor&& tensor){
				this->src = tensor.src;
				tensor.src.clear();
				return *this;
			}
			Tensor::Tensor(const void* buf, size_t size){
				src = "";
				for(int i=0;i<size;i++){
					src += ((char*)buf)[i];
				}
			}
			Tensor::Tensor(const std::vector<unsigned char>& data){
				src = "";
				for(int i=0;i<data.size();i++){
					src += data[i];
				}
			}      
			Tensor::Tensor(const std::string& path){
				this->set_data(path);
			}
			void Tensor::set_data(const std::string& path){
				std::ifstream ifile(path);
				std::ostringstream buf;
				char ch;
				while(buf&&ifile.get(ch))
					buf.put(ch);
				this->src = buf.str();
				this->path = path;
			}
			void Tensor::set_data(const void* buf, size_t size){
				if(buf==nullptr)
					src = "";
				else{
					src = "";
					for(int i=0;i<size;i++){
						src += ((char*)buf)[i];
					}
				}
			}
			void Tensor::set_data(const std::vector<unsigned char>& data){
				src = "";
				for(int i=0;i<data.size();i++){
					src += data[i];
				}
			}
			const void* Tensor::get_data_nocopy() const{
				return src.c_str();
			}
			 void* Tensor::get_internal_type() const{
				return static_cast<void*>(const_cast<char*>(src.c_str()));
			}
			
			std::string Tensor::get_backend() const{
				return "string";			
			}
			int Tensor::get_size() const{
				return src.size();
			}          
			std::string Tensor::get_path() const{
				return this->path;
			}
		}
	}
}
