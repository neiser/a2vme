// *******************************************************************
// *
// * Settings and Calibrations
// * For 450 MeV beam
// * Full Tagger?
// *
// *******************************************************************


//Settings and Errors
double MPt = 0.082; //valid from June 2010  //MPt = 0.077; //Value from Mai 2010
double DMPt = 0.00089;//Measured by Duncan ~9.6.2010
double MAlpha = 25*TMath::Pi()/180;
double DMAlpha = 0.1*TMath::Pi()/180; //Absolute Error of 0.1°, maybe due to an offset
double MAMIBeamEnergy = 450.; //In MeV

int TaggerSectionsUsedForFluxNormalisation[22] = {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0 };
						//      A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V

double * TaggerCalibrationEnergyActual;
double * TaggerCalibrationEnergyBiteActual;

// Valid for measurements 
// From 2011.02.11 to ?
void LoadMoellerCalibration() { 
	//Actual MAMI Energy
	TaggerCalibrationEnergyActual = TaggerCalibrationEnergy450;
	TaggerCalibrationEnergyBiteActual = TaggerCalibrationEnergyBite450; //Error on Energy, Energy width;

	//What is connected to the VUPROM modules
	ModuleLeftChStart[0] = 22;
	ModuleRightChStart[0] = 323;
	ModuleLeftChStart[1] = 70;
	ModuleRightChStart[1] = 292;
	ModuleLeftChStart[2] = 113;
	ModuleRightChStart[2] = 258-2; //offset in Tagger?
	ModuleLeftChStart[3] = 135;
	ModuleRightChStart[3] = 239-2; //offset in Tagger?


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
	CalibMoellerPairs[0][0] = 0;
	CalibMoellerPairs2nd[0][0] = 1;
	CalibMoellerPairs[0][1] = -1;
	CalibMoellerPairs[0][2] = 0;
	CalibMoellerPairs[0][3] = 0;
	CalibMoellerPairs[0][4] = 0;
	CalibMoellerPairs[0][5] = 0;
	CalibMoellerPairs2nd[0][5] = 1;
	CalibMoellerPairs[0][6] = 1;
	CalibMoellerPairs[0][7] = 1;
	CalibMoellerPairs[0][8] = 1;
	CalibMoellerPairs[0][9] = 1;


	//Module 1
	CalibMoellerPairs[1][0] = -1;
	CalibMoellerPairs[1][1] = -1;
	CalibMoellerPairs[1][2] = 0;
	CalibMoellerPairs[1][3] = 0;
	CalibMoellerPairs[1][4] = 0;
	CalibMoellerPairs[1][5] = 0;
	CalibMoellerPairs2nd[1][5] = 1;
	CalibMoellerPairs[1][6] = 0;
	CalibMoellerPairs2nd[1][6] = 1;
	CalibMoellerPairs[1][7] = 1;
	CalibMoellerPairs2nd[1][7] = 2;
	CalibMoellerPairs[1][8] = 1;
	CalibMoellerPairs2nd[1][8] = 2;
	CalibMoellerPairs[1][9] = 1;
	CalibMoellerPairs2nd[1][9] = 2;


	//Module 2
	CalibMoellerPairs[2][0] = 1;
	CalibMoellerPairs2nd[2][0] = 2;
	CalibMoellerPairs[2][1] = 1;
	CalibMoellerPairs2nd[2][1] = 2;
	CalibMoellerPairs[2][2] = 1;
	CalibMoellerPairs2nd[2][2] = 2;
	CalibMoellerPairs[2][3] = 2;
	CalibMoellerPairs[2][4] = 2;
	CalibMoellerPairs2nd[2][4] = 3;
	CalibMoellerPairs[2][5] = 2;
	CalibMoellerPairs[2][6] = 2;
	CalibMoellerPairs2nd[2][6] = 3;
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
	CalibMoellerPairs[3][2] = 2;
	CalibMoellerPairs[3][3] = 2;
	CalibMoellerPairs[3][4] = 2;
	CalibMoellerPairs[3][5] = 1;
	CalibMoellerPairs[3][6] = 3;
	CalibMoellerPairs[3][7] = 2;
	CalibMoellerPairs2nd[3][7] = 3;
	CalibMoellerPairs[3][8] = 2;
	CalibMoellerPairs2nd[3][8] = 3;
	CalibMoellerPairs[3][9] = 2;
	CalibMoellerPairs2nd[3][9] = 3;

}
