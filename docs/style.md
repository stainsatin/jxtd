# coding style
## baseline
guidelines released before
## specific
### namespace
|type|spec|
|:--:|:--:|
|normal class/struct|::jxtd::_directory_::_structure_|
|protobuf message(internal)|::jxtd::_directory_::_structure_|
|protobuf message(multiunit)|::jxtd::proto::_protocolname_|
### extra paths
|type|spec|
|:--:|:--:|
|headers|\<\> from the top level of sources, \"\" for the current directory|
|3rdparty|_vendors_ directory for sources, _thirdparty_ for build and target|
|scripts|_script_ directory|
|toolchain extras|_cmake_ for cmake, _bazel_ for bazel|
### CMakeLists.txt
1. force to use target\_\* directives, except 3rdparty.
2. forbid to add include directory in subdirectories, except binary-related directory(CMAKE\_CURRENT\_BINARY\_DIR, etc.).
3. forbids find\_\*, build from source if needed.
