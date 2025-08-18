# libabmp
My own library for BMP files made from scratch

It does not compile using cl.exe (Windows compiler, but it works using g++.exe using mingw64)

cmake -DCMAKE_C_COMPILER=D:\mingw64\bin\gcc.exe -DCMAKE_CXX_COMPILER=D:\mingw64\bin\g++.exe -B build -G "MinGW Makefiles"

TODO:
- Create funcitons that directly reads from file and not copy to memory to copy again
