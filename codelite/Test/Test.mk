##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Test
ConfigurationName      :=Debug
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
WorkspacePath          := "/root/UtilityEngine_New/trunk/codelite"
ProjectPath            := "/root/UtilityEngine_New/trunk/codelite/Test"
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=root
Date                   :=ú±ž¥ 
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
ObjectsFileList        :="/root/UtilityEngine_New/trunk/codelite/Test/Test.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
CmpOptions             := -g -ggdb -fPIC -std=c++0x -gstabs  $(Preprocessors)
C_CmpOptions           := -g -ggdb -fPIC -std=c++0x -gstabs  $(Preprocessors)
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../../UtilityEngine/include $(IncludeSwitch)/usr/include/mysql++ $(IncludeSwitch)/usr/include/mysql 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)pthread $(LibrarySwitch)UtilityEngine $(LibrarySwitch)mysqlpp 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch)../lib 


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects=$(IntermediateDirectory)/UtilityTest_Service_Test$(ObjectSuffix) $(IntermediateDirectory)/UtilityTest_Singleton_Test$(ObjectSuffix) $(IntermediateDirectory)/UtilityTest_UtilityTest$(ObjectSuffix) $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(ObjectSuffix) $(IntermediateDirectory)/UtilityTest_Mysql_Test$(ObjectSuffix) 

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
$(IntermediateDirectory)/UtilityTest_Service_Test$(ObjectSuffix): ../../UtilityTest/Service_Test.cpp $(IntermediateDirectory)/UtilityTest_Service_Test$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/Service_Test.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_Service_Test$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_Service_Test$(DependSuffix): ../../UtilityTest/Service_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_Service_Test$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_Service_Test$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/Service_Test.cpp"

$(IntermediateDirectory)/UtilityTest_Service_Test$(PreprocessSuffix): ../../UtilityTest/Service_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_Service_Test$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/Service_Test.cpp"

$(IntermediateDirectory)/UtilityTest_Singleton_Test$(ObjectSuffix): ../../UtilityTest/Singleton_Test.cpp $(IntermediateDirectory)/UtilityTest_Singleton_Test$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/Singleton_Test.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_Singleton_Test$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_Singleton_Test$(DependSuffix): ../../UtilityTest/Singleton_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_Singleton_Test$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_Singleton_Test$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/Singleton_Test.cpp"

$(IntermediateDirectory)/UtilityTest_Singleton_Test$(PreprocessSuffix): ../../UtilityTest/Singleton_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_Singleton_Test$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/Singleton_Test.cpp"

$(IntermediateDirectory)/UtilityTest_UtilityTest$(ObjectSuffix): ../../UtilityTest/UtilityTest.cpp $(IntermediateDirectory)/UtilityTest_UtilityTest$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/UtilityTest.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_UtilityTest$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_UtilityTest$(DependSuffix): ../../UtilityTest/UtilityTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_UtilityTest$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_UtilityTest$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/UtilityTest.cpp"

$(IntermediateDirectory)/UtilityTest_UtilityTest$(PreprocessSuffix): ../../UtilityTest/UtilityTest.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_UtilityTest$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/UtilityTest.cpp"

$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(ObjectSuffix): ../../UtilityTest/ThreadPool_Test.cpp $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/ThreadPool_Test.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(DependSuffix): ../../UtilityTest/ThreadPool_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/ThreadPool_Test.cpp"

$(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(PreprocessSuffix): ../../UtilityTest/ThreadPool_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/ThreadPool_Test.cpp"

$(IntermediateDirectory)/UtilityTest_Mysql_Test$(ObjectSuffix): ../../UtilityTest/Mysql_Test.cpp $(IntermediateDirectory)/UtilityTest_Mysql_Test$(DependSuffix)
	$(CompilerName) $(IncludePCH) $(SourceSwitch) "/root/UtilityEngine_New/trunk/UtilityTest/Mysql_Test.cpp" $(CmpOptions) $(ObjectSwitch)$(IntermediateDirectory)/UtilityTest_Mysql_Test$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/UtilityTest_Mysql_Test$(DependSuffix): ../../UtilityTest/Mysql_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/UtilityTest_Mysql_Test$(ObjectSuffix) -MF$(IntermediateDirectory)/UtilityTest_Mysql_Test$(DependSuffix) -MM "/root/UtilityEngine_New/trunk/UtilityTest/Mysql_Test.cpp"

$(IntermediateDirectory)/UtilityTest_Mysql_Test$(PreprocessSuffix): ../../UtilityTest/Mysql_Test.cpp
	@$(CompilerName) $(CmpOptions) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/UtilityTest_Mysql_Test$(PreprocessSuffix) "/root/UtilityEngine_New/trunk/UtilityTest/Mysql_Test.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/UtilityTest_Service_Test$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Service_Test$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Service_Test$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Singleton_Test$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Singleton_Test$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Singleton_Test$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_UtilityTest$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_UtilityTest$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_UtilityTest$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_ThreadPool_Test$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Mysql_Test$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Mysql_Test$(DependSuffix)
	$(RM) $(IntermediateDirectory)/UtilityTest_Mysql_Test$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) "/root/UtilityEngine_New/trunk/codelite/.build-debug/Test"


