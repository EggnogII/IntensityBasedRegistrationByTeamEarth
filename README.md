 IntensityBasedRegistrationByTeamEarth

Program that does Mutual Information Image Registration of CT, and MRI Images.

	It is necessary to build this program with a automation compiler such as CMake.
It may be necessary to re-build this program for your particular Unix-based OS (OS X, or 
any Linux-based Distro). Note: this program is not compatible with Windows
based systems.

After building with CMake, the program is run by invoking the following command.

./project [path_to_fixedImage] [path_to_movingImage] [path_to_outputImage] {background_greyLevel_value} {path_to_beforeCheckerboard} {path_to_afterCheckerboard}

	Essentially the first 4 arguments are necessary to perform any MI Registration operation.
If the user desires a particular background grey level, then they optionally can set that as the 5th argument.
Similarly if the user desires to have a results for the results before and/or after the Checkerboard they can
designate a path for the 6th and 7th arguments respectively.

To process several images at a time the following bash command is necessary

for i in {000000..n};
do ./project [path_to_fixedImage] [path_to_movingImage/"$i".dcm] [path_to_outputImage/"$i".dcm];
done  
 
