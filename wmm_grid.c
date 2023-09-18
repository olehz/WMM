#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "GeomagnetismHeader.h"
#include "EGM9615.h"

/*
WMM grid program.

The Geomagnetism Library is used to make a command prompt program. The program prompts
the user to enter a location, performs the computations and prints the results to the
standard output. The program expects the files GeomagnetismLibrary.c, GeomagnetismHeader.h,
WMM2010SHDF.COF and EGM9615.h to be in the same directory.

Manoj.C.Nair@Noaa.Gov
April 21, 2011
 */

int MAG_Grid(MAGtype_CoordGeodetic minimum,
             MAGtype_CoordGeodetic maximum,
             double cord_step_size,
             MAGtype_MagneticModel *MagneticModel,
             MAGtype_Geoid *Geoid,
             MAGtype_Ellipsoid Ellip,
             MAGtype_Date StartDate,
             int ElementOption);

int main(int argc, char *argv[])
{
    MAGtype_MagneticModel *MagneticModels[1];
    MAGtype_Ellipsoid Ellip;
    MAGtype_CoordGeodetic minimum, maximum;
    MAGtype_Geoid Geoid;
    MAGtype_Date startdate;
    int ElementOption, i, epochs = 1;
    double cord_step_size;
    char filename[] = "WMM.COF";
    char VersionDate[12];

    if (!MAG_robustReadMagModels(filename, &MagneticModels, 1))
    {
        printf("\n WMM.COF not found.\n");
        return 1;
    }
    strncpy(VersionDate, VERSIONDATE_LARGE + 39, 11);
    VersionDate[11] = '\0';

    MAG_SetDefaults(&Ellip, &Geoid);
    /* Set EGM96 Geoid parameters */
    Geoid.GeoidHeightBuffer = GeoidHeightBuffer;
    Geoid.Geoid_Initialized = 1;
    Geoid.UseGeoid = 1;

    if (argc > 1)
    {
        startdate.DecimalYear = atof(argv[1]);
        // MAG_YearToDate(&startdate);
        // printf("%d/%d/%d", startdate.Month, startdate.Day, startdate.Year);
    }
    else
    {
        printf("Specify Date for AIRAC cycle.");
        return 1;
    }
    if (argc > 2)
    {
        cord_step_size = atof(argv[2]);
    }
    else
    {
        cord_step_size = 0.1;
    }
    if (argc > 3)
    {
        minimum.HeightAboveGeoid = atof(argv[3]);
    }
    else
    {
        minimum.HeightAboveGeoid = 1;
    }
    if (argc > 4)
    {
        ElementOption = atoi(argv[4]);
    }
    else
    {
        ElementOption = 1;
    }

    minimum.phi = -90 + cord_step_size;
    maximum.phi = 90 - cord_step_size;
    minimum.lambda = -180;
    maximum.lambda = 180;

    MAG_Grid(minimum, maximum, cord_step_size, MagneticModels[0], &Geoid, Ellip, startdate, ElementOption);

    for (i = 0; i < epochs; i++)
        MAG_FreeMagneticModelMemory(MagneticModels[i]);

    return 0;
}

int MAG_Grid(MAGtype_CoordGeodetic minimum, MAGtype_CoordGeodetic maximum, double cord_step_size,
             MAGtype_MagneticModel *MagneticModel, MAGtype_Geoid *Geoid,
             MAGtype_Ellipsoid Ellip,
             MAGtype_Date StartDate, int ElementOption)
{
    int NumTerms;
    double b, c, PrintElement;

    MAGtype_MagneticModel *TimedMagneticModel;
    MAGtype_CoordSpherical CoordSpherical;
    MAGtype_MagneticResults MagneticResultsSph, MagneticResultsGeo, MagneticResultsSphVar, MagneticResultsGeoVar;
    MAGtype_SphericalHarmonicVariables *SphVariables;
    MAGtype_GeoMagneticElements GeoMagneticElements, Errors;
    MAGtype_LegendreFunction *LegendreFunction;

    if (fabs(cord_step_size) < 1.0e-10)
        cord_step_size = 99999.0; /*checks to make sure that the step_size is not too small*/

    NumTerms = ((MagneticModel->nMax + 1) * (MagneticModel->nMax + 2) / 2);
    TimedMagneticModel = MAG_AllocateModelMemory(NumTerms);
    LegendreFunction = MAG_AllocateLegendreFunctionMemory(NumTerms); /* For storing the ALF functions */
    SphVariables = MAG_AllocateSphVarMemory(MagneticModel->nMax);
    b = minimum.phi;
    c = minimum.lambda;

    for (minimum.lambda = c; minimum.lambda < maximum.lambda; minimum.lambda += cord_step_size) /*Longitude loop*/
    {
        for (minimum.phi = b; minimum.phi <= maximum.phi; minimum.phi += cord_step_size) /*Latitude loop*/
        {
            MAG_ConvertGeoidToEllipsoidHeight(&minimum, Geoid); /* This converts the height above mean sea level to height above the WGS-84 ellipsoid */

            MAG_GeodeticToSpherical(Ellip, minimum, &CoordSpherical);
            MAG_ComputeSphericalHarmonicVariables(Ellip, CoordSpherical, MagneticModel->nMax, SphVariables); /* Compute Spherical Harmonic variables  */
            MAG_AssociatedLegendreFunction(CoordSpherical, MagneticModel->nMax, LegendreFunction);           /* Compute ALF  Equations 5-6, WMM Technical report*/

            MAG_TimelyModifyMagneticModel(StartDate, MagneticModel, TimedMagneticModel);                                      /*This modifies the Magnetic coefficients to the correct date. */
            MAG_Summation(LegendreFunction, TimedMagneticModel, *SphVariables, CoordSpherical, &MagneticResultsSph);          /* Accumulate the spherical harmonic coefficients Equations 10:12 , WMM Technical report*/
            MAG_SecVarSummation(LegendreFunction, TimedMagneticModel, *SphVariables, CoordSpherical, &MagneticResultsSphVar); /*Sum the Secular Variation Coefficients, Equations 13:15 , WMM Technical report  */
            MAG_RotateMagneticVector(CoordSpherical, minimum, MagneticResultsSph, &MagneticResultsGeo);                       /* Map the computed Magnetic fields to Geodetic coordinates Equation 16 , WMM Technical report */
            MAG_RotateMagneticVector(CoordSpherical, minimum, MagneticResultsSphVar, &MagneticResultsGeoVar);                 /* Map the secular variation field components to Geodetic coordinates, Equation 17 , WMM Technical report*/
            MAG_CalculateGeoMagneticElements(&MagneticResultsGeo, &GeoMagneticElements);                                      /* Calculate the Geomagnetic elements, Equation 18 , WMM Technical report */
            MAG_CalculateGridVariation(minimum, &GeoMagneticElements);
            MAG_CalculateSecularVariationElements(MagneticResultsGeoVar, &GeoMagneticElements); /*Calculate the secular variation of each of the Geomagnetic elements, Equation 19, WMM Technical report*/
            MAG_WMMErrorCalc(GeoMagneticElements.H, &Errors);

            switch (ElementOption)
            {
            case 1:
                PrintElement = GeoMagneticElements.Decl; /*1. Angle between the magnetic field vector and true north, positive east*/
                break;
            case 2:
                PrintElement = GeoMagneticElements.Incl; /*2. Angle between the magnetic field vector and the horizontal plane, positive downward*/
                break;
            case 3:
                PrintElement = GeoMagneticElements.F; /*3. Magnetic Field Strength*/
                break;
            case 4:
                PrintElement = GeoMagneticElements.H; /*4. Horizontal Magnetic Field Strength*/
                break;
            case 5:
                PrintElement = GeoMagneticElements.X; /*5. Northern component of the magnetic field vector*/
                break;
            case 6:
                PrintElement = GeoMagneticElements.Y; /*6. Eastern component of the magnetic field vector*/
                break;
            case 7:
                PrintElement = GeoMagneticElements.Z; /*7. Downward component of the magnetic field vector*/
                break;
            case 8:
                PrintElement = GeoMagneticElements.GV; /*8. The Grid Variation*/
                break;
            case 9:
                PrintElement = GeoMagneticElements.Decldot * 60; /*9. Yearly Rate of change in declination*/
                break;
            case 10:
                PrintElement = GeoMagneticElements.Incldot * 60; /*10. Yearly Rate of change in inclination*/
                break;
            case 11:
                PrintElement = GeoMagneticElements.Fdot; /*11. Yearly rate of change in Magnetic field strength*/
                break;
            case 12:
                PrintElement = GeoMagneticElements.Hdot; /*12. Yearly rate of change in horizontal field strength*/
                break;
            case 13:
                PrintElement = GeoMagneticElements.Xdot; /*13. Yearly rate of change in the northern component*/
                break;
            case 14:
                PrintElement = GeoMagneticElements.Ydot; /*14. Yearly rate of change in the eastern component*/
                break;
            case 15:
                PrintElement = GeoMagneticElements.Zdot; /*15. Yearly rate of change in the downward component*/
                break;
            case 16:
                PrintElement = GeoMagneticElements.GVdot;
                /*16. Yearly rate of change in grid variation*/;
                break;
            default:
                PrintElement = GeoMagneticElements.Decl; /* 1. Angle between the magnetic field vector and true north, positive east*/
            }

            printf("%5.2f %6.2f %10.4f\n", minimum.lambda, minimum.phi, PrintElement);

        } /*Longitude Loop */

    } /* Latitude Loop */

    MAG_FreeMagneticModelMemory(TimedMagneticModel);
    MAG_FreeLegendreMemory(LegendreFunction);
    MAG_FreeSphVarMemory(SphVariables);

    return TRUE;
} /*MAG_Grid*/
