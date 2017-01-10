var build = Builder.Create("MDK");
build.Init();
build.CPU = "Cortex-M4F";
build.Defines.Add("STM32F4");
build.AddIncludes("..\\", false);
build.AddFiles("..\\Core");
build.AddFiles("..\\Kernel");
build.AddFiles("..\\Device");
build.AddFiles("..\\", "*.c;*.cpp", false);
build.AddFiles("..\\Net");
build.AddFiles("..\\Message");
build.AddFiles("..\\Security", "*.cpp");
build.AddFiles("..\\Board");
build.AddFiles("..\\Storage");
build.AddFiles("..\\App");
build.AddFiles("..\\Drivers");
build.AddFiles("..\\Test");
build.AddFiles("..\\TinyIP", "*.c;*.cpp", false, "HttpClient");
build.AddFiles("..\\TinyNet");
build.AddFiles("..\\TokenNet");
build.Libs.Clear();
build.CompileAll();
build.BuildLib("..\\SmartOS_M4");

build.Debug = true;
build.CompileAll();
build.BuildLib("..\\SmartOS_M4");