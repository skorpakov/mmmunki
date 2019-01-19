#include "lib/ColorMunkiSDK/colormunki.h"
#include "lib/stringext.h"

// Color data file formats
#define MM_FILE_TYPE_NONE			0x00
#define MM_FILE_TYPE_PM5			0x01  // ProfileMaker 5
#define MM_FILE_TYPE_PM5_COLORBASE	0x02  // Epson ColorBase
#define MM_FILE_TYPE_TI2			0x04  // Agryll peference chart
#define MM_FILE_TYPE_TI3			0x08  // Agryll measurement data
#define MM_FILE_TYPE_PM5_DENSITY	0x10  // ProfileMaker 5 with colordensity

// Positions of color data components in color data array
#define MM_LAB_L	0
#define MM_LAB_a	1
#define MM_LAB_b	2
#define MM_RGB_R	3
#define MM_RGB_G	4
#define MM_RGB_B	5
#define MM_XYZ_X	6
#define MM_XYZ_Y	7
#define MM_XYZ_Z	8
#define MM_CMYK_C	9
#define MM_CMYK_M	10
#define MM_CMYK_Y	11
#define MM_CMYK_K	12
#define MM_SPECTRUM	13

// Device data formats
#define MM_DD_RGB	0x01
#define MM_DD_CMYK	0x02

// Color data parameters
#define MM_COLOR_DATA_SIZE 13
#define MM_STANDART_SPECTRUM_SIZE 36
#define MM_HIRES_SPECTRUM_SIZE 36
#define MM_NO_COLOR_DATA -9999.0f
#define MM_MAX_MEASURES_COUNT 100

// ColorMunkiSDK errors
#define MM_OK								0
#define MM_ERROR							1
#define MM_ERROR_NO_REFERENCE_CHART			2
#define MM_ERROR_NO_REFERENCE_COLOR_DATA	3
#define MM_ERROR_SETTING_REFERENCE_LINE		4
#define MM_ERROR_DOING_MEASUREMENTS			5
#define MM_ERROR_PATCHES_NUMBER_DONT_MATCH	6
#define MM_ERROR_GETTING_RESULTS			7

// Color difference calculation methods
#define MM_DELTA_E_76	1
#define MM_DELTA_E_94	2
#define MM_DELTA_E_2000	3

// Measurement optimisation methods
#define MM_MEASURE_BEST			-1
#define MM_MEASURE_BEST_BY_DIST	-2


using namespace std;
///////////////////////////////////////////////////////////
// Device access classes

// Abstract measurement device access class
class CMultiMeasureDevice
{
public:
	virtual int GetLastError(string &strErrorText) { return eUnknownError; }
	virtual bool IsConnected() { return false; }
	virtual void WaitForButton() {}
	virtual void WaitForButton(char &cKey) {}
	virtual unsigned short Calibrate(const string &strMeasureMode) { return eUnknownError; }
	virtual unsigned short SetMeasurementMode(const string &strMeasureMode) { return eUnknownError; }
	virtual unsigned short SetRecognitionReference(const string &strColorSpace, const float *pReferenceChartLine, long nLineSize) { return eUnknownError; }
	virtual unsigned short DoMeasurements() { return eUnknownError; }
	virtual long GetResultsNumber() { return 0; }
	virtual unsigned short GetResult(long nIndex, float faData[MM_COLOR_DATA_SIZE], unsigned short &uSpectrumSize, float *&faSpectrum) { return eUnknownError; }
	virtual void Reset() {}
};

// X-rite ColorMunki device access class
class CMultiMeasureColorMunkiXRite : public CMultiMeasureDevice
{
public:
	int GetLastError(string &strErrorText);
	bool IsConnected();
	void WaitForButton();
	void WaitForButton(char &cKey);
	unsigned short Calibrate(const string &strMeasureMode);
	unsigned short SetMeasurementMode(const string &strMeasureMode); // xMeasureMode: MUNKI_SINGLE_REFLECTANCE, MUNKI_PATCH_RECOGNITION_BASIC, MUNKI_PATCH_RECOGNITION_CORRELATION
	unsigned short SetRecognitionReference(const string &strColorSpace, const float *pReferenceChartLine, long nLineSize);
	unsigned short DoMeasurements();
	long GetResultsNumber();
	unsigned short GetResult(long nIndex, float faData[MM_COLOR_DATA_SIZE], unsigned short &uSpectrumSize, float *&faSpectrum);
	void Reset();
};

///////////////////////////////////////////////////////////
// Measurements data storage and manipulation classes

// Single reference or measured color data class
class CMultiMeasureColorData
{
public:
	// Color data
	float m_faData[MM_COLOR_DATA_SIZE];  // Color data array from measurement device; can contain LAB, RGB, XYZ and CMYK color data components
	float *m_faSpectrum;
	unsigned short m_uSpectrumSize;
	bool m_bHasLab, m_bHasRGB, m_bHasXYZ, m_bHasCMYK, m_bHasSpectrum;
	// Statistic data (see comments to CalculateStatistic member function of CMultiMeasurePatch class)
	float m_fMeasureTime, m_fDeltaE, m_fFromBestDeltaE, m_fFromBestByDistDeltaE, m_fDist;

	CMultiMeasureColorData(float *faLab = NULL, float *faRGB = NULL, float *faXYZ = NULL, float *faCMYK = NULL, unsigned short uSpectrumSize = 0, float *faSpectrum = NULL, float fMeasureTime = 0);
	CMultiMeasureColorData(float *faData, unsigned short uSpectrumSize = 0, float *faSpectrum = NULL, float fMeasureTime = 0);
	CMultiMeasureColorData(CMultiMeasureColorData &ColorData);
	~CMultiMeasureColorData();
	CMultiMeasureColorData &operator=(CMultiMeasureColorData &ColorData);
	float CalculateDeltaE(float *faData, unsigned short uStandart = MM_DELTA_E_76);
};

// Single patch color data class
// Contains reference color data and one or several measured color data for one patch
class CMultiMeasurePatch
{
public:
	unsigned short m_uRefID;
	stringext m_strRefName;

	// Reference data
	CMultiMeasureColorData m_RefData;
	// Pointers to measured color data objects
	CMultiMeasureColorData **m_pMeasureData;
	unsigned short m_uMeasuresCount, m_uMeasuresCountMax;
	unsigned short m_uDeltaEStandart;
	// Statistic data (see comments to CalculateStatistic member function of CMultiMeasurePatch class)
	float m_faAverageData[MM_COLOR_DATA_SIZE], m_fBestDeltaE, m_fWorstDeltaE, m_fWorstFromBestDeltaE;
	unsigned short m_uBestMeasure, m_uWorstMeasure, m_uWorstFromBestMeasure;
	float m_fBestByDistDeltaE, m_fWorstByDistDeltaE, m_fWorstFromBestByDistDeltaE, m_fMinDist, m_fMaxDist;
	unsigned short m_uBestByDistMeasure, m_uWorstByDistMeasure, m_uWorstFromBestByDistMeasure;
	float m_fFromBestByDistAverageDeltaE;

	CMultiMeasurePatch(unsigned short uRefID = 0, stringext strRefName = "", CMultiMeasureColorData *pRefData = NULL);
	~CMultiMeasurePatch();
	CMultiMeasurePatch &operator=(CMultiMeasurePatch &PatchData);
	CMultiMeasureColorData &SetRefData(CMultiMeasureColorData *pRefData);
	CMultiMeasureColorData *AddMeasureData(CMultiMeasureColorData *pRefData, unsigned short uStandart = MM_DELTA_E_76);
	void CalculateStatistic(unsigned short uStandart = MM_DELTA_E_76);
	void EraseMeasurements();
	void EraseMeasurement(unsigned short uIndex);
	void EraseWorstMeasurement(bool bFromBest = true, int bMethod = MM_MEASURE_BEST_BY_DIST);
	bool operator>(CMultiMeasurePatch &patch);
};

// Strip color data class
// Contains color data of all patches in one stirp
class CMultiMeasureStrip
{
public:
	unsigned short m_uStripSize;
	CMultiMeasurePatch **m_pStripPatches;  // Pointers to patches objects in this strip
	// Statistic data (see comments to CalculateStatistic member function of CMultiMeasurePatch class)
	float m_faMaxDeltaE[MM_MAX_MEASURES_COUNT], m_faAverageDeltaE[MM_MAX_MEASURES_COUNT], m_faMeasureTime[MM_MAX_MEASURES_COUNT];
	float m_fMaxBestDeltaE, m_fAverageBestDeltaE, m_fTotalMaxDeltaE, m_fTotalAverageDeltaE;
	unsigned short m_uStripMeasuresCount;

	CMultiMeasureStrip(unsigned short uStripSize = 0);
	~CMultiMeasureStrip();
	unsigned short SetSize(unsigned short uStripSize);
	void CalculateStatistic(int bMethod = MM_MEASURE_BEST_BY_DIST);
	void EraseMeasurements();
	void EraseMeasurement(unsigned short uIndex);
	void EraseWorstMeasurement(bool bFromBest = true, int bMethod = MM_MEASURE_BEST_BY_DIST);
};

// Main data storage and manipulation class
class CMultiMeasure
{
public:
	// Device
	CMultiMeasureDevice *m_pDevice;
	unsigned short m_uLastDeviceError;

	// Referense file info
	string m_strInFile, m_strInFileName;
	int m_nInFileType;

	// Reference file parameters
	unsigned short m_uRowLength, m_uNumberOfPages, m_uNumberOfFields, m_uNumberOfSets;
	int m_naDataIndex[14], m_nIDIndex, m_nNameIndex;
	// Agryll specific parameters
	unsigned short m_uAgryll_COMP_GREY_STEPS, m_uAgryll_SINGLE_DIM_STEPS;
	string m_strAgryll_ACCURATE_EXPECTED_VALUES, m_strAgryll_APPROX_WHITE_POINT, m_strArgillCalibrationInfo, m_strAgryll_TOTAL_INK_LIMIT, m_strAgryll_COLOR_REP;

	// Patches Color Data
	unsigned short m_uPatchesCount;
	CMultiMeasurePatch **m_pPatches;        // Pointers to all patches objects in one array
	CMultiMeasurePatch **m_pAgryllPatches;  // Store initial order of patches in Agryll reference file

	// Strips Data
	unsigned short m_uStripSize, m_uStripsCount;
	CMultiMeasureStrip **m_pStrips;

	// Measurement file info
	string m_strOutFile, m_strOutFileName;
	int m_nOutFileType;

	CMultiMeasure(CMultiMeasureDevice *pDevice = NULL);
	~CMultiMeasure();
	CMultiMeasurePatch **SetupPatchesList(unsigned short nPatchesCount);
	unsigned short ReadReferenceFile(string strInFileName);
	unsigned short WriteMeasurementFile(string strOutFileName, unsigned short uOutputFormats, unsigned short uOutputDeviceData = 0);
	unsigned short WriteMeasurementFilePM5(string strOutFileName, int nMeasure = MM_MEASURE_BEST_BY_DIST, unsigned short uOutputDeviceData = 0);
	unsigned short WriteMeasurementFileCB(string strOutFileName, int nMeasure = MM_MEASURE_BEST_BY_DIST);
	unsigned short WriteMeasurementFileTI3(string strOutFileName, int nMeasure = MM_MEASURE_BEST_BY_DIST);
	unsigned short WriteMeasurementFileTI3CB(string strOutFileName, bool bSet = false, int nMeasure = MM_MEASURE_BEST_BY_DIST);
	unsigned short WriteMeasurementFilePM5Density(string strOutFileName, int nMeasure = MM_MEASURE_BEST_BY_DIST);
	unsigned short ReadMeasurementsFile(string strInFileName, unsigned short uStandart = MM_DELTA_E_76);
	void TransposeStrips();
	void SetDevice(CMultiMeasureDevice *pDevice);
	unsigned short SetReferenseLine(unsigned short &uStripIndex, unsigned short uFromPatchIndex = 0, unsigned short uPatchesNumber = 0);
//	unsigned short MeasureStrip(unsigned short &uStripIndex, unsigned short uStandart = MM_DELTA_E_76);
//	unsigned short MeasureStripMultiPass(unsigned short &uStripIndex, unsigned short uPassesNumber = 2, bool bReverse = false, unsigned short uStandart = MM_DELTA_E_76);
	unsigned short MeasureStripPatches(unsigned short &uStripIndex, CMultiMeasureColorData **pMeasureData, float &fReadingTime, unsigned short uPatchFrom = 0, unsigned short uPatchesNumber = 0);
	unsigned short AddStripMeasurements(unsigned short &uStripIndex, CMultiMeasureColorData **pMeasureData, float fReadingTime = 0.0f, unsigned short uStandart = MM_DELTA_E_76);
	unsigned short MeasurePatch(unsigned short &uStripIndex, unsigned short &uPatchIndex, unsigned short uStandart = MM_DELTA_E_76);
};
