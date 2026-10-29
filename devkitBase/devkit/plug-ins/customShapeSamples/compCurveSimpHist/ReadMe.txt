
MayaAppWizard has created this compCurveSimpHist application for you.
  You can get regular updates for the Wizards at http://www.autodesk.com/developmaya
  and report issues at: oarxwiz-feedback@autodesk.com

This project uses the Maya devkit .props file from the Maya devkit to set the C++ compiler, MIDL, 
and Linker options appropriately. The project .props file is located in your project directory and reference
the Maya devkit and Maya executable paths.
To create a new configuration and/or change the paths references copy or modify the Autodesk.maya-2013.props
File, like this:
  #7 	<MayaDir Condition="'$(Platform)'=='x64'">[maya path on your x64 windows platform]</MayaDir>
  #8	<MayaDir Condition="'$(Platform)'=='Win32'">[maya path on your win32 windows platform]</MayaDir>
  
Note the Wizards installer also provided an Maya 2012 .props file for your convenience as example in
case you want to have multiple configuration (Checkout the Wizards install directory).
To undestand and use .props file, please visit the Microsoft WEB site at http://msdn.microsoft.com/en-us/library/a4xbdz1e.aspx
and http://blogs.msdn.com/b/visualstudio/archive/2010/05/14/a-guide-to-vcxproj-and-props-file-structure.aspx
In Visual Studio 2010, the .props editor is in -> View -> Other Windows -> Property Manager

-----------------------------------------------------------------------------
StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named compCurveSimpHist.pch and a precompiled types file named StdAfx.obj.

	StdAfx.h includes mayaHeaders.h which includes all the Maya headers, uses #pragma to
	import the required .lib file and exports mandatory symbols.
