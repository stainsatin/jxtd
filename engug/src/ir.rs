use std::collections::HashMap;

#[derive(Clone, Hash, PartialEq, Eq, Debug)]
pub enum IntegerValue {
    Byte(u8),
    Word(u16),
    Double(u32),
    Quad(u64),
}

// Ensure that floating-point value is not NaN in practice,
// so reflexivity is satisfied.
// NOTE:
// Reflexivity is risky to floating-point values,
// and only current when NaN is excluded.
// Modify the following trait implementations carefully,
// only if you fully understand what you do.
#[derive(Clone, PartialEq, Debug)]
pub enum FloatingValue {
    Single(f32),
    Double(f64),
}

//Implement Hash trait manually.
impl std::hash::Hash for FloatingValue{
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        match self{
            Self::Single(value) => {
                0.hash(state);
                value.to_bits().hash(state);
            },
            Self::Double(value) => {
                1.hash(state);
                value.to_bits().hash(state);
            }
        }
    }
}

//manual implementation for the same reason
impl Eq for FloatingValue{}

#[derive(Clone, PartialEq, Eq, Debug)]
pub enum IRValue {
    Integer(IntegerValue),
    Floating(FloatingValue),
    Seq(String),
}

pub struct Section {
    name: String,
    entries: HashMap<String, Vec<IRValue>>,
}

//cast to IRValue
impl From<u8> for IRValue {
    fn from(value: u8) -> Self {
        Self::Integer(IntegerValue::Byte(value))
    }
}
impl From<u16> for IRValue {
    fn from(value: u16) -> Self {
        Self::Integer(IntegerValue::Word(value))
    }
}
impl From<u32> for IRValue {
    fn from(value: u32) -> Self {
        Self::Integer(IntegerValue::Double(value))
    }
}
impl From<u64> for IRValue {
    fn from(value: u64) -> Self {
        Self::Integer(IntegerValue::Quad(value))
    }
}
impl From<f32> for IRValue {
    fn from(value: f32) -> Self {
        assert!(!value.is_nan(), "value is NaN");
        Self::Floating(FloatingValue::Single(value))
    }
}
impl From<f64> for IRValue {
    fn from(value: f64) -> Self {
        assert!(!value.is_nan(), "value is NaN");
        Self::Floating(FloatingValue::Double(value))
    }
}
impl From<String> for IRValue {
    fn from(value: String) -> Self {
        Self::Seq(value)
    }
}
impl From<&str> for IRValue {
    fn from(value: &str) -> Self {
        Self::Seq(value.to_owned())
    }
}

//cast from IRValue
//type may dismatch, TryFrom is suitable.
impl TryFrom<IRValue> for u8 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Integer(IntegerValue::Byte(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for u16 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Integer(IntegerValue::Word(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for u32 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Integer(IntegerValue::Double(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for u64 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Integer(IntegerValue::Quad(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for f32 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Floating(FloatingValue::Single(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for f64 {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Floating(FloatingValue::Double(value)) = value {
            return Ok(value);
        }
        Err(value)
    }
}
impl TryFrom<IRValue> for String {
    type Error = IRValue;

    fn try_from(value: IRValue) -> Result<Self, Self::Error> {
        if let IRValue::Seq(value) = value {
            return Ok(value);
        }
        Err(value)
    }
}

//section impl
impl Section {
    pub fn new(name: String) -> Self {
        let entries = HashMap::new();
        Self { name, entries }
    }

    pub fn name(&self) -> &str {
        &self.name
    }

    pub fn add_entry<V>(&mut self, key: &str, value: V) -> Option<()>
    where
        V: Into<IRValue>,
    {
        match self.entries.get_mut(key) {
            None => {
                let values = vec![value.into()];
                self.entries.insert(key.to_owned(), values);
            }
            Some(values) => {
                values.push(value.into());
            }
        };
        Some(())
    }

    pub fn get_entry<V>(&self, key: &str) -> Option<Vec<Option<V>>>
    where
        V: TryFrom<IRValue>,
    {
        match self.entries.get(key) {
            None => None,
            Some(values) => Some(
                values
                    .iter()
                    .map(|v| match v.to_owned().try_into() {
                        Ok(value) => Some(value),
                        Err(_) => None,
                    })
                    .collect(),
            ),
        }
    }

    pub fn get_keys(&self) -> Vec<String>{
        self.entries.keys().map(|x| x.clone()).collect()
    }

    pub fn get_entry_boxed(&self, key: &str) -> Option<Vec<IRValue>>{
        self.entries.get(key).map(|x| x.to_owned())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn section_push_pop() {
        let mut section = Section::new(String::from("section"));
        assert_eq!(section.name(), "section");

        section.add_entry("name", "value");
        let values: Vec<Option<String>> =
            section.get_entry("name").expect("value should be present");
        assert_eq!(values.len(), 1);
        assert_eq!(
            values[0].clone().expect("try_into should succeed"),
            String::from("value")
        );
    }

    #[test]
    fn section_multi_value() {
        let mut section = Section::new(String::from("section"));
        section.add_entry("extra", 0u32);
        section.add_entry("extra", 1u32);
        let values: Vec<Option<u32>> = section.get_entry("extra").expect("value should be present");
        assert_eq!(values.len(), 2);
    }

    #[test]
    fn section_boxed_value(){
        let mut section = Section::new(String::from("section"));
        section.add_entry("extra", 0u32);
        section.add_entry("extra", 1u32);
        let values = section.get_entry_boxed("extra").expect("entry is present");
        assert_eq!(values[0], IRValue::Integer(IntegerValue::Double(0u32)));
    }
}
