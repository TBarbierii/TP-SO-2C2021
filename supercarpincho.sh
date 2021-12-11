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
ls -a

cd ..
PRUEBACARPINCHO="carpinchos-pruebas"
echo -e "\n\n Instalando las pruebas de carpinchoide... \n\n"
git clone "https://github.com/sisoputnfrba/carpinchos-pruebas.git" $PRUEBACARPINCHO



cd $CWD
echo -e "\n\nCompilando el proyecto...\n\n"
make -C ./SWAmP
make -C ./Memoria
make -C ./Kernel
make -C ./MateLib

echo -e "\n\n Copio la matelib a /usr/lib"
sudo cp -u MateLib/src/libmatelib.so /usr/lib
ls -a
echo -e "\n\n Compilar los carpinchos de prueba\n\n"
cd $PRUEBACARPINCHO
make compile

cd
mkdir dumps
mkdir dumps/tlb

echo -e "\n\nDale boooooo, dale boooo!\n\n"