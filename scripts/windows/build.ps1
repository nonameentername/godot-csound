Write-Host "Configure build"

$CxxPath = "C:\ProgramData\mingw64\mingw64\bin\g++.exe"
$CPath = "C:\ProgramData\mingw64\mingw64\bin\gcc.exe"

$InstallPrefix = Join-Path (Get-Location) bin\csound

pushd .\modules\csound
cmake -B build -S . -DUSE_VCPKG=1 -DCUSTOM_CMAKE="./platform/windows/Custom-vs.cmake" -DCMAKE_CXX_COMPILER=$CxxPath -DCMAKE_C_COMPILER=$CPath -DCMAKE_INSTALL_PREFIX:PATH=$InstallPrefix

Write-Host "Build Csound"

cmake --build build --config Release

Write-Host "Install Csound"

cmake --build build --target install --config Release
popd
