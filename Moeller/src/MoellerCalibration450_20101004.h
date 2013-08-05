// *******************************************************************
// *
// * Settings and Calibrations
// * For 450 MeV beam
// *
// *******************************************************************


//Settings and Errors
double MPt = 0.082; //valid from June 2010  //MPt = 0.077; //Value from Mai 2010
double DMPt = 0.00089;//Measured by Duncan ~9.6.2010
double MAlpha = 25*TMath::Pi()/180;
double DMAlpha = 0.1*TMath::Pi()/180; //Absolute Error of 0.1°, maybe due to an offset

// Valid for measurements 
// From 2010.09.23-15.39 to 2010.10.03-07.49
void LoadMoellerCalibration() { 
	//What is connected to the VUPROM modules
	ModuleLeftChStart[0] = 22;
	ModuleRightChStart[0] = 323;
	ModuleLeftChStart[1] = 70;
	ModuleRightChStart[1] = 292;
	ModuleLeftChStart[2] = 113;
	ModuleRightChStart[2] = 258;
	ModuleLeftChStart[3] = 135;
	ModuleRightChStart[3] = 239;


	//Which Pairs should be used?
	//First: Reset everything
	for (int i=0; i < NumberOfModules;i++) {
		for (int k=0; k < NumberOfLeftChannels;k++) {
			CalibMoellerPairs[i][k] = -1; //Set to Malfunction
			CalibMoellerPairs2nd[i][k] = -1; //Set to Malfunction
		}
	}


	//Settings for 450MeV beam, last edited on 28.10.2010
	//Module 0
	CalibMoellerPairs[0][0] = 2;
	CalibMoellerPairs2nd[0][0] = 3;
	CalibMoellerPairs[0][1] = -1;
	CalibMoellerPairs2nd[0][1] = -1;
	CalibMoellerPairs[0][2] = 2;
	CalibMoellerPairs2nd[0][2] = 3;
	CalibMoellerPairs[0][3] = 2;
	CalibMoellerPairs2nd[0][3] = 3;
	CalibMoellerPairs[0][4] = 2;
	CalibMoellerPairs2nd[0][4] = 3;
	CalibMoellerPairs[0][5] = 2;
	CalibMoellerPairs2nd[0][5] = 3;
	CalibMoellerPairs[0][6] = 3;
	CalibMoellerPairs2nd[0][6] = 4;
	CalibMoellerPairs[0][7] = 3;
	CalibMoellerPairs2nd[0][7] = 4;
	CalibMoellerPairs[0][8] = 3;
	CalibMoellerPairs2nd[0][8] = 4;
	CalibMoellerPairs[0][9] = 4;
	CalibMoellerPairs2nd[0][9] = -1;

	//Module 1
	CalibMoellerPairs[1][0] = 1;
	CalibMoellerPairs2nd[1][0] = 2;
	CalibMoellerPairs[1][1] = 1;
	CalibMoellerPairs2nd[1][1] = 2;
	CalibMoellerPairs[1][2] = 1;
	CalibMoellerPairs2nd[1][2] = 2;
	CalibMoellerPairs[1][3] = 2;
	CalibMoellerPairs[1][4] = -1;
	CalibMoellerPairs[1][5] = -1;
	CalibMoellerPairs[1][6] = -1;
	CalibMoellerPairs[1][7] = 3;
	CalibMoellerPairs[1][8] = 3;
	CalibMoellerPairs2nd[1][8] = 4;
	CalibMoellerPairs[1][9] = 3;
	CalibMoellerPairs2nd[1][9] = 4;

	//Module 2
	CalibMoellerPairs[2][0] = 1;
	CalibMoellerPairs2nd[2][0] = 2;
	CalibMoellerPairs[2][1] = 1;
	CalibMoellerPairs2nd[2][1] = 2;
	CalibMoellerPairs[2][2] = 1;
	CalibMoellerPairs2nd[2][2] = 2;
	CalibMoellerPairs[2][3] = 2;
	CalibMoellerPairs[2][4] = 2;
	CalibMoellerPairs[2][5] = -1;
	CalibMoellerPairs[2][6] = 2;
	CalibMoellerPairs[2][7] = 2;
	CalibMoellerPairs2nd[2][7] = 3;
	CalibMoellerPairs[2][8] = 2;
	CalibMoellerPairs2nd[2][8] = 3;
	CalibMoellerPairs[2][9] = 2;
	CalibMoellerPairs2nd[2][9] = 3;

	//Module 3
	CalibMoellerPairs[3][0] = 1;
	CalibMoellerPairs2nd[3][0] = 2;
	CalibMoellerPairs[3][1] = 1;
	CalibMoellerPairs2nd[3][1] = 2;
	CalibMoellerPairs[3][2] = 1;
	CalibMoellerPairs2nd[3][2] = 2;
	CalibMoellerPairs[3][3] = 2;
	CalibMoellerPairs[3][4] = 2;
	CalibMoellerPairs[3][5] = 1;
	CalibMoellerPairs2nd[3][5] = 2;
	CalibMoellerPairs[3][6] = 3;
	CalibMoellerPairs[3][7] = 2;
	CalibMoellerPairs2nd[3][7] = 3;
	CalibMoellerPairs[3][8] = 2;
	CalibMoellerPairs2nd[3][8] = 3;
	CalibMoellerPairs[3][9] = 2;
	CalibMoellerPairs2nd[3][9] = 3;

}
