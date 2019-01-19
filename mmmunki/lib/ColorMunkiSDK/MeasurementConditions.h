/*-------------------------------------------------------------------------------*/
/* colormunki C-Style API                                                        */
/* Defines for the used string- constants in MeasureConditions                   */
/*                                                                               */
/* Version 2.0                                                                   */
/*                                                                               */
/* Copyright (c) 2006 by X-Rite Inc. Switzerland.                                */
/*                                                                               */
/* ALL RIGHTS RESERVED.                                                          */
/*                                                                               */
/* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS             */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED             */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE            */
/* ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY               */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL            */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE             */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS                 */
/* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,                  */
/* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING                     */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS            */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                  */
/*-------------------------------------------------------------------------------*/
#ifndef MEASUREMENT_CONDITIONS_H
#define MEASUREMENT_CONDITIONS_H


/*-----------------------------------------------------------------------------
  defines for the used string- constants in MeasureConditions
  -----------------------------------------------------------------------------*/

#define UNDEFINED                     "Undefined"


  /* Illumination */
#define ILLUMINATION_KEY              "Colorimetric.Illumination"
#define ILLUMINATION_A                "A"
#define ILLUMINATION_B                "B"
#define ILLUMINATION_C                "C"
#define ILLUMINATION_D50              "D50"
#define ILLUMINATION_D55              "D55"
#define ILLUMINATION_D65              "D65"
#define ILLUMINATION_D75              "D75"
#define ILLUMINATION_F2               "F2"
#define ILLUMINATION_F7               "F7"
#define ILLUMINATION_F11              "F11"
#define ILLUMINATION_EMISSION         "Emission"

  /* Observer */
#define OBSERVER_KEY                  "Colorimetric.Observer"
#define OBSERVER_TWO_DEGREE           "TwoDegree"
#define OBSERVER_TEN_DEGREE           "TenDegree"

  /* WhiteBase */
#define WHITE_BASE_KEY                "Colorimetric.WhiteBase"
#define WHITE_BASE_ABSOLUTE           "Absolute"
#define WHITE_BASE_PAPER              "Paper"
#define WHITE_BASE_AUTOMATIC          "Automatic"
  
  /* DensityStandard */
#define DENSITY_STANDARD_KEY          "Colorimetric.DensityStandard"
#define DENSITY_STANDARD_DIN          "DIN"
#define DENSITY_STANDARD_DINNB        "DINNB"
#define DENSITY_STANDARD_ANSIA        "ANSIA"
#define DENSITY_STANDARD_ANSIE        "ANSIE"
#define DENSITY_STANDARD_ANSII        "ANSII"
#define DENSITY_STANDARD_ANSIT        "ANSIT"
#define DENSITY_STANDARD_SPI          "SPI"

  /* DensityFilterMode */
#define DENSITY_FILTER_MODE_KEY       "Colorimetric.DensityFilterMode"
#define DENSITY_FILTER_MODE_BLACK     "Black"
#define DENSITY_FILTER_MODE_CYAN      "Cyan"
#define DENSITY_FILTER_MODE_MAGENTA   "Magenta"
#define DENSITY_FILTER_MODE_YELLOW    "Yellow"
#define DENSITY_FILTER_MODE_MAX       "Max"
#define DENSITY_FILTER_MODE_AUTO      "Auto"

  /* Maximum / minimum wavelength */
#define WAVE_LENGTH_730               "730nm"
#define WAVE_LENGTH_380               "380nm"

  /* ColorSpace */
#define COLOR_SPACE_KEY               "ColorSpaceDescription.Type"
#define COLOR_SPACE_CIELab            "CIELab"
#define COLOR_SPACE_CIELCh            "CIELCh"
#define COLOR_SPACE_CIELuv            "CIELuv"
#define COLOR_SPACE_CIELChuv          "CIELChuv"
#define COLOR_SPACE_CIE_UV_Y1960      "CIEuvY1960"
#define COLOR_SPACE_CIE_UV_Y1976      "CIEuvY1976"
#define COLOR_SPACE_CIEXYZ            "CIEXYZ"
#define COLOR_SPACE_CIExyY            "CIExyY"

#define COLOR_SPACE_HunterLab         "HunterLab"
#define COLOR_SPACE_RXRYRZ            "RxRyRz"
#define COLOR_SPACE_LAB_MG            "LABmg"
#define COLOR_SPACE_LCH_MG            "LCHmg"
#define COLOR_SPACE_RGB               "RGB"

#define SPECTRUM_SIZE 36
#define TRISTIMULUS_SIZE 3
#define DENSITY_SIZE 4

#endif

