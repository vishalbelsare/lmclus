 /* ********************************************************************************
  * * TITLE:        lmclus.hpp
  * *
  * * PURPOSE:      A linear manifold clustering algorithm based on the paper
  * *               "Linear manifold clustering in high dimensional spaces by stochastic search", 
  * *                Pattern Recognition (2007), vol. 40(10), pp 2672-2684.
  * *
  * * AUTHOR:       Rave Harpaz 
  * *               Pattern Recognition Laboratory
  * *               Department of Computer Science
  * *               The Graduate Center
  * *               The City University of New York
  * *               365 Fifth Avenue, New York, New York 10016
  * *               email: rbharpaz@sci.brooklyn.cuny.edu
  * * DATE:         01/01/2005
  * *
  * * VERSION:      1.00
  * *
  * * LANGUAGE:     C++
  * *
  * * SYSTEM:       Ubuntu 64 bit linux workstation using kernal 2.6.15-26-amd64
  * *
  * * COMPILER:     gcc/g++ compiler version 4.0.3
  * *
  * * REFERENCES:   Rave Harpaz and Robert Haralick, 
  * *               "Linear manifold clustering in high dimensional spaces by stochastic search", 
  * *               Pattern Recognition (2007), vol. 40(10), pp 2672-2684.
  * *
  * * REVISIONS:    Art Diky
  * *               Compiled as R-module with armadillo and boost
  * *               email: adiky@gc.cuny.edu
  * *               02/18/2013
  * *
  * * Copyright 2005-2013 Pattern Recognition Laboratory, The City University of New York
  * **********************************************************************************/

#include <iostream>
#include <vector>
#include <armadillo>

#define CPPLOG_FILTER_LEVEL 2
#include "cpplog.hpp"

namespace clustering {
namespace lmclus {
/* Separation
 *-----------
 * class used to store separation info generated by kittler's thresholding algorithm.
 * the separation is considered to be the valley between two modes in a histogram.
 * the goodness of the separation is measured by the: valley's width * valley's depth.
 */
class Separation
{
public:
	double get_width() const {
		return sep_width;
	}
	double get_depth() const {
		return sep_depth;
	}
	double get_criteria() const {
		return criteria;
	}
	double get_threshold () const {
		return threshold;
	}
	arma::uvec get_histo() const {
		return histo;
	}
	arma::mat get_projection () const {
		return projection;
	}
	arma::rowvec get_origin() const {
		return origin;
	}
	void reset(){
		criteria = -1;
	}
	
	Separation(double w, double d, double thres, const arma::rowvec &org ,const arma::mat &p, arma::uvec &h):
		origin(org),projection(p),sep_width(w),sep_depth(d),threshold(thres),histo(h)
	{
		criteria=sep_width*sep_depth;
	}

	Separation ():
		origin(1),projection(1,1),sep_width(0),sep_depth(0),threshold(0),criteria(-1){}
		
	virtual ~Separation () {};
	
private:
	arma::rowvec origin;                  // origin of subspace, used to find points that will be separated from data
	arma::mat projection;                 // subspace projection matrix, used to find points that will be separated from data
	double sep_width;                     // separation width
	double sep_depth;                     // separation depth
	double threshold;                     // histogram's threshold
	arma::uvec histo;                      // the histogram kittler's algorithm is applied on		
	double criteria;                      // goodness of separation (width*depth)

};

/* Parameters
 *-----------
 * structure used to store parameters for lmclus algoritm.
 */
struct Parameters
{
    int MAX_DIM;
    int NUM_OF_CLUS;
    unsigned int LABEL_COL;
    int CONST_SIZE_HIS;
    unsigned int NOISE_SIZE;
    double BEST_BOUND;
    double ERROR_BOUND; 
    double MAX_BIN_PORTION;
    
    unsigned long int RANDOM_SEED;
    int SAMPLING_HEURISTIC;
    double SAMPLING_FACTOR;
    bool HIS_SAMPLING;
    bool SAVE_RESULT;
    
    friend std::ostream & operator<<(std::ostream &o, Parameters &p)
    {
        o<<"MAX_DIM="<<p.MAX_DIM<<std::endl;
        o<<"NUM_OF_CLUS="<<p.NUM_OF_CLUS<<std::endl;
        o<<"BEST_BOUND="<<p.BEST_BOUND<<std::endl;
        o<<"ERROR_BOUND="<<p.ERROR_BOUND<<std::endl;
        o<<"LABEL_COL="<<p.LABEL_COL<<std::endl;
        o<<"CONST_SIZE_HIS="<<p.CONST_SIZE_HIS<<std::endl;
        o<<"MAX_BIN_PORTION="<<p.MAX_BIN_PORTION<<std::endl;
        o<<"NOISE_SIZE="<<p.NOISE_SIZE<<std::endl;
        o<<"RANDOM_SEED="<<p.RANDOM_SEED<<std::endl;
        o<<"SAMPLING_HEURISTIC="<<p.SAMPLING_HEURISTIC<<std::endl;
        o<<"SAMPLING_FACTOR="<<p.SAMPLING_FACTOR<<std::endl;
        o<<"HIS_SAMPLING="<<p.HIS_SAMPLING<<std::endl;
        o<<"SAVE_RESULT="<<p.SAVE_RESULT<<std::endl;

        return o;
    }

};

typedef void (*callback_t) (const char *);

class LMCLUS
{
private:
    LMCLUS(const LMCLUS& rhs) = delete;
    void operator=(const LMCLUS& rhs) = delete;
    
    // sampling functions
    int sampleQuantity(int lmDim, int fullSpcDim, const int dataSize, const Parameters &para);
    arma::uvec samplePoints(const arma::mat &data, const int lmDim);
    arma::uvec sample(const int n, const int k);

        
    // spearation detection functions
    Separation findBestSeparation(const arma::mat &data, const int SubSpaceDim, const Parameters &para);
    //std::pair<arma::uvec, arma::uvec> findBestPoints(const arma::mat &data, const Separation &best_sep);
    
    arma::mat findBestPoints(const arma::mat &data, const Separation &sep, arma::mat &nonClusterPoints);
    arma::vec determineDistances(const arma::mat &data, const arma::mat &P, const arma::rowvec &origin, const Parameters &para);
    
    unsigned int randromNumber();
    
    cpplog::OstreamLogger *log;
    bool logCreated;
    std::mt19937 engine;
    std::uniform_int_distribution<unsigned int> dist;
    
public:
    LMCLUS() : logCreated(true), engine(std::random_device{}()), dist(std::uniform_int_distribution<unsigned int>())
    {
        // Setup logger
        log = new cpplog::StdErrLogger();
    }    
    
    LMCLUS(cpplog::OstreamLogger *mlog) : logCreated(false), engine(std::random_device{}()), dist(std::uniform_int_distribution<unsigned int>())
    {
        log = mlog;
    }    
    
    ~LMCLUS()
    {
        if (logCreated)
            delete log;
    }
    
    // basis generation functions
    arma::mat formBasis(const arma::mat &points, arma::rowvec& origin);
    arma::mat gramSchmidtOrthogonalization(const arma::mat &M);
    static double distanceToManifold(const arma::rowvec &point, const arma::mat &B_T);
    
    void find_manifold(const arma::mat &data, const Parameters &para, 
                 arma::uvec &points_index, std::vector<unsigned int> &nonClusterPoints,
                 std::vector<Separation> &separations, bool &Noise, int &SepDim);
    
    void cluster(const arma::mat &data, const Parameters &para, 
                 std::vector<arma::uvec> &labels, std::vector<double> &thresholds, 
                 std::vector<arma::mat> &bases, std::vector<int> &clusterDims,
                 std::vector<arma::vec> &origins, callback_t progress = nullptr);
};

} // lmclus namespace
} // clustering namespace
