@ECHO OFF

pushd %~dp0

REM Command file for Sphinx documentation

IF "%SPHINXBUILD%" == "" (
	SET SPHINXBUILD=sphinx-build
)
SET SOURCEDIR=.
SET BUILDDIR=_build

%SPHINXBUILD% >NUL 2>NUL
IF ERRORLEVEL 9009 (
	ECHO.
	ECHO.The 'sphinx-build' command was not found. Ensure Sphinx is installed and
	ECHO.available on your PATH or set the SPHINXBUILD environment variable to
	ECHO.the full path of the executable.
	ECHO.
	EXIT /B 1
)

IF "%1" == "" GOTO help

%SPHINXBUILD% -M %1 %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%
GOTO end

:help
%SPHINXBUILD% -M help %SOURCEDIR% %BUILDDIR% %SPHINXOPTS% %O%

:end
popd
