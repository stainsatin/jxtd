# localrepo reference
## localrepo
### synopis
localrepo(name
        \[PATCH\_COMMAND cmd\]
        \[CONFIGURE\_COMMAND cmd\]
        \[BUILD\_COMMAND cmd\]
        \[INSTALL\_COMMAND cmd\]
        )
### description
generic build step, project default steps:patch,configure,build,install

## local\_repo\_addstep
### synopis
local\_repo\_addstep(name step
        \[COMMAND cmd\]
        \[BEFORE dependencies\]
        \[AFTER dependee\]
        )
### description
manual project step management, and the project depends on all steps implicitly.

## local\_repo\_cmake
### synopis
local\_repo\_cmake(name
        SOURCE srcdir
        \[SHARED\] \[RELEASE\]
        \[OPTIONS confopt\]
        \[BUILD builddir\]
        \[DESTINATION installdir\]
        \[PATCH\_COMMAND cmd\]
        \[CONFIGURE\_COMMAND cmd\]
        \[INSTALL\_COMMAND cmd\]
        \[BUILD\_COMMAND cmd\]
        )
### description
SHARED: equivalent to -DBUILD\_SHARED\_LIBS=ON  
RELEASE: equivalent to -DCMAKE\_BUILD\_TYPE=Release

cmake-based project construction.

## local\_repo\_autotool
### synopis
local\_repo\_autotool(name
        SOURCE srcdir
        \[SHARED\]
        \[OPTIONS confopt\]
        \[BUILD builddir\]
        \[DESTINATION installdir\]
        \[PATCH\_COMMAND cmd\]
        \[CONFIGURE\_COMMAND cmd\]
        \[TARGET buildtarget\]
        \[INSTALL installtarget\]
        )
### description
autotool-based project construction.
