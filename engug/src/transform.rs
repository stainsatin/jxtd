use std::collections::HashMap;
use crate::ir::{IntegerValue, FloatingValue, IRValue, Section};

#[derive(Hash, PartialEq, Eq)]
pub enum EdtValue{
	Integer{len: u8, value: IntegerValue},
	Floating{len: u8, value: FloatingValue},
	Seq{len: u64, offset: u64}
}

pub struct EdtData{
	pub strings: Vec<String>
}

pub struct EdtSection{
	pub name: EdtValue,
	pub entries: HashMap<EdtValue, Vec<EdtValue>>,
	pub data: EdtData
}

impl From<Section> for EdtSection{
    fn from(value: Section) -> Self{
        let mut strings = Vec::new();
        let mut entries = HashMap::new();
        let offset = 0 as usize;
        let name = value.name();
        let len = name.len();
        strings.push(name.to_owned());
        let name = EdtValue::Seq { len: len as u64, offset: offset as u64};
        let offset = offset + len;
        value.get_keys().iter().fold(offset, |offset, key| {
            strings.push(key.clone());
            let entry_key = EdtValue::Seq{len: key.len() as u64, offset: offset as u64};
            let mut offset = offset + key.len();
            let entry_values = value.get_entry_boxed(&key).expect("key exists because of the same origin").iter().map(|value| {
                match value {
                    IRValue::Integer(value) => {
                        match value {
                            IntegerValue::Byte(value) => EdtValue::Integer {len: 1, value: IntegerValue::Byte(value.clone())},
                            IntegerValue::Word(value) => EdtValue::Integer {len: 2, value: IntegerValue::Word(value.clone())},
                            IntegerValue::Double(value) => EdtValue::Integer {len: 4, value: IntegerValue::Double(value.clone())},
                            IntegerValue::Quad(value) => EdtValue::Integer {len: 8, value: IntegerValue::Quad(value.clone())}
                        }
                    },
                    IRValue::Floating(value) => {
                        match value {
                            FloatingValue::Single(value) => EdtValue::Floating { len: 4, value: FloatingValue::Single(value.clone())},
                            FloatingValue::Double(value) => EdtValue::Floating { len: 8, value: FloatingValue::Double(value.clone())}
                        }
                    },
                    IRValue::Seq(value) => {
                        strings.push(value.to_owned());
                        let len = value.len();
                        let entry_value = EdtValue::Seq { len: len as u64, offset: offset as u64 };
                        offset += len;
                        entry_value
                    }
                }
            }).collect();
            entries.insert(entry_key, entry_values);
            offset
        });
        Self{
            name,
            entries,
            data: EdtData{strings}
        }
    }
}
impl From<EdtSection> for Section{
    fn from(value: EdtSection) -> Self{
        let strings = value.data.strings;

        let name = match value.name {
            EdtValue::Seq { len, offset } => search_string(&strings, offset.to_owned(), len.to_owned()).expect("edt section name should exist"),
            _ => panic!("edt section name should be string")
        };

        let mut section = Self::new(name);
        value.entries.iter().for_each(|(key, value)| {
            match key {
                EdtValue::Seq { len, offset } => {
                    let key = search_string(&strings, offset.clone(), len.clone()).expect("Edt key exists because of the same origin");
                    value.iter().for_each(|value| {
                        match value{
                            EdtValue::Integer { len: _, value } => {
                                match value{
                                    IntegerValue::Byte(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                    IntegerValue::Word(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                    IntegerValue::Double(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                    IntegerValue::Quad(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                };
                            },
                            EdtValue::Floating { len: _, value } => {
                                match value{
                                    FloatingValue::Single(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                    FloatingValue::Double(value) => section.add_entry(&key, value.to_owned()).expect("Add edc entry failed"),
                                };
                            },
                            EdtValue::Seq { len, offset } => {
                                section.add_entry(&key, search_string(&strings, offset.to_owned(), len.to_owned()).expect("Edt String is wrong(len or offset)")).expect("Add edc entry failed");
                            }
                        }
                    });
                },
                _ => panic!("EdtSection key should be Seq type")
            }
        });

        section
    }
}

fn search_string(strings: &Vec<String>, offset: u64, len: u64) -> Option<String>{
    let mut cursor = 0 as usize;
    strings.iter().find(|value| {
        if cursor == offset as usize && value.len() == len as usize{
            return true;
        }
        cursor += value.len();
        false
    }).map(|x| x.to_owned())
}

#[cfg(test)]
mod test{
    use super::*;
    use crate::ir::*;

    #[test]
    fn edc_to_edt(){
        let mut section = Section::new(String::from("section"));
        //subnormal value
        section.add_entry("extra", 0.0f32);
        section.add_entry("extra", 1.0f32);
        //normal value
        section.add_entry("extra", 2.0f32);

        let section = EdtSection::from(section);
        //0: section
        //1: extra
        assert_eq!(section.data.strings.len(), 2);
        let entry = EdtValue::Seq { len: 5, offset: 7 };

        match section.entries.get(&entry){
            None => panic!("entry should exist"),
            Some(value) => {
                let first = value.first().expect("entry should not be empty");
                match first{
                    EdtValue::Floating { len, value } => {
                        assert_eq!(len.to_owned(), 4);
                        match value {
                            FloatingValue::Single(value) => {
                                match value.partial_cmp(&0.0f32).expect("value is not NaN"){
                                    std::cmp::Ordering::Equal => {},
                                    _ => panic!("value not equal")
                                }
                            },
                            _ => panic!("value is single-precious")
                        };
                    },
                    _ => panic!("value is floating-point value")
                };
            }
        };
    }
}
