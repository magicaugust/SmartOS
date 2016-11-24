var build = Builder.Create("GCCArm");
build.Init();
build.CPU = "Cortex-M3";
build.Linux = true;
build.Defines.Add("STM32F1");
build.AddIncludes("..\\..\\..\\Lib\\CMSIS");
build.AddIncludes("..\\..\\..\\Lib\\Inc");
build.AddIncludes("..\\..\\", false);
build.AddFiles(".", "*.c;*.cpp;startup_stm32f10x_gcc.S");
build.AddFiles("..\\CortexM", "*.c;*.cpp");
build.Libs.Clear();
build.CompileAll();
build.BuildLib("..\\..\\libSmartOS_F1.a");

build.Debug = true;
build.CompileAll();
build.BuildLib("..\\..\\libSmartOS_F1.a");
