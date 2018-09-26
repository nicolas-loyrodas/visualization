**************** PLY model visualization

author: Nicolas Loy Rodas

Application to load and visualize a .ply model as it rotates continously

Requirements
·	VTK version 5.10.1
·       Boost
.	Eigen3

Files
main.cpp

Input parameters

-h	<Print help>
-i 	<Input .ply file to visualize (full path without file extension)>
-a 	<Axis to rotate object>


Example to visualize model.ply while rotating it around its x axis
./plyModelsVisuRotation /myFolder/model.ply x

