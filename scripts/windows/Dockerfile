FROM mcr.microsoft.com/dotnet/framework/runtime:4.8-windowsservercore-ltsc2016

ADD system-dependencies.ps1 C:/
ADD dependencies.ps1 C:/

RUN powershell -file "C:/system-dependencies.ps1"
RUN powershell -file "C:/dependencies.ps1"
