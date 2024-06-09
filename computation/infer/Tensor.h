#pragma once
#include<memory>
#include<vector>
namespace jxtd{
	namespace computation{
		namespace infer{
			class Tensor{
				public:
					constexpr static int TYPE_BIN=0;
					constexpr static int TYPE_UINT8=1;
					constexpr static int TYPE_UINT16=2;
					constexpr static int TYPE_UINT32=3;
					constexpr static int TYPE_FLOAT=4;
					
					Tensor();
					Tensor(const Tensor& tensor);
					Tensor(Tensor&& tensor) noexcept;
					Tensor& operator=(const Tensor& tensor);
					Tensor& operator=(Tensor&& tensor);
					Tensor(const void* buf, size_t size);
					Tensor(const std::vector<unsigned char>& data);
					Tensor(const std::string& path);
					void set_data(const std::string& path);		
					void set_data(const void* buf, size_t size);
					void set_data(const std::vector<unsigned char>& data);
					const void* get_data_nocopy() const;
					int get_size() const;
					virtual void* get_internal_type() const;
					std::string get_backend() const;		   
					std::string get_path() const;                            
					std::vector<char> get_data() const{
						std::vector<char> data;
						unsigned char* buf = (unsigned char*)(this->get_internal_type());
						if(buf != nullptr){
							int size = src.size();
							for(int i = 0;i<size;i++){
								data.insert(data.end(),static_cast<char>(buf[i]));
							}
							return data;
						}
						else{
							data.resize(0);
							return data;
						}
					}
				
				private:
					std::string src;   
					std::string path;
			};
		}
	}
}
