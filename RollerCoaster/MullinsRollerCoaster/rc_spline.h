/** @file  rc_spline.h 
*
*   @brief contains the classes to represent and load splines
*
*   @author Created by Roberto Lublinerman on Mon Feb 20 2007.
*
*   CSE 418: Programming Project 4
*
**/

#include "util_vectors.h"
#include <vector>

class rc_SplineSegment;
class rc_Spline;

/** @brief type alias for a vector of Vec3f*/
typedef std::vector<Vec3f> pointVector;
/** @brief type alias for an iterator of a pointVector*/
typedef pointVector::iterator pointVectorIter;


/** @brief class to represent a spline */
class rc_Spline
{
private:
	/** @brief vector of control points */
	pointVector m_vPoints;

	/** @brief load the definition of this spline segment from a file 
	*  
	*  @param filename file containing the definition for this spline segment
	*/
	void loadSegmentFrom(char* filename);

public:

	/** @brief add a point to the spline segment 
	*  
	*  @param v point to add
	*/
	void addPoint(const Vec3f& v)  	{	m_vPoints.push_back(v);	}

	/** @brief get the length of the spline segment 
	*/
	int length()	{ m_vPoints.size(); }


	/** @brief accessor to the vector of control points 
	*  
	*	Normaly used for iterating, e.g.
	*		for(pointVectorIter iter = pSpline->start(); iter != pSpline->end(); iter++)
	*		{
	*			Vec3f curpt = *iter;
	*			// do whatever is needed with each point
	*		}
	*
	*/
	pointVector& points() { return m_vPoints; }

	/** @brief load the definition of this spline from a file 
	*  
	*  @param filename file containing the definition for this spline
	*/
	void loadSplineFrom(char* filename);


};


