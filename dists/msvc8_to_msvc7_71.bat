@echo off
rem This batch file is used to convert MSVC8 (Visual Studio 2005) project files to 
rem MSVC71 (Visual Studio 2003) and MSVC7 (Visual Studio 2002) ones
rem You need the Windows version of GNU rpl
rem Get it here:
rem http://gnuwin32.sourceforge.net/packages/rpl.htm
rem Place rpl.exe from the bin folder inside the archive in the folder where
rem this batch file resides

if not exist rpl.exe goto no_rpl

echo Creating MSVC71 project files from the MSVC8 ones
copy /y msvc8\*.vcproj msvc71\
copy /y msvc8\*.sln msvc71\
rpl -e -q "Version=\"8,00\"" "Version=\"7.10\"" msvc71\*.vcproj
for %%i in (msvc71\*.vcproj) do rpl -e -q "RootNamespace=\"%%~ni\"\n" "" %%i
rpl -e -q "RootNamespace=" "#RootNamespace=" msvc71\*.vcproj
rpl -e -q "\t\tKeyword=" "\tKeyword=" msvc71\*.vcproj
rpl -e -q "\t<ToolFiles>\n\t</ToolFiles>\n" "" msvc71\*.vcproj
rpl -e -q " /wd4996" "" msvc71\*.vcproj
rpl -e -q "ExceptionHandling=\"1\"" "ExceptionHandling=\"true\"" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCManagedResourceCompilerTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCPreLinkEventTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCALinkTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCXDCMakeTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCBscMakeTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCFxCopTool\"\n\t\t\t/>\n" "" msvc71\*.vcproj
rpl -e -q "WholeProgramOptimization=\"1\"" "WholeProgramOptimization=\"true\"" msvc71\*.vcproj
rpl -e -q "ExceptionHandling=\"1\"" "ExceptionHandling=\"true\"" msvc71\*.vcproj
rem Change multi-line XML closing tags to single line
rpl -e -q "\"\n\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t\t\t\0x3e\n" "\"\0x3e\n" msvc71\*.vcproj
rpl -e -q "\"\n\t\t\t\t\t\t/\0x3e\n" "\"/\0x3e\n" msvc71\*.vcproj
rem Change lower case true and false to upper case
rpl -e -q "\"true\"" "\"TRUE\"" msvc71\*.vcproj
rpl -e -q "\"false\"" "\"FALSE\"" msvc71\*.vcproj
rem The following are mainly line swaps to keep layout the same
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCPreBuildEventTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCCustomBuildTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCMIDLTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCAppVerifierTool\"/>\n" "" msvc71\*.vcproj
rpl -e -q ".lib\"/\0x3e\n" ".lib\"/\0x3e\n\t\t\t<Tool\n\t\t\t\tName=\"VCMIDLTool\"/\0x3e\n" msvc71\*.vcproj	
rem This is needed, but needs to go after the line that deletes the VCCustomBuildTool entry
rpl -e -q "\"VCResourceCompilerTool\"" "\"VCCustomBuildTool\"" msvc71\*.vcproj
rem This is needed too
rpl -e -q "\"VCManifestTool\"" "\"VCMIDLTool\"" msvc71\*.vcproj
rpl -e -q "\"VCPostBuildEventTool\"/>\n" "\"VCPostBuildEventTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCPreBuildEventTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCPreBuildEventTool\"/>\n" "\"VCPreBuildEventTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCPreLinkEventTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCPreLinkEventTool\"/>\n" "\"VCPreLinkEventTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCResourceCompilerTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCResourceCompilerTool\"/>\n" "\"VCResourceCompilerTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCWebServiceProxyGeneratorTool\"/>\n" "\"VCWebServiceProxyGeneratorTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCXMLDataGeneratorTool\"/>\n" "\"VCXMLDataGeneratorTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>\n" msvc71\*.vcproj
rpl -e -q "\"VCManagedWrapperGeneratorTool\"/>\n" "\"VCManagedWrapperGeneratorTool\"/>\n\t\t\t<Tool\n\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n" msvc71\*.vcproj
			
rpl -e -q "Format Version 9.00" "Format Version 8.00" msvc71\scummvm.sln
rpl -e -q "# Visual C++ Express 2005\n" "" msvc71\scummvm.sln
rpl -e -q "# Visual Studio 2005\n" "" msvc71\scummvm.sln
rpl -e -q "\"\nEndProject\n" "\"\n\tProjectSection(ProjectDependencies) = postProject\n\tEndProjectSection\nEndProject\n" msvc71\scummvm.sln
rpl -e -q "SolutionConfigurationPlatforms" "SolutionConfiguration" msvc71\scummvm.sln
rpl -e -q "ProjectConfigurationPlatforms" "ProjectDependencies" msvc71\scummvm.sln
rpl -e -q "Debug|Win32 = Debug|Win32" "Debug = Debug" msvc71\scummvm.sln
rpl -e -q "Release|Win32 = Release|Win32" "Release = Release" msvc71\scummvm.sln
rpl -e -q "EndGlobal\n" "\tGlobalSection(ExtensibilityGlobals) = postSolution\n\tEndGlobalSection\nEndGlobal\n" msvc71\scummvm.sln
rpl -e -q "EndGlobal\n" "\tGlobalSection(ExtensibilityAddIns) = postSolution\n\tEndGlobalSection\nEndGlobal\n" msvc71\scummvm.sln

echo Creating MSVC71 project files from the MSVC7 ones
copy /y msvc71\*.vcproj msvc7\
copy /y msvc71\*.sln msvc7\
rpl -e -q "Version=\"7.10\"" "Version=\"7.00\"" msvc7\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\n" "" msvc7\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>\n" "" msvc7\*.vcproj
rpl -e -q "\t\t\t<Tool\n\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n" "" msvc7\*.vcproj
rpl -e -q "\t<References>\n\t</References>\n" "" msvc7\*.vcproj

rpl -e -q "Format Version 8.00" "Format Version 7.00" msvc7\scummvm.sln
rpl -e -q "\tProjectSection(ProjectDependencies) = postProject\n\tEndProjectSection\n" "" msvc7\scummvm.sln
goto the_end

:no_rpl
echo You need the Windows version of GNU rpl
echo Get it here:
echo http://gnuwin32.sourceforge.net/packages/rpl.htm
echo Place rpl.exe from the bin folder inside the archive in the folder where
echo this batch file resides

:the_end
pause
