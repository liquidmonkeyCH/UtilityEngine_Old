##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=ServerTest
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/root/UtilityEngine_New/trunk/codelite"
ProjectPath            := "/root/UtilityEngine_New/trunk/codelite/ServerTest"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=root
Date                   :=üŸ²§­ç¿¿
CodeLitePath           :="/root/.codelite"
LinkerName             :=g++-5
ArchiveTool            :=ar rcus
SharedObjectLinkerName :=g++-5 -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
CompilerName           :=g++-5
C_CompilerName         :=gcc-5
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="/root/UtilityEngine_New/trunk/codelite/ServerTest/ServerTest.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
CmpOptions             := -g -ggdb -fPIC -std=c++0x -gstabs  $(Preprocessors)
C_CmpOptions           := -g -ggdb -fPIC -std=c++0x -gstabs  $(Preprocessors)
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../UtilityEngine/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)UtilityEngine 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../lib 


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects=$(IntermediateDirectory)/UtilityTest_ServerTest$(ObjectSuffix) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects) > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/UtilityTest_ServerTest$(ObjectSuffix): ../../UtilityTest/ServerTest.cpp $(IntermediateDirectory)/UtilityTest_ServerTest$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/ServerTest.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_ServerTest$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_ServerTest$(DependSuffix): ../../UtilityTest/ServerTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_ServerTest$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_ServerTest$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/ServerTest.cpp"

$(IntermediateDirectory)/UtilityTest_ServerTest$(PreprocessSuffix): ../../UtilityTest/ServerTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_ServerTest$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/ServerTest.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/UtilityTest_ServerTest$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ServerTest$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ServerTest$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "/root/UtilityEngine_New/trunk/codelite/.build-debug/ServerTest"


