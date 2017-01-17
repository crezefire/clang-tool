#!/bin/bash

echo ""

OS="WIN32"

if [[ "$OSTYPE" == "linux-gnu" ]]; then
	OS="UNIX"
fi

instructions() {
	echo "Usage as follows. All paramters are optional:-"
	echo ""
	echo "-f | --folder=<folder name>"
	echo "-m|--make=<custom cmake params>"
  	echo "-G=\"Generator Name\" <required if using -m|--make>"
	if [[ "$OS" == "UNIX" ]]; then
		echo "-b | --build=<build type>"
		echo ""
		echo "Flags are as follows [no paramters required]:"
		echo "-c|--clang [Uses clang compiler in cmake]"
		echo "-v|--vscode [Does NOT set up VSCode configs]"
	fi
}

if ! type "cmake" > /dev/null; then
	echo "ERROR: CMake is not installed or unavailable in the path!!"
  exit -1
fi

#defaults
FOLDER_NAME=build
BUILD_TYPE=Debug
CLANG=0
VSCODE=1

CMAKE_GENERATOR="Visual Studio 14 Win64"
if [[ "$OS" == "UNIX" ]]; then
	CMAKE_GENERATOR="Unix Makefiles"
fi

#arguments parsing
for i in "$@"
do
case $i in
    -f=*|--folder=*)
    FOLDER_NAME="${i#*=}"
    ;;
	
    -b=*|--build=*)
    BUILD_TYPE="${i#*=}"
    ;;
	
	-m=*|--make=*)
    CMAKE_PARAMS="${i#*=}"
    ;;
  
  -G=*)
    CMAKE_GENERATOR="${i#*=}"
    ;;
	
	-c|--clang)
    CLANG=1
    ;;
	
	-v|--vscode)
    VSCODE=0
    ;;
  
  -h|--help)
    instructions
    exit 0
    ;;

    *)
	echo "ERROR: Unknown paramter: $i"
	instructions
	exit -1
    ;;
esac
shift
done

#print all used variables
echo ""
echo "Build folder: $FOLDER_NAME"
echo "Build Type: $BUILD_TYPE"
echo "CMake Generator: $CMAKE_GENERATOR"
echo "CMake Params: $CMAKE_PARAMS"

if [[ "$OS" == "UNIX" ]]; then
	echo "Using VSCode: $VSCODE"
	echo "Using Clang: $CLANG"
fi

echo ""

if [ ! -d "$FOLDER_NAME" ]; then
  mkdir "$FOLDER_NAME"
  echo "$FOLDER_NAME/" >> .gitignore
fi

#Only required on buntu but do it anyways
if [ ! -d "bin" ]; then
  mkdir bin
fi

pushd .
cd "$FOLDER_NAME"
echo ""

if [[ "$OS" == "UNIX" ]]; then
		
		if [[ $VSCODE -eq 1 ]] && [[ ! -d "../.vscode" ]]; then
			cp -r ../cmake/.vscode ../
			sed -i -e "s/BUILDFOLDERHERE/$FOLDER_NAME/g" "../.vscode/tasks.json"
		fi

    cmake -DUSE_CLANG=$CLANG -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_PARAMS -G "$CMAKE_GENERATOR" ..
else
    cmake $CMAKE_PARAMS -G "$CMAKE_GENERATOR" ..
fi

#Check return code of CMake Process
if [ $? == 0 ]; then
	echo "==========>SUCCESS"
else
	echo "============>FAILED"
fi

popd

#TODO(vim): Should return -1 on failure
exit 0
