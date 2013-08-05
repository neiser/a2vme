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
	TaggerCalibrationEnergyActual = TaggerCalibrationEnergy1557;
	TaggerCalibrationEnergyBiteActual = TaggerCalibrationEnergyBite1557; //Error on Energy, Energy width;

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


	//Settings for 1557MeV beam, last edited on 15.7.2013
	//Module 2
	CalibMoellerPairs[2][0]    = 1;
	CalibMoellerPairs2nd[2][0] = 2;
	CalibMoellerPairs[2][1]    = -1;
	CalibMoellerPairs2nd[2][1] = -1;
	CalibMoellerPairs[2][2]    = -1;
	CalibMoellerPairs2nd[2][2] = -1;
	CalibMoellerPairs[2][3]    = 2;
	CalibMoellerPairs2nd[2][3] = 3;
	CalibMoellerPairs[2][4]    = 2;
	CalibMoellerPairs2nd[2][4] = 3;
	CalibMoellerPairs[2][5]    = 2;
	CalibMoellerPairs2nd[2][5] = 3;
	CalibMoellerPairs[2][6]    = 2;
	CalibMoellerPairs2nd[2][6] = 3;
	CalibMoellerPairs[2][7]    = 2;
	CalibMoellerPairs2nd[2][7] = 3;
	CalibMoellerPairs[2][8]    = 3;
	CalibMoellerPairs2nd[2][8] = 4;
	CalibMoellerPairs[2][9]    = 3;
	CalibMoellerPairs2nd[2][9] = 4;

	//Module 3
	CalibMoellerPairs[3][0]    = 2;
	CalibMoellerPairs2nd[3][0] = 3;
	CalibMoellerPairs[3][1]    = 3;
	CalibMoellerPairs2nd[3][1] = -1;
	CalibMoellerPairs[3][2]    = -1;
	CalibMoellerPairs[3][3]    = -1;
	CalibMoellerPairs[3][4]    = 3;
	CalibMoellerPairs2nd[3][4] = -1;
	CalibMoellerPairs[3][5]    = 3;
	CalibMoellerPairs2nd[3][5] = 4;
	CalibMoellerPairs[3][6]    = 3;
	CalibMoellerPairs[3][7]    = 3;
	CalibMoellerPairs2nd[3][7] = -1;
	CalibMoellerPairs[3][8]    = 3;
	CalibMoellerPairs2nd[3][8] = 4;
	CalibMoellerPairs[3][9]    = 3;
	CalibMoellerPairs2nd[3][9] = 4;

}
