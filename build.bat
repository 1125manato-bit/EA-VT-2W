@echo off
setlocal

echo ========================================
echo   VT-2W White - Build Script (Windows)
echo   EMU AUDIO
echo ========================================
echo.

set BUILD_DIR=build
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo [1/3] Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release -DAUDIO_PLUGIN_HOST_GUI=ON
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    exit /b %errorlevel%
)

echo.
echo [2/3] Building...
cmake --build . --config Release --parallel 4
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)

echo.
echo [3/3] Build Complete!
echo.
echo Built plugins can be found in %BUILD_DIR%\EA_VT_2W_artefacts\Release
pause
