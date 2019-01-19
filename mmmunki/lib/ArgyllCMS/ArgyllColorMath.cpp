/* 
 * International Color Consortium Format Library (icclib)
 * For ICC profile version 3.4
 *
 * Author:  Graeme W. Gill
 * Date:    2002/04/22
 * Version: 2.15
 *
 * Copyright 1997 - 2013 Graeme W. Gill
 *
 * This material is licensed with an "MIT" free use license:-
 * see the License.txt file in this directory for licensing details.
 */

#include <math.h>


/* Return the normal Delta E given two Lab values */
double icmLabDE(double *Lab0, double *Lab1) {
	double rv = 0.0, tt;

	tt = Lab0[0] - Lab1[0];
	rv += tt * tt;
	tt = Lab0[1] - Lab1[1];
	rv += tt * tt;
	tt = Lab0[2] - Lab1[2];
	rv += tt * tt;
	
	return sqrt(rv);
}

/* Return the CIE94 Delta E color difference measure */
double icmCIE94(double Lab0[3], double Lab1[3]) {
	double desq, dhsq;
	double dlsq, dcsq;
	double c12;

	{
		double dl, da, db;
		dl = Lab0[0] - Lab1[0];
		dlsq = dl * dl;		/* dl squared */
		da = Lab0[1] - Lab1[1];
		db = Lab0[2] - Lab1[2];

		/* Compute normal Lab delta E squared */
		desq = dlsq + da * da + db * db;
	}

	{
		double c1, c2, dc;

		/* Compute chromanance for the two colors */
		c1 = sqrt(Lab0[1] * Lab0[1] + Lab0[2] * Lab0[2]);
		c2 = sqrt(Lab1[1] * Lab1[1] + Lab1[2] * Lab1[2]);
		c12 = sqrt(c1 * c2);	/* Symetric chromanance */

		/* delta chromanance squared */
		dc = c1 - c2;
		dcsq = dc * dc;
	}

	/* Compute delta hue squared */
	if ((dhsq = desq - dlsq - dcsq) < 0.0)
		dhsq = 0.0;
	{
		double sc, sh;

		/* Weighting factors for delta chromanance & delta hue */
		sc = 1.0 + 0.045 * c12;
		sh = 1.0 + 0.015 * c12;
		return sqrt(dlsq + dcsq/(sc * sc) + dhsq/(sh * sh));
	}
}

/* Return the CIE2DE000 Delta E color difference measure for two Lab values */
double icmCIE2K(double *Lab0, double *Lab1) {
	double C1, C2;
	double h1, h2;
	double dL, dC, dH;
	double dsq;

	/* The trucated value of PI is needed to ensure that the */
	/* test cases pass, as one of them lies on the edge of */
	/* a mathematical discontinuity. The precision is still */
	/* enough for any practical use. */
#define RAD2DEG(xx) (180.0/3.14159265358979 * (xx))
#define DEG2RAD(xx) (3.14159265358979/180.0 * (xx))

	/* Compute Cromanance and Hue angles */
	{
		double C1ab, C2ab;
		double Cab, Cab7, G;
		double a1, a2;

		C1ab = sqrt(Lab0[1] * Lab0[1] + Lab0[2] * Lab0[2]);
		C2ab = sqrt(Lab1[1] * Lab1[1] + Lab1[2] * Lab1[2]);
		Cab = 0.5 * (C1ab + C2ab);
		Cab7 = pow(Cab,7.0);
		G = 0.5 * (1.0 - sqrt(Cab7/(Cab7 + 6103515625.0)));
		a1 = (1.0 + G) * Lab0[1];
		a2 = (1.0 + G) * Lab1[1];
		C1 = sqrt(a1 * a1 + Lab0[2] * Lab0[2]);
		C2 = sqrt(a2 * a2 + Lab1[2] * Lab1[2]);

		if (C1 < 1e-9)
			h1 = 0.0;
		else {
			h1 = RAD2DEG(atan2(Lab0[2], a1));
			if (h1 < 0.0)
				h1 += 360.0;
		}

		if (C2 < 1e-9)
			h2 = 0.0;
		else {
			h2 = RAD2DEG(atan2(Lab1[2], a2));
			if (h2 < 0.0)
				h2 += 360.0;
		}
	}

	/* Compute delta L, C and H */
	{
		double dh;

		dL = Lab1[0] - Lab0[0];
		dC = C2 - C1;
		if (C1 < 1e-9 || C2 < 1e-9) {
			dh = 0.0;
		} else {
			dh = h2 - h1;
			if (dh > 180.0)
				dh -= 360.0;
			else if (dh < -180.0)
				dh += 360.0;
		}

		dH = 2.0 * sqrt(C1 * C2) * sin(DEG2RAD(0.5 * dh));
	}

	{
		double L, C, h, T;
		double hh, ddeg;
		double C7, RC, L50sq, SL, SC, SH, RT;
		double dLsq, dCsq, dHsq, RCH;

		L = 0.5 * (Lab0[0]  + Lab1[0]);
		C = 0.5 * (C1 + C2);
		if (C1 < 1e-9 || C2 < 1e-9) {
			h = h1 + h2;
		} else {
			h = h1 + h2;
			if (fabs(h1 - h2) > 180.0) {
				if (h < 360.0)
					h += 360.0;
				else if (h >= 360.0)
					h -= 360.0;
			}
			h *= 0.5;
		}
		T = 1.0 - 0.17 * cos(DEG2RAD(h-30.0)) + 0.24 * cos(DEG2RAD(2.0 * h))
		  + 0.32 * cos(DEG2RAD(3.0 * h + 6.0)) - 0.2 * cos(DEG2RAD(4.0 * h - 63.0));
		hh = (h - 275.0)/25.0;
		ddeg = 30.0 * exp(-hh * hh);
		C7 = pow(C,7.0);
		RC = 2.0 * sqrt(C7/(C7 + 6103515625.0));
		L50sq = (L - 50.0) * (L - 50.0);
		SL = 1.0 + (0.015 * L50sq)/sqrt(20.0 + L50sq);
		SC = 1.0 + 0.045 * C;
		SH = 1.0 + 0.015 * C * T;
		RT = -sin(DEG2RAD(2 * ddeg)) * RC;

		dLsq = dL/SL;
		dCsq = dC/SC;
		dHsq = dH/SH;

		RCH = RT * dCsq * dHsq;

		dLsq *= dLsq;
		dCsq *= dCsq;
		dHsq *= dHsq;

		dsq = dLsq + dCsq + dHsq + RCH;
	}

	return sqrt(dsq);

#undef RAD2DEG
#undef DEG2RAD
}