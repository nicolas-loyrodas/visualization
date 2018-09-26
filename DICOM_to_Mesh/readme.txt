**************** DICOM to MHD

author: Nicolas Loy Rodas
contact: nicolas.loyrodas@gmail.com

Application to transform a CT in DICOM format to an MHD file compatible with XAware and GGEMS simulation.

Requirements
·	VTK version 5.10.1
·       Boost

Files
main.cpp

Input parameters

-h	<Print help>
-i	<Input folder containing the DICOM images>
-o	<Output MHD file>
-f	<Filter table threshold>
-v 	<Include to visualize the CT with volume rendering>

Example
./dicomToMHD -i ~/camma/svn/camma/code/projects/cpp/xaware/xaware_transfer/dicomToMHD/input/Porc/DICOM -o PORC -f -v -c

Output
PORC.mhd
PORD.raw
