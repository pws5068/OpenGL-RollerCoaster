/*** @file rc_spline.cpp
*
*   @brief Implementation of the rc_Spline classes
*
*   You should add some code here probably.
**/

#ifdef WIN32
/* get rid of ridiculous warnings */
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "rc_spline.h"


/* load a spline segment from a file */
void rc_Spline::loadSegmentFrom(char* filename)
{	
	printf("Entered: loadSegmentFrom()\n\tLoading segment: %s\n", filename);
	FILE* fileSplineSegment = fopen(filename, "r");

	if (fileSplineSegment == NULL) 
	{
		printf ("can't open file %s\n", filename);
		system("PAUSE");
		exit(1);
	}

	int iLength;

	/* gets length for spline segment */
	fscanf(fileSplineSegment, "%d", &iLength);

	Vec3f pt;

	/* add it to the control point list */
	while (fscanf(fileSplineSegment, "%f %f %f", 
				&pt.x(), &pt.y(),&pt.z()) != EOF) 
	{
		m_vPoints.push_back(pt);
	}

	/* now close the file */
	fclose(fileSplineSegment);
	printf("Exited: loadSegmentFrom()\n");
}


/* load a spline from a file */
void rc_Spline::loadSplineFrom(char* filename)
{	
	printf("Entered: loadSplineFrom()\n\tLoading spline: %s\n", filename);
	/* load the track file */
	FILE* fileSpline = fopen(filename, "r");

	if (fileSpline == NULL) 
	{
		printf ("can't open file %s\n", filename);
		system("PAUSE");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	int nSegments;
	fscanf(fileSpline, "%d", &nSegments);

	/* reads through the spline files */
	for (int j = 0; j < nSegments; j++) 
	{
		/* define a variable of a reasonable size for a filename */
		char segmentfilename[1024];

		fscanf(fileSpline, "%s", segmentfilename);
		loadSegmentFrom(segmentfilename);
	}

	/* now close the file */
	fclose(fileSpline);
	printf("Exited: loadSplineFrom()\n");
}