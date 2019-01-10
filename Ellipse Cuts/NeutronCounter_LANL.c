#include "TH2.h"
#include "TGraph.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TMath.h"
#include <fstream>
#include <string.h>


TCanvas *CPSDN;  

void Processor(char *basedir, char *subdir0, char *subdir1,int degree, char *rotaxis,char *source, float dist)
{

   
	//ifstream fI(fnameI, std::ifstream::in);

	int ipsd;
	int inrg;
	int icounts;
	double psd;
	double nrg;
	double counts;
	int numneutrons; 
	int neutron = 0;
	char header[1000];
	char rdata[100];
	char *data;
	int i = 0;
	int j = 0;
	int mod_idx = 0;

	TLine *ECut0[4];
	TLine *ECut1[4]; 
	TLine *PCut0[4]; 
	TLine *PCut1[4]; 
	TEllipse *CutEll[4];


	// MaxNRG *= 1.2;
	// MinNRG *= 0.8;
	double MaxNRG_C0[2][4] = { 143.60,  140.30,   142.51,   149.89,   134.67,   140.69,   147.52,  148.08};
	double MaxNRG_C1[2][4] = { 2.9196,  2.1951,   2.8573,   3.5465,   2.1884,   2.4109,   3.7654,  2.7265};
	double MinNRG_C0[2][4] = { 107.88,  105.47,   102.23,   104.80,   89.283,   102.45,   93.524,  99.865};
	double MinNRG_C1[2][4] = { 2.4292,  2.1978,   2.2977,   2.7017,   1.6472,   1.9591,   2.8101,  2.1276};
	double MaxPSD_C0[2][4] = { 0.3599,  0.3627,   0.3593,   0.3764,   0.4187,   0.34404,  0.3564,  0.3567};
	double MaxPSD_C1[2][4] = {-0.0022, -0.00355, -0.00255, -0.00391, -0.00406, -0.00185, -0.00300, -0.00197};
	double MaxPSD_C2[2][4] = { 4.49e-5, 7.62e-5,  5.24e-5,  8.57e-5,  1.06e-4,  3.86e-5,  6.92e-5,  3.53e-5};
	double MinPSD_C0[2][4] = { 0.10948, 0.13519,  0.10577,  0.15994,  0.19321,  0.11577,  0.11185,  0.11321};
	double MinPSD_C1[2][4] = { 0.00127, 9.41e-5,  0.00135,  6.56e-4,  5.65e-4,  0.00103,  0.00141,  0.00118};
	double MaxNRG = 0;
	double MinNRG = 0;
	double MaxPSD = 0;
	double MinPSD = 0;
	double brad = 0;
	double arad = 0;
	double mean_nrg;
	double mean_psd;

	TH2D *hPSD[4];
	TH2D *hPSD_Cut[4];

	char psdtitle[100];
	char psdname[100];
	char psdcuttitle[100];
	char psdcutname[100];
	char fpsdname[2000];
	char dirname[2000];
	int year;
	int mon;
	int day;
	int hour; 
	int min;
	int sec;
	int temp;
	int detc;
	int runtime;
	char runinfo0[100];
	char runinfo1[100];
	int posi;
	double nTOTAL = 0.0;

	sscanf(subdir1,"D%d_875_%døC_%s",&detc,&temp,&runinfo0);
	//cout<< runinfo0 << endl;
	posi = strcspn(runinfo0,"_");
	memmove(runinfo1,runinfo0+posi+1,strlen(runinfo0)-posi);
	//cout<<posi<<endl;
	//cout<<runinfo1;
	//getchar();
	sscanf(runinfo1,"%d_%d_%d_%d_%d_%d_%d",&runtime,&year,&mon,&day,&hour,&min,&sec);

	sprintf(dirname,"%s/%s/%s/",basedir,subdir0,subdir1);

	char sumfname[1000];
	sprintf(sumfname,"%s/NeutronCountSummary.dat",basedir);

	FILE * fO = fopen(sumfname, "a");
	// Det Date	Time Temp(C) Degree RunTime XorZ_Rot Source CH00 CH01 CH02 CH03
	fprintf(fO,"%d \t %d/%d/%d \t %d:%d:%d \t %d \t %d \t %d \t %s \t %s \t %g \t",detc,year,mon,day,hour,min,sec,temp,degree,runtime,rotaxis,source,dist);


	for (mod_idx=0; mod_idx<4; mod_idx++) { 
		numneutrons = 0;
		sprintf(fpsdname,"%sPSD%d.dat",dirname,mod_idx);
		sprintf(psdtitle,"PSD CH = %d, %s",mod_idx, subdir1);
		sprintf(psdname,"hPSD%d",mod_idx);
		sprintf(psdcuttitle,"PSD Cut CH = %d",mod_idx);
		sprintf(psdcutname,"hPSD_Cut%d",mod_idx);
		hPSD[mod_idx] = new TH2D(psdname,psdtitle,500,0,500,100,0,2);
		hPSD[mod_idx]->SetXTitle("Energy (Arb. Units)");
		hPSD[mod_idx]->SetYTitle("PSD");
		hPSD_Cut[mod_idx] = new TH2D(psdcutname,psdcuttitle,500,0,500,100,0,2);
		hPSD_Cut[mod_idx]->SetMarkerSize(0.5);

		
		MaxNRG = MaxNRG_C0[detc][mod_idx] + MaxNRG_C1[detc][mod_idx]*temp;
		MinNRG = MinNRG_C0[detc][mod_idx] + MinNRG_C1[detc][mod_idx]*temp;
		MaxPSD = MaxPSD_C0[detc][mod_idx] + MaxPSD_C1[detc][mod_idx]*temp + MaxPSD_C2[detc][mod_idx]*temp*temp;
		MinPSD = MinPSD_C0[detc][mod_idx] + MinPSD_C1[detc][mod_idx]*temp;
		MaxNRG *= 1.2;
		MinNRG *= 0.8;
		brad = (MaxPSD - MinPSD)/2.0;
		arad = (MaxNRG - MinNRG)/2.0;
		mean_nrg = (MaxNRG + MinNRG)/2.0;
		mean_psd = (MaxPSD + MinPSD)/2.0;


		ECut0[mod_idx] = new TLine(MinNRG,0.0, MinNRG, 2.0);
		ECut1[mod_idx] = new TLine(MaxNRG,0.0, MaxNRG, 2.0);
		PCut0[mod_idx] = new TLine(0  ,MinPSD, 350, MinPSD);
		PCut1[mod_idx] = new TLine(0  ,MaxPSD, 350, MaxPSD);
		CutEll[mod_idx] = new TEllipse(mean_nrg,mean_psd,arad,brad);
		ECut0[mod_idx]->SetLineWidth(2);
		ECut1[mod_idx]->SetLineWidth(2);
		PCut0[mod_idx]->SetLineWidth(2);
		PCut1[mod_idx]->SetLineWidth(2);
		CutEll[mod_idx]->SetLineWidth(2);
		CutEll[mod_idx]->SetFillStyle(0);

		// PSD Data
		FILE * fI = fopen(fpsdname, "r");
		
		for (i = 0; i<5; i++ ) { fscanf(fI,"%s",&header); }

		while ( !feof(fI)) {	
			fscanf(fI,"%d,%d,%d",&inrg,&ipsd,&icounts);
			psd = ((double)ipsd);
			nrg = ((double)inrg);
			counts = ((double)icounts);
			psd /= 99.0;
			psd *= 2.0;
			hPSD[mod_idx]->Fill(nrg,psd,counts);
			
			if ( (psd - mean_psd) <  brad/arad*sqrt(arad*arad - (nrg-mean_nrg)*(nrg-mean_nrg)) && 
				 (psd - mean_psd) > -brad/arad*sqrt(arad*arad - (nrg-mean_nrg)*(nrg-mean_nrg)) ) { 
					hPSD_Cut[mod_idx]->Fill(nrg,psd,counts); 
					numneutrons+=((int)counts); 
			}

		}
		fclose(fI);

		CPSDN->cd();
		CPSDN->SetGridy();
		CPSDN->SetGridx();
		hPSD[mod_idx]->SetAxisRange(0,350);
		hPSD[mod_idx]->Draw("COLZ");
		//hPSD_Cut[mod_idx]->Draw("Same");
		ECut0[mod_idx]->Draw("same");
		ECut1[mod_idx]->Draw("same");
		PCut0[mod_idx]->Draw("same");
		PCut1[mod_idx]->Draw("same");
		CutEll[mod_idx]->Draw("same");

		CPSDN->Update();
		for(j=0; j<10000; ) { j++; }
		nTOTAL += ((double)numneutrons);
		fprintf(fO," %d \t ",numneutrons);

	}
	
	fprintf(fO," %g \t %g ",nTOTAL,nTOTAL/((double)runtime));
	fprintf(fO,"\n");


	fclose(fO);


	for (mod_idx=0; mod_idx<4; mod_idx++) {  
		delete hPSD[mod_idx]; 
		delete hPSD_Cut[mod_idx]; 
		delete ECut0[mod_idx]; 
		delete ECut1[mod_idx]; 
		delete PCut0[mod_idx]; 
		delete PCut1[mod_idx];
		delete CutEll[mod_idx];
	}

	return;

	
}



void NeutronCounter_LANL(char *basedir)
{
	CPSDN = new TCanvas("CPSDN","CPSDN",800,600);
	CPSDN->SetWindowPosition(50,50);

	char sumfname[1000];
	sprintf(sumfname,"%s/NeutronCountSummary.dat",basedir);

	FILE * fO = fopen(sumfname, "w");
	fprintf(fO,"Det \t Date \t Time \t Temp (C) \t ");
	fprintf(fO,"Degree \t Run Time \t X or Z Rot \t Source \t Distance (m) \t");
	fprintf(fO,"CH00 \t  CH01 \t CH02 \t CH03 \t Total \t Rate (cps) \n");
	fclose(fO);


	// LANLCal_MiniNS_MONDAY
	// 2inchpoly, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/2inchpoly","D00_875_22øC_0deg_180_2018_12_03_14_30_57",0,"Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/2inchpoly","D01_875_22øC_0deg_180_2018_12_03_14_31_13",0,"Z","2InPoly",2.0);

	// LANLCal_MiniNS_TUESDAY
	// 2inchpoly, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_20øC_0deg_900_2018_12_04_06_36_57",  0,  "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_23øC_15deg_900_2018_12_04_07_06_32", 15, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_23øC_180deg_900_2018_12_04_09_03_32",180,"Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_24øC_180gd_900_2018_12_04_09_37_12", 180,"Z","2InPoly Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_24øC_30deg_900_2018_12_04_07_37_11", 30, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D00_875_25øC_45deg_900_2018_12_04_08_06_15", 45, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_20øC_0deg_900_2018_12_04_06_37_15",  0,  "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_23øC_15deg_900_2018_12_04_07_06_48", 15, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_23øC_180deg_900_2018_12_04_09_03_54",180,"Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_24øC_180gd_900_2018_12_04_09_37_29", 180,"Z","2InPoly Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_24øC_30deg_900_2018_12_04_07_37_29", 30, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/2inchpoly","D01_875_25øC_45deg_900_2018_12_04_08_06_32", 45, "Z","2InPoly",2.0);


	// LANLCal_MiniNS_WEDNESDAY
	// 2inchpoly, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D00_875_18øC_90deg_900_2018_12_05_12_41_03",     90,  "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D00_875_21øC_135deg_900_2018_12_05_13_09_31",    135, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D00_875_22øC_0degBOXss_900_2018_12_05_13_47_44", 0,   "Z","2InPoly SBox",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D00_875_23øC_0degIodine_900_2018_12_05_14_28_19",0,   "Z","2InPoly Iodine",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D00_875_24øC_60deg_900_2018_12_05_10_34_11",     60,  "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D01_875_18øC_90deg_900_2018_12_05_12_41_17",     90,  "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D01_875_21øC_135deg_900_2018_12_05_13_09_53",    135, "Z","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D01_875_23øC_0degBOXss_900_2018_12_05_13_48_00", 0,   "Z","2InPoly SBox",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D01_875_23øC_0degIodine_900_2018_12_05_14_28_37",0,   "Z","2InPoly Iodine",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyZrot","D01_875_25øC_60deg_900_2018_12_05_10_34_26",     60,  "Z","2InPoly",2.0);


	// LANLCal_MiniNS_WEDNESDAY
	// 2inchpoly, X, 2m
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyXrot","D00_875_22øC_0deg_900_2018_12_05_15_30_18",     0,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/2inchpolyXrot","D01_875_22øC_0deg_900_2018_12_05_15_30_33",     0,  "X","2InPoly",2.0);


	// LANLCal_MiniNS_THURSDAY
	// 2inchpoly, X, 2m
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_20øC_15deg_900_2018_12_06_06_23_52",     15,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_23øC_30deg_900_2018_12_06_06_51_30",     30,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_25øC_0degSC_900_2018_12_06_11_06_59",    0,   "X","2InPoly SC",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_25øC_45deg_900_2018_12_06_07_20_22",     45,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_135deg_900_2018_12_06_09_02_18",    135, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_150deg_900_2018_12_06_09_32_17",    150, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_180degGD_900_2018_12_06_10_31_54",  180, "X","2InPoly Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_180deg_900_2018_12_06_09_57_48",    180, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_60deg_900_2018_12_06_07_47_33",     60,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_75deg_900_2018_12_06_08_14_07",     75,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D00_875_26øC_90deg_900_2018_12_06_08_39_29",     90,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_20øC_15deg_900_2018_12_06_06_24_11",     15,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_23øC_30deg_900_2018_12_06_06_51_47",     30,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_24øC_45deg_900_2018_12_06_07_20_41",     45,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_0degSC_900_2018_12_06_11_06_03",    0,   "X","2InPoly SC",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_135deg_900_2018_12_06_09_02_34",    135, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_150deg_900_2018_12_06_09_32_32",    150, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_180degGD_900_2018_12_06_10_32_13",  180, "X","2InPoly Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_180deg_900_2018_12_06_09_58_05",    180, "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_60deg_900_2018_12_06_07_47_49",     60,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_25øC_75deg_900_2018_12_06_08_14_24",     75,  "X","2InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/2m2inchpolyXrot","D01_875_26øC_90deg_900_2018_12_06_08_39_45",     90,  "X","2InPoly",2.0);


	// LANLCal_MiniNS_THURSDAY
	// 2inchpoly, X, 1m
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_25øC_180deg1mgd_180_2018_12_06_15_45_44", 180, "X","2InPoly Gd",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_0deg1m_300_2018_12_06_13_41_33",     0,   "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_135deg1m_180_2018_12_06_15_03_04",   135, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_150deg1m_180_2018_12_06_15_13_15",   150, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_180deg1m_180_2018_12_06_14_00_41",   180, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_180deg1m_180_2018_12_06_15_24_22",   180, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_30deg1m_180_2018_12_06_14_12_26",    30,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_45deg1m_180_2018_12_06_14_23_40",    45,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_60deg1m_180_2018_12_06_14_34_42",    60,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_75deg1m_180_2018_12_06_14_44_51",    75,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D00_875_26øC_90deg1m_180_2018_12_06_14_54_42",    90,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_0deg1m_300_2018_12_06_13_41_47",     0,   "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_135deg1m_180_2018_12_06_15_03_18",   135, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_150deg1m_180_2018_12_06_15_13_31",   150, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_15deg1m_180_2018_12_06_14_00_59",    15,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_180deg1mgd_180_2018_12_06_15_46_03", 180, "X","2InPoly Gd",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_180deg1m_180_2018_12_06_15_24_38",   180, "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_30deg1m_180_2018_12_06_14_12_42",    30,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_45deg1m_180_2018_12_06_14_23_58",    45,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_60deg1m_180_2018_12_06_14_34_57",    60,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_75deg1m_180_2018_12_06_14_45_07",    75,  "X","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/1m2inchpolyXrot","D01_875_25øC_90deg1m_180_2018_12_06_14_54_57",    90,  "X","2InPoly",1.0);


	// LANLCal_MiniNS_FRIDAY
	// 2inchpoly, Z, 1m
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_19øC_0deg1mZ_180_2018_12_07_09_00_06",     0,   "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_20øC_0deg1mZbox_180_2018_12_07_09_15_29",  0,   "Z","2InPoly SBox",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_21øC_15deg1mZ_180_2018_12_07_09_23_41",    15,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_22øC_30deg1mZ_180_2018_12_07_09_36_02",    30,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_23øC_45deg1mZ_180_2018_12_07_09_48_26",    45,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_23øC_60deg1mZ_180_2018_12_07_10_00_14",    60,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_23øC_75deg1mZ_180_2018_12_07_10_11_31",    75,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_24øC_135deg1mZ_180_2018_12_07_10_33_06",   135, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_24øC_150deg1mZ_180_2018_12_07_10_45_11",   150, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_24øC_180deg1mZgd_180_2018_12_07_11_10_25", 180, "Z","2InPoly Gd",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_24øC_180deg1mZ_180_2018_12_07_10_57_52",   180, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D00_875_24øC_90deg1mZ_180_2018_12_07_10_23_17",    90,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_19øC_0deg1mz_180_2018_12_07_09_00_20",     0,   "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_20øC_0deg1mZbox_180_2018_12_07_09_15_54",  0,   "Z","2InPoly SBox",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_21øC_15deg1mZ_180_2018_12_07_09_23_58",    15,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_22øC_30deg1mZ_180_2018_12_07_09_36_20",    30,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_23øC_45deg1mZ_180_2018_12_07_09_48_40",    45,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_23øC_60deg1mZ_180_2018_12_07_10_00_29",    60,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_135deg1mZ_180_2018_12_07_10_33_22",   135, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_150deg1mZ_180_2018_12_07_10_45_27",   150, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_180deg1mZgd_180_2018_12_07_11_10_39", 180, "Z","2InPoly Gd",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_180deg1mZ_180_2018_12_07_10_58_12",   180, "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_75deg1mZ_180_2018_12_07_10_11_47",    75,  "Z","2InPoly",1.0);
	Processor(basedir, "LANLCal_MiniNS_FRIDAY/2inchpoly1mZrot","D01_875_24øC_90deg1mZ_180_2018_12_07_10_23_32",    90,  "Z","2InPoly",1.0);


	// LANLCal_MiniNS_MONDAY
	// 6inchpoly, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/6inchpoly","D00_875_22øC_0deg_180_2018_12_03_14_42_12",0,"Z","6InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/6inchpoly","D01_875_22øC_0deg_180_2018_12_03_14_42_31",0,"Z","6InPoly",2.0);


	// LANLCal_MiniNS_MONDAY
	// Background, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/background","D00_875_22øC_0deg_1800_2018_12_03_15_09_22",0,"Z","background",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/background","D01_875_22øC_0deg_1800_2018_12_03_15_09_35",0,"Z","background",2.0);

	// LANLCal_MiniNS_MONDAY
	// bareCf, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/bareCf","D00_875_21øC_0deg_180_2018_12_03_14_03_15",0,"Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/bareCf","D01_875_21øC_0deg_180_2018_12_03_14_03_32",0,"Z","bareCf",2.0);

	// LANLCal_MiniNS_MONDAY
	// bareshadowCf, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/bareshadowCf","D00_875_22øC_0deg_180_2018_12_03_14_58_26",0,"Z","bareshadowCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/bareshadowCf","D01_875_22øC_0deg_180_2018_12_03_14_58_50",0,"Z","bareshadowCf",2.0);

	// LANLCal_MiniNS_MONDAY
	// D2O, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_MONDAY/D20","D00_875_21øC_0deg_180_2018_12_03_14_18_59",0,"Z","D20",2.0);
	Processor(basedir, "LANLCal_MiniNS_MONDAY/D20","D01_875_21øC_0deg_180_2018_12_03_14_19_17",0,"Z","D20",2.0);

	// LANLCal_MiniNS_TUESDAY
	// bareCf, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D00_875_24øC_0degSC_1500_2018_12_04_14_24_38", 0, "Z","bareCf SC",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D00_875_24øC_0deg_1500_2018_12_04_13_37_59",   0, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D00_875_25øC_15deg_1500_2018_12_04_15_01_22",  15,"Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D01_875_24øC_0deg_1500_2018_12_04_13_38_13",   0, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D01_875_25øC_0degSC_1500_2018_12_04_14_24_54", 0, "Z","bareCf SC",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/bare","D01_875_25øC_15deg_1500_2018_12_04_15_01_38",  15,"Z","bareCf",2.0);

	// LANLCal_MiniNS_TUESDAY
	// D2O, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_21øC_45deg_900_2018_12_04_11_50_21", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_22øC_30deg_900_2018_12_04_12_17_38", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_23øC_15deg_900_2018_12_04_12_44_21", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_24øC_0deg_900_2018_12_04_13_11_13",  0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_24øC_180gd_900_2018_12_04_10_05_58", 0, "Z","D2O Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D00_875_25øC_180deg_900_2018_12_04_10_34_27",0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_21øC_45deg_900_2018_12_04_11_50_36", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_22øC_30deg_900_2018_12_04_12_17_56", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_23øC_15deg_900_2018_12_04_12_44_45", 0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_24øC_0deg_900_2018_12_04_13_11_26",  0, "Z","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_24øC_180gd_900_2018_12_04_10_06_21", 0, "Z","D2O Gd",2.0);
	Processor(basedir, "LANLCal_MiniNS_TUESDAY/D2O","D01_875_25øC_180deg_900_2018_12_04_10_34_44",0, "Z","D2O",2.0);


	// LANLCal_MiniNS_WEDNESDAY
	// bare, Z, 2m
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D00_875_20øC_30deg_1500_2018_12_05_06_29_26", 30, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D00_875_24øC_45deg_1500_2018_12_05_07_20_33", 45, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D00_875_24øC_90deg_1500_2018_12_05_09_46_41", 90, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D00_875_25øC_60deg_1500_2018_12_05_08_14_40", 60, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D01_875_20øC_30deg_1500_2018_12_05_06_29_44", 30, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D01_875_24øC_45deg_1500_2018_12_05_07_20_53", 45, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D01_875_24øC_90deg_1500_2018_12_05_09_47_00", 90, "Z","bareCf",2.0);
	Processor(basedir, "LANLCal_MiniNS_WEDNESDAY/bare","D01_875_25øC_60deg_1500_2018_12_05_08_15_09", 60, "Z","bareCf",2.0);


	// LANLCal_MiniNS_THURSDAY
	// 6inchpoly, X, 2m
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/6inXrot","D00_875_25øC_60deg_900_2018_12_06_12_52_45", 60, "X","6InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/6inXrot","D00_875_26øC_90deg_900_2018_12_06_13_16_37", 90, "X","6InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/6inXrot","D01_875_25øC_60deg_900_2018_12_06_12_53_01", 60, "X","6InPoly",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/6inXrot","D01_875_25øC_90deg_900_2018_12_06_13_17_04", 90, "X","6InPoly",2.0);


	// LANLCal_MiniNS_THURSDAY
	// D2O, X, 2m
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D00_875_26øC_0degBox_900_2018_12_06_12_29_47", 0,  "X","D2O SBox",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D00_875_26øC_60deg_900_2018_12_06_11_30_52",   60, "X","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D00_875_26øC_90deg_900_2018_12_06_11_55_37",   90, "X","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D01_875_25øC_0degBox_900_2018_12_06_12_30_08", 0,  "X","D2O SBox",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D01_875_25øC_60deg_900_2018_12_06_11_31_16",   60, "X","D2O",2.0);
	Processor(basedir, "LANLCal_MiniNS_THURSDAY/D2O2mXrot","D01_875_25øC_90deg_900_2018_12_06_11_55_53",   90, "X","D2O",2.0);
	

	return;
}


