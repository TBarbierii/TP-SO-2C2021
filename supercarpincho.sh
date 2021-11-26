#!/bin/bash
length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD
echo -e "\n\nInstalando commons Library, o eso creo...\n\n"
COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS

sudo make uninstall
make all
sudo make install

echo -e "\n\n Instalando las pruebas de carpinchoide... \n\n"
git clone "https://github.com/sisoputnfrba/carpinchos-pruebas.git"
PRUEBACARPINCHO="carpinchos-pruebas"


cd $CWD
echo -e "\n\nBuilding projects...\n\n"
make -C ./SWAmP
make -C ./Memoria
make -C ./Kernel
make -C ./MateLib
echo -e "\n\nDale boooooo, dale boooo!\n\n"