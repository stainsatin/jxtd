use std::error::Error;
use std::fs;
use nom::bytes::complete::{take, tag, take_until};
use nom::bytes::complete::is_not;
use nom::{
    IResult,
    character::complete::{space0},
    error::{ErrorKind},
    multi::many0,
    sequence::{terminated},
    AsBytes
};
use bytes::{Buf, Bytes};

pub struct EdcParser {
    path: String,
}

impl EdcParser {
    pub fn new(path: &str) -> Self {
        EdcParser {
            path: path.to_string(),
        }
    }

    pub fn parse(&self) -> Result<Vec<crate::ir::Section>, Box<dyn Error>> {
        let file_content = fs::read_to_string(&self.path)?;
        
        // 删除文件内容中的所有空白
        let trimmed_content = file_content.trim().replace(|c: char| c.is_whitespace(), "");
        // 将处理过的字符串转换为字节数组，以便使用parse_section进行解析
        let bytes = trimmed_content.as_bytes();
        // 使用parse_section循环解析section
        let _result = match many0(parse_section)(bytes) {
            Ok(_result) => {
                return Ok(_result.1);
            }
            Err(_e) => {
                return Err("parse error".into());
            }
        };
    }
    
}

fn parse_section(input: &[u8]) -> IResult<&[u8], crate::ir::Section> {
    let (input, section_name) = parse_section_name(input)?;
    let mut section = crate::ir::Section::new(section_name.into());
    
    let mut keys = Vec::new();
    let (input, properties) = many0(parse_property)(input)?;
    for (key,value) in properties{
        if key=="outputs"||key=="inputs" {
            if value.chars().all(|c| c.is_digit(10)){
                let number: u32 = value.parse().unwrap();
                section.add_entry(key.as_str(),number);
                keys.push(key.clone());
                continue;
            }
            else{
                 return Err(nom::Err::Error(nom::error::Error::new(input, ErrorKind::Tag)));
            }
        }
        if key=="lazy"||key=="persistent"||key=="lazypar"||key=="persistentpar"{
            if value=="true"{
                let f:u8 = 1;
                section.add_entry(key.as_str(),f);
                keys.push(key.clone());
                continue;
            }
            else if value=="false"{
                let f:u8 = 0;
                section.add_entry(key.as_str(),f);
                keys.push(key.clone());
                continue;
            }
            else{
                return Err(nom::Err::Error(nom::error::Error::new(input, ErrorKind::Tag)));
            }
        }
        keys.push(key.clone());
        section.add_entry(key.as_str(),value);
    }
    let flag = check(&mut section,keys);
    if !flag{
        return Err(nom::Err::Error(nom::error::Error::new(input, ErrorKind::Tag)));
    }

    Ok((input,section))
}

fn parse_section_name(input: &[u8]) -> IResult<&[u8], &str> {
    let (input, _) = space0(input)?;
    let (input, _) = tag("[")(input)?;
    let (input, section_name) = is_not("]\n")(input)?;
    let (input, _) = tag("]")(input)?;

    let section_name_str = std::str::from_utf8(section_name).unwrap();

    Ok((input, section_name_str))
}

fn parse_property(input: &[u8]) -> IResult<&[u8], (String, String)> {
    let (input, _) = space0(input)?;
    // 检查是否是下个section开头
    if input.first() == Some(&b'[') {
        return Err(nom::Err::Error(nom::error::Error::new(input, nom::error::ErrorKind::Tag)));
    }
    let (input, key) = take_until("=")(input)?;
    let (input, _) = tag("=")(input)?;
    let (input, value) = terminated(is_not(";"), tag(";"))(input)?;
    
    let key_str = std::str::from_utf8(key).unwrap().trim().to_string();
    let value_str = std::str::from_utf8(value).unwrap().trim().to_string();
    Ok((input, (key_str, value_str)))
}

fn check(section:&mut crate::ir::Section,keys:Vec<String>)->bool{
    let section_name = section.name();
    match section_name{
        "meta" => {
            let mut contain_keys = Vec::new();
            contain_keys.push(String::from("name"));
            contain_keys.push(String::from("description"));
            contain_keys.push(String::from("mid"));
            for i in &contain_keys {
                if !keys.contains(&i) {
                    if i=="name"{
                        section.add_entry("name","");

                    }
                    else if i=="description"{
                        section.add_entry("description","");
                    }
                    else{ 
                        return false;
                    }
                }
            }
            for i in contain_keys{
                let values: Vec<Option<String>> =section.get_entry(i.as_str()).expect("missing necessary property in section meta");
                if values.len() != 1{
                    return false;
                }
            }
        }
        "buffers" => {
            if !keys.contains(&String::from("outputs"))||!keys.contains(&String::from("inputs")){
                return false;
            }
            let num1: Vec<Option<u32>> = section.get_entry("inputs").expect("missing property(inputs) in section buffers");
            if num1.len() != 1{
                return false;
            }
            let inputs = num1[0].unwrap();
            let num2: Vec<Option<u32>> = section.get_entry("outputs").expect("missing property(outputs) in section buffers");
            if num2.len() != 1{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
                return false;
            }
            let outputs = num2[0].unwrap();
            let values: Vec<Option<String>> = match section.get_entry("extra") {
                Some(values) => values,
                None => {
                    vec![]
                }
            };
            let mut names: Vec<String> = values.into_iter().filter_map(|opt| opt).collect();
            names.extend((0..inputs).map(|i| format!("input{}", i)));
            names.extend((0..outputs).map(|i| format!("output{}", i)));
            let mut some_keys = Vec::new();
            some_keys.push("unspecific");
            some_keys.push("pilotage");
            some_keys.push("lane");
            for some_key in some_keys{
                if keys.contains(&some_key.to_string()){
                    let res1:Vec<Option<String>> = section.get_entry(some_key).expect("the key in section buffers:value in buffers should be present");
                    let res2: Vec<String> = res1.into_iter().filter_map(|opt| opt).collect();
                    for res in res2{
                        if !names.contains(&res){
                            return false;
                        }
                    }
                }
            }
            for name in names{
                let bindkey = name.clone() + "bind";
                let bindvalue: Vec<Option<String>> =section.get_entry(bindkey.as_str()).expect("the key <name>bind in section buffers:value in buffers should be present"); 
                if bindvalue.len() != 1{
                    return false;
                }
                let bindvalues = ["free","imm","remote","eng"];
                let binddata = bindvalue[0].clone().unwrap(); 
                if !bindvalues.contains(&binddata.as_str()){
                    return false;
                }
                if binddata=="imm"{
                    let immkey = name.clone() + "immtype";
                    let immtype_res: Vec<Option<String>> =section.get_entry(immkey.as_str()).expect("the key <name>immtype in section buffers:value in buffers should be present"); 
                    if immtype_res.len() != 1{
                        println!("{}",immtype_res.len());
                        return false;
                    }
                    let immtypes = ["empty","imm","file","url","id"];
                    let immtype = immtype_res[0].clone().unwrap();
                    if !immtypes.contains(&immtype.as_str()){
                        return false;
                    }
                    let immkey2 = name.clone() + "immvalue";
                    let immvalue: Vec<Option<String>> =section.get_entry(immkey2.as_str()).expect("the key <name>immvalue in section buffers:value in buffers should be present"); 
                    if immvalue.len() != 1{
                        return false;
                    }
                }
                else if binddata=="remote"{
                    let remotekey = name.clone() + "remotetype";
                    let remoyetype_res: Vec<Option<String>> =section.get_entry(remotekey.as_str()).expect("the key <name>remotetype in section buffers:value in buffers should be present"); 
                    if remoyetype_res.len() != 1{
                        return false;
                    }
                    let remotetypes = ["file","url","id"];
                    let remotetype = remoyetype_res[0].clone().unwrap();
                    if !remotetypes.contains(&remotetype.as_str()){
                        return false;
                    }
                    let remotekey2 = name.clone() + "remotevalue";
                    let remotevalue: Vec<Option<String>> =section.get_entry(remotekey2.as_str()).expect("the key <name>remotevalue in section buffers:value in buffers should be present"); 
                    if remotevalue.len() != 1{
                        return false;
                    }
                }
                else if binddata=="eng"{
                    let engkey1 = name.clone() + "engid";
                    let engkey2 = name.clone() + "engbuf";
                    let engres1: Vec<Option<String>> =section.get_entry(engkey1.as_str()).expect("the key <name>engid in section buffers:value in buffers should be present"); 
                    let engres2: Vec<Option<String>> =section.get_entry(engkey2.as_str()).expect("the key <name>engbuf in section buffers:value in buffers should be present"); 
                    if engres1.len() != 1 || engres2.len() != 1{
                        return false;
                    }
                }
            }
        }
        "runtime" => {
            let deps:Vec<Option<String>> = section.get_entry("deps").expect("the key deps in section runtime:value should be present");
            let names: Vec<String> = deps.into_iter().filter_map(|opt| opt).collect();
            for name in names{
                let typekey = name.clone() + "type";
                let types = ["file","url","id"];
                let res1: Vec<Option<String>> =section.get_entry(typekey.as_str()).expect("the key <name>type in section runtime:value should be present"); 
                if res1.len() != 1{
                    return false;
                }
                let runtype = res1[0].clone().unwrap();
                if !types.contains(&runtype.as_str()){
                    return false;
                }
                let key1 = name.clone() + "path";
                let key2 = name.clone() + "relative";
                let res2: Vec<Option<String>> =section.get_entry(key1.as_str()).expect("the key <name>path in section runtime:value should be present"); 
                let res3: Vec<Option<String>> =section.get_entry(key2.as_str()).expect("the key <name>relative in section runtime:value should be present"); 
                if res2.len() != 1||res3.len() != 1{
                    return false;
                }
            }
        }
        "context" => {
            let parent:Vec<Option<String>> = section.get_entry("parent").expect("the key parent in section context:value should be present");
            if parent.len()!=1{
                return false;
            }
            if keys.contains(&String::from("lazypar")){
                let res:Vec<Option<u8>> = section.get_entry("lazypar").expect("the key lazypar in section context:value should be present");
                if res.len()!=1{
                    return false;
                }
            }
            if keys.contains(&String::from("persistentpar")){
                let res:Vec<Option<u8>> = section.get_entry("persistent").expect("the key persistentpar in section context:value should be present");
                if res.len()!=1{
                    return false;
                }
            }
        }
        "routine" => {
            if keys.len()!=1{
                return false;
            }
            let res:Vec<Option<String>> = section.get_entry("proc").expect("the key proc in section routine:value should be present");
            if res.len() != 1{
                return false;
            }
        }
        "attribute" => {
            for key in keys{
                if key=="lazy"||key=="persistent"{
                    let res: Vec<Option<String>> =section.get_entry(key.as_str()).expect("the key in section attribute:value should be present"); 
                    if res.len()!=1{
                        return false;
                    }
                }
                else{
                    return false;
                }
            }
        }
        _ =>{
            return false;
        }
    }
    return true;
}

//下面是EdtParser的内容
pub struct EdtParser{
	path: String,
}

//Header中后续可能会用到的信息
#[derive(PartialEq,Debug,Eq)]
struct EdtHeader{
    file_type:Bytes,
    api_version:Bytes,
    endian:Bytes,
    offset:Bytes,
    size:Bytes
}

#[derive(PartialEq,Debug,Eq)]
struct EdtSectionEntry{
    name:Bytes,
    section_type:Bytes,
    offset:Bytes,
    size:Bytes,
    entry_count:Bytes
}

#[derive(PartialEq,Debug,Eq)]
struct EdtSectionHeader{
    count:Bytes,
    entry:Vec<EdtSectionEntry>
}

#[derive(PartialEq,Debug,Eq)]
struct EdtData{
    data_type:Bytes,
    length:Bytes,
    value:Bytes
}

#[derive(PartialEq,Debug,Eq)]
struct EdtEntry{
    key:String,
    value:EdtData
}

impl EdtParser{

    //解析header部分
    fn header_parser(header: &[u8]) -> IResult<&[u8] ,EdtHeader>{
        let (header ,_) = tag(b"edtf")(header)?;
        let (header ,file_type) = take(1u8)(header)?;
        if file_type == b"\x00" {
            //白名单 不做处理
        } else {
            return Err(nom::Err::Failure(nom::error::Error::new(header ,nom::error::ErrorKind::Not)));
        }
        let (header ,api_version) = take(1u8)(header)?;
        //对api_version进行检查，此时api_version仅为0x0
        //抛出Not表示没有满足条件
        if api_version == b"\x00" {
            //白名单 不做处理
        } else {
            return Err(nom::Err::Failure(nom::error::Error::new(header ,nom::error::ErrorKind::Not)));
        }
        let (header ,endian) = take(1u8)(header)?;
        //下面对endian进行检查，此时endian为0x0 = little ,0x1 = big两种可能
        if (endian == b"\x00") | (endian == b"\x01") {
            //白名单 不做处理
        } else {
            return Err(nom::Err::Failure(nom::error::Error::new(header ,nom::error::ErrorKind::Not)));
        }
        let (header ,offset) = take(8u8)(header)?;
        let (header,size) = take(8u8)(header)?;
        let (header ,_) = tag(&b"\x00"[..])(header)?;
        Ok((header ,EdtHeader {file_type:Bytes::copy_from_slice(file_type),
                            api_version:Bytes::copy_from_slice(api_version),
                            endian:Bytes::copy_from_slice(endian),
                            offset:Bytes::copy_from_slice(offset),
                            size:Bytes::copy_from_slice(size)}))
    }

    fn section_entry_parser(section_entry:&[u8]) -> IResult<&[u8] ,EdtSectionEntry>{
        let (section_entry ,name) = take(17u8)(section_entry)?;
        let (section_entry ,section_type) = take(1u8)(section_entry)?;
        let (section_entry ,offset) = take(8u8)(section_entry)?;
        let (section_entry ,size) = take(8u8)(section_entry)?;
        let (section_entry ,entry_count) = take(4u8)(section_entry)?;
        Ok((section_entry ,EdtSectionEntry{name:Bytes::copy_from_slice(name),
                                        section_type:Bytes::copy_from_slice(section_type),
                                        offset:Bytes::copy_from_slice(offset),
                                        size:Bytes::copy_from_slice(size),
                                        entry_count:Bytes::copy_from_slice(entry_count)}))
    }

    fn section_header_parser(section_header:&[u8] ,is_big_endian:bool) -> IResult<&[u8] ,EdtSectionHeader>{
        let (section_header ,section_count) = take(8u8)(section_header)?;
        let mut mut_section_count = section_count;
        let count = if is_big_endian {mut_section_count.get_u64() as usize} else {mut_section_count.get_u64_le() as usize};
        let (section_header ,entry) = nom::multi::count(Self::section_entry_parser, count)(section_header).expect("Error occurred when try to parse section entry");
        Ok((section_header ,EdtSectionHeader {count:Bytes::copy_from_slice(section_count),
                                                  entry:entry}))
    }


    fn data_parser(data:&[u8]) -> IResult<&[u8] ,EdtData>{
        let (data ,data_type) = take(1u8)(data)?;
        match data_type {
            b"\x00" => {let (data ,length) = take(1u8)(data)?;
                        match length {
                             b"\x01" => {let(data ,value) = take(1u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             b"\x02" => {let(data ,value) = take(2u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             b"\x04" => {let(data ,value) = take(4u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             b"\x08" => {let(data ,value) = take(8u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             _ => return Err(nom::Err::Failure(nom::error::Error::new(data ,nom::error::ErrorKind::Not)))

                        };},
            b"\x01" => {let (data ,length) = take(1u8)(data)?;
                        match length {
                             b"\x04" => {let(data ,value) = take(4u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             b"\x08" => {let(data ,value) = take(8u8)(data)?; return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                                                             length: Bytes::copy_from_slice(length),
                                                                                             value: Bytes::copy_from_slice(value)}));},
                             _ => return Err(nom::Err::Failure(nom::error::Error::new(data ,nom::error::ErrorKind::Not)))
                        };},
            b"\x02" => {let (data ,length) = take(8u8)(data)?;
                        let(data ,value) = take(8u8)(data)?; 
                        return Ok((data ,EdtData{data_type: Bytes::copy_from_slice(data_type),
                                                 length: Bytes::copy_from_slice(length),
                                                 value: Bytes::copy_from_slice(value)}));
                        },
                  _ => return Err(nom::Err::Failure(nom::error::Error::new(data ,nom::error::ErrorKind::Not)))
        }
    }

    fn section_parser<'a>(entry:&'a[u8] ,data_section:&'a[u8] ,is_big_endian:bool) -> IResult<&'a[u8] ,EdtEntry>{
        let (entry ,key) = Self::data_parser(entry).expect("Error occurred when try to parse the key of the section");
        let name;
        //下面检查第一个数据是否为entry的key中要求的String类型
        match key.data_type.as_bytes() {
            b"\x02" => {let offset = if is_big_endian {key.value.as_bytes().get_u64() as usize} else {key.value.as_bytes().get_u64_le() as usize};
                        let length = if is_big_endian {key.length.as_bytes().get_u64() as usize} else {key.length.as_bytes().get_u64_le() as usize};
                        let begin = offset;
                        let end = begin + length;
                        let target = &data_section[begin..end];
                        name = std::str::from_utf8(target).unwrap().to_string();},
                _   => return Err(nom::Err::Failure(nom::error::Error::new(entry ,nom::error::ErrorKind::Tag)))
        };
        //下面就是解析value部分
        let (entry ,value) = Self::data_parser(entry).expect("Error occurred when try to parse the value of the section");
        Ok((entry ,EdtEntry{key: name,
                         value: value}))
    }

	//根据路径构建Parser
	pub fn new(path: &str) -> Self{
        EdtParser{
            path: path.into()
        }
    }


	//解析Edt文件
	pub fn parse(&self) -> Result<Vec<crate::ir::Section>, Box<dyn std::error::Error>>{
        //用于处理偏移量的定位函数
        fn offset_process<'a>(edtf:&'a[u8] ,offset:&'a Bytes ,is_big_endian:bool) -> &'a[u8]{
            let begin = if is_big_endian {offset.as_bytes().get_u64() as usize} else {offset.as_bytes().get_u64_le() as usize};
            &edtf[begin..]
        }

        //读文件指令
        let edtf_string = fs::read_to_string(&self.path)?;
        let edtf = edtf_string.as_bytes();

        //下面对文件头进行解析
        let (_ ,header_result) = Self::header_parser(edtf).expect("Error occurred when try to parse the header");
        //下面对endian的状态做个记录
        let is_big_endian = header_result.endian.as_bytes() == b"\x01";
        //通过header部分的解析结果得到section_header的位置
        let section_header = offset_process(edtf ,&header_result.offset ,is_big_endian);
        //下面解析section_header
        let (_ ,section_header_result) = Self::section_header_parser(section_header ,is_big_endian).expect("Error occurred when try to parse the section header");
        //下面在解析结果中找data区的描述
        let data_section_description = section_header_result.entry.iter().find(|&x| x.section_type.as_bytes() == b"\x02").expect("Error occurred when try to find data section");
        //找到data区的位置
        let data_section = offset_process(edtf ,&data_section_description.offset ,is_big_endian);
        //准备返回的容器
        let mut result: Vec<crate::ir::Section> = Vec::new();
        //遍历entry并填数据
        for i in &section_header_result.entry{
            //避免重复解析section_type为\x02即数据区的header
            if i.section_type.as_bytes() != b"\x02"{
                let name = std::str::from_utf8(i.name.as_bytes()).unwrap().to_string();
                let mut section = crate::ir::Section::new(name);
                //找到该header描述的entry的位置
                let entry_position = offset_process(edtf ,&i.offset ,is_big_endian);
                //依据entry数目逐次解析
                let mut mut_entry_count = i.entry_count.clone();
                let count = if is_big_endian {mut_entry_count.get_u32()} else {mut_entry_count.get_u32_le()};
                let mut times:u32 = 0x0;
                let mut entry_parsed = entry_position;
                while times < count{
                    //开始解析section,返回的entry中包含String类型的key和Bytes类型的value
                    let (parsed ,entry) = Self::section_parser(entry_parsed ,data_section ,is_big_endian).expect("Error occurred when try to parse the entry");
                    entry_parsed = parsed;
                    let length = entry.value.length.as_bytes();
                    let mut value = entry.value.value.as_bytes();
                    match entry.value.data_type.as_bytes() {
                        b"\x00" => match length {
                                            b"\x01" => section.add_entry(&entry.key ,value.get_u8()),
                                            b"\x02" => section.add_entry(&entry.key ,if is_big_endian {value.get_u16()} else {value.get_u16_le()}),
                                            b"\x04" => section.add_entry(&entry.key ,if is_big_endian {value.get_u32()} else {value.get_u32_le()}),
                                            b"\x08" => section.add_entry(&entry.key ,if is_big_endian {value.get_u64()} else {value.get_u64_le()}),
                                            //由于在解析时已经对数据进行检查，所以下面是unreachable
                                                  _ => unreachable!("Error occurred when try to check the length of the data")
                                         },
                        b"\x01" => match length {
                                            b"\x04" => section.add_entry(&entry.key ,if is_big_endian {value.get_f32()} else {value.get_f32_le()}),
                                            b"\x08" => section.add_entry(&entry.key ,value.get_f64()),
                                                  _ => unreachable!("Error occurred when try to check the length of the data")
                                         },
                        //下面对于String类型就需要对偏移量进行处理
                        b"\x02" =>  {let data_position = offset_process(data_section ,&entry.value.value ,is_big_endian);
                                     let end = if is_big_endian {entry.value.value.as_bytes().get_u64() as usize} else {entry.value.value.as_bytes().get_u64_le() as usize};
                                     let value_bytes = &data_position[..end];
                                     let value = std::str::from_utf8(value_bytes).unwrap().to_string();
                                     section.add_entry(&entry.key ,value)},
                        //由于先前的语义检查，后面应该不会达到
                              _ => unreachable!("Error occurred when try to check the type of the data")
                                    };
                    times = times + 1;
                    }
                   result.push(section);
                }
            }
        Ok(result)
        }
    }


#[cfg(test)]
mod test {
    use super::*;
    use bytes::BytesMut;

    #[test]
    fn test_parse_file() {
	    let path = "src/test.edc";
	    let parser = EdcParser::new(path);
	    let sections = parser.parse().unwrap();
	    assert_eq!(sections.len(), 5); 

        for section in sections{
            let section_name = section.name();
            match section_name {
                "meta" => {
                    let res:Vec<Option<String>> = section.get_entry("name").expect("key name in section meta:value should be present");
                    let name = res[0].clone().unwrap();
                    assert_eq!(name,String::from("demo"));
                }
                "buffers" => {
                    let res:Vec<Option<u32>> = section.get_entry("inputs").expect("key inputs in section buffers:value should be present");
                    let input = res[0].clone().unwrap();
                    assert_eq!(input,0);
                    let res1:Vec<Option<u32>> = section.get_entry("outputs").expect("key outputs in section buffers:value should be present");
                    let output = res1[0].clone().unwrap();
                    assert_eq!(output,1);
                    let res2:Vec<Option<String>> = section.get_entry("input0bind").expect("key input0bind in section buffers:value should be present");
                    let bind = res2[0].clone().unwrap();
                    assert_eq!(bind,String::from("free"));
                }
                "attribute" => {
                    let res :Vec<Option<u8>> = section.get_entry("lazy").expect("key lazy in section attribute:value should be present");
                    let lazy = res[0].clone().unwrap();
                    assert_eq!(lazy,1);
                }
                _ => {
                }
            }
        }
	}

    #[test]
    fn edt_header_parser(){
        assert_eq!(EdtParser::header_parser(b"edtf\x00\x00\x00ddddddddeeeeeeee\x00fff") ,
                    Ok((&b"fff"[..],EdtHeader{file_type:Bytes::copy_from_slice(&b"\x00"[..]),
                                           api_version:Bytes::copy_from_slice(&b"\x00"[..]),
                                           endian:Bytes::copy_from_slice(&b"\x00"[..]),
                                           offset:Bytes::copy_from_slice(&b"dddddddd"[..]),
                                           size:Bytes::copy_from_slice(&b"eeeeeeee"[..])})));
    }

    #[test]
    fn edt_section_entry_parser(){
        assert_eq!(EdtParser::section_entry_parser(b"aaaaaaaaaaaaaaaaabccccccccddddddddeeeefffff"),
                   Ok((&b"fffff"[..] ,EdtSectionEntry {name:Bytes::copy_from_slice(&b"aaaaaaaaaaaaaaaaa"[..]),
                                                    section_type:Bytes::copy_from_slice(&b"b"[..]),
                                                    offset:Bytes::copy_from_slice(&b"cccccccc"[..]),
                                                    size:Bytes::copy_from_slice(&b"dddddddd"[..]),
                                                    entry_count:Bytes::copy_from_slice(&b"eeee"[..])})));
    }

    #[test]
    fn edt_section_header_parser(){
        let count = b"\x00\x00\x00\x00\x00\x00\x00\x03";
        let entry1 = b"aaaaaaaaaaaaaaaaa\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03";
        let entry2 = b"bbbbbbbbbbbbbbbbb\x01\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x02";
        let entry3 = b"ccccccccccccccccc\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x01";
        let mut section_header = BytesMut::with_capacity(0);
        section_header.extend_from_slice(&count[..]);
        section_header.extend_from_slice(&entry1[..]);
        section_header.extend_from_slice(&entry2[..]);
        section_header.extend_from_slice(&entry3[..]);
        let section_count = Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x03"[..]);
        let mut expect_section_entry = Vec::new();
        expect_section_entry.push(EdtSectionEntry {name:Bytes::copy_from_slice(&b"aaaaaaaaaaaaaaaaa"[..]),
                                                section_type:Bytes::copy_from_slice(&b"\x00"[..]),
                                                offset:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x03"[..]),
                                                size:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x03"[..]),
                                                entry_count:Bytes::copy_from_slice(&b"\x00\x00\x00\x03"[..]),
                                            });
        expect_section_entry.push(EdtSectionEntry {name:Bytes::copy_from_slice(&b"bbbbbbbbbbbbbbbbb"[..]),
                                                section_type:Bytes::copy_from_slice(&b"\x01"[..]),
                                                offset:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x02"[..]),
                                                size:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x03"[..]),
                                                entry_count:Bytes::copy_from_slice(&b"\x00\x00\x00\x02"[..])});
        expect_section_entry.push(EdtSectionEntry {name:Bytes::copy_from_slice(&b"ccccccccccccccccc"[..]),
                                                section_type:Bytes::copy_from_slice(&b"\x02"[..]),
                                                offset:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x01"[..]),
                                                size:Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x03"[..]),
                                                entry_count:Bytes::copy_from_slice(&b"\x00\x00\x00\x01"[..])});
        assert_eq!(EdtParser::section_header_parser(section_header.as_bytes() ,true),
                   Ok((&b""[..] ,EdtSectionHeader{count:section_count,
                                           entry:expect_section_entry})))
    }

    #[test]
    fn edt_data_parser(){
        assert_eq!(EdtParser::data_parser(b"\x00\x01\x01123") ,
                   Ok((&b"123"[..] ,EdtData{data_type: Bytes::copy_from_slice(&b"\x00"[..]),
                                    length: Bytes::copy_from_slice(&b"\x01"[..]),
                                    value: Bytes::copy_from_slice(&b"\x01"[..])})));
        assert_eq!(EdtParser::data_parser(b"\x02\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01123") ,
                   Ok((&b"123"[..] ,EdtData{data_type: Bytes::copy_from_slice(&b"\x02"[..]),
                                            length: Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x01"[..]),
                                            value: Bytes::copy_from_slice(&b"\x00\x00\x00\x00\x00\x00\x00\x01"[..])})));
    }

    #[test]
    fn edt_entry_parser(){
        assert_eq!(EdtParser::section_parser(b"\x02\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01\x01" ,b"\x00AAAAAAAA" ,true) ,
                   Ok((&b""[..] ,EdtEntry{key:String::from("AAAAAAAA"),
                                       value:EdtData{data_type: Bytes::copy_from_slice(&b"\x00"[..]),
                                                  length: Bytes::copy_from_slice(&b"\x01"[..]),
                                                  value: Bytes::copy_from_slice(&b"\x01"[..])}})))
    }
}

