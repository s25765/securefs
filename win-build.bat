REM Copied from https://github.com/wsk2001/securefs/blob/master/win-build.bat
@ECHO off

IF "%1"=="clean" (
    goto clean
) ELSE (
    goto all
)

:clean
	rm -f CMakeCache.txt
	rm -f *.vcxproj
	rm -f *.sln
	rm -f *.filters
	rm -f *.cmake
	rm -f *.user
	GOTO:EOF

:all
	rm -f CMakeCache.txt
	rm -f *.vcxproj
	rm -f *.sln
	rm -f *.filters
	rm -f *.cmake
	rm -f *.user

	cmake CMakeLists.txt
	@echo.
	@echo Load securefs.sln in Visual Studio
	@echo.

	GOTO:EOF

