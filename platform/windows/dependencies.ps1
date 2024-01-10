Write-Host "Install csound dependencies"

choco install -y winflexbison3 innosetup mingw llvm javaruntime jdk8 swig python
choco install -y cmake --installargs '"ADD_CMAKE_TO_PATH=System"'
choco install -y visualstudio2022community --package-parameters "add Microsoft.VisualStudio.Workload.NativeDesktop --includeRecommended --includeOptional --passive --locale en-US"

Import-Module $env:ChocolateyInstall\helpers\chocolateyProfile.psm1
refreshenv

Write-Host "Bootstrap VCPKG"

pushd .\modules\csound
.\vcpkg\bootstrap-vcpkg.bat
popd

Write-Host "Install godot-csound dependencies"

choco install -y python make dotnet-6.0-sdk

Import-Module $env:ChocolateyInstall\helpers\chocolateyProfile.psm1
refreshenv

pip install scons
