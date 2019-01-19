#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <Windows.h>
#include <conio.h>
#include <time.h>

#include "MultiMeasure.h"
#include "lib/ArgyllCMS/ArgyllColorMath.h"


///////////////////////////////////////////////////////
// CMultiMeasureColorMunkiXRite implementation

int CMultiMeasureColorMunkiXRite::GetLastError(string &strErrorText)
{
	strErrorText = MUNKI_GetOption(MUNKI_LAST_ERROR_TEXT);
	return stoi(MUNKI_GetOption(MUNKI_LAST_ERROR_NUMBER));
}

bool CMultiMeasureColorMunkiXRite::IsConnected()
{
	return MUNKI_IsConnected() == eNoError;
}

void CMultiMeasureColorMunkiXRite::WaitForButton()
{
	MUNKI_ButtonPressed();
	while(MUNKI_ButtonPressed() == eButtonNotPressed) Sleep(100);
}


void CMultiMeasureColorMunkiXRite::WaitForButton(char &cKey)
{
	MUNKI_ButtonPressed();
	while(1)
	{
		if(MUNKI_ButtonPressed() != eButtonNotPressed) break;
		if(_kbhit()) { cKey = (char)_getch(); break; }
		Sleep(100);
	}
}

unsigned short CMultiMeasureColorMunkiXRite::Calibrate(const string &strMeasureMode)
{
	if(MUNKI_GetSensorPosition() != eSensorWhiteTile) return eWrongSensorPosition;
	MUNKI_ResultType uResult = MUNKI_SetOption(MUNKI_MEASUREMENT_MODE, strMeasureMode.c_str()); 
	if(uResult != eNoError) return uResult;
	return MUNKI_Calibrate();
}

// xMeasureMode: MUNKI_SINGLE_REFLECTANCE, MUNKI_PATCH_RECOGNITION_BASIC, MUNKI_PATCH_RECOGNITION_CORRELATION
unsigned short CMultiMeasureColorMunkiXRite::SetMeasurementMode(const string &strMeasureMode)
{
	if(strMeasureMode == MUNKI_SINGLE_REFLECTANCE)
		return MUNKI_SetOption(MUNKI_MEASUREMENT_MODE, MUNKI_SINGLE_REFLECTANCE); 
	else
	{
		MUNKI_ResultType uResult = MUNKI_SetOption(MUNKI_MEASUREMENT_MODE, MUNKI_SCANNING_REFLECTANCE); 
		if(uResult != eNoError) return uResult;
		return MUNKI_SetOption(MUNKI_PATCH_RECOGNITION, strMeasureMode.c_str());
	}
}

unsigned short CMultiMeasureColorMunkiXRite::SetRecognitionReference(const string &strColorSpace, const float *pReferenceChartLine, long nLineSize)
{
	MUNKI_ResultType uResult = MUNKI_SetOption(MUNKI_REFERENCE_CHART_COLOR_SPACE, strColorSpace.c_str());
	if(uResult != eNoError) return uResult;
	return MUNKI_SetReferenceChartLine(pReferenceChartLine, nLineSize);
}

unsigned short CMultiMeasureColorMunkiXRite::DoMeasurements()
{
	return MUNKI_TriggerMeasurement();
}

long CMultiMeasureColorMunkiXRite::GetResultsNumber()
{
	return MUNKI_GetNumberOfAvailableSamples();
}

unsigned short CMultiMeasureColorMunkiXRite::GetResult(long nIndex, float faData[MM_COLOR_DATA_SIZE], unsigned short &uSpectrumSize, float *&faSpectrum)
{
	uSpectrumSize = MM_STANDART_SPECTRUM_SIZE;
	faSpectrum = new float[uSpectrumSize];
	MUNKI_ResultType uResult = MUNKI_GetSpectrum(faSpectrum, nIndex); if(uResult != eNoError) return uResult;
    uResult = MUNKI_SetOption(COLOR_SPACE_KEY, COLOR_SPACE_CIELab); if(uResult != eNoError) return uResult;
	uResult = MUNKI_GetTriStimulus(&(faData[MM_LAB_L]), nIndex); if(uResult != eNoError) return uResult;
    uResult = MUNKI_SetOption(COLOR_SPACE_KEY, COLOR_SPACE_RGB); if(uResult != eNoError) return uResult;
	uResult = MUNKI_GetTriStimulus(&(faData[MM_RGB_R]), nIndex); if(uResult != eNoError) return uResult;
    uResult = MUNKI_SetOption(COLOR_SPACE_KEY, COLOR_SPACE_CIEXYZ); if(uResult != eNoError) return uResult;
	uResult = MUNKI_GetTriStimulus(&(faData[MM_XYZ_X]), nIndex); if(uResult != eNoError) return uResult;
	uResult = MUNKI_GetDensities(&(faData[MM_CMYK_C]), 0, nIndex); if(uResult != eNoError) return uResult;
	return eNoError;
}

void CMultiMeasureColorMunkiXRite::Reset()
{
	MUNKI_SetOption(MUNKI_RESET, MUNKI_ALL);
}

///////////////////////////////////////////////////////
// CMultiMeasureColorData implementation
CMultiMeasureColorData::CMultiMeasureColorData(float *faLab, float *faRGB, float *faXYZ, float *faCMYK, unsigned short uSpectrumSize, float *faSpectrum, float fMeasureTime)
{
	float faTempData[MM_COLOR_DATA_SIZE];
	for(int i = 0; i < 3; i++)
	{
		faTempData[MM_LAB_L + i] = faLab != NULL ? faLab[i] : MM_NO_COLOR_DATA;
		faTempData[MM_RGB_R + i] = faRGB != NULL ? faRGB[i] : MM_NO_COLOR_DATA;
		faTempData[MM_XYZ_X + i] = faXYZ != NULL ? faXYZ[i] : MM_NO_COLOR_DATA;
		faTempData[MM_CMYK_C + i] = faCMYK != NULL ? faCMYK[i] : MM_NO_COLOR_DATA;
	}
	faTempData[MM_CMYK_C + 3] = faCMYK != NULL ? faCMYK[3] : MM_NO_COLOR_DATA;
	*this = CMultiMeasureColorData(faTempData, uSpectrumSize, faSpectrum, fMeasureTime);
}

CMultiMeasureColorData::CMultiMeasureColorData(float *faData, unsigned short uSpectrumSize, float *faSpectrum, float fMeasureTime)
{
	for(int i = 0; i < MM_COLOR_DATA_SIZE; i++)
		m_faData[i] = faData[i];
	if((uSpectrumSize > 0) && (faSpectrum != NULL))
	{
		m_uSpectrumSize = uSpectrumSize; m_faSpectrum = faSpectrum; //new float[uSpectrumSize];
//		for(int i = 0; i < uSpectrumSize; i++)
//			m_faSpectrum[i] = faSpectrum[i];
	}
	else
	{
		m_uSpectrumSize = 0; m_faSpectrum = NULL;
	}
	m_bHasLab = m_faData[MM_LAB_L] != MM_NO_COLOR_DATA;
	m_bHasRGB = m_faData[MM_RGB_R] != MM_NO_COLOR_DATA;
	m_bHasXYZ = m_faData[MM_XYZ_X] != MM_NO_COLOR_DATA;
	m_bHasCMYK = m_faData[MM_CMYK_C] != MM_NO_COLOR_DATA;
	m_bHasSpectrum = (m_uSpectrumSize > 0) && (m_faSpectrum != NULL);
	m_fMeasureTime = fMeasureTime; m_fDeltaE = MM_NO_COLOR_DATA; m_fFromBestDeltaE = MM_NO_COLOR_DATA, m_fFromBestByDistDeltaE = MM_NO_COLOR_DATA, m_fDist = MM_NO_COLOR_DATA;
}

CMultiMeasureColorData::CMultiMeasureColorData(CMultiMeasureColorData &ColorData)
{
	*this = ColorData;
}

CMultiMeasureColorData::~CMultiMeasureColorData()
{
	delete [] m_faSpectrum;
}

CMultiMeasureColorData &CMultiMeasureColorData::operator=(CMultiMeasureColorData &ColorData)
{
	for(int i = 0; i < MM_COLOR_DATA_SIZE; i++) m_faData[i] = ColorData.m_faData[i];
	m_uSpectrumSize = ColorData.m_uSpectrumSize;
	m_faSpectrum = NULL;
	if((ColorData.m_uSpectrumSize > 0) && (ColorData.m_faSpectrum != NULL))
	{
		delete [] m_faSpectrum;
		m_faSpectrum = new float[m_uSpectrumSize];
		for(int i = 0; i < ColorData.m_uSpectrumSize; i++)
			m_faSpectrum[i] = ColorData.m_faSpectrum[i];
	}
	m_bHasLab = ColorData.m_bHasLab, m_bHasRGB = ColorData.m_bHasRGB, m_bHasXYZ = ColorData.m_bHasXYZ, m_bHasCMYK = ColorData.m_bHasCMYK, m_bHasSpectrum = ColorData.m_bHasSpectrum;
	m_fMeasureTime = ColorData.m_fMeasureTime; m_fDeltaE = ColorData.m_fDeltaE; m_fFromBestDeltaE = ColorData.m_fFromBestDeltaE; m_fFromBestByDistDeltaE = ColorData.m_fFromBestByDistDeltaE; m_fDist = ColorData.m_fDist;
	return *this;
}


// Calcilates delta E (color difference, https://en.wikipedia.org/wiki/Color_difference)
// between this color data and other color data using the specified standard
float CMultiMeasureColorData::CalculateDeltaE(float *faData, unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	double Lab1[3] = { m_faData[MM_LAB_L], m_faData[MM_LAB_a], m_faData[MM_LAB_b] };
	double Lab2[3] = { faData[MM_LAB_L], faData[MM_LAB_a], faData[MM_LAB_b] };
	float fDeltaE = MM_NO_COLOR_DATA;
	switch(uStandart)
	{
	case MM_DELTA_E_76:
		fDeltaE = (float)icmLabDE(Lab1, Lab2);  //sqrt((m_faData[MM_LAB_L]-faData[MM_LAB_L])*(m_faData[MM_LAB_L]-faData[MM_LAB_L])+(m_faData[MM_LAB_a]-faData[MM_LAB_a])*(m_faData[MM_LAB_a]-faData[MM_LAB_a])+(m_faData[MM_LAB_b]-faData[MM_LAB_b])*(m_faData[MM_LAB_b]-faData[MM_LAB_b]));
		break;
	case MM_DELTA_E_94:
		fDeltaE = (float)icmCIE94(Lab1, Lab2);
		break;
	case MM_DELTA_E_2000:
		fDeltaE = (float)icmCIE2K(Lab1, Lab2);
	}
	return fDeltaE;
}

///////////////////////////////////////////////////////
// CMultiMeasurePatch implementation
CMultiMeasurePatch::CMultiMeasurePatch(unsigned short uRefID, stringext strRefName, CMultiMeasureColorData *pRefData)
{
	m_uRefID = uRefID; m_strRefName = strRefName;
	if(pRefData != NULL) m_RefData = *pRefData;
	m_uMeasuresCount = 0; m_uMeasuresCountMax = 10;
	m_pMeasureData = new CMultiMeasureColorData *[m_uMeasuresCountMax];  // allocate memory for 10 pointers to measurements data
	for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++) m_faAverageData[i] = MM_NO_COLOR_DATA;
	m_uBestMeasure = m_uWorstMeasure = m_uWorstFromBestMeasure = m_uBestByDistMeasure = m_uWorstByDistMeasure = m_uWorstFromBestByDistMeasure = 0;
	m_fBestDeltaE = m_fBestByDistDeltaE = m_fFromBestByDistAverageDeltaE = 9999.0f; m_uDeltaEStandart = MM_DELTA_E_76;
}

CMultiMeasurePatch::~CMultiMeasurePatch()
{
	for(unsigned short m = 0; m < m_uMeasuresCount; m++) delete m_pMeasureData[m];
	delete [] m_pMeasureData;
}

CMultiMeasurePatch &CMultiMeasurePatch::operator=(CMultiMeasurePatch &PatchData)
{
	m_uRefID = PatchData.m_uRefID;
	m_strRefName = PatchData.m_strRefName;
	m_RefData = PatchData.m_RefData;
	for(unsigned short m = 0; m < m_uMeasuresCount; m++) delete m_pMeasureData[m];
	delete [] m_pMeasureData;
	m_uMeasuresCount = PatchData.m_uMeasuresCount, m_uMeasuresCountMax = PatchData.m_uMeasuresCountMax;
	m_pMeasureData = new CMultiMeasureColorData *[m_uMeasuresCountMax];
	for(unsigned short i = 0; i < m_uMeasuresCount; i++)
		m_pMeasureData[i] = PatchData.m_pMeasureData[i];
	for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++) m_faAverageData[i] = PatchData.m_faAverageData[i];
	m_uDeltaEStandart = PatchData.m_uDeltaEStandart;
	m_uBestMeasure = PatchData.m_uBestMeasure; m_uWorstMeasure = PatchData.m_uWorstMeasure;  m_uWorstFromBestMeasure = PatchData.m_uWorstFromBestMeasure; m_fBestDeltaE = PatchData.m_fBestDeltaE;
	m_uBestByDistMeasure = PatchData.m_uBestByDistMeasure; m_uWorstByDistMeasure = PatchData.m_uWorstByDistMeasure;  m_uWorstFromBestByDistMeasure = PatchData.m_uWorstFromBestByDistMeasure; m_fBestByDistDeltaE = PatchData.m_fBestByDistDeltaE;
	m_fFromBestByDistAverageDeltaE = PatchData.m_fFromBestByDistAverageDeltaE;
	return *this;
}

CMultiMeasureColorData &CMultiMeasurePatch::SetRefData(CMultiMeasureColorData *pRefData)
{
	return m_RefData = *pRefData;
}

CMultiMeasureColorData *CMultiMeasurePatch::AddMeasureData(CMultiMeasureColorData *pRefData, unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	if(m_uMeasuresCount == m_uMeasuresCountMax)
	{
		m_uMeasuresCountMax += 10; 
		if (m_uMeasuresCountMax > MM_MAX_MEASURES_COUNT)
			return NULL;
		CMultiMeasureColorData **pTempMeasureData = new CMultiMeasureColorData *[m_uMeasuresCountMax];
		for(int i = 0; i < m_uMeasuresCount; i++)
			pTempMeasureData[i] = m_pMeasureData[i];
		delete m_pMeasureData;
		m_pMeasureData = pTempMeasureData;
	}
	m_pMeasureData[m_uMeasuresCount] = pRefData;
	m_uMeasuresCount++;
	m_uDeltaEStandart = uStandart;
	CalculateStatistic(uStandart);
	return m_pMeasureData[m_uMeasuresCount - 1];
}

// The sensor of the spectrophotometer Colormunki Photo is almost the same as that of 
// a much more expensive i1pro 2, but its reading frequency is about 10 times less. 
// Therefore, it requires larger patches and has a lower accuracy. To be able to use 
// patches of normal size and to increase the accuracy of measurements, you can read 
// each strip several times and then select the best values for each patch.
// Two methods implemented for finding the best values.
// First method: select measurement closest (by color difference, delta E) to 
// the average value of all measurements.
// Second method: select measurement with minimum total color difference with all other
// measurements.

void CMultiMeasurePatch::CalculateStatistic(unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	if(m_uMeasuresCount > 1)
	{
		// First method: closest to average
		// Find average values of each components of color data from all measurements of one patch.
		for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++) m_faAverageData[i] = 0;
		for(unsigned short m = 0; m < m_uMeasuresCount; m++)
			for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++)
				m_faAverageData[i] += m_pMeasureData[m]->m_faData[i];
		for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++)
			m_faAverageData[i] /= m_uMeasuresCount;

		// Calculate delta E between average values and each measurement, find min (best) and max (worst)
		// delta E values and remember measurements with best and worst delta E.
		m_fBestDeltaE = 9999.0f; m_fWorstDeltaE = 0.0f;
		for(unsigned short m = 0; m < m_uMeasuresCount; m++)
		{
			m_pMeasureData[m]->m_fDeltaE = m_pMeasureData[m]->CalculateDeltaE(m_faAverageData, uStandart);
			if(m_pMeasureData[m]->m_fDeltaE < m_fBestDeltaE) { m_fBestDeltaE = m_pMeasureData[m]->m_fDeltaE; m_uBestMeasure = m; }
			if(m_pMeasureData[m]->m_fDeltaE > m_fWorstDeltaE) { m_fWorstDeltaE = m_pMeasureData[m]->m_fDeltaE; m_uWorstMeasure = m; }
		}

		// Calculate delta E between each measurement and best measurement from previous step.
		// Find max (worst) delta E value and remember measurement that has the greatest color difference 
		// between it and measurement closest to average.
		m_fWorstFromBestDeltaE = 0.0f;
		for(unsigned short m = 0; m < m_uMeasuresCount; m++)
		{
			m_pMeasureData[m]->m_fFromBestDeltaE = m_pMeasureData[m]->CalculateDeltaE(m_pMeasureData[m_uBestMeasure]->m_faData, uStandart);
			if(m_pMeasureData[m]->m_fFromBestDeltaE > m_fWorstFromBestDeltaE) { m_fWorstFromBestDeltaE = m_pMeasureData[m]->m_fFromBestDeltaE; m_uWorstFromBestMeasure = m; }
		}

		// Second method: by minimum total distance
		// Delta E can be considered as the distance between two colors in the color space.
		// Calculate the sum of the distances (delta E) between each measurement and all other measurements.
		// Find max and min distances and remember measurements
		m_fMinDist = 9999.0f; m_fMaxDist = 0.0f;
		for(unsigned short m = 0; m < m_uMeasuresCount; m++)
		{
			m_pMeasureData[m]->m_fDist = 0.0f;
			for(unsigned short m2 = 0; m2 < m_uMeasuresCount; m2++)
				m_pMeasureData[m]->m_fDist += (m == m2 ? 0 : m_pMeasureData[m]->CalculateDeltaE(m_pMeasureData[m2]->m_faData, uStandart));
//			m_pMeasureData[m]->m_fDist /= (m_uMeasuresCount - 1);  // try average distance
			if(m_pMeasureData[m]->m_fDist < m_fMinDist) { m_fMinDist = m_pMeasureData[m]->m_fDist; m_uBestByDistMeasure = m; }
			if(m_pMeasureData[m]->m_fDist > m_fMaxDist) { m_fMaxDist = m_pMeasureData[m]->m_fDist; m_uWorstByDistMeasure = m; }
		}

		// Calculate delta E between each measurement and best measurement from previous step.
		// Find max (worst) delta E value and remember measurement that has the greatest color difference 
		// between it and measurement with minimal total distanst with all other measurements
		m_fWorstFromBestByDistDeltaE = 0.0f; m_fFromBestByDistAverageDeltaE = 0.0f;
		for(unsigned short m = 0; m < m_uMeasuresCount; m++)
		{
			m_pMeasureData[m]->m_fFromBestByDistDeltaE = m_pMeasureData[m]->CalculateDeltaE(m_pMeasureData[m_uBestByDistMeasure]->m_faData, uStandart);
			if(m_pMeasureData[m]->m_fFromBestByDistDeltaE > m_fWorstFromBestByDistDeltaE) { m_fWorstFromBestByDistDeltaE = m_pMeasureData[m]->m_fFromBestByDistDeltaE; m_uWorstFromBestByDistMeasure = m; }
			m_fFromBestByDistAverageDeltaE += m_pMeasureData[m]->m_fFromBestByDistDeltaE;
		}

		// Also find average delta E between the best measurement and all other measurements
		m_fFromBestByDistAverageDeltaE /= (m_uMeasuresCount - 1);

		// To estimate the difference between the methods, remember the delta Es, found by the first method 
		// for the best and worst measurement, determined by second method.
		m_fBestByDistDeltaE = m_pMeasureData[m_uBestByDistMeasure]->m_fDeltaE;
		m_fWorstByDistDeltaE = m_pMeasureData[m_uWorstByDistMeasure]->m_fDeltaE;
	}
}

// Removes all measurements from patch
void CMultiMeasurePatch::EraseMeasurements()
{
	for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++) m_faAverageData[i] = MM_NO_COLOR_DATA;
	m_uBestMeasure = 0; m_fBestDeltaE = 9999.0f;
	for(unsigned short m = 0; m < m_uMeasuresCount; m++) delete m_pMeasureData[m];
	delete [] m_pMeasureData;
	m_uMeasuresCount = 0; m_uMeasuresCountMax = 10;
	m_pMeasureData = new CMultiMeasureColorData *[m_uMeasuresCountMax];
}

// Removes one specifed measurement from patch
void CMultiMeasurePatch::EraseMeasurement(unsigned short uIndex)
{
	if (uIndex < m_uMeasuresCount)
	{
		delete m_pMeasureData[uIndex];
		for (unsigned short m = uIndex + 1; m < m_uMeasuresCount; m++) m_pMeasureData[m - 1] = m_pMeasureData[m];
		m_uMeasuresCount--;
		CalculateStatistic(m_uDeltaEStandart);
	}
}

// Removes the worst measurement from the patch
void CMultiMeasurePatch::EraseWorstMeasurement(bool bFromBest/* = true*/, int bMethod/* = MM_MEASURE_BEST_BY_DIST*/)
{
	if (bMethod == MM_MEASURE_BEST)
		EraseMeasurement(bFromBest ? m_uWorstFromBestMeasure : m_uWorstMeasure);			  // the worst by first method: by closeness to average
	else
		EraseMeasurement(bFromBest ? m_uWorstFromBestByDistMeasure : m_uWorstByDistMeasure);  // the worst by second method: by total distance to all other measurements
}

// Compares two patch names. Used to sort charts with Agryll optimized placement.
bool CMultiMeasurePatch::operator>(CMultiMeasurePatch &patch)
{
	unsigned short nLettersCount1 = 0, nLettersCount2 = 0;
	while((nLettersCount1 < m_strRefName.length()) && (m_strRefName[nLettersCount1]>='A') && (m_strRefName[nLettersCount1]<='Z')) nLettersCount1++;                    // Find lettsers count 
	while((nLettersCount2 < patch.m_strRefName.length()) && (patch.m_strRefName[nLettersCount2]>='A') && (patch.m_strRefName[nLettersCount2]<='Z')) nLettersCount2++;  // in both names
	if(nLettersCount1 != nLettersCount2) 
		return nLettersCount1 > nLettersCount2;                                                          // The one with more letters is greater
	if(m_strRefName.substr(0, nLettersCount1) != patch.m_strRefName.substr(0, nLettersCount2))			 // If letters is not equal,
		return m_strRefName.substr(0, nLettersCount1) > patch.m_strRefName.substr(0, nLettersCount2);    // compare letters
	return stoi(m_strRefName.substr(nLettersCount1)) > stoi(patch.m_strRefName.substr(nLettersCount2));  // else compare numbers
}

///////////////////////////////////////////////////////
// CMultiMeasureStrip implementation
CMultiMeasureStrip::CMultiMeasureStrip(unsigned short uStripSize)
{
	m_uStripSize = uStripSize;
	if(m_uStripSize > 0) m_pStripPatches = new CMultiMeasurePatch *[m_uStripSize];
	else m_pStripPatches = NULL;
	m_uStripMeasuresCount = 0;
	m_fMaxBestDeltaE = m_fAverageBestDeltaE = m_fTotalMaxDeltaE = m_fTotalAverageDeltaE = MM_NO_COLOR_DATA;
	for(unsigned short i = 0; i < MM_MAX_MEASURES_COUNT; i++) { m_faMaxDeltaE[i] = MM_NO_COLOR_DATA; m_faAverageDeltaE[i] = MM_NO_COLOR_DATA; }
}

CMultiMeasureStrip::~CMultiMeasureStrip()
{
	delete m_pStripPatches;
}

unsigned short CMultiMeasureStrip::SetSize(unsigned short uStripSize)
{
	m_uStripSize = uStripSize;
	delete m_pStripPatches;
	if(m_uStripSize > 0) m_pStripPatches = new CMultiMeasurePatch *[m_uStripSize];
	return m_uStripSize;
}

// Calculates statistic of strip based on statistic of all patches in it:
// - maximum and average delta E for each strip measurement
// - maximum and average delta E across all measurements of all patches in strip
// - maximum and average delta E among the best measurements from all patches in strip
void CMultiMeasureStrip::CalculateStatistic(int bMethod/* = MM_MEASURE_BEST_BY_DIST*/)
{
	if(m_uStripMeasuresCount < 2) return;
	for(unsigned short m = 0; m < MM_MAX_MEASURES_COUNT; m++) { m_faMaxDeltaE[m] = m_faAverageDeltaE[m] = 0; }
	m_fMaxBestDeltaE = m_fAverageBestDeltaE = m_fTotalMaxDeltaE = m_fTotalAverageDeltaE = 0;
	unsigned short uTotalMeasurementsCount = 0;
	for(unsigned short p = 0; p < m_uStripSize; p++)
	{
		if (bMethod == MM_MEASURE_BEST) {
			if (m_pStripPatches[p]->m_fBestDeltaE > m_fMaxBestDeltaE) m_fMaxBestDeltaE = m_pStripPatches[p]->m_fBestDeltaE;
			m_fAverageBestDeltaE += m_pStripPatches[p]->m_fBestDeltaE;
		}
		else {
			if (m_pStripPatches[p]->m_fBestByDistDeltaE > m_fMaxBestDeltaE) m_fMaxBestDeltaE = m_pStripPatches[p]->m_fBestByDistDeltaE;
			m_fAverageBestDeltaE += m_pStripPatches[p]->m_fBestByDistDeltaE;
		}
		for(unsigned short m = 0; m < m_pStripPatches[p]->m_uMeasuresCount; m++)
		{
			float fCurrentDeltaE = (bMethod == MM_MEASURE_BEST ? m_pStripPatches[p]->m_pMeasureData[m]->m_fFromBestDeltaE : m_pStripPatches[p]->m_pMeasureData[m]->m_fFromBestByDistDeltaE);
			if(fCurrentDeltaE > m_faMaxDeltaE[m]) m_faMaxDeltaE[m] = fCurrentDeltaE;
			if(fCurrentDeltaE > m_fTotalMaxDeltaE) m_fTotalMaxDeltaE = fCurrentDeltaE;
			m_faAverageDeltaE[m] += fCurrentDeltaE;
			m_fTotalAverageDeltaE += fCurrentDeltaE;
		}
		uTotalMeasurementsCount += m_pStripPatches[p]->m_uMeasuresCount;
	}
	m_fAverageBestDeltaE /= m_uStripSize;
	for(unsigned short m = 0; m < m_uStripMeasuresCount; m++)
		m_faAverageDeltaE[m] /= m_uStripSize;
	m_fTotalAverageDeltaE /= uTotalMeasurementsCount;
}

// Removes all measurements from all patches in strip
void CMultiMeasureStrip::EraseMeasurements()
{
	for(unsigned short m = 0; m < MM_MAX_MEASURES_COUNT; m++) { m_faMaxDeltaE[m] = m_faAverageDeltaE[m] = m_faMeasureTime[m] = 0; }
	m_fMaxBestDeltaE = m_fAverageBestDeltaE = m_fTotalMaxDeltaE = m_fTotalAverageDeltaE = 0;
	for(unsigned short p = 0; p < m_uStripSize; p++)
		m_pStripPatches[p]->EraseMeasurements();
	m_uStripMeasuresCount = 0;
}

// Removes one specifed measurement from all patches in strip
void CMultiMeasureStrip::EraseMeasurement(unsigned short uIndex)
{
	if(m_uStripMeasuresCount == 0) return;
	if(uIndex >= m_uStripMeasuresCount) uIndex = m_uStripMeasuresCount - 1;
	for(unsigned short p = 0; p < m_uStripSize; p++)
		m_pStripPatches[p]->EraseMeasurement(uIndex);
	for(unsigned short m = uIndex + 1; m < m_uStripMeasuresCount; m++) m_faMeasureTime[m - 1] = m_faMeasureTime[m];
	m_uStripMeasuresCount--;
	CalculateStatistic();
}

// Removes the worst measurement from each patch in strip
void CMultiMeasureStrip::EraseWorstMeasurement(bool bFromBest/* = true*/, int bMethod/* = MM_MEASURE_BEST_BY_DIST*/)
{
	if(m_uStripMeasuresCount < 2) return;
	for(unsigned short p = 0; p < m_uStripSize; p++)
		m_pStripPatches[p]->EraseWorstMeasurement(bFromBest, bMethod);
	m_uStripMeasuresCount--;
	CalculateStatistic();
}

///////////////////////////////////////////////////////
// CMultiMeasure implementation
CMultiMeasure::CMultiMeasure(CMultiMeasureDevice *pDevice)
{
	m_pDevice = pDevice;
	m_uPatchesCount = 0; m_pPatches = m_pAgryllPatches = NULL;
	m_uStripSize = m_uStripsCount = 0; m_pStrips = NULL;
	m_strInFile = m_strInFileName = "";
	m_nInFileType = MM_FILE_TYPE_NONE;
	m_strOutFile = m_strOutFileName = "";
	m_nOutFileType = MM_FILE_TYPE_NONE;
}

CMultiMeasure::~CMultiMeasure()
{
	for(unsigned short s = 0; s < m_uStripsCount; s++) delete m_pStrips[s];
	delete [] m_pStrips;
	for(unsigned short p = 0; p < m_uPatchesCount; p++) delete m_pPatches[p];
	delete [] m_pPatches;
	delete [] m_pAgryllPatches;
}

CMultiMeasurePatch **CMultiMeasure::SetupPatchesList(unsigned short uPatchesCount)
{
	for(unsigned short p = 0; p < m_uPatchesCount; p++) delete m_pPatches[p];
	delete [] m_pPatches;
	m_uPatchesCount = uPatchesCount;
	return m_pPatches = new CMultiMeasurePatch *[uPatchesCount];
}

// Reads reference chart file. File format determined by its extension.
unsigned short CMultiMeasure::ReadReferenceFile(string strInFileName)
{
	m_nInFileType = MM_FILE_TYPE_NONE;
	m_uRowLength = m_uNumberOfFields = m_uNumberOfSets = m_uStripSize = m_uStripsCount = 0;
	m_uNumberOfPages = 1;
	m_uAgryll_COMP_GREY_STEPS = m_uAgryll_SINGLE_DIM_STEPS = 0;
	m_strAgryll_ACCURATE_EXPECTED_VALUES = m_strAgryll_APPROX_WHITE_POINT = m_strArgillCalibrationInfo = m_strAgryll_TOTAL_INK_LIMIT = "";
	m_strInFileName = strInFileName;
	ifstream stmInFile(m_strInFileName.c_str());
	if(stmInFile.fail()) return MM_FILE_TYPE_NONE;
	stringext strLine, strInFileExt = m_strInFileName.substr(m_strInFileName.length()-4, 4);
	// ProfileMaker 5 and i1Profiler data format
	if(strInFileExt == ".txt")
	{
		m_nInFileType = MM_FILE_TYPE_PM5;
		for(m_strInFile = ""; getline(stmInFile, strLine); m_strInFile += (strLine + "\n"))
		{
			if(strLine == "") continue;
			vector<stringext> strLineFields = strLine.split("\t ");
			// Reads common chart parameters
			if(strLineFields[0].upper() == "LGOROWLENGTH") m_uRowLength = stoi(strLineFields[1]);
			if(strLineFields[0].upper() == "NUMBEROFSTRIPS") m_uNumberOfPages = stoi(strLineFields[1]);
			if(strLineFields[0].upper() == "NUMBER_OF_FIELDS") m_uNumberOfFields = stoi(strLineFields[1]);
			if(strLineFields[0].upper() == "NUMBER_OF_SETS") m_uNumberOfSets = stoi(strLineFields[1]);
			// Reads format of patches table
			if(strLineFields[0].upper() == "BEGIN_DATA_FORMAT")
			{
				getline(stmInFile, strLine);
				vector<stringext> strLineDataFields = strLine.split("\t ");
				for(int i = 0; i < MM_COLOR_DATA_SIZE + 1; i++) m_naDataIndex[i] = -1;
				m_nIDIndex = m_nNameIndex = -1;
				string strDataCaptions[] = { "LAB_L", "LAB_A", "LAB_B", "RGB_R", "RGB_G", "RGB_B", "XYZ_X", "XYZ_Y", "XYZ_Z", "CMYK_C", "CMYK_M", "CMYK_Y", "CMYK_K", "NM380" };
				for(unsigned short col = 0; col < strLineDataFields.size(); col++)
				{
					if(strLineDataFields[col].upper() == "SAMPLEID") m_nIDIndex = col;
					if((strLineDataFields[col].upper() == "SAMPLE_NAME") || (strLineDataFields[col].upper() == "SAMPLENAME")) m_nNameIndex = col;
					for(int i = 0; i < MM_COLOR_DATA_SIZE + 1; i++) if(strLineDataFields[col].upper() == strDataCaptions[i]) m_naDataIndex[i] = col;
				}
			}
			// Reads patches reference data and stores pointer to patches objects in one array
			if(strLineFields[0].upper() == "BEGIN_DATA")
			{
				SetupPatchesList(m_uNumberOfSets);
				float faTempData[MM_COLOR_DATA_SIZE], *faTempSpectrum = NULL;
				unsigned short uRefID, uSpectrumSize = 0;
				stringext strRefName;
				for(unsigned short p = 0; p < m_uNumberOfSets; p++)
				{
					getline(stmInFile, strLine);
					vector<stringext> strLineDataFields = strLine.split("\t ");
					uRefID = m_nIDIndex != -1 ? stoi(strLineDataFields[m_nIDIndex]) : 0;
					strRefName = m_nNameIndex != -1 ? strLineDataFields[m_nNameIndex] : "";
					for(int i = 0; i < MM_COLOR_DATA_SIZE; i++) faTempData[i] = ((m_naDataIndex[i] != -1) && (m_naDataIndex[i] < (int)strLineDataFields.size())) ? stof(strLineDataFields[m_naDataIndex[i]]) : MM_NO_COLOR_DATA;
					if((m_naDataIndex[MM_COLOR_DATA_SIZE] != -1) && ((m_naDataIndex[MM_COLOR_DATA_SIZE] + MM_STANDART_SPECTRUM_SIZE) <= (int)strLineDataFields.size()))
					{
						uSpectrumSize = MM_STANDART_SPECTRUM_SIZE;
						faTempSpectrum = new float[MM_STANDART_SPECTRUM_SIZE];
						for(unsigned short s = 0; s < MM_STANDART_SPECTRUM_SIZE; s++)
							faTempSpectrum[s] = stof(strLineDataFields[m_naDataIndex[MM_COLOR_DATA_SIZE] + s]);
					}
					m_pPatches[p] = new CMultiMeasurePatch(uRefID, strRefName, &CMultiMeasureColorData(faTempData, uSpectrumSize, faTempSpectrum));
				}
			}
		}
		// Creates strips and fills them with pointers to patches.
		m_uStripsCount = m_uRowLength * m_uNumberOfPages;
		m_uStripSize = (unsigned short)ceil((float)m_uNumberOfSets/(float)m_uStripsCount);
		m_pStrips = new CMultiMeasureStrip *[m_uStripsCount];
		for(unsigned short s = 0; s < m_uStripsCount; s++)
		{
			unsigned short uCurrentStripSize = m_uStripSize - ((s + (m_uStripSize - 1) * m_uStripsCount) >= m_uNumberOfSets);  // last strip may has less patches than others
			m_pStrips[s] = new CMultiMeasureStrip(uCurrentStripSize);
			for(unsigned short p = 0; p < uCurrentStripSize; p++)
				if((s + p * m_uStripsCount) < m_uNumberOfSets) m_pStrips[s]->m_pStripPatches[p] = m_pPatches[s + p * m_uStripsCount];
				else m_pStrips[s]->m_pStripPatches[p] = NULL;
		}
	}
	// Agryll data format
	else if(strInFileExt == ".ti2")
	{
		m_nInFileType = MM_FILE_TYPE_TI2;
		for(m_strInFile = ""; getline(stmInFile, strLine); m_strInFile += (strLine + "\n"))
		{
			if(strLine == "") continue;
			vector<stringext> strLineFields = strLine.split("\t ");
			// Reads Agryll specific parameters
			if(strLineFields[0].upper() == "COMP_GREY_STEPS") m_uAgryll_COMP_GREY_STEPS = stoi(strLineFields[1].cutQuotes());
			if(strLineFields[0].upper() == "SINGLE_DIM_STEPS") m_uAgryll_SINGLE_DIM_STEPS = stoi(strLineFields[1].cutQuotes());
			if(strLineFields[0].upper() == "ACCURATE_EXPECTED_VALUES") m_strAgryll_ACCURATE_EXPECTED_VALUES = strLineFields[1];
			if(strLineFields[0].upper() == "APPROX_WHITE_POINT") m_strAgryll_APPROX_WHITE_POINT = strLineFields[1];
			if(strLineFields[0].upper() == "TOTAL_INK_LIMIT") m_strAgryll_TOTAL_INK_LIMIT = strLineFields[1].cutQuotes();
			if(strLineFields[0].upper() == "STEPS_IN_PASS") m_uRowLength = m_uStripSize = stoi(strLineFields[1].cutQuotes());
			if(strLineFields[0].upper() == "COLOR_REP") m_strAgryll_COLOR_REP = strLineFields[1].cutQuotes();
			// Reads the number of pages and strips
			if(strLineFields[0].upper() == "PASSES_IN_STRIPS2")
			{
				vector<stringext> strStripsPerPages = strLineFields[1].cutQuotes().split(",");
				for(unsigned short i = 0; i < strStripsPerPages.size(); i++)
					m_uStripsCount += stoi(strStripsPerPages[i]);
				m_uNumberOfPages = (unsigned short)strStripsPerPages.size();
			}
			// Reads common chart parameters
			if(strLineFields[0].upper() == "NUMBER_OF_FIELDS") m_uNumberOfFields = stoi(strLineFields[1]);
			if(strLineFields[0].upper() == "NUMBER_OF_SETS") m_uNumberOfSets = stoi(strLineFields[1]);
			// Reads format of patches table
			if(strLineFields[0].upper() == "BEGIN_DATA_FORMAT")
			{
				getline(stmInFile, strLine);
				vector<stringext> strLineDataFields = strLine.split("\t ");
				for(int i = 0; i < MM_COLOR_DATA_SIZE + 1; i++) m_naDataIndex[i] = -1;
				m_nIDIndex = m_nNameIndex = -1;
				string strDataCaptions[] = { "LAB_L", "LAB_A", "LAB_B", "RGB_R", "RGB_G", "RGB_B", "XYZ_X", "XYZ_Y", "XYZ_Z", "CMYK_C", "CMYK_M", "CMYK_Y", "CMYK_K" };
				if(m_strAgryll_COLOR_REP == "CMY") { strDataCaptions[MM_CMYK_C] = "CMY_C"; strDataCaptions[MM_CMYK_M] = "CMY_M"; strDataCaptions[MM_CMYK_Y] = "CMY_Y"; }
				for(unsigned short col = 0; col < strLineDataFields.size(); col++)
				{
					if(strLineDataFields[col].upper() == "SAMPLE_ID") m_nIDIndex = col;
					if(strLineDataFields[col].upper() == "SAMPLE_LOC") m_nNameIndex = col;
					for(int i = 0; i < MM_COLOR_DATA_SIZE; i++) if(strLineDataFields[col].upper() == strDataCaptions[i]) m_naDataIndex[i] = col;
				}
			}
			// Reads patches reference data and stores pointer to patches objects in one array
			if(strLineFields[0].upper() == "BEGIN_DATA")
			{
				SetupPatchesList(m_uNumberOfSets);
				float faTempData[MM_COLOR_DATA_SIZE], *faTempSpectrum = NULL;
				unsigned short uRefID, uSpectrumSize = 0;
				stringext strRefName;
				for(unsigned short p = 0; p < m_uNumberOfSets; p++)
				{
					getline(stmInFile, strLine);
					vector<stringext> strLineDataFields = strLine.split("\t ");
					uRefID = m_nIDIndex != -1 ? stoi(strLineDataFields[m_nIDIndex]) : 0;
					strRefName = m_nNameIndex != -1 ? strLineDataFields[m_nNameIndex].cutQuotes() : "";
					for(int i = 0; i < MM_COLOR_DATA_SIZE; i++) faTempData[i] = ((m_naDataIndex[i] != -1) && (m_naDataIndex[i] < (int)strLineDataFields.size())) ? stof(strLineDataFields[m_naDataIndex[i]]) : MM_NO_COLOR_DATA;
					for(int i = MM_RGB_R; i <= MM_RGB_B; i++) if(faTempData[i] != MM_NO_COLOR_DATA) faTempData[i] *= 2.55f;
					if((m_strAgryll_COLOR_REP == "CMY") && (faTempData[MM_CMYK_C] != MM_NO_COLOR_DATA)) faTempData[MM_CMYK_K] = 0.0f;
/*					if(m_naDataIndex[MM_COLOR_DATA_SIZE] != -1)
					{
						uSpectrumSize = MM_STANDART_SPECTRUM_SIZE;
						faTempSpectrum = new float[MM_STANDART_SPECTRUM_SIZE];
						for(unsigned short s = 0; s < MM_STANDART_SPECTRUM_SIZE; s++)
							faTempSpectrum[s] = stof(strLineDataFields[m_naDataIndex[MM_COLOR_DATA_SIZE] + s]);
					}*/
					m_pPatches[p] = new CMultiMeasurePatch(uRefID, strRefName, &CMultiMeasureColorData(faTempData, uSpectrumSize, faTempSpectrum));
				}
			}
			if(strLineFields[0].upper() == "CAL")
				for(m_strArgillCalibrationInfo = strLine; getline(stmInFile, strLine); m_strArgillCalibrationInfo += ("\n" + strLine));
		}

		// Sorts patches by their names, but first saves the original order of the patches in order to correctly place them in the output Agryll format file.
		m_pAgryllPatches = new CMultiMeasurePatch *[m_uPatchesCount];
		for(unsigned short p = 0; p < m_uPatchesCount; p++)
			m_pAgryllPatches[p] = m_pPatches[p];

		for(unsigned short p = 1; p < m_uPatchesCount; p++)
			for(unsigned short p1 = m_uPatchesCount - 1; p1 >= p; p1--)
				if(*m_pPatches[p1-1] > *m_pPatches[p1]) 
				{
					CMultiMeasurePatch *pTempPatch = m_pPatches[p1-1];
					m_pPatches[p1-1] = m_pPatches[p1];
					m_pPatches[p1] = pTempPatch;
				}

		// Creates strips and fills them with pointers to patches.
		m_pStrips = new CMultiMeasureStrip *[m_uStripsCount];
		for(unsigned short s = 0; s < m_uStripsCount; s++)
		{
			unsigned short uCurrentStripSize = m_uStripSize; // - ((s + (m_uStripSize - 1) * m_uStripsCount) >= m_uNumberOfSets);  // Agryll adds patches to fill last one to full size
			m_pStrips[s] = new CMultiMeasureStrip(uCurrentStripSize);
			for(unsigned short p = 0; p < uCurrentStripSize; p++)
				if((s * m_uStripSize + p) < m_uNumberOfSets) m_pStrips[s]->m_pStripPatches[p] = m_pPatches[s * m_uStripSize + p];
				else m_pStrips[s]->m_pStripPatches[p] = NULL;
		}
	}
	stmInFile.close();
	return m_nInFileType;
}

// Writes measurement data to files in specified formats.
unsigned short CMultiMeasure::WriteMeasurementFile(string strOutFileName, unsigned short uOutputFormats, unsigned short uOutputDeviceData/* = 0*/)
{
	if(m_uPatchesCount == 0) return MM_ERROR_NO_REFERENCE_CHART;
	if(uOutputFormats & MM_FILE_TYPE_PM5)
	{
		unsigned short uMinMeasuresCount = 9999;
		for(unsigned short p = 0; p < m_uPatchesCount; p++)
			if((m_pPatches[p]->m_uMeasuresCount < uMinMeasuresCount) && (m_pPatches[p]->m_uMeasuresCount > 0)) uMinMeasuresCount = m_pPatches[p]->m_uMeasuresCount;
		for(int m = 0; m < (int)uMinMeasuresCount; m++)
		{
			stringstream strFileName; strFileName << strOutFileName << "_" << m;
			WriteMeasurementFilePM5(strFileName.str(), m, uOutputDeviceData);
		}
		WriteMeasurementFilePM5(strOutFileName + "_avr", MM_MEASURE_BEST, uOutputDeviceData);
		WriteMeasurementFilePM5(strOutFileName, MM_MEASURE_BEST_BY_DIST, uOutputDeviceData);
	}
	if(uOutputFormats & MM_FILE_TYPE_PM5_COLORBASE)
	{
		WriteMeasurementFileCB(strOutFileName, MM_MEASURE_BEST_BY_DIST);
		WriteMeasurementFileTI3CB(strOutFileName + "_CB1", 0, MM_MEASURE_BEST_BY_DIST);
		WriteMeasurementFileTI3CB(strOutFileName + "_CB2", 1, MM_MEASURE_BEST_BY_DIST);
	}
	if(uOutputFormats & MM_FILE_TYPE_PM5_DENSITY)
		WriteMeasurementFilePM5Density(strOutFileName + "_D", MM_MEASURE_BEST_BY_DIST);
	if(uOutputFormats & MM_FILE_TYPE_TI3)
	{
		unsigned short uMinMeasuresCount = 9999;
		for(unsigned short p = 0; p < m_uPatchesCount; p++)
			if((m_pPatches[p]->m_uMeasuresCount < uMinMeasuresCount) && (m_pPatches[p]->m_uMeasuresCount > 0)) uMinMeasuresCount = m_pPatches[p]->m_uMeasuresCount;
		for(int m = 0; m < (int)uMinMeasuresCount; m++)
		{
			stringstream strFileName; strFileName << strOutFileName << "_" << m;
			WriteMeasurementFileTI3(strFileName.str(), m);
		}
		WriteMeasurementFileTI3(strOutFileName + "_avr", MM_MEASURE_BEST);
		WriteMeasurementFileTI3(strOutFileName, MM_MEASURE_BEST_BY_DIST);
		if(m_strAgryll_COLOR_REP == "CMY")
		{
			m_strAgryll_COLOR_REP = "CMYK";
			WriteMeasurementFileTI3(strOutFileName + "_CMYK", MM_MEASURE_BEST_BY_DIST);
		}
	}
	
	{
		stringstream sout3;
/*		sout3 << "LGOROWLENGTH: " << m_uRowLength << endl;
		sout3 << "NUMBEROFSTRIPS: " << m_uNumberOfPages << endl;
		sout3 << "NUMBER_OF_FIELDS: " << m_uNumberOfFields << endl;
		sout3 << "NUMBER_OF_SETS: " << m_uNumberOfSets << endl;
		sout3 << "Strips Count: " << m_uStripsCount << endl;
		sout3 << "Strip Size: " << m_uStripSize << endl;

		for(unsigned short s = 0; s < m_uStripsCount; s++)
		{
			sout3 << m_pStrips[s]->m_uStripSize << " patches:\t";
			for(unsigned short p = 0; p < m_pStrips[s]->m_uStripSize; p++)
				sout3 << (m_pStrips[s]->m_pStripPatches[p] != NULL ? m_pStrips[s]->m_pStripPatches[p]->m_strRefName : "") << "\t";
			sout3 << endl;
		}

		for(unsigned short p = 0; p < m_uNumberOfSets; p++)
		{
			sout3 << m_pPatches[p]->m_uRefID << "\t" << m_pPatches[p]->m_strRefName;
			for(int i=0; i<MM_COLOR_DATA_SIZE; i++) if(m_pPatches[p]->m_RefData.m_faData[i] != MM_NO_COLOR_DATA) sout3 << "\t" << m_pPatches[p]->m_RefData.m_faData[i];
			if(m_pPatches[p]->m_RefData.m_bHasSpectrum) for(unsigned short i=0; i<m_pPatches[p]->m_RefData.m_uSpectrumSize; i++) sout3 << "\t" << m_pPatches[p]->m_RefData.m_faSpectrum[i];
			sout3 << endl;
		}

		sout3 << m_strArgillCalibrationInfo;
*/
		for(unsigned short p = 0; p < m_uPatchesCount; p++)
		{
			sout3 << m_pPatches[p]->m_uRefID << "\t" << m_pPatches[p]->m_strRefName;
			if(m_pPatches[p]->m_uMeasuresCount > 0) 
				for(unsigned short m = 0; m < m_pPatches[p]->m_uMeasuresCount; m++)
					sout3 << "\t" << m_pPatches[p]->m_pMeasureData[m]->m_faData[MM_LAB_L] << "\t" << m_pPatches[p]->m_pMeasureData[m]->m_faData[MM_LAB_a] << "\t" << m_pPatches[p]->m_pMeasureData[m]->m_faData[MM_LAB_b];
			sout3 << endl;
		}
		FILE* fp;
		if (fopen_s(&fp, "tempMeasureData", "w") == 0)
		{
			fputs(sout3.str().c_str(), fp);
			fclose(fp);
		}
	}

	return MM_OK;
}

// Writes measuremens data to file in ProfileMaker 5 and i1Profiler format
unsigned short CMultiMeasure::WriteMeasurementFilePM5(string strOutFileName, int nMeasure/* = MM_MEASURE_BEST_BY_DIST*/, unsigned short uOutputDeviceData/* = 0*/)
{
	stringstream sout;
	string strDataFormat = "SampleID\tSAMPLE_NAME";
	unsigned short uNewNumberOfFields = 2;
	if((m_pPatches[0]->m_RefData.m_bHasRGB) || (uOutputDeviceData & MM_DD_RGB)) { strDataFormat += "\tRGB_R\tRGB_G\tRGB_B"; uNewNumberOfFields += 3; }
	if((m_pPatches[0]->m_RefData.m_bHasCMYK) || (uOutputDeviceData & MM_DD_CMYK)) { strDataFormat += "\tCMYK_C\tCMYK_M\tCMYK_Y\tCMYK_K"; uNewNumberOfFields += 4; }
	bool bHasLab = false, bHasXYZ = false, bHasSpectrum  = false;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
		if(m_pPatches[p]->m_uMeasuresCount > 0)
		{
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasLab) bHasLab = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasXYZ) bHasXYZ = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasSpectrum) bHasSpectrum = true;
		}
	if(bHasLab) { strDataFormat += "\tLAB_L\tLAB_A\tLAB_B"; uNewNumberOfFields += 3; }
	if(bHasXYZ) { strDataFormat += "\tXYZ_X\tXYZ_Y\tXYZ_Z"; uNewNumberOfFields += 3; }
	if(bHasSpectrum) 
	{ 
		strDataFormat += "\tnm380\tnm390\tnm400\tnm410\tnm420\tnm430\tnm440\tnm450\tnm460\tnm470\tnm480\tnm490\tnm500\tnm510\tnm520\tnm530\tnm540\tnm550\tnm560\tnm570\tnm580\tnm590\tnm600\tnm610\tnm620\tnm630\tnm640\tnm650\tnm660\tnm670\tnm680\tnm690\tnm700\tnm710\tnm720\tnm730"; 
		uNewNumberOfFields += MM_STANDART_SPECTRUM_SIZE; 
	}
	sout << "LGOROWLENGTH\t" << m_uRowLength << endl;
	sout << "NumberOfStrips\t" << m_uNumberOfPages << endl;
	sout << "ILLUMINATION_NAME\t\"D50\"" << endl;
	sout << "OBSERVER_ANGLE\t\"2\"" << endl;
	sout << "MEASUREMENT_SOURCE\t\"Illumination=D50  WhiteBase=Absolute  Filter=UVcut  ObserverAngle=2°\"" << endl;
	sout << "MEASUREMENT_MODE\t\"patch\"" << endl;
	sout << "KEYWORD\t\"DEVCALSTD\"" << endl;
	sout << "DEVCALSTD\t\"XRGA\"" << endl;
	sout << "KEYWORD\t\"SampleID\"" << endl;
	sout << "KEYWORD\t\"SAMPLE_NAME\"" << endl;
	sout << endl;
	sout << "NUMBER_OF_FIELDS\t" << uNewNumberOfFields << endl;
	sout << "BEGIN_DATA_FORMAT" << endl;
	sout << strDataFormat << endl;
	sout << "END_DATA_FORMAT" << endl;
	sout << endl;
	sout << "NUMBER_OF_SETS\t" << m_uNumberOfSets << endl;
	sout << "BEGIN_DATA" << endl;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
	{
		sout << (p + 1) /*m_pPatches[p]->m_uRefID*/ << "\t" << m_pPatches[p]->m_strRefName;
		// Write reference data in RGB/CMYK format only if it's not specified to write measurement data in this formats.
		if((m_pPatches[p]->m_RefData.m_bHasRGB) && (!(uOutputDeviceData & MM_DD_RGB))) sout << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_RGB_R] << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_RGB_G] << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_RGB_B];
		if((m_pPatches[p]->m_RefData.m_bHasCMYK) && (!(uOutputDeviceData & MM_DD_CMYK))) sout << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_CMYK_C] << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_CMYK_M] << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_CMYK_Y] << "\t" << m_pPatches[p]->m_RefData.m_faData[MM_CMYK_K];
		if(m_pPatches[p]->m_uMeasuresCount > 0) 
		{
			unsigned short uMeasure = (nMeasure == MM_MEASURE_BEST_BY_DIST ? m_pPatches[p]->m_uBestByDistMeasure : (nMeasure == MM_MEASURE_BEST ? m_pPatches[p]->m_uBestMeasure : (unsigned short)nMeasure));
			if(uMeasure >= m_pPatches[p]->m_uMeasuresCount) uMeasure = m_pPatches[p]->m_uBestByDistMeasure;
			CMultiMeasureColorData *pBestMeasure = m_pPatches[p]->m_pMeasureData[uMeasure];
			if((uOutputDeviceData & MM_DD_RGB) && (pBestMeasure->m_bHasRGB)) sout << "\t" << pBestMeasure->m_faData[MM_RGB_R] << "\t" << pBestMeasure->m_faData[MM_RGB_G] << "\t" << pBestMeasure->m_faData[MM_RGB_B];
			if((uOutputDeviceData & MM_DD_CMYK) && (pBestMeasure->m_bHasCMYK)) sout << "\t" << pBestMeasure->m_faData[MM_CMYK_C] << "\t" << pBestMeasure->m_faData[MM_CMYK_M] << "\t" << pBestMeasure->m_faData[MM_CMYK_Y] << "\t" << pBestMeasure->m_faData[MM_CMYK_K];
			if(pBestMeasure->m_bHasLab) sout << "\t" << pBestMeasure->m_faData[MM_LAB_L] << "\t" << pBestMeasure->m_faData[MM_LAB_a] << "\t" << pBestMeasure->m_faData[MM_LAB_b];
			if(pBestMeasure->m_bHasXYZ) sout << "\t" << pBestMeasure->m_faData[MM_XYZ_X] << "\t" << pBestMeasure->m_faData[MM_XYZ_Y] << "\t" << pBestMeasure->m_faData[MM_XYZ_Z];
			if(pBestMeasure->m_bHasSpectrum)
				for(unsigned short s = 0; s < pBestMeasure->m_uSpectrumSize; s++) sout << "\t" << pBestMeasure->m_faSpectrum[s];
		}
		sout << endl;
	}
	sout << "END_DATA" << endl;

	FILE* fp;
	if (fopen_s(&fp, (strOutFileName + ".txt").c_str(), "w") == 0)
	{
		fputs(sout.str().c_str(), fp);
		fclose(fp);
	}
	return MM_OK;
}

// Writes measuremens data to file in Epson ColorBase format
unsigned short CMultiMeasure::WriteMeasurementFileCB(string strOutFileName, int nMeasure/* = MM_MEASURE_BEST_BY_DIST*/)
{
	stringstream sout2;
	sout2 << "LGOROWLENGTH " << m_uRowLength << "\nCREATED             \"11/2/2004\"  # Time: 20:25\nINSTRUMENTATION     \"SpectroScan\"\nMEASUREMENT_SOURCE     \"Illumination=D50\"	ObserverAngle=2°	WhiteBase=Abs	Filter=Unknown\nKEYWORD             \"SampleID\"\nKEYWORD             \"SampleName\"\nBEGIN_DATA_FORMAT\n";
	string strDataFormat = "SampleID SampleName";
	bool bHasLab = false, bHasXYZ = false;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
		if(m_pPatches[p]->m_uMeasuresCount > 0)
		{
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasLab) bHasLab = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasXYZ) bHasXYZ = true;
		}
	if(bHasXYZ) strDataFormat += "\tXYZ_X\tXYZ_Y\tXYZ_Z";
	if(bHasLab) strDataFormat += "\tLab_L\tLab_a\tLab_b";
	sout2 << strDataFormat;
	sout2 << "\nEND_DATA_FORMAT\nNUMBER_OF_SETS " << m_uNumberOfSets << "\nBEGIN_DATA" << endl;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
	{
		sout2 << m_pPatches[p]->m_uRefID << "\t" << m_pPatches[p]->m_strRefName;
		if(m_pPatches[p]->m_uMeasuresCount > 0) 
		{
			unsigned short uMeasure = (nMeasure == MM_MEASURE_BEST_BY_DIST ? m_pPatches[p]->m_uBestByDistMeasure : (nMeasure == MM_MEASURE_BEST ? m_pPatches[p]->m_uBestMeasure : (unsigned short)nMeasure));
			if(uMeasure >= m_pPatches[p]->m_uMeasuresCount) uMeasure = m_pPatches[p]->m_uBestByDistMeasure;
			CMultiMeasureColorData *pBestMeasure = m_pPatches[p]->m_pMeasureData[uMeasure];
			if(pBestMeasure->m_bHasXYZ) sout2 << "\t" << pBestMeasure->m_faData[MM_XYZ_X] << "\t" << pBestMeasure->m_faData[MM_XYZ_Y] << "\t" << pBestMeasure->m_faData[MM_XYZ_Z];
			if(pBestMeasure->m_bHasLab) sout2 << "\t" << pBestMeasure->m_faData[MM_LAB_L] << "\t" << pBestMeasure->m_faData[MM_LAB_a] << "\t" << pBestMeasure->m_faData[MM_LAB_b];
		}
		sout2 << endl;
	}
	sout2 << "END_DATA" << endl;

	FILE* fp;
	if (fopen_s(&fp, (strOutFileName + "_CB.txt").c_str(), "w") == 0)
	{
		fputs(sout2.str().c_str(), fp);
		fclose(fp);
	}
	return MM_OK;
}

// Writes measuremens data to file in Agryll format
unsigned short CMultiMeasure::WriteMeasurementFileTI3(string strOutFileName, int nMeasure/* = MM_MEASURE_BEST_BY_DIST*/)
{
	stringstream sout;
	string strDataFormat = "SAMPLE_ID SAMPLE_LOC", strColorRep = "";
	unsigned short uNewNumberOfFields = 2;
	if(m_pPatches[0]->m_RefData.m_bHasRGB) { strDataFormat += " RGB_R RGB_G RGB_B"; uNewNumberOfFields += 3; strColorRep = "iRGB"; }
	if(m_pPatches[0]->m_RefData.m_bHasCMYK) 
	{ 
		if(m_strAgryll_COLOR_REP == "CMY") { strDataFormat += " CMY_C CMY_M CMY_Y"; uNewNumberOfFields += 3; strColorRep = "CMY"; }
		else { strDataFormat += " CMYK_C CMYK_M CMYK_Y CMYK_K"; uNewNumberOfFields += 4; strColorRep = "CMYK"; }
	}
	bool bHasLab = false, bHasXYZ = false, bHasSpectrum  = false;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
		if(m_pPatches[p]->m_uMeasuresCount > 0)
		{
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasLab) bHasLab = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasXYZ) bHasXYZ = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasSpectrum) bHasSpectrum = true;
		}
	if(bHasXYZ) { strDataFormat += " XYZ_X XYZ_Y XYZ_Z"; uNewNumberOfFields += 3; if(!bHasLab) strColorRep += "_XYZ"; }
	if(bHasLab) { strDataFormat += " LAB_L LAB_A LAB_B"; uNewNumberOfFields += 3; strColorRep += "_LAB"; }
	if(bHasSpectrum)
	{ 
		strDataFormat += " SPEC_380 SPEC_390 SPEC_400 SPEC_410 SPEC_420 SPEC_430 SPEC_440 SPEC_450 SPEC_460 SPEC_470 SPEC_480 SPEC_490 SPEC_500 SPEC_510 SPEC_520 SPEC_530 SPEC_540 SPEC_550 SPEC_560 SPEC_570 SPEC_580 SPEC_590 SPEC_600 SPEC_610 SPEC_620 SPEC_630 SPEC_640 SPEC_650 SPEC_660 SPEC_670 SPEC_680 SPEC_690 SPEC_700 SPEC_710 SPEC_720 SPEC_730 "; 
		uNewNumberOfFields += MM_STANDART_SPECTRUM_SIZE; 
	}
	char timebuf[26];
	time_t ltime = time(NULL);
	ctime_s(timebuf, 26, &ltime); timebuf[24] = 0;
	sout << "CTI3   \n\nDESCRIPTOR \"Argyll Calibration Target chart information 3\"\nORIGINATOR \"Argyll target\"\nCREATED \"" << timebuf << "\"\nKEYWORD \"DEVICE_CLASS\"\nDEVICE_CLASS \"OUTPUT\"\n";
	if(m_uAgryll_SINGLE_DIM_STEPS > 0)
		sout << "KEYWORD \"SINGLE_DIM_STEPS\"\nSINGLE_DIM_STEPS \"" << m_uAgryll_SINGLE_DIM_STEPS <<"\"\n";
	if(m_uAgryll_COMP_GREY_STEPS > 0)
		sout << "KEYWORD \"COMP_GREY_STEPS\"\nCOMP_GREY_STEPS \"" << m_uAgryll_COMP_GREY_STEPS <<"\"\n";
	if(m_strAgryll_TOTAL_INK_LIMIT != "")
		sout << "KEYWORD \"TOTAL_INK_LIMIT\"\nTOTAL_INK_LIMIT \"" << m_strAgryll_TOTAL_INK_LIMIT <<"\"\n";
	sout << "KEYWORD \"COLOR_REP\"\nCOLOR_REP \"" << strColorRep << "\"\nKEYWORD \"TARGET_INSTRUMENT\"\nTARGET_INSTRUMENT \"X-Rite ColorMunki\"\n";

	if(bHasSpectrum)
		sout << "KEYWORD \"SPECTRAL_BANDS\"\nSPECTRAL_BANDS \"36\"\nKEYWORD \"SPECTRAL_START_NM\"\nSPECTRAL_START_NM \"380.000000\"\nKEYWORD \"SPECTRAL_END_NM\"\nSPECTRAL_END_NM \"730.000000\"\n";
	sout << "\nKEYWORD \"SAMPLE_LOC\"\n";
	if(bHasSpectrum)
		sout << "KEYWORD \"SPEC_380\"\nKEYWORD \"SPEC_390\"\nKEYWORD \"SPEC_400\"\nKEYWORD \"SPEC_410\"\nKEYWORD \"SPEC_420\"\nKEYWORD \"SPEC_430\"\nKEYWORD \"SPEC_440\"\nKEYWORD \"SPEC_450\"\nKEYWORD \"SPEC_460\"\nKEYWORD \"SPEC_470\"\nKEYWORD \"SPEC_480\"\nKEYWORD \"SPEC_490\"\nKEYWORD \"SPEC_500\"\nKEYWORD \"SPEC_510\"\nKEYWORD \"SPEC_520\"\nKEYWORD \"SPEC_530\"\nKEYWORD \"SPEC_540\"\nKEYWORD \"SPEC_550\"\nKEYWORD \"SPEC_560\"\nKEYWORD \"SPEC_570\"\nKEYWORD \"SPEC_580\"\nKEYWORD \"SPEC_590\"\nKEYWORD \"SPEC_600\"\nKEYWORD \"SPEC_610\"\nKEYWORD \"SPEC_620\"\nKEYWORD \"SPEC_630\"\nKEYWORD \"SPEC_640\"\nKEYWORD \"SPEC_650\"\nKEYWORD \"SPEC_660\"\nKEYWORD \"SPEC_670\"\nKEYWORD \"SPEC_680\"\nKEYWORD \"SPEC_690\"\nKEYWORD \"SPEC_700\"\nKEYWORD \"SPEC_710\"\nKEYWORD \"SPEC_720\"\nKEYWORD \"SPEC_730\"\n";

	sout << "NUMBER_OF_FIELDS " << uNewNumberOfFields << "\nBEGIN_DATA_FORMAT\n" << strDataFormat << "\nEND_DATA_FORMAT\n\nNUMBER_OF_SETS " << m_uNumberOfSets << "\nBEGIN_DATA\n";
	CMultiMeasurePatch **pTempPatches = m_pAgryllPatches != NULL ? m_pAgryllPatches : m_pPatches;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
	{
		sout << pTempPatches[p]->m_uRefID << " \"" << pTempPatches[p]->m_strRefName << "\"";
		if(pTempPatches[p]->m_RefData.m_bHasRGB) sout << " " << pTempPatches[p]->m_RefData.m_faData[MM_RGB_R] / 2.55f << " " << pTempPatches[p]->m_RefData.m_faData[MM_RGB_G] / 2.55f << " " << pTempPatches[p]->m_RefData.m_faData[MM_RGB_B] / 2.55f;
		if(pTempPatches[p]->m_RefData.m_bHasCMYK) 
		{
			sout << " " << pTempPatches[p]->m_RefData.m_faData[MM_CMYK_C] << " " << pTempPatches[p]->m_RefData.m_faData[MM_CMYK_M] << " " << pTempPatches[p]->m_RefData.m_faData[MM_CMYK_Y];
			if(m_strAgryll_COLOR_REP != "CMY") sout << " " << pTempPatches[p]->m_RefData.m_faData[MM_CMYK_K];
		}
		if(pTempPatches[p]->m_uMeasuresCount > 0) 
		{
			unsigned short uMeasure = (nMeasure == MM_MEASURE_BEST_BY_DIST ? pTempPatches[p]->m_uBestByDistMeasure : (nMeasure == MM_MEASURE_BEST ? pTempPatches[p]->m_uBestMeasure : (unsigned short)nMeasure));
			if(uMeasure >= pTempPatches[p]->m_uMeasuresCount) uMeasure = m_pPatches[p]->m_uBestByDistMeasure;
			CMultiMeasureColorData *pBestMeasure = pTempPatches[p]->m_pMeasureData[uMeasure];
			if(pBestMeasure->m_bHasXYZ) sout << " " << pBestMeasure->m_faData[MM_XYZ_X] << " " << pBestMeasure->m_faData[MM_XYZ_Y] << " " << pBestMeasure->m_faData[MM_XYZ_Z];
			if(pBestMeasure->m_bHasLab) sout << " " << pBestMeasure->m_faData[MM_LAB_L] << " " << pBestMeasure->m_faData[MM_LAB_a] << " " << pBestMeasure->m_faData[MM_LAB_b];
			if(pBestMeasure->m_bHasSpectrum)
				for(unsigned short s = 0; s < pBestMeasure->m_uSpectrumSize; s++) sout << " " << pBestMeasure->m_faSpectrum[s] * 100.0f;
		}
		sout << endl;
	}
	sout << "END_DATA\n";
	if(m_strArgillCalibrationInfo != "")
		sout << m_strArgillCalibrationInfo << endl;

	FILE* fp;
	if (fopen_s(&fp, (strOutFileName + ".ti3").c_str(), "w") == 0)
	{
		fputs(sout.str().c_str(), fp);
		fclose(fp);
	}
	return MM_OK;
}

// Some experiments with per channel Epson printer calibration
unsigned short CMultiMeasure::WriteMeasurementFileTI3CB(string strOutFileName, bool bSet/* = false*/, int nMeasure/* = MM_MEASURE_BEST_BY_DIST*/)
{
	float faCMYKRef[264][8];
	unsigned short uPatchesCount = 1;
	ifstream stmInFile("Grad.txt");
	if(stmInFile.fail()) return MM_FILE_TYPE_NONE;
	stringext strLine;
	while(getline(stmInFile, strLine))
	{
		if(strLine == "") continue;
		vector<stringext> strLineFields = strLine.split("\t ");
		if(strLineFields[0].upper() == "BEGIN_DATA")
		{
			for(unsigned short p = 0; p < 264; p++)
			{
				getline(stmInFile, strLine);
				vector<stringext> strLineDataFields = strLine.split("\t ");
				for(unsigned short c = 0; c < 8; c++)
					faCMYKRef[p][c] = stof(strLineDataFields[c]);
				if((faCMYKRef[p][0 + bSet * 4] != 0.0f) || (faCMYKRef[p][1 + bSet * 4] != 0.0f) || (faCMYKRef[p][2 + bSet * 4] != 0.0f) || (faCMYKRef[p][3 + bSet * 4] != 0.0f))
					uPatchesCount++;
			}
		}
	}

	stringstream sout;
	string strDataFormat = "SAMPLE_ID SAMPLE_LOC", strColorRep = "";
	unsigned short uNewNumberOfFields = 2;
	strDataFormat += " CMYK_C CMYK_M CMYK_Y CMYK_K"; uNewNumberOfFields += 4; strColorRep = "CMYK";
//	strDataFormat += " CMYKcmk1k_C CMYKcmk1k_M CMYKcmk1k_Y CMYKcmk1k_K CMYKcmk1k_c CMYKcmk1k_m CMYKcmk1k_k CMYKcmk1k_1k"; uNewNumberOfFields += 8; strColorRep = "CMYKcmk1k";
	bool bHasLab = false, bHasXYZ = false, bHasSpectrum  = false;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
		if(m_pPatches[p]->m_uMeasuresCount > 0)
		{
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasLab) bHasLab = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasXYZ) bHasXYZ = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasSpectrum) bHasSpectrum = true;
		}
	if(bHasXYZ) { strDataFormat += " XYZ_X XYZ_Y XYZ_Z"; uNewNumberOfFields += 3; if(!bHasLab) strColorRep += "_XYZ"; }
	if(bHasLab) { strDataFormat += " LAB_L LAB_A LAB_B"; uNewNumberOfFields += 3; strColorRep += "_LAB"; }
	if(bHasSpectrum)
	{ 
		strDataFormat += " SPEC_380 SPEC_390 SPEC_400 SPEC_410 SPEC_420 SPEC_430 SPEC_440 SPEC_450 SPEC_460 SPEC_470 SPEC_480 SPEC_490 SPEC_500 SPEC_510 SPEC_520 SPEC_530 SPEC_540 SPEC_550 SPEC_560 SPEC_570 SPEC_580 SPEC_590 SPEC_600 SPEC_610 SPEC_620 SPEC_630 SPEC_640 SPEC_650 SPEC_660 SPEC_670 SPEC_680 SPEC_690 SPEC_700 SPEC_710 SPEC_720 SPEC_730 "; 
		uNewNumberOfFields += MM_STANDART_SPECTRUM_SIZE; 
	}
	char timebuf[26];
	time_t ltime = time(NULL);
	ctime_s(timebuf, 26, &ltime); timebuf[24] = 0;
	sout << "CTI3   \n\nDESCRIPTOR \"Argyll Calibration Target chart information 3\"\nORIGINATOR \"Argyll target\"\nCREATED \"" << timebuf << "\"\nKEYWORD \"DEVICE_CLASS\"\nDEVICE_CLASS \"OUTPUT\"\n";
	sout << "KEYWORD \"SINGLE_DIM_STEPS\"\nSINGLE_DIM_STEPS \"" << 32 <<"\"\n";
	sout << "KEYWORD \"COLOR_REP\"\nCOLOR_REP \"" << strColorRep << "\"\nKEYWORD \"TARGET_INSTRUMENT\"\nTARGET_INSTRUMENT \"X-Rite ColorMunki\"\n";

	if(bHasSpectrum)
		sout << "KEYWORD \"SPECTRAL_BANDS\"\nSPECTRAL_BANDS \"36\"\nKEYWORD \"SPECTRAL_START_NM\"\nSPECTRAL_START_NM \"380.000000\"\nKEYWORD \"SPECTRAL_END_NM\"\nSPECTRAL_END_NM \"730.000000\"\n";
	sout << "\nKEYWORD \"SAMPLE_LOC\"\n";
//	sout << "KEYWORD \"CMYKcmk1k_C\"\nKEYWORD \"CMYKcmk1k_M\"\nKEYWORD \"CMYKcmk1k_Y\"\nKEYWORD \"CMYKcmk1k_K\"\nKEYWORD \"CMYKcmk1k_c\"\nKEYWORD \"CMYKcmk1k_m\"\nKEYWORD \"CMYKcmk1k_k\"\nKEYWORD \"CMYKcmk1k_1k\"\n";
	if(bHasSpectrum)
		sout << "KEYWORD \"SPEC_380\"\nKEYWORD \"SPEC_390\"\nKEYWORD \"SPEC_400\"\nKEYWORD \"SPEC_410\"\nKEYWORD \"SPEC_420\"\nKEYWORD \"SPEC_430\"\nKEYWORD \"SPEC_440\"\nKEYWORD \"SPEC_450\"\nKEYWORD \"SPEC_460\"\nKEYWORD \"SPEC_470\"\nKEYWORD \"SPEC_480\"\nKEYWORD \"SPEC_490\"\nKEYWORD \"SPEC_500\"\nKEYWORD \"SPEC_510\"\nKEYWORD \"SPEC_520\"\nKEYWORD \"SPEC_530\"\nKEYWORD \"SPEC_540\"\nKEYWORD \"SPEC_550\"\nKEYWORD \"SPEC_560\"\nKEYWORD \"SPEC_570\"\nKEYWORD \"SPEC_580\"\nKEYWORD \"SPEC_590\"\nKEYWORD \"SPEC_600\"\nKEYWORD \"SPEC_610\"\nKEYWORD \"SPEC_620\"\nKEYWORD \"SPEC_630\"\nKEYWORD \"SPEC_640\"\nKEYWORD \"SPEC_650\"\nKEYWORD \"SPEC_660\"\nKEYWORD \"SPEC_670\"\nKEYWORD \"SPEC_680\"\nKEYWORD \"SPEC_690\"\nKEYWORD \"SPEC_700\"\nKEYWORD \"SPEC_710\"\nKEYWORD \"SPEC_720\"\nKEYWORD \"SPEC_730\"\n";

	sout << "NUMBER_OF_FIELDS " << uNewNumberOfFields << "\nBEGIN_DATA_FORMAT\n" << strDataFormat << "\nEND_DATA_FORMAT\n\nNUMBER_OF_SETS " << uPatchesCount << "\nBEGIN_DATA\n";
	CMultiMeasurePatch **pTempPatches = m_pPatches;
	unsigned short uPatchIndex = 1;
	for(unsigned short p = 0; p < 264; p++)
	{
		if((p == 0) || (faCMYKRef[p][0 + bSet * 4] != 0.0f) || (faCMYKRef[p][1 + bSet * 4] != 0.0f) || (faCMYKRef[p][2 + bSet * 4] != 0.0f) || (faCMYKRef[p][3 + bSet * 4] != 0.0f))
		{
			sout << uPatchIndex << " \"" << pTempPatches[p]->m_strRefName << "\"";
			for(unsigned short c = 0; c < 4; c++)
				sout << " " << faCMYKRef[p][c + bSet * 4] / 2.55f;
			if(pTempPatches[p]->m_uMeasuresCount > 0) 
			{
				unsigned short uMeasure = (nMeasure == MM_MEASURE_BEST_BY_DIST ? pTempPatches[p]->m_uBestByDistMeasure : (nMeasure == MM_MEASURE_BEST ? pTempPatches[p]->m_uBestMeasure : (unsigned short)nMeasure));
				if(uMeasure >= pTempPatches[p]->m_uMeasuresCount) uMeasure = m_pPatches[p]->m_uBestByDistMeasure;
				CMultiMeasureColorData *pBestMeasure = pTempPatches[p]->m_pMeasureData[uMeasure];
				if(pBestMeasure->m_bHasXYZ) sout << " " << pBestMeasure->m_faData[MM_XYZ_X] << " " << pBestMeasure->m_faData[MM_XYZ_Y] << " " << pBestMeasure->m_faData[MM_XYZ_Z];
				if(pBestMeasure->m_bHasLab) sout << " " << pBestMeasure->m_faData[MM_LAB_L] << " " << pBestMeasure->m_faData[MM_LAB_a] << " " << pBestMeasure->m_faData[MM_LAB_b];
				if(pBestMeasure->m_bHasSpectrum)
					for(unsigned short s = 0; s < pBestMeasure->m_uSpectrumSize; s++) sout << " " << pBestMeasure->m_faSpectrum[s] * 100.0f;
			}
			sout << endl;
			uPatchIndex++;
		}
	}
	sout << "END_DATA\n";
	if(m_strArgillCalibrationInfo != "")
		sout << m_strArgillCalibrationInfo << endl;

	FILE* fp;
	if (fopen_s(&fp, (strOutFileName + ".ti3").c_str(), "w") == 0)
	{
		fputs(sout.str().c_str(), fp);
		fclose(fp);
	}
	return MM_OK;
}

// Writes measuremens data to file in ProfileMaker 5 format with CMYK densities
unsigned short CMultiMeasure::WriteMeasurementFilePM5Density(string strOutFileName, int nMeasure/* = MM_MEASURE_BEST_BY_DIST*/)
{
	stringstream sout;
	string strDataFormat = "SampleID\tSAMPLE_NAME";
	unsigned short uNewNumberOfFields = 2;
	if(m_pPatches[0]->m_RefData.m_bHasRGB) { strDataFormat += "\tRGB_R\tRGB_G\tRGB_B"; uNewNumberOfFields += 3; }
	if(m_pPatches[0]->m_RefData.m_bHasCMYK) { strDataFormat += "\tCMYK_C\tCMYK_M\tCMYK_Y\tCMYK_K"; uNewNumberOfFields += 4; }
	bool bHasLab = false, bHasXYZ = false, bHasCMYK  = false;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
		if(m_pPatches[p]->m_uMeasuresCount > 0)
		{
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasLab) bHasLab = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasXYZ) bHasXYZ = true;
			if(m_pPatches[p]->m_pMeasureData[0]->m_bHasCMYK) bHasCMYK = true;
		}
	if(bHasXYZ) { strDataFormat += "\tXYZ_X\tXYZ_Y\tXYZ_Z"; uNewNumberOfFields += 3; }
	if(bHasLab) { strDataFormat += "\tLAB_L\tLAB_A\tLAB_B"; uNewNumberOfFields += 3; }
	if(bHasCMYK) { strDataFormat += "\tDCyan\tDMagenta\tDYellow\tDBlack"; uNewNumberOfFields += 4; }
	sout << "LGOROWLENGTH\t" << m_uRowLength << endl;
	sout << "NumberOfStrips\t" << m_uNumberOfPages << endl;
	sout << "ILLUMINATION_NAME\t\"D50\"" << endl;
	sout << "OBSERVER_ANGLE\t\"2\"" << endl;
	sout << "MEASUREMENT_SOURCE\t\"Illumination=D50  WhiteBase=Absolute  Filter=UVcut  ObserverAngle=2°\"" << endl;
	sout << "MEASUREMENT_MODE\t\"patch\"" << endl;
	sout << "KEYWORD\t\"DEVCALSTD\"" << endl;
	sout << "DEVCALSTD\t\"XRGA\"" << endl;
	sout << "KEYWORD\t\"SampleID\"" << endl;
	sout << "KEYWORD\t\"SAMPLE_NAME\"" << endl;
	sout << endl;
	sout << "NUMBER_OF_FIELDS\t" << uNewNumberOfFields << endl;
	sout << "BEGIN_DATA_FORMAT" << endl;
	sout << strDataFormat << endl;
	sout << "END_DATA_FORMAT" << endl;
	sout << endl;
	sout << "NUMBER_OF_SETS\t" << m_uNumberOfSets << endl;
	sout << "BEGIN_DATA" << endl;
	for(unsigned short p = 0; p < m_uPatchesCount; p++)
	{
		sout << (p + 1) /*m_pAgryllPatches[p]->m_uRefID*/ << "\t" << m_pAgryllPatches[p]->m_strRefName;
		if(m_pAgryllPatches[p]->m_RefData.m_bHasRGB) sout << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_RGB_R] << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_RGB_G] << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_RGB_B];
		if(m_pAgryllPatches[p]->m_RefData.m_bHasCMYK) sout << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_CMYK_C] << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_CMYK_M] << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_CMYK_Y] << "\t" << m_pAgryllPatches[p]->m_RefData.m_faData[MM_CMYK_K];
		if(m_pAgryllPatches[p]->m_uMeasuresCount > 0) 
		{
			unsigned short uMeasure = (nMeasure == MM_MEASURE_BEST_BY_DIST ? m_pAgryllPatches[p]->m_uBestByDistMeasure : (nMeasure == MM_MEASURE_BEST ? m_pAgryllPatches[p]->m_uBestMeasure : (unsigned short)nMeasure));
			if(uMeasure >= m_pAgryllPatches[p]->m_uMeasuresCount) uMeasure = m_pAgryllPatches[p]->m_uBestByDistMeasure;
			CMultiMeasureColorData *pBestMeasure = m_pAgryllPatches[p]->m_pMeasureData[uMeasure];
			if(pBestMeasure->m_bHasXYZ) sout << "\t" << pBestMeasure->m_faData[MM_XYZ_X] << "\t" << pBestMeasure->m_faData[MM_XYZ_Y] << "\t" << pBestMeasure->m_faData[MM_XYZ_Z];
			if(pBestMeasure->m_bHasLab) sout << "\t" << pBestMeasure->m_faData[MM_LAB_L] << "\t" << pBestMeasure->m_faData[MM_LAB_a] << "\t" << pBestMeasure->m_faData[MM_LAB_b];
			if(pBestMeasure->m_bHasCMYK) sout << "\t" << pBestMeasure->m_faData[MM_CMYK_C] << "\t" << pBestMeasure->m_faData[MM_CMYK_M] << "\t" << pBestMeasure->m_faData[MM_CMYK_Y] << "\t" << pBestMeasure->m_faData[MM_CMYK_K];
		}
		sout << endl;
	}
	sout << "END_DATA" << endl;

	FILE* fp;
	if (fopen_s(&fp, (strOutFileName + ".txt").c_str(), "w") == 0)
	{
		fputs(sout.str().c_str(), fp);
		fclose(fp);
	}
	return MM_OK;
}

// Reads measurements form previously saved file
unsigned short CMultiMeasure::ReadMeasurementsFile(string strInFileName, unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	unsigned short uNumberOfSets = 0;
	int naDataIndex[14], nIDIndex, nNameIndex;
	ifstream stmInFile(strInFileName.c_str());
	if(stmInFile.fail()) { cout << "0"; return MM_ERROR; }
	stringext strLine, strInFileExt = strInFileName.substr(strInFileName.length()-4, 4);
	bool bAgryll = (strInFileExt == ".ti3");
	if(bAgryll && (m_pAgryllPatches == NULL))  { stmInFile.close(); cout << "1"; return MM_ERROR; }

	while(getline(stmInFile, strLine))
	{
		if(strLine == "") continue;
		vector<stringext> strLineFields = strLine.split("\t ");
		if(strLineFields[0].upper() == "NUMBER_OF_SETS") 
		{ 
			uNumberOfSets = stoi(strLineFields[1]); 
			if(uNumberOfSets != m_uNumberOfSets) { stmInFile.close(); cout << "2"; return MM_ERROR; }
		}
		if(strLineFields[0].upper() == "BEGIN_DATA_FORMAT")
		{
			getline(stmInFile, strLine);
			vector<stringext> strLineDataFields = strLine.split("\t ");
			for(int i = 0; i < MM_COLOR_DATA_SIZE + 1; i++) naDataIndex[i] = -1;
			nIDIndex = nNameIndex = -1;
			string strDataCaptions[] = { "LAB_L", "LAB_A", "LAB_B", "RGB_R", "RGB_G", "RGB_B", "XYZ_X", "XYZ_Y", "XYZ_Z", "CMYK_C", "CMYK_M", "CMYK_Y", "CMYK_K", (bAgryll ? "SPEC_380" : "NM380") };
			if(m_strAgryll_COLOR_REP == "CMY") { strDataCaptions[MM_CMYK_C] = "CMY_C"; strDataCaptions[MM_CMYK_M] = "CMY_M"; strDataCaptions[MM_CMYK_Y] = "CMY_Y"; }
			for(unsigned short col = 0; col < strLineDataFields.size(); col++)
			{
				if((strLineDataFields[col].upper() == "SAMPLEID") || (strLineDataFields[col].upper() == "SAMPLE_ID")) nIDIndex = col;
				if((strLineDataFields[col].upper() == "SAMPLE_NAME") || (strLineDataFields[col].upper() == "SAMPLENAME") || (strLineDataFields[col].upper() == "SAMPLE_LOC")) nNameIndex = col;
				for(int i = 0; i < MM_COLOR_DATA_SIZE + 1; i++) if(strLineDataFields[col].upper() == strDataCaptions[i]) naDataIndex[i] = col;
			}
		}
		if(strLineFields[0].upper() == "BEGIN_DATA")
		{
			float faTempData[MM_COLOR_DATA_SIZE], *faTempSpectrum = NULL;
			unsigned short uRefID, uSpectrumSize = 0;
			stringext strRefName;
			CMultiMeasurePatch ** m_pTempPatches = (bAgryll && (m_pAgryllPatches != NULL)) ? m_pAgryllPatches : m_pPatches;
			for(unsigned short p = 0; p < uNumberOfSets; p++)
			{
				getline(stmInFile, strLine);
				vector<stringext> strLineDataFields = strLine.split("\t ");
				uRefID = nIDIndex != -1 ? stoi(strLineDataFields[nIDIndex]) : 0;
				strRefName = nNameIndex != -1 ? (bAgryll ? strLineDataFields[nNameIndex].cutQuotes() : strLineDataFields[nNameIndex]) : "";
				if(strRefName != m_pTempPatches[p]->m_strRefName) { stmInFile.close(); cout << "3"; return MM_ERROR; }
				for(unsigned short i = 0; i < MM_COLOR_DATA_SIZE; i++) faTempData[i] = ((naDataIndex[i] != -1) && (naDataIndex[i] < (int)strLineDataFields.size())) ? stof(strLineDataFields[naDataIndex[i]]) : MM_NO_COLOR_DATA;
				if(bAgryll) for(int i = MM_RGB_R; i <= MM_RGB_B; i++) if(faTempData[i] != MM_NO_COLOR_DATA) faTempData[i] *= 2.55f;
				if((naDataIndex[MM_COLOR_DATA_SIZE] != -1) && ((naDataIndex[MM_COLOR_DATA_SIZE] + MM_STANDART_SPECTRUM_SIZE) <= (int)strLineDataFields.size()))
				{
					uSpectrumSize = MM_STANDART_SPECTRUM_SIZE;
					faTempSpectrum = new float[MM_STANDART_SPECTRUM_SIZE];
					for(unsigned short s = 0; s < MM_STANDART_SPECTRUM_SIZE; s++)
						faTempSpectrum[s] = stof(strLineDataFields[naDataIndex[MM_COLOR_DATA_SIZE] + s]) / (bAgryll ? 100.0f : 1.0f);
				}
				else { uSpectrumSize = 0; faTempSpectrum = NULL; }
				if((faTempData[MM_LAB_L] != MM_NO_COLOR_DATA) && (faTempData[MM_LAB_a] != MM_NO_COLOR_DATA) && (faTempData[MM_LAB_b] != MM_NO_COLOR_DATA))
					m_pTempPatches[p]->AddMeasureData(new CMultiMeasureColorData(faTempData, uSpectrumSize, faTempSpectrum), uStandart);
			}
		}
		if(bAgryll && (strLineFields[0].upper() == "CAL")) break;
	}
	// Increases the measurement counter for strips with newly read data
	for(unsigned short s = 0; s < m_uStripsCount; s++)
	{
		unsigned short p = 0;
		while((p < m_pStrips[s]->m_uStripSize) && (m_pStrips[s]->m_pStripPatches[p]->m_uMeasuresCount > m_pStrips[s]->m_uStripMeasuresCount)) p++;
		if(p == m_pStrips[s]->m_uStripSize)
		{
			m_pStrips[s]->m_faMeasureTime[m_pStrips[s]->m_uStripMeasuresCount] = 0;
			m_pStrips[s]->m_uStripMeasuresCount++;
			m_pStrips[s]->CalculateStatistic();
		}
	}
	return MM_OK;
}

// Swithes strips measurement direction; it's may by necessary for charts generatied by x-rite products
void CMultiMeasure::TransposeStrips()
{
	CMultiMeasureStrip **pTempStrips = new CMultiMeasureStrip *[m_uStripSize];
	for(unsigned short s = 0; s < m_uStripSize; s++)
	{
		unsigned short uNewStripSize = m_uStripsCount;
		while(m_pStrips[uNewStripSize-1]->m_uStripSize <= s) uNewStripSize--;
		pTempStrips[s] = new CMultiMeasureStrip(uNewStripSize);
		for(unsigned short p = 0; p < uNewStripSize; p++)
			pTempStrips[s]->m_pStripPatches[p] = m_pStrips[p]->m_pStripPatches[s];
	}
	for(unsigned short s = 0; s < m_uStripsCount; s++) delete m_pStrips[s];
	delete [] m_pStrips;
	m_pStrips = pTempStrips;
	unsigned short uTemp = m_uStripsCount; m_uStripsCount = m_uStripSize; m_uStripSize = uTemp;
}

void CMultiMeasure::SetDevice(CMultiMeasureDevice *pDevice)
{
	m_pDevice = pDevice;
}

// Uploads the reference data of the specified strip to device; it's necessary for strip correlation measurement mode.
unsigned short CMultiMeasure::SetReferenseLine(unsigned short &uStripIndex, unsigned short uFromPatchIndex/* = 0*/, unsigned short uPatchesNumber/* = 0*/)
{
	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
	if(uFromPatchIndex >= m_pStrips[uStripIndex]->m_uStripSize) uFromPatchIndex = m_pStrips[uStripIndex]->m_uStripSize - 1;
	if((uPatchesNumber == 0) || ((uFromPatchIndex + uPatchesNumber) > m_pStrips[uStripIndex]->m_uStripSize)) uPatchesNumber = m_pStrips[uStripIndex]->m_uStripSize - uFromPatchIndex;
	string strColorSpace = "";
	unsigned short uColorDataIndex = 0, uColorDataSize = 3;
	if(m_pStrips[uStripIndex]->m_pStripPatches[0]->m_RefData.m_bHasLab)	{ strColorSpace = MUNKI_REFERENCE_CHART_LAB; uColorDataIndex = MM_LAB_L; }
	else if(m_pStrips[uStripIndex]->m_pStripPatches[0]->m_RefData.m_bHasRGB) { strColorSpace = MUNKI_REFERENCE_CHART_RGB; uColorDataIndex = MM_RGB_R; }
	else if(m_pStrips[uStripIndex]->m_pStripPatches[0]->m_RefData.m_bHasCMYK) { strColorSpace = MUNKI_REFERENCE_CHART_CMYK; uColorDataIndex = MM_CMYK_C; uColorDataSize = 4;}
	else return MM_ERROR_NO_REFERENCE_COLOR_DATA;
	float *pRefData = new float[uPatchesNumber * uColorDataSize];
	for(unsigned short p = 0; p < uPatchesNumber; p++)
		for(unsigned short i = 0; i < uColorDataSize; i++)
			pRefData[p * uColorDataSize + i] = m_pStrips[uStripIndex]->m_pStripPatches[p + uFromPatchIndex]->m_RefData.m_faData[uColorDataIndex + i];
	unsigned short uResult = m_pDevice->SetRecognitionReference(strColorSpace, pRefData, uPatchesNumber); 
	if(uResult != eNoError) { delete [] pRefData; m_uLastDeviceError = uResult; return MM_ERROR_SETTING_REFERENCE_LINE; }
	delete [] pRefData;
	return MM_OK;
}

// Measures whole strip in a single pass
//unsigned short CMultiMeasure::MeasureStrip(unsigned short &uStripIndex, unsigned short uStandart/* = MM_DELTA_E_76*/)
//{
//	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
//	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
//	DWORD dwBeginReadingTimer = GetTickCount();
//	unsigned short uResult = m_pDevice->DoMeasurements(); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_DOING_MEASUREMENTS; }
//	float fReadingTime = ((float)(GetTickCount() - dwBeginReadingTimer)) / 1000;
//	long nResultsNumber = m_pDevice->GetResultsNumber();
//	
//	cout << " " << nResultsNumber << " ";
//
//	if(nResultsNumber != m_pStrips[uStripIndex]->m_uStripSize) return MM_ERROR_PATCHES_NUMBER_DONT_MATCH;
//	float faData[MM_COLOR_DATA_SIZE], *faSpectrum;
//	unsigned short uSpectrumSize;
//	for(unsigned short p = 0; p < nResultsNumber; p++)
//	{
//		uResult = m_pDevice->GetResult(p, faData, uSpectrumSize, faSpectrum); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_GETTING_RESULTS; }
//		m_pStrips[uStripIndex]->m_pStripPatches[p]->AddMeasureData(new CMultiMeasureColorData(faData, uSpectrumSize, faSpectrum, fReadingTime), uStandart);
//	}
//	m_pStrips[uStripIndex]->m_faMeasureTime[m_pStrips[uStripIndex]->m_uStripMeasuresCount] = fReadingTime;
//	m_pStrips[uStripIndex]->m_uStripMeasuresCount++;
//	m_pStrips[uStripIndex]->CalculateStatistic();
//	return MM_OK;
//}

// Colormunki Photo has another one disadvantage. It has relatively small buffer memory to store measurement data.
// Therefore, when you try to measure a long strip with a standard patch size and slowly move the device 
// to increase the number of readings, the buffer may overflow and some data will be lost.
// To avoid this, you can measure the long strip in parts. 
// This method split the strip into a specified number of blocks and measure each block individually in direct or reverse order.
//unsigned short CMultiMeasure::MeasureStripMultiPass(unsigned short &uStripIndex, unsigned short uPassesNumber/* = 2*/, bool bReverse/* = false*/, unsigned short uStandart/* = MM_DELTA_E_76*/)
//{
//	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
//	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
//	float fReadingTime = 0.0f;
//	for(short pass = (bReverse ? uPassesNumber - 1 : 0); bReverse ? pass >= 0 : pass < uPassesNumber; bReverse ? pass-- : pass++)
//	{
//		unsigned short uPatchesNumber = (unsigned short)floor((double)m_pStrips[uStripIndex]->m_uStripSize / uPassesNumber);
//		unsigned short uPatchesRemainder = m_pStrips[uStripIndex]->m_uStripSize - uPatchesNumber * uPassesNumber;
//		unsigned short uPatchFrom = pass * uPatchesNumber + (pass < uPatchesRemainder ? pass : uPatchesRemainder);
//		uPatchesNumber += (pass < uPatchesRemainder);
//
//		cout << " " << m_pStrips[uStripIndex]->m_pStripPatches[uPatchFrom]->m_strRefName << "-" << m_pStrips[uStripIndex]->m_pStripPatches[uPatchFrom + uPatchesNumber - 1]->m_strRefName << " ";
//
//		if(fReadingTime > 0.0f) m_pDevice->WaitForButton();
//		SetReferenseLine(uStripIndex, uPatchFrom, uPatchesNumber);
//		DWORD dwBeginReadingTimer = GetTickCount();
//		unsigned short uResult = m_pDevice->DoMeasurements(); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_DOING_MEASUREMENTS; }
//		float fPassReadingTime = ((float)(GetTickCount() - dwBeginReadingTimer)) / 1000;
//		fReadingTime += fPassReadingTime;
//		long nResultsNumber = m_pDevice->GetResultsNumber();
//		
//		cout << " " << nResultsNumber << " " << fPassReadingTime << " ";
//
//		if(nResultsNumber != uPatchesNumber) return MM_ERROR_PATCHES_NUMBER_DONT_MATCH;
//		float faData[MM_COLOR_DATA_SIZE], *faSpectrum;
//		unsigned short uSpectrumSize;
//		for(unsigned short p = 0; p < nResultsNumber; p++)
//		{
//			uResult = m_pDevice->GetResult(p, faData, uSpectrumSize, faSpectrum); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_GETTING_RESULTS; }
//			m_pStrips[uStripIndex]->m_pStripPatches[p + uPatchFrom]->AddMeasureData(new CMultiMeasureColorData(faData, uSpectrumSize, faSpectrum, fReadingTime), uStandart);
//		}
//	}
//	m_pStrips[uStripIndex]->m_faMeasureTime[m_pStrips[uStripIndex]->m_uStripMeasuresCount] = fReadingTime;
//	m_pStrips[uStripIndex]->m_uStripMeasuresCount++;
//	m_pStrips[uStripIndex]->CalculateStatistic();
//	return MM_OK;
//}

// Measures specified patches of the strip
unsigned short CMultiMeasure::MeasureStripPatches(unsigned short &uStripIndex, CMultiMeasureColorData **pMeasureData, float &fReadingTime, unsigned short uPatchFrom/* = 0*/, unsigned short uPatchesNumber/* = 0*/)
{
	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
	if(uPatchFrom >= m_pStrips[uStripIndex]->m_uStripSize) uPatchFrom = m_pStrips[uStripIndex]->m_uStripSize - 1;
	if((uPatchesNumber == 0) || ((uPatchFrom + uPatchesNumber) > m_pStrips[uStripIndex]->m_uStripSize)) uPatchesNumber = m_pStrips[uStripIndex]->m_uStripSize - uPatchFrom;
	DWORD dwBeginReadingTimer = GetTickCount();
	unsigned short uResult = m_pDevice->DoMeasurements(); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_DOING_MEASUREMENTS; }
	fReadingTime = ((float)(GetTickCount() - dwBeginReadingTimer)) / 1000;
	long nResultsNumber = m_pDevice->GetResultsNumber();
	if(nResultsNumber != uPatchesNumber) return MM_ERROR_PATCHES_NUMBER_DONT_MATCH;
	float faData[MM_COLOR_DATA_SIZE], *faSpectrum;
	unsigned short uSpectrumSize;
	for(unsigned short p = 0; p < nResultsNumber; p++)
	{
		uResult = m_pDevice->GetResult(p, faData, uSpectrumSize, faSpectrum); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_GETTING_RESULTS; }
		pMeasureData[p + uPatchFrom] = new CMultiMeasureColorData(faData, uSpectrumSize, faSpectrum, fReadingTime);
	}
	return MM_OK;
}

// Adds measurement data to the strip
unsigned short CMultiMeasure::AddStripMeasurements(unsigned short &uStripIndex, CMultiMeasureColorData **pMeasureData, float fReadingTime/* = 0.0f*/, unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
	for(unsigned short p = 0; p < m_pStrips[uStripIndex]->m_uStripSize; p++)
		if(pMeasureData[p] != NULL)
			m_pStrips[uStripIndex]->m_pStripPatches[p]->AddMeasureData(pMeasureData[p], uStandart);
	m_pStrips[uStripIndex]->m_faMeasureTime[m_pStrips[uStripIndex]->m_uStripMeasuresCount] = fReadingTime;
	m_pStrips[uStripIndex]->m_uStripMeasuresCount++;
	m_pStrips[uStripIndex]->CalculateStatistic();
	return MM_OK;
}

// Measures single patch of the strip
unsigned short CMultiMeasure::MeasurePatch(unsigned short &uStripIndex, unsigned short &uPatchIndex, unsigned short uStandart/* = MM_DELTA_E_76*/)
{
	if(uStripIndex >= m_uStripsCount) uStripIndex = m_uStripsCount;
	if(m_pStrips[uStripIndex]->m_uStripSize == 0) return MM_ERROR_NO_REFERENCE_CHART;
	if(uPatchIndex >= m_pStrips[uStripIndex]->m_uStripSize) uPatchIndex = m_pStrips[uStripIndex]->m_uStripSize;
	unsigned short uResult = m_pDevice->DoMeasurements(); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_DOING_MEASUREMENTS; }
	float faData[MM_COLOR_DATA_SIZE], *faSpectrum;
	unsigned short uSpectrumSize;
	uResult = m_pDevice->GetResult(0, faData, uSpectrumSize, faSpectrum); if(uResult != eNoError) { m_uLastDeviceError = uResult; return MM_ERROR_GETTING_RESULTS; }
	m_pStrips[uStripIndex]->m_pStripPatches[uPatchIndex]->AddMeasureData(new CMultiMeasureColorData(faData, uSpectrumSize, faSpectrum, 0), uStandart);
	m_pStrips[uStripIndex]->CalculateStatistic();

	cout << " Lab: " << faData[MM_LAB_L] << " " << faData[MM_LAB_a] << " " << faData[MM_LAB_b] << " ";

	return MM_OK;
}