// mmmunki.cpp: определяет точку входа для консольного приложения.
//

//#include "stdafx.h"

#include "vld.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>

#include "MultiMeasure.h"
#include "lib/ArgyllCMS/ArgyllColorMath.h"
#include "lib/ColorTemperature.h"

using namespace std;

float R(float f) { return (float)floor(f*1000+0.5)/1000; }

float R2(float f) { return (float)floor(f*100+0.5)/100; }


// Displays float number with color depends of its
// value and optionaly on white background.
// Used to warn when delta E values is too high.
void D(float fValue, bool bBG = false)
{
	HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
	int nColor = 7;
	if (fValue >= 0.2f) 
		nColor = 4;
	else {
		if (fValue >= 0.1f)
			nColor = 6;
	}
	if(bBG) 
		nColor += 128;
	SetConsoleTextAttribute(hstdout, nColor);
	cout << R(fValue);
	SetConsoleTextAttribute(hstdout, 7);
}

int main(int argc, char* argv[])
{
	cout << "MultiMeasure Munki - measurement utility for ColorMunki Photo\nAuthor: Sergey Korpakov" << endl;

////////////////////////////////////////////////////
//	Ambient light measurement mode
	if((argc == 3) && (argv[1][0] == '-') && (argv[1][1] == 'l'))
	{
		// Inits ColorMunki device
		CMultiMeasureColorMunkiXRite MunkiDevice;
		if(!MunkiDevice.IsConnected()) { 
			cout << "\n\nColorMunki Device is not connected" << endl; 
			MunkiDevice.Reset(); 
			return 0;
		}

		// Calibrates device
		cout << "\n\nSet ColorMunki to calibration position then press button to calibrate." << endl;
		MunkiDevice.WaitForButton();
		cout << "Calibrating... ";
		if(MunkiDevice.Calibrate(MUNKI_SINGLE_AMBIENT_LIGHT) != eNoError) { 
			cout << " Calibration failed" << endl; 
			MunkiDevice.Reset(); 
			return 0; 
		}
		cout << " OK" << endl;
		cout << "\nSet ColorMunki to ambient light measurement position and\npress device button to make measurement\nor press 'q' to quit." << endl;

		stringstream sout; 
		sout << "ILLUMINATION_NAME Emission\n";
		sout << "BEGIN_DATA_FORMAT\n";
		sout << "ColorTemp\tSampleID\tSAMPLE_NAME\tnm380\tnm390\tnm400\tnm410\tnm420\tnm430\tnm440\tnm450\tnm460\tnm470\tnm480\tnm490\tnm500\tnm510\tnm520\tnm530\tnm540\tnm550\tnm560\tnm570\tnm580\tnm590\tnm600\tnm610\tnm620\tnm630\tnm640\tnm650\tnm660\tnm670\tnm680\tnm690\tnm700\tnm710\tnm720\tnm730\n";
		sout << "END_DATA_FORMAT\n";
		sout << "BEGIN_DATA\n";

		float faData[MM_COLOR_DATA_SIZE], *faSpectrum;
		unsigned short uSpectrumSize;
		unsigned short uSampleID = 1;
		while(1)
		{
			char cKey = 0;
			MunkiDevice.WaitForButton(cKey);
			if(cKey == 'q') 
				break;
			unsigned short uResult = MunkiDevice.DoMeasurements(); 
			if(uResult == eNoError) 
			{
				Beep(1200,200);
				uResult = MunkiDevice.GetResult(0, faData, uSpectrumSize, faSpectrum);
				if(faSpectrum != NULL)
				{
					double XYZ[3] = { faData[MM_XYZ_X], faData[MM_XYZ_Y], faData[MM_XYZ_Z]}, fColorTemp;;
					XYZtoCorColorTemp(XYZ, &fColorTemp);
					sout << floor(fColorTemp) << "K"; 
					cout << floor(fColorTemp) << "K";

					sout << "\t" << uSampleID << "\tLight_Source_" << uSampleID;
					for(unsigned short i = 0; i < uSpectrumSize; i++)
					{
						sout << "\t" << faSpectrum[i];
						cout << "\t" << R(faSpectrum[i]);
					}
					sout << endl;
					cout << endl;
					delete faSpectrum;
				}
				uSampleID++;
			}
			else
			{
				Beep(800, 200); Sleep(350); Beep(800,200);
				if(uResult == eWrongSensorPosition) 
					cout << "ERROR: Wrong Sensor Position" << endl;
				else 
					cout << "Unknown error" << endl;
			}
		}

		sout << "END_DATA\n";

		MunkiDevice.Reset();

		string strLogFile = argv[2];
		FILE* fp;
		if (fopen_s(&fp, strLogFile.c_str(), "w") == 0)
		{
			fputs(sout.str().c_str(), fp);
			fclose(fp);
		}

		return 0;
	}

////////////////////////////////////////////////////
//	Color chart measurement mode

	int nCurrentArg = 1;
	bool bTransposeReference = false;
	unsigned short uOutputFormats = 0; // MM_FILE_TYPE_PM5 | MM_FILE_TYPE_PM5_COLORBASE | MM_FILE_TYPE_TI3 | MM_FILE_TYPE_PM5_DENSITY;
	unsigned short uAdvanceAfter = 0;
	vector<stringext> strMeasurementFileNames;
	unsigned short uOutputDeviceData = 0;
	unsigned short uDeltaEStandart = MM_DELTA_E_2000;

	while((nCurrentArg < argc) && (argv[nCurrentArg][0] == '-'))
	{
		stringext strArg = argv[nCurrentArg];
		if(strArg.substr(1) == "t") bTransposeReference = true;
		else if(strArg.substr(1) == "PM5") uOutputFormats |= MM_FILE_TYPE_PM5;
		else if(strArg.substr(1) == "CB") uOutputFormats |= MM_FILE_TYPE_PM5_COLORBASE;
		else if(strArg.substr(1) == "TI3") uOutputFormats |= MM_FILE_TYPE_TI3;
		else if(strArg.substr(1) == "PM5D") uOutputFormats |= MM_FILE_TYPE_PM5_DENSITY;
		else if(strArg.substr(1, 1) == "M") strMeasurementFileNames = ((stringext)strArg.substr(2)).split(",");
		else if(strArg.substr(1, 1) == "A") uAdvanceAfter = stoi(strArg.substr(2));
		else if(strArg.substr(1, 1) == "D") { if(strArg.substr(2) == "RGB") uOutputDeviceData = MM_DD_RGB; if(strArg.substr(2) == "CMYK") uOutputDeviceData = MM_DD_CMYK; }
		else if(strArg.substr(1, 1) == "E") { if(strArg.substr(2) == "76") uDeltaEStandart = MM_DELTA_E_76; if (strArg.substr(2) == "94") uDeltaEStandart = MM_DELTA_E_94; }
		nCurrentArg++;
	}

	if((argc < 3) || (nCurrentArg > (argc - 2)))
	{
		cout << "usage:" << endl << endl;
		cout << "  Ambient light measurement mode:  mmmunki -l <measurements file>" << endl << endl;
		cout << "  Color chart measurement mode:  mmmunki [options] <reference file> <measurements file>" << endl;
		cout << "    <reference file> - input reference chart file" << endl;
		cout << "    <measurements file> - output measurements file name (without extension)" << endl;
		cout << "    options:" << endl;
		cout << "      -t - transpose reference data" << endl;
		cout << "      -PM5 - save measurements data in ProfileMaker 5 format" << endl;
		cout << "      -CB - save measurements data in Epson ColorBase format" << endl;
		cout << "      -TI3 - save measurements data in Agryll TI3 format" << endl;
		cout << "      -PM5D - save measurements data in ProfileMaker 5 format with density values" << endl;
		cout << "      -M<input measurements file1>[,<input measurements file2>[,<input measurements file3> ... ]] - read measurements data from files" << endl;
		cout << "      -A<N> - automatically advance to next strip after N measurements (default: 1)" << endl;
		cout << "      -D<RGB|CMYK> - save measurements data values in RGB|CMYK format (only supported for ProfileMaker 5 file format)" << endl;
		cout << "      -E<76|94|2000> - set delta E calculation method to CIE76|CIE94|CIEDE2000 (default: CIEDE2000)" << endl;

		return 0;
	}

	string strInFileName = argv[nCurrentArg], strOutFileName = argv[nCurrentArg + 1];

	// Inits ColorMunki device
	CMultiMeasureColorMunkiXRite MunkiDevice;
	if (!MunkiDevice.IsConnected()) { cout << "\n\nColorMunki Device is not connected" << endl; MunkiDevice.Reset(); return 0; }

	// Reads input reference chart file
	CMultiMeasure MMMain((CMultiMeasureDevice *)&MunkiDevice);
	if(MMMain.ReadReferenceFile(strInFileName) == MM_FILE_TYPE_NONE) { cout << "\n\nCannot read input reference chart file" << endl; return 0;  } else cout << "\n\nReference data from " << strInFileName << " has been read" << endl;
	// Reads input measurements files if specified
	for(unsigned short i = 0; i < strMeasurementFileNames.size(); i++) 
		if(MMMain.ReadMeasurementsFile(strMeasurementFileNames[i], uDeltaEStandart) != MM_OK) { cout << "\nCannot read input measurements file " << strMeasurementFileNames[i] << " or it doesn't match reference file" << endl; return 0;  }
		else cout << "\nMeasurement data from " << strMeasurementFileNames[i] << " has been read" << endl;
	// Transposes strips if specified; usually needed for ProfileMaker 5 format
	if(bTransposeReference) MMMain.TransposeStrips();

	// Calibrates device
	cout << "\n\nSet ColorMunki to calibration position then press button to calibrate." << endl;
	MunkiDevice.WaitForButton();
	cout << "Calibrating... ";
	if(MunkiDevice.Calibrate(MUNKI_SCANNING_REFLECTANCE) != eNoError) { cout << " Calibration failed" << endl; MunkiDevice.Reset(); return 0; }
	cout << " OK" << endl;
	MunkiDevice.SetMeasurementMode(MUNKI_PATCH_RECOGNITION_CORRELATION);

	cout << "\nSet ColorMunki to reflectance substrate position." << endl;

	unsigned short uCurrentStrip = 0, uCurrentPatch = 0, uPasses = 1;
	bool bStripCorrelationMode = true, bPatchMode = false, bDisplayAll = true, bPassesReverse = false, bAdvanceNext = false;
		
	// Uploads the reference data of the first strip to device;
	MMMain.SetReferenseLine(uCurrentStrip);

	while(1) // Main input loop
	{
		// Displays various information about the current strip, measured and statistical data, as well as a list of keyboard commands
		cout << "---------------------------------------------------------------------------------------" << endl;
		cout << "Reading Strip " << (uCurrentStrip + 1) << ":";
		for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) 
		{
			cout << "\t" << (bPatchMode && (p == uCurrentPatch) ? "|" : "");
			cout << (MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p] != NULL ? MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_strRefName : "");
		}
		cout << endl << "Strip Measurements Count: " << MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount;
		if(MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount > 1)
		{
			
			if(bDisplayAll)
			{
				cout << "\nPass\tMax\tAverage\tPer Patch DeltaE (form Best)";
				for(unsigned short m = 0; m < MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m++)
				{
					cout << endl << (m + 1) << "|" << R2(MMMain.m_pStrips[uCurrentStrip]->m_faMeasureTime[m]) << "\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_faMaxDeltaE[m]); cout << "\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_faAverageDeltaE[m]);
					for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) 
					{ 
						cout << "\t" << (bPatchMode && (p == uCurrentPatch) ? "|" : "");
						D(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_pMeasureData[m]->m_fFromBestByDistDeltaE, m == MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uWorstFromBestByDistMeasure); 
					}
//					cout << "\t|" << R(MMMain.m_pStrips[uCurrentStrip]->m_faMeasureTime[m]) << " sec";
				}
				unsigned short uMaxMeasuresCount = 0;
				for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) 
					if(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uMeasuresCount > uMaxMeasuresCount) uMaxMeasuresCount = MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uMeasuresCount;
				for(unsigned short m = MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m < uMaxMeasuresCount; m++)
				{
					cout << "\n\t\t";
					for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++)
					{
						cout << "\t" << (bPatchMode && (p == uCurrentPatch) ? "|" : "");
						if(m < MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uMeasuresCount)
							D(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_pMeasureData[m]->m_fFromBestByDistDeltaE, m == MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uWorstFromBestByDistMeasure); 
					}
				}
				cout << "\nPer Patch Average DeltaE:\n\t\t";
				for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) 
				{ 
					cout << "\t" << (bPatchMode && (p == uCurrentPatch) ? "|" : "");
					D(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_fFromBestByDistAverageDeltaE); 
				}
				cout << "\nTotal\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_fTotalMaxDeltaE); cout << "\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_fTotalAverageDeltaE);
				cout << "\nBest\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_fMaxBestDeltaE); cout << "\t"; D(MMMain.m_pStrips[uCurrentStrip]->m_fAverageBestDeltaE); cout << "\t(from Average)";
			}
			else
			{
				cout << "\tCurrent DeltaE (from Best):" << endl;
				cout << "\tBest\tTotal"; for(unsigned short m = 0; m < MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m++) cout << "\tPass " << (m + 1);
				cout << "\nMax\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_fMaxBestDeltaE) << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_fTotalMaxDeltaE);
				for(unsigned short m = 0; m < MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m++) cout << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_faMaxDeltaE[m]);
				cout << "\nAverage\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_fAverageBestDeltaE) << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_fTotalAverageDeltaE);
				for(unsigned short m = 0; m < MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m++) cout << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_faAverageDeltaE[m]);
				cout << "\nMeasure Time\t"; for(unsigned short m = 0; m < MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount; m++) cout << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_faMeasureTime[m]);
			}
//			cout << "\nBest,Worst,BfW Measure:"; for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) cout << "\t" << (MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uBestMeasure + 1) << "," << (MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uWorstMeasure + 1) << "," << (MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_uWorstFromBestMeasure + 1);
			cout << "\nBest DeltaE per Patch:"; for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) cout << "\t" << R(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[p]->m_fBestByDistDeltaE);
		}
		else if(MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount == 1) cout << "\nMeasure Time:" << R(MMMain.m_pStrips[uCurrentStrip]->m_faMeasureTime[0]);
		cout << "\n\nDelta E calculation method: " << (uDeltaEStandart == MM_DELTA_E_76 ? "CIE76" : (uDeltaEStandart == MM_DELTA_E_94 ? "CIE94" : "CIEDE2000"));
		cout << "\nMeasuring Mode: "; 
		if(bPatchMode) cout << "Patch"; 
		else 
		{ 
			cout << "Strip, Correlation: " << (bStripCorrelationMode ? "ON" : "OFF"); 
			if(uPasses > 1) cout << ", " << uPasses << " passes" << (bPassesReverse ? " (reverse order)" : ""); 
			if(uAdvanceAfter > 0) cout << ", Advance after " << uAdvanceAfter << " measure(s)";
		}
		unsigned short uStripDoneCount = 0;
		for(unsigned short s = 0; s < MMMain.m_uStripsCount; s++) if(MMMain.m_pStrips[s]->m_uStripMeasuresCount > 0) uStripDoneCount++;
		if(uStripDoneCount < MMMain.m_uStripsCount) cout << "\t\tStrips measured: " << uStripDoneCount << ", left to measure: " << (MMMain.m_uStripsCount - uStripDoneCount) << endl;
		else cout << "\t\tAll " << MMMain.m_uStripsCount << " strips has been measured!!!" << endl;
		cout << "Press: 'x' to switch between strip and patch mode,\n";
		if(!bPatchMode) cout << "'c' to on/off strip correlation mode,\n";
		cout << "'f' to move to next strip, 'b' to move to previous strip,\n";
		if(bPatchMode) cout << "'n' to move to previous patch, 'm' to move to next patch,\n";
		cout << "'d' to save measurements to file and exit, 'q' to quit without saving,\n";
		cout << "'s' to save measurements to specified file without quit,\n";
		cout << "'e' to erase all measurements in current strip,\n";
		cout << "'a' to erase all measurements in all strips,\n";
		cout << "'1' to '9' to erase corresponding measurement in current strip,\n";
		cout << "'w' to erase worst measurements in current " << (bPatchMode ? "patch" : "strip") << ",\n";
		cout << "'z' to switch display mode, 't' to transpose strips,\n";
//		cout << "'u' to switch delta E calculation method,\n";
		cout << "'+'/'-' to increase/decrease passes count, '*' to reverse passes order.\n";
		cout << "Press ColorMunki button to start measuring current strip.";
		if(uPasses > 1)
		{
			cout << " Passes:";
			for(short pass = (bPassesReverse ? uPasses - 1 : 0); bPassesReverse ? pass >= 0 : pass < uPasses; bPassesReverse ? pass-- : pass++)
			{
				unsigned short uPatchesNumber = (unsigned short)floor((double)MMMain.m_pStrips[uCurrentStrip]->m_uStripSize / uPasses);
				unsigned short uPatchesRemainder = MMMain.m_pStrips[uCurrentStrip]->m_uStripSize - uPatchesNumber * uPasses;
				unsigned short uPatchFrom = pass * uPatchesNumber + (pass < uPatchesRemainder ? pass : uPatchesRemainder);
				uPatchesNumber += (pass < uPatchesRemainder);
				cout << " " << MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uPatchFrom]->m_strRefName << "-" << MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uPatchFrom + uPatchesNumber - 1]->m_strRefName;
			}
		}
		cout << endl << endl;

		// Auto advance to next strip
		if(bAdvanceNext)
		{
			bAdvanceNext = false;
			if(uCurrentStrip < (MMMain.m_uStripsCount - 1)) uCurrentStrip++; else uCurrentStrip = 0; 
			if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip); 
			uCurrentPatch = 0;
			continue; 
		}

		// Wait for device button or keyboard key press
		char cKey = 0;
		MunkiDevice.WaitForButton(cKey);
		if(cKey == 'f') // move to next strip
		{ 
			if(uCurrentStrip < (MMMain.m_uStripsCount - 1)) uCurrentStrip++; else uCurrentStrip = 0; 
			if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip); 
			uCurrentPatch = 0;
			continue; 
		}
		if(cKey == 'b') // move to previous strip
		{ 
			if(uCurrentStrip > 0) uCurrentStrip--; else uCurrentStrip = MMMain.m_uStripsCount - 1; 
			if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip); 
			uCurrentPatch = 0;
			continue; 
		}
		if((cKey == 'm') && bPatchMode) // move to next patch
		{ 
			if(uCurrentPatch < (MMMain.m_pStrips[uCurrentStrip]->m_uStripSize - 1)) uCurrentPatch++; else uCurrentPatch = 0; 
		}
		if((cKey == 'n') && bPatchMode) // move to previous patch
		{ 
			if(uCurrentPatch > 0) uCurrentPatch--; else uCurrentPatch = MMMain.m_pStrips[uCurrentStrip]->m_uStripSize - 1; 
		}
		if(cKey == 'd') // save measurements to file and exit
		{
			cout << "Save measurements to file and exit? (y/n): ";
			string strTemp;
			cin >> strTemp;
			if(strTemp[0] == 'y')
			{
				cout << "Writing measurement data to " << strOutFileName << endl;
				MMMain.WriteMeasurementFile(strOutFileName, uOutputFormats, uOutputDeviceData);
				break;
			}
			else
				continue;
		}
		if(cKey == 'q') // quit without saving
		{
			cout << "Are you sure you want to quit without saving? (y/n): ";
			string strTemp;
			cin >> strTemp;
			if(strTemp[0] == 'y') break; else continue;
		}
		if((cKey == 'c') && (!bPatchMode)) // switch strip correlation mode on or off
		{
			bStripCorrelationMode = !bStripCorrelationMode; 
			MunkiDevice.SetMeasurementMode(bStripCorrelationMode ? MUNKI_PATCH_RECOGNITION_CORRELATION : MUNKI_PATCH_RECOGNITION_BASIC);
			if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip);
			continue;
		}
		if(cKey == 's') // save measurements to specified file without quit
		{
			string strTempFileName;
			cout << "Enter file name: ";
			cin >> strTempFileName;
			cout << "Writing measurement data to " << strTempFileName << "... ";
			MMMain.WriteMeasurementFile(strTempFileName, uOutputFormats, uOutputDeviceData);
			cout << "Done" << endl;
			continue;
		}
		if(cKey == 'e') // erase all measurements in current strip
		{ 
			MMMain.m_pStrips[uCurrentStrip]->EraseMeasurements(); 
			continue; 
		}
		if(cKey == 'a') // erase all measurements in all strip
		{
			cout << "Are you sure you want to erase all measurements in all strips? (y/n): ";
			string strTemp;
			cin >> strTemp;
			if(strTemp[0] == 'y')
				for(unsigned short s = 0; s < MMMain.m_uStripsCount; s++)
					MMMain.m_pStrips[s]->EraseMeasurements();
			cout << "All measurements in all strips has been erased\n";
			continue;
		}
		if((cKey >= '1') && (cKey <='9')) // erase specified measurement in current strip
		{ 
			MMMain.m_pStrips[uCurrentStrip]->EraseMeasurement((cKey - '0') - 1); 
			continue; 
		}
		if(cKey == 'w') // erase statisticaly worst measurement in the current patch or strip
		{ 
			if(bPatchMode)
			{
				if(MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uCurrentPatch]->m_uMeasuresCount > MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount)
				{
					MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uCurrentPatch]->EraseWorstMeasurement(true); 
					MMMain.m_pStrips[uCurrentStrip]->CalculateStatistic();
				}
			}
			else MMMain.m_pStrips[uCurrentStrip]->EraseWorstMeasurement(true); 
			continue; 
		}
		if(cKey == 'z') // switch between short and detailed display mode
		{ 
			bDisplayAll = !bDisplayAll; 
			continue; 
		}
		if(cKey == 'x') // switch between patch and strip measurement mode
		{ 
			bPatchMode = !bPatchMode;
			if(bPatchMode)
			{
				MunkiDevice.SetMeasurementMode(MUNKI_SINGLE_REFLECTANCE);
			}
			else
			{
				MunkiDevice.SetMeasurementMode(bStripCorrelationMode ? MUNKI_PATCH_RECOGNITION_CORRELATION : MUNKI_PATCH_RECOGNITION_BASIC);
				if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip);
			}
			continue;
		}
		if((cKey == '+') && (uPasses < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize)) // increase the number of passes in the multipass strip measurement mode
		{ 
			uPasses++; 
			continue; 
		}
		if((cKey == '-') && (uPasses > 1)) // decrease the number of passes in the multipass strip measurement mode
		{ 
			uPasses--; 
			if(uPasses == 1) MMMain.SetReferenseLine(uCurrentStrip); 
			continue; 
		}
		if(cKey == '*') // reverse order of patch blocks in the multipass strip measurement mode
		{ 
			bPassesReverse = !bPassesReverse; 
			continue; 
		}
		if(cKey == 't') // Transpose strips
		{ 
			MMMain.TransposeStrips(); 
			continue; 
		}
/*		if (cKey == 'u') // Switch delta E calculation method
		{
			uDeltaEStandart++;
			if (uDeltaEStandart > MM_DELTA_E_2000)
				uDeltaEStandart = MM_DELTA_E_76;
			MMMain.RecalculateStatistic(uDeltaEStandart);
		}*/

		if(cKey != 0) continue;

		// If it's not a keyboard key than it's the device button pressed
		unsigned short uResult;
		if(bPatchMode)
		{
			// Do measurement in patch mode
			Beep(1000, 200);
			uResult = MMMain.MeasurePatch(uCurrentStrip, uCurrentPatch, uDeltaEStandart);
			if(uResult == MM_OK) { Beep(1200,200); cout << "OK" << endl; }
		}
		else
		{
			// Do measurement in strip mode

			// Colormunki Photo has another one disadvantage. It has relatively small buffer memory to store measurement data.
			// Therefore, when you try to measure a long strip with a standard patch size and slowly move the device 
			// to increase the number of readings, the buffer may overflow and some data will be lost.
			// To avoid this, you can measure the long strip in parts. 
			// This code split the strip into a specified number of blocks and measure each block individually in direct or reverse order.
			float fReadingTimeTotal = 0.0f, fReadingTimeCurrentPass;
			CMultiMeasureColorData **pMeasureData = new CMultiMeasureColorData *[MMMain.m_pStrips[uCurrentStrip]->m_uStripSize];
			for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) pMeasureData[p] = NULL;
			for(short pass = (bPassesReverse ? uPasses - 1 : 0); bPassesReverse ? pass >= 0 : pass < uPasses; bPassesReverse ? pass-- : pass++)
			{
				unsigned short uPatchesNumber = (unsigned short)floor((double)MMMain.m_pStrips[uCurrentStrip]->m_uStripSize / uPasses);
				unsigned short uPatchesRemainder = MMMain.m_pStrips[uCurrentStrip]->m_uStripSize - uPatchesNumber * uPasses;
				unsigned short uPatchFrom = pass * uPatchesNumber + (pass < uPatchesRemainder ? pass : uPatchesRemainder);
				uPatchesNumber += (pass < uPatchesRemainder);
				cout << "Reading patches " << MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uPatchFrom]->m_strRefName << "-" << MMMain.m_pStrips[uCurrentStrip]->m_pStripPatches[uPatchFrom + uPatchesNumber - 1]->m_strRefName << ".	";
				if(fReadingTimeTotal > 0.0f) MunkiDevice.WaitForButton();
				if(bStripCorrelationMode) MMMain.SetReferenseLine(uCurrentStrip, uPatchFrom, uPatchesNumber);
				cout << "Start scanning... ";
				Beep(1000, 200);
				uResult = MMMain.MeasureStripPatches(uCurrentStrip, pMeasureData, fReadingTimeCurrentPass, uPatchFrom, uPatchesNumber);
				if(uResult == MM_OK)
				{ 
					fReadingTimeTotal += fReadingTimeCurrentPass;
					Beep(1200,200); cout << "OK, Reading Time: " << fReadingTimeCurrentPass << "sec." << endl; 
				}
				else break;
			}
			// Store measurement results in strips array
			if(uResult == MM_OK) 
			{
				MMMain.AddStripMeasurements(uCurrentStrip, pMeasureData, fReadingTimeTotal, uDeltaEStandart);
				if(MMMain.m_pStrips[uCurrentStrip]->m_uStripMeasuresCount >= uAdvanceAfter)
					bAdvanceNext = true;
			}
			else for(unsigned short p = 0; p < MMMain.m_pStrips[uCurrentStrip]->m_uStripSize; p++) if(pMeasureData[p] != NULL) delete pMeasureData[p];
			delete pMeasureData;
		}
		// Handle possible measurement errors
		if(uResult != MM_OK) 
		{
			Beep(800, 200); Sleep(350); Beep(800,200);
			if(uResult == MM_ERROR_DOING_MEASUREMENTS)
			{
				if(MMMain.m_uLastDeviceError == eWrongSensorPosition)  cout << "ERROR: Wrong Sensor Position" << endl; //break;
				else if(MMMain.m_uLastDeviceError == eChartCorrelationFailed) cout << "ERROR: Chart Correlation Failed" << endl;
				else if(MMMain.m_uLastDeviceError == eStripRecognitionFailed) cout << "ERROR: Strip Recognition Failed" << endl;
			}
			else if(uResult == MM_ERROR_PATCHES_NUMBER_DONT_MATCH) cout << "ERROR: Patches Number Don't Match" << endl;
			else cout << "Unknown error" << endl;
		}
	}
	MunkiDevice.Reset();
	return 0;
}