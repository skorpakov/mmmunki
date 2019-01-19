/*---------------------------------------------------------------------------*/
/* colormunki  C-Style  API                                                  */
/* This API provides high-level methods to access the colormunki device      */
/*                                                                           */
/* Version 1.0                                                               */
/*                                                                           */
/* Copyright (c) 2007 by GretagMacbeth AG Switzerland, an X-Rite company.    */
/*                                                                           */
/* ALL RIGHTS RESERVED.                                                      */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS         */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED         */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE        */
/* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY           */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE         */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS             */
/* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,              */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING                 */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              */
/*---------------------------------------------------------------------------*/
#ifndef COLORMUNKI_H
#define COLORMUNKI_H 1


#ifdef __cplusplus
  extern "C" {
#endif


#include "MeasurementConditions.h"


/* Key/values used with MUNKI_GetOption() / MUNKI_SetOption() */
/* Global options */
#define MUNKI_LAST_ERROR                     "LastError"                   /* Last error (MUNKI_ResultType as string) */
#define MUNKI_LAST_ERROR_TEXT                "LastErrorText"               /* Last error text */
#define MUNKI_LAST_ERROR_NUMBER              "LastErrorNumber"             /* colormunki exception number of last error */

#define MUNKI_SDK_VERSION                    "SDKVersion"                  /* Version of sdk */
#define MUNKI_SDK_VERSION_MAJOR              "SDKVersionMajor"             /* Major number as string */
#define MUNKI_SDK_VERSION_MINOR              "SDKVersionMinor"             /* Minor number as string */
#define MUNKI_SDK_VERSION_REVISION           "SDKVersionRevision"          /* Revision number as string */
#define MUNKI_SDK_VERSION_BUILD              "SDKVersionBuild"             /* Build number as string */
#define MUNKI_SDK_VERSION_SUFFIX             "SDKVersionSuffix"            /* Suffix */

#define MUNKI_USE_ADVANCED_BUTTON_PRESS_MODE "UseAdvancedButtonPressMode"  /* key - mode to get more accurate information for MUNKI_ButtonPressed() */


/* Device specific options */
#define MUNKI_SERIAL_NUMBER                  "SerialNumber"                /* Serial number of device */
#define MUNKI_SUPPLIER                       "Supplier"                    /* Name of supplier */

#define MUNKI_SENSOR_POSITION                "SensorPosition"              /* key - sensor position of device. Read only */
#define MUNKI_SENSOR_POSITION_PROJECTOR      "Projector"                   /* value - sensor is in projector position */
#define MUNKI_SENSOR_POSITION_SUBSTRATE      "Substrate"                   /* value - sensor is in reflectance or emission position */
#define MUNKI_SENSOR_POSITION_WHITE_TILE     "WhiteTile"                   /* value - sensor is in calibration position */
#define MUNKI_SENSOR_POSITION_AMBIENT        "Ambient"                     /* value - sensor is in ambient light position */

#define MUNKI_MEASUREMENT_MODE               "MeasurementMode"             /* key - active measurement mode */
#define MUNKI_MEASUREMENT_MODE_UNDEFINED     "MeasurementModeUndefined"    /* value - default */
#define MUNKI_SINGLE_REFLECTANCE             "SingleReflectance"           /* value - spot reflection measurement mode */
#define MUNKI_SINGLE_EMISSION                "SingleEmission"              /* value - spot emission measurement mode */
#define MUNKI_SINGLE_AMBIENT_LIGHT           "SingleAmbientLight"          /* value - spot ambient measurement mode */
#define MUNKI_SCANNING_REFLECTANCE           "ScanningReflectance"         /* value - scan reflection measurement mode */

#define MUNKI_AVAILABLE_MEASUREMENT_MODES    "AvailableMeasurementModes"   /* key - all available measurement modes, separated by ';'. Read only */
#define MUNKI_IS_PROJECTOR_LICENSED          "ProjectorLicensed"           /* key - may device measure emission in projector position. Read only */
#define MUNKI_IS_GET_SPECTRUM_LICENSED       "GetSpectrumLicensed"         /* key - may device return spectrum. Read only */
#define MUNKI_IS_GET_TRISTIMULUS_LICENSED    "GetTristimulusLicensed"      /* key - may device return tristimulus. Read only */
#define MUNKI_IS_GET_DENSITY_LICENSED        "GetDensityLicensed"          /* key - may device return densities. Read only */


/* Measurement mode specific options */
#define MUNKI_PATCH_RECOGNITION              "Recognition"                 /* key - patch recognition in scan mode */
#define MUNKI_PATCH_RECOGNITION_DISABLED     "RecognitionDisabled"         /* value - default - No patch recognition */
#define MUNKI_PATCH_RECOGNITION_BASIC        "RecognitionBasic"            /* value - basic patch recognition algorithm */
#define MUNKI_PATCH_RECOGNITION_CORRELATION  "RecognitionCorrelation"      /* value - algorithm to correlate patches with references */

#define MUNKI_TIME_SINCE_LAST_CALIBRATION    "TimeSinceLastCalibration"    /* key - time in seconds since last calibration. Read only */
#define MUNKI_TIME_UNTIL_CALIBRATION_EXPIRE  "TimeUntilCalibrationExpire"  /* key - time in seconds until calibration expires. Read only */
#define MUNKI_MEASURE_COUNT                  "MeasureCount"                /* key - number of measurements since last calibration. Read only */

/* Recognition correlation specific options */
#define MUNKI_REFERENCE_CHART_COLOR_SPACE    "ReferenceChartColorSpace"    /* key - color space for SetReferenceChartLine */
#define MUNKI_REFERENCE_CHART_RGB            "ReferenceChartRGB"           /* value - RGB values */
#define MUNKI_REFERENCE_CHART_CMYK           "ReferenceChartCMYK"          /* value - CMYK values */
#define MUNKI_REFERENCE_CHART_LAB            "ReferenceChartLab"           /* value - Lab values */

#define MUNKI_PATCH_RECOGNITION_RECOGNIZED   "RecognitionNrRecognized"     /* key - number of recognized patches after basic recognition before correlation. Read only */


/* Reset options */
#define MUNKI_RESET                          "Reset"                       /* key - reset command */
#define MUNKI_ALL                            "All"                         /* value - reset SDK inclusive registered handlers */
#define MUNKI_MUNKI                          "Munki"                       /* value - default device */


/* Boolean values */
#define MUNKI_YES                            "yes"
#define MUNKI_NO                             "no"



typedef enum
{
  /* error codes */
  eNoError                      =  0,     /* no error */
  eDeviceNotReady               =  1,     /* device not ready */
  eDeviceNotConnected           =  2,     /* device not connected */
  eDeviceCommunicationError     =  3,     /* USB communication error occurred */
  eDeviceCorrupted              =  4,     /* data inside device are corrupt */
  eNoMeasureModeSet             =  5,     /* no measure mode has been set */
  eDeviceNotCalibrated          =  6,     /* device not calibrated */
  eWrongSensorPosition          =  7,     /* wrong sensor position */
  eNoSubstrateWhite             =  8,     /* no substrate white reference set */
  eStripRecognitionFailed       =  9,     /* if the measurement mode is set to scanning, recognition is enabled and failed*/
  eChartCorrelationFailed       = 10,     /* could not map scanned date to reference chart */
  eNoReferenceChartLine         = 11,     /* no reference chart line for correlation set */
  eNoDataAvailable              = 12,     /* measurement not triggered, index out of range (scanning) */
  eInvalidArgument              = 13,     /* if a passed method argument is invalid (i.e. NULL) */
  eNotLicensed                  = 14,     /* feature unlicensed */
  eException                    = 15,     /* internal exception, use GetOption(MUNKI_LAST_ERROR) for more details */
  eUnknownError                 = 16,     /* unknown error occurred */

  /* button states returned by MUNKI_ButtonPressed() */
  eButtonIsPressed              = 1000,   /* if button is pressed */
  eButtonNotPressed             = 1001,   /* if button is not pressed */
  eButtonWasPressed             = 1002,   /* only for advanced mode, once if button was pressed but is not pressed any more */

  /* sensor positions returned by MUNKI_GetSensorPosition() */
  eSensorSubstrate              = 2000,   /* for reflectance and emission measurement */
  eSensorWhiteTile              = 2001,   /* calibration position */
  eSensorProjector              = 2002,   /* projector position */
  eSensorAmbient                = 2003    /* ambient position */
} MUNKI_ResultType;


/* device messages used in device message callback function */
typedef enum
{
  eDeviceButtonPressed          =  0,     /* button pressed on device */
  eDeviceButtonReleased         =  1,     /* button released on device */
  eDeviceSensorRotated          =  2,     /* sensor of device was rotated */
  eDeviceDisconnected           =  3,     /* device disconnected */
  eDeviceAttached               =  4      /* new device attached */
} MUNKI_DeviceMessage;



/*--------------------- prototypes of exported functions --------------------*/

/*------------------ setting/getting measurement mode & conditions ------------------*/
/*
possible options (see MeasurementConditions.h for possible values)
COLOR_SPACE_KEY
ILLUMINATION_KEY
OBSERVER_KEY
DENSITY_STANDARD_KEY
DENSITY_FILTER_MODE_KEY

MUNKI_MEASUREMENT_MODE, possible values : MUNKI_SINGLE_EMISSION, MUNKI_SINGLE_REFLECTANCE, MUNKI_SCANNING_REFLECTANCE
MUNKI_PATCH_RECOGNITION, possible values : MUNKI_PATCH_RECOGNITION_DISABLED, MUNKI_PATCH_RECOGNITION_BASIC
...
*/
MUNKI_ResultType MUNKI_SetOption(const char* xKey, const char* xValue);

const char* MUNKI_GetOption(const char* xKey);


/*-------------------- Information about device and SDK ---------------------*/
/*
Test if the colormunki is connected
return eNoError if connected
return eDeviceNotConnected if no colormunki is present
*/
MUNKI_ResultType MUNKI_IsConnected(void);


/*
Test if the button is pressed
return eButtonIsPressed if colormunki key has been pressed
return eButtonNotPressed if colormunki key not been pressed
return eDeviceNotConnected if no colormunki is present
Advanced mode only:
return eButtonWasPressed if colormunki key was pressed once but not any more
*/
MUNKI_ResultType MUNKI_ButtonPressed(void);


/*
Test current sensor position
return eSensorSubstrate, eSensorWhiteTile, eSensorProjector, eSensorAmbient
return eDeviceNotConnected if no colormunki is present
*/
MUNKI_ResultType MUNKI_GetSensorPosition(void);


/*--------------------- special function for Density ------------------------*/
/*
Set the substrate spectrum for density calculations
This method has to be called before the first call to GetDensity()
*/
MUNKI_ResultType MUNKI_SetSubstrate(const float xSpectrum[SPECTRUM_SIZE]);


/*------------------- calibration / trigger measurements --------------------*/
/*
Calibrate the colormunki
*/
MUNKI_ResultType MUNKI_Calibrate(void);


/*
Trigger measurement

Triggers a measurement depending on the measurement mode set by MUNKI_SetOption
It is necessary to calibrate the colormunki before any measurement can be triggered

use MUNKI_GetSpectrum(index), MUNKI_GetTriStimulus(index) or MUNKI_GetDensity(index)
to fetch the result

returns eDeviceNotConnected if no device is available
returns eDeviceNotCalibrated if a (re)calibration is necessary
*/
MUNKI_ResultType MUNKI_TriggerMeasurement(void);


/*------------------------------- get samples -------------------------------*/
/*General remarks:
Use 0 as Index, to fetch the result of a previously triggered single measurement
To fetch a result of a previously triggered scan, specify an index between 0 - (MUNKI_GetNumberOfScannedSamples() - 1)
If no measurement has been triggered or if the specified index is out of range eNoDataAvailable is returned
*/

/*
returns amount of currently available samples
*/
long MUNKI_GetNumberOfAvailableSamples(void);


/*
Get the spectrum of a previous triggered measurement
*/
MUNKI_ResultType MUNKI_GetSpectrum(float xSpectrum[SPECTRUM_SIZE], long xIndex);


/*
Get the color vector of a previous triggered measurement
*/
MUNKI_ResultType MUNKI_GetTriStimulus(float xTristimulus[TRISTIMULUS_SIZE], long xIndex);


/*
Get all densities (cyan, magenta, yellow, black) of a previous triggered measurement
if autoDensityIndex is not null, *autoDensityIndex will be set accordingly
*/
MUNKI_ResultType MUNKI_GetDensities(float xDensities[DENSITY_SIZE], long* xAutoDensityIndex, long xIndex);


/*
Get the auto density of a previous triggered measurement
*/
MUNKI_ResultType MUNKI_GetDensity(float* xDensity, long xIndex);


/*----------------------------- reference chart -----------------------------*/
/*
Set reference data row of chart for next chart correlation
xReferenceChart is an array of all reference colors for next scanned line
colors must be in Lab
*/
MUNKI_ResultType MUNKI_SetReferenceChartLine(const float *xReferenceChartLine, long xLineSize);


/*---------------------------- callback functions ---------------------------*/
/* void MyErrorHandlerFunction(const char* xContext, MUNKI_ResultType xCode, const char* xText, void* xRefCon); */
typedef void (* FPtr_MUNKI_ErrorHandler)(const char*, MUNKI_ResultType, const char*, void*);
MUNKI_ResultType MUNKI_AddErrorHandler(FPtr_MUNKI_ErrorHandler xHandler, void* xRefCon);
MUNKI_ResultType MUNKI_RemoveErrorHandler(FPtr_MUNKI_ErrorHandler xHandler);

/* void MyDeviceMessageFunction(const MUNKI_DeviceMessage xMessage, const char* xSerialNumber, void* xRefCon); */
typedef void(*FPtr_MUNKI_DeviceMessageHandler)(const MUNKI_DeviceMessage, const char*, void*);
MUNKI_ResultType MUNKI_AddDeviceMessageHandler(FPtr_MUNKI_DeviceMessageHandler xHandler, void* xRefCon);
MUNKI_ResultType MUNKI_RemoveDeviceMessageHandler(FPtr_MUNKI_DeviceMessageHandler xHandler);



/*-------- deprecated, these functions will go away in next release --------*/
/* DO NOT mix new and deprecated functions!                                 */
/* void MyErrorHandlerFunction_dep(const char* xContext, MUNKI_ResultType xCode, const char* xText); */
typedef void (* FPtr_MUNKI_ErrorHandler_dep)(const char*, MUNKI_ResultType, const char*);
FPtr_MUNKI_ErrorHandler_dep MUNKI_RegisterErrorHandler(FPtr_MUNKI_ErrorHandler_dep xHandler);

/* void MyDeviceMessageFunction(const MUNKI_DeviceMessage xMessage, const char* xSerialNumber); */
typedef void(*FPtr_MUNKI_DeviceMessageHandler_dep)(const MUNKI_DeviceMessage, const char*);
FPtr_MUNKI_DeviceMessageHandler_dep MUNKI_RegisterDeviceMessageHandler(FPtr_MUNKI_DeviceMessageHandler_dep xHandler);



#ifdef __cplusplus
  }
#endif

#endif /*COLORMUNKI_H*/

