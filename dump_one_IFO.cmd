set PATH=D:\msys64\mingw64\bin;%PATH%

mkdir tmp

copy %1 tmp\VIDEO_TS.IFO

cd tmp

"d:\Mais documentos\Projectos\Ruby scripts\dvdtools\dump_ifo.exe" .

cd ..