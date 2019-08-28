##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=ClientTest
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/root/UtilityEngine_New/trunk/codelite"
ProjectPath            := "/root/UtilityEngine_New/trunk/codelite/ClientTest"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=root
Date                   :=üº±º¨€ç¿¿ý”ªŸ¿¬ç¾›üº±º¨€ç¿¿üº±¶“ç¿¿üº±º¼ç¿¿ý”ª ‚ç¾›üº±º¼ç¿¿üº±º¨€ç¿¿üº±»ç¿¿ý”©œ…ºç¾›Â•Ä·ç¿¿üº±¶¡°ç¿¿
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
ObjectsFileList        :="/root/UtilityEngine_New/trunk/codelite/ClientTest/ClientTest.txt"
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
Objects=$(IntermediateDirectory)/UtilityTest_ClientTest$(ObjectSuffix) 

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
$(IntermediateDirectory)/UtilityTest_ClientTest$(ObjectSuffix): ../../UtilityTest/ClientTest.cpp $(IntermediateDirectory)/UtilityTest_ClientTest$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/ClientTest.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_ClientTest$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_ClientTest$(DependSuffix): ../../UtilityTest/ClientTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_ClientTest$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_ClientTest$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/ClientTest.cpp"

$(IntermediateDirectory)/UtilityTest_ClientTest$(PreprocessSuffix): ../../UtilityTest/ClientTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_ClientTest$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/ClientTest.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/UtilityTest_ClientTest$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ClientTest$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ClientTest$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "/root/UtilityEngine_New/trunk/codelite/.build-debug/ClientTest"


