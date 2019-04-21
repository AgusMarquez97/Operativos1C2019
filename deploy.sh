#/bin/bash
echo "Seteando variables de entorno..."
cd /home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/FileSystem/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/Debug
cd ~/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/Kernel/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/Debug
cd ~/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/PoolMemorias/Debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/workspace/Segmentation-Fault/tp-2019-1c-Segmentation-Fault/BibliotecaCompartida/Debug
echo "Seteo completo"

