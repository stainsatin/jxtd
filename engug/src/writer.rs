use std::fs::File;
use std::io::{Write};
use std::convert::TryInto;
use bytes::{BytesMut, BufMut};
use crate::transform::{EdtSection, EdtValue};
use crate::ir::IntegerValue;
use crate::ir::FloatingValue;



pub struct EdtWriter {
    file: File,
    is_big_endian: bool,
}

impl EdtWriter {
    pub fn new(path: &str,is_big_endian:bool) -> Self {
        let file = File::create(path).expect("Failed to create edt file");
        EdtWriter { file, is_big_endian }
    }

    pub fn write(&mut self, sections: Vec<EdtSection>) {
        self.write_header(&sections);
        self.write_section_header(&sections);
        self.write_section(&sections);
    }

    fn write_header(&mut self, sections: &Vec<EdtSection>){
        let mut header = BytesMut::with_capacity(24);
        header.extend_from_slice(b"edtf");
        header.put_u8(0); // edt
        header.put_u8(0); // api version
        header.put_u8(if self.is_big_endian { 1 } else { 0 }); // 字节序
        
        let nums = sections.len() as u64; 
        let header_len:u64 = 1 + 38 * (nums+1);
        let head_size = 24;
        let extended_number: u64 = head_size as u64;
        // 根据字节序写入 header_len
        if self.is_big_endian {
            let extended_number_be = extended_number.to_be_bytes(); 
            header.extend_from_slice(&extended_number_be); 
            header.extend_from_slice(&header_len.to_be_bytes());
        } else {
            let extended_number_le = extended_number.to_le_bytes(); 
            header.extend_from_slice(&extended_number_le); 
            header.extend_from_slice(&header_len.to_le_bytes());
        }
        header.put_u8(0);
        self.file.write_all(&header).expect("Failed to write header");
    }

    fn write_section_header(&mut self, sections: &Vec<EdtSection>){
        let section_header_len:u64 = 1 + 38 * (sections.len() as u64 +1);
        //header+section_header的长度
        let mut total_offset:u64 = (24 + section_header_len).into();
        // 写入 section 的数量(sections+data)
        if self.is_big_endian {
            self.file.write_all(&((sections.len()+1) as u8).to_be_bytes()).expect("Failed to write header");;
        } else {
            
        }
        self.file.write_all(&((sections.len()+1) as u8).to_le_bytes()).expect("Failed to write header");;
        let mut data_len:u64 = 0;
        //写入entries
        sections.iter().for_each(|section| {
            //当前section相对于edt位置
            let tmp_offset:u64 = total_offset.into();
            let section_name = &section.name;
            let section_name_len:u64;
            let name_offset:u64;
            match section_name {
                EdtValue::Seq { len, offset } => {
                    section_name_len = *len;
                    name_offset = *offset;
                },
                _ => {
                    panic!("Unhandled section name variant");
                },
            }
            let mut entry_count:u32 = 0;
            let entries = &section.entries;
            let mut entry_len = 0;
            //在存储section key-value的时候,key为16B,value长度不一致但统一存储为64位
            for (_, values) in entries {
                for value in values{
                    entry_count += 1;
                    entry_len += 17;//key故定长度为type+len+offset=1+8+8=17
                    entry_len += 1;//type
                    match &value {
                        EdtValue::Integer { len , ..} => {
                            entry_len += 1;//len
                            let value_len = *len;
                            entry_len +=value_len;//value所占字节数
                        },
                        EdtValue::Floating { len, ..} => {
                            entry_len += 1;
                            let value_len = *len;
                            entry_len +=value_len;//value所占字节数
                        },
                        EdtValue::Seq { .. } => {
                            entry_len += 17;
                        },
                    }
                }
            }
            //更新偏移量，方便下一个section偏移量计算
            total_offset += entry_len as u64;
            let data = &section.data;
            //data整合为一个字符串
            let dataset = data.strings.join("");
            let name_len = section_name_len;
            let name = &dataset[name_offset as usize..(name_offset as usize + name_len as usize)];
            let section_type:u8 = match name {
                "meta" => 0,
                _ => 1,
            };
            //section中只记录了key value
            let section_size:u64 = entry_len.try_into().unwrap();
            
            data_len += dataset.len() as u64;
            let mut section_entries_name = BytesMut::with_capacity(17);
            // 首先验证字符串是否是有效的UTF-8编码
            if let Ok(name_str) = std::str::from_utf8(name.as_bytes()) {
                section_entries_name.extend_from_slice(name_str.as_bytes());
                let padding_byte = b' ';
                while section_entries_name.len() < 17 {
                    section_entries_name.put_u8(padding_byte); // 使用 put_u8 方法填充单个字节
                }
                self.file.write_all(&section_entries_name).expect("Failed to write section entry name");
            } 
            else {
                panic!("Invalid UTF-8 encoding in section name");
            }   
            //entries除了name外的其他元素
            let mut section_entries_body = BytesMut::with_capacity(21);
            section_entries_body.put_u8(section_type);
            if self.is_big_endian {
                let offset_be_bytes = tmp_offset.to_be_bytes(); 
                section_entries_body.extend_from_slice(&offset_be_bytes); 
                let size_be_bytes = section_size.to_be_bytes(); 
                section_entries_body.extend_from_slice(&size_be_bytes); 
                let count_be_bytes = entry_count.to_be_bytes(); 
                section_entries_body.extend_from_slice(&count_be_bytes); 
            } else {
                let offset_le_bytes = tmp_offset.to_le_bytes(); 
                section_entries_body.extend_from_slice(&offset_le_bytes); 
                let size_le_bytes = section_size.to_le_bytes(); 
                section_entries_body.extend_from_slice(&size_le_bytes); 
                let count_le_bytes = entry_count.to_le_bytes(); 
                section_entries_body.extend_from_slice(&count_le_bytes); 
            }
            // 写入 section_entries 到文件
            self.file.write_all(&section_entries_body).expect("Failed to write section entry body");
        });
        //data section_entry的记录
        let mut data_entry_name = BytesMut::with_capacity(17);
        data_entry_name.extend_from_slice("data".as_bytes()); 
        let padding_byte = b' ';
        while data_entry_name.len() < 17 {
            data_entry_name.put_u8(padding_byte); // 使用 put_u8 方法填充单个字节
        }
        self.file.write_all(&data_entry_name).expect("Failed to write data entry name");
        let mut data_entry_body = BytesMut::with_capacity(21);
        data_entry_body.put_u8(2 as u8);
        if self.is_big_endian {
            let offset_be_bytes = total_offset.to_be_bytes(); 
            data_entry_body.extend_from_slice(&offset_be_bytes); 
            let len_be_bytes = data_len.to_be_bytes(); 
            data_entry_body.extend_from_slice(&len_be_bytes); 
            data_entry_body.put_u32(0);
        } else {
            let offset_le_bytes = total_offset.to_le_bytes(); 
            data_entry_body.extend_from_slice(&offset_le_bytes); 
            let len_le_bytes = data_len.to_le_bytes(); 
            data_entry_body.extend_from_slice(&len_le_bytes); 
            data_entry_body.put_u32(0);
        }
        self.file
            .write_all(&data_entry_body)
            .expect("Failed to write section header");
    }
    
    fn write_section(&mut self, sections: &Vec<EdtSection>){
        let mut all_datas = Vec::new();
        //记录data区偏移
        let mut pre_offset = 0;
        sections.iter().for_each(|section| {
            all_datas.push(&section.data);
            let data = &section.data;
            let dataset = data.strings.join("");
            let section_name = &section.name;
            let name_len:u64;
            let name_offset:u64;
            match section_name {
                EdtValue::Seq { len, offset } => {
                    name_len = *len;
                    name_offset = *offset;
                },
                _ => {
                    panic!("Unhandled section name variant");
                },
            }
                section.entries.iter().for_each(|(key, values)|{
                    let key_type:u8 = 2;
                    let key_len:u64;
                    let key_offset:u64;
                    match key {
                        EdtValue::Seq { len, offset } => {
                            key_len = *len;
                            key_offset = *offset;
                        },
                        _ => {
                            panic!("Unhandled key name variant");
                        },
                    }
                    let key_offset_real = key_offset + pre_offset;
                    for value in values{
                        self.file.write_all(&(key_type as u8).to_be_bytes()).unwrap();
                        if self.is_big_endian{
                            self.file.write_all(&(key_len as u64).to_be_bytes()).unwrap();
                            self.file.write_all(&(key_offset_real as u64).to_be_bytes()).unwrap();
                        }
                        else{
                            self.file.write_all(&(key_len as u64).to_le_bytes()).unwrap();
                            self.file.write_all(&(key_offset_real as u64).to_le_bytes()).unwrap();
                        }
                        match &value {
                            EdtValue::Integer { len, value } => {
                                let value_type :u8 = 0;
                                self.file.write_all(&(value_type as u8).to_be_bytes()).unwrap();
                                let value_len = *len;
                                self.file.write_all(&(value_len as u8).to_be_bytes()).unwrap();
                                let mut value_offset = match value {
                                    IntegerValue::Byte(val) => *val as u64,
                                    IntegerValue::Word(val) => *val as u64,
                                    IntegerValue::Double(val) => *val as u64,
                                    IntegerValue::Quad(val) => *val as u64,
                                };
                                if value_len==1{
                                    self.file.write_all(&(value_offset).to_be_bytes()).unwrap();
                                }
                                else if value_len==2{
                                    if self.is_big_endian{
                                        self.file.write_all(&(value_offset as u16).to_be_bytes()).unwrap();
                                    }
                                    else{
                                        self.file.write_all(&(value_offset as u16).to_le_bytes()).unwrap();
                                    }
                                }
                                else if value_len==4{
                                    if self.is_big_endian{
                                        self.file.write_all(&(value_offset as u32).to_be_bytes()).unwrap();
                                    }
                                    else{
                                        self.file.write_all(&(value_offset as u32).to_le_bytes()).unwrap();
                                    }
                                }
                                else{
                                    if self.is_big_endian{
                                        self.file.write_all(&(value_offset as u64).to_be_bytes()).unwrap();
                                    }
                                    else{
                                        self.file.write_all(&(value_offset as u64).to_le_bytes()).unwrap();
                                    }
                                }
                            },
                            EdtValue::Floating { len,value } => {
                                let value_type :u8 = 1;
                                self.file.write_all(&(value_type as u8).to_be_bytes()).unwrap();
                                let value_len = *len;
                                self.file.write_all(&(value_len as u8).to_be_bytes()).unwrap();
                                let value_offset = match value {
                                    FloatingValue::Single(val) => *val as f64,
                                    FloatingValue::Double(val) => *val as f64,
                                };
                                if value_len==4{
                                    if self.is_big_endian{
                                        self.file.write_all(&(value_offset as f32).to_be_bytes()).unwrap();
                                    }
                                    else{
                                        self.file.write_all(&(value_offset as f32).to_le_bytes()).unwrap();
                                    }
                                }
                                else{
                                    if self.is_big_endian{
                                        self.file.write_all(&(value_offset).to_be_bytes()).unwrap();
                                    }
                                    else{
                                        self.file.write_all(&(value_offset).to_le_bytes()).unwrap();
                                    }
                                }
                            },
                            EdtValue::Seq { len,offset } => {
                                let value_type :u8 = 2;
                                self.file.write_all(&(value_type as u8).to_be_bytes()).unwrap();
                                let value_len = *len;
                                let value_offset = offset+pre_offset;
                                if self.is_big_endian{
                                    self.file.write_all(&(value_len as u64).to_be_bytes()).unwrap();                           
                                    self.file.write_all(&(value_offset as u64).to_be_bytes()).unwrap();
                                }
                                else{
                                    self.file.write_all(&(value_len as u64).to_le_bytes()).unwrap();        
                                    self.file.write_all(&(value_offset as u64).to_le_bytes()).unwrap();    
                                }
                            },
                        };
                    }
                });    
            for string_data in &data.strings{
                pre_offset += string_data.len() as u64;
            }
        });

        let mut combined_string = String::new();
        all_datas.iter().for_each(|datas| {
            datas.strings.iter().for_each(|string_data| {
                combined_string.push_str(string_data);
            });
        });
        let mut combined_bytes: Vec<u8> = Vec::new();
        all_datas.iter().for_each(|datas| {
            datas.strings.iter().for_each(|string_data| {
                combined_bytes.extend_from_slice(string_data.as_bytes());
            });
        });
        self.file.write_all(&combined_bytes)
        .expect("Failed to write data to file");
    }
}

#[cfg(test)]
mod test{
    use super::*;
    use crate::ir::*;
    use crate::parser::*;

    #[test]
    fn test_writer(){
        let mut section1 = Section::new(String::from("meta"));
        section1.add_entry("name", String::from("demo"));
        section1.add_entry("mid", String::from("123"));
        let section1 = EdtSection::from(section1);

        let mut section2 = Section::new(String::from("buffers"));
        section2.add_entry("inputs", 1u32);
        section2.add_entry("outputs", 1u32);
        let section2 = EdtSection::from(section2);

        let mut sections = Vec::new();
        sections.push(section1);
        sections.push(section2);
        let path = "src/test_writer.edt";
        let mut writer = EdtWriter::new(path,true);
        writer.write(sections);

        let parser = EdtParser::new(path);
        parser.parse().unwrap();
    }
}
