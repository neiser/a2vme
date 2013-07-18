while true;
	 do
		sysctl hw.acpi.thermal.tz0.temperature ;
		sysctl hw.acpi.thermal.tz1.temperature ;
		sysctl dev.cpu.0.freq ;
	done
